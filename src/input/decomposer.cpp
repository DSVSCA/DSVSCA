#include "decomposer.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/log.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
}


char Decomposer::strbuf[512];   // Send filter graph args through this

AVFormatContext *Decomposer::ic = NULL;  // Format I/O Context ????
AVStream *Decomposer::audio_stream = NULL;  // Stream structure 

AVFrame *Decomposer::oframe = NULL; // Describes decoded (raw) audio/video 

AVFilterGraph *Decomposer::filter_graph;  // Holds filter steps

AVFilterContext *Decomposer::abuffer_ctx = NULL; // Buffers audio frames to expose to filter chain
AVFilterContext *Decomposer::channelsplit_ctx = NULL;  // Splits channels into multiple output streams
AVFilterContext *Decomposer::abuffersink_ctx = NULL;
// Buffer audio streams and make available at the end of a filter chain. 6 here for a 5.1
// channel audio setup
std::vector<AVFilterContext*> Decomposer::abuffersinks_ctx = std::vector<AVFilterContext*>(6); 



Decomposer::Decomposer(std::string fileName, bool verbose) {
    // TODO: Add error checking for fileName
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    avfilter_register_all();
    
    if(verbose)
      std::cout << "Loading " << fileName << std::endl;

    char *filename = new char[fileName.length() + 1];
    strcpy(filename, fileName.c_str());
    if(avformat_open_input(&ic, filename, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "error opening %s\n", filename);
        // TODO: throw an exception
    }

    if (avformat_find_stream_info(ic, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: could not find codec parameters\n", filename);
        //TODO: throw an error
        std::cout << "Error on stream info";
    }
    
    // discard all the streams, we don't care about the video and will save audio stream later
    for(int i = 0; i < ic->nb_streams; i++)
        ic->streams[i]->discard = AVDISCARD_ALL;

    AVCodec *decoder = NULL;
    int audio_stream_index = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
    
    // There should be 1 audio stream? 
    if(verbose) 
        std::cout << "Index of audio stream: " << audio_stream_index << std::endl;

    if(audio_stream_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: no audio stream found\n", ic->filename);
        //TODO throw exception
    }    

    if(!decoder) {
        av_log(NULL, AV_LOG_ERROR, "%s: no decoder found\n", ic->filename);
    }
    
    audio_stream = ic->streams[audio_stream_index];
    audio_stream->discard = AVDISCARD_DEFAULT;
    
    AVCodecContext *avcctx = audio_stream->codec; 

    if(!avcctx->channel_layout)
      avcctx->channel_layout = av_get_default_channel_layout(avcctx->channels);
    if(!avcctx->channel_layout)   
      av_log(NULL, AV_LOG_ERROR, "unable to guess chnnel layout\n");
    if(verbose) { 
        std::cout << "Channel layout was successfully found." << std::endl;
        std::cout << "Here is some more info about it: " << std::endl;
        std::cout << "Layout: " << avcctx->channel_layout << std::endl;
        std::cout << "# Channels: " << avcctx->channels << std::endl;
    }
    // at this point we should be able to init a filter graph
    init_filter_graph(avcctx);
}

int Decomposer::init_filter_graph(AVCodecContext *avcctx) {
    filter_graph = avfilter_graph_alloc();
    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "FILTER GRAPH: unable to create filter graph: out of memory!\n");
        return -1;
    }
    
    std::cout << std::endl;
    std::cout << "Beginning creation of the filter graph" << std::endl;

    int error;
 
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();

    error = init_abuffer_ctx(abuffer);
    //if(error >= 0) error = init_channelsplit_ctx();
    if(error >= 0) error = init_abuffersinks_ctx(abuffersink);
    
    if(error < 0) {
     av_log(NULL, AV_LOG_ERROR, "error initializing filter graphs\n"); 
     return error;
    };
   
    /* Setup endpoints of filter graph */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = abuffer_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = abuffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    const char *filter_descr = "aresample=8000"; 
    error = avfilter_graph_parse(filter_graph, filter_descr, inputs, outputs, NULL);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error linking filter graph\n");
        return error;
    }
  
    /* endpoints of filter graph */
    outputs->name = av_strdup("in");

    error = avfilter_graph_config(filter_graph, NULL);
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error configuring filter graph\n");
        return error;
    }
    
    std::cout << filter_graph->filters[0]->name << std::endl;
    std::cout << filter_graph->filters[1]->name << std::endl;
    std::cout << error << std::endl; 
   
    std::cout << "Graph has been created!" << std::endl;
   
    // char *options;
    //std::string filter_dump = avfilter_graph_dump(filter_graph, options);
    return 0;
}


int Decomposer::init_abuffer_ctx(AVFilter *abuffer) {
    AVCodecContext *avcctx = audio_stream->codec;
    AVRational time_base = audio_stream->time_base;
    
    // Create an abuffer filter
    snprintf(strbuf, sizeof(strbuf),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64, 
             time_base.num, time_base.den, avcctx->sample_rate,
             av_get_sample_fmt_name(avcctx->sample_fmt),
             avcctx->channel_layout);

    fprintf(stderr, "abuffer: %s\n", strbuf);
    int error = avfilter_graph_create_filter(&abuffer_ctx, abuffer,
                "input_buffer", strbuf, NULL, filter_graph);

    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffer filter\n");
        return error;
    }
    

    return error;
}

int Decomposer::init_channelsplit_ctx(AVFilter *channelsplit) {
    
    snprintf(strbuf, sizeof(strbuf), 
             "channel_layout=FL+FR+FC+LFE+BL+BR");

    fprintf(stderr, "channelsplit: %s\n", strbuf);
    int error = avfilter_graph_create_filter(&channelsplit_ctx, channelsplit,
                NULL, strbuf, NULL, filter_graph);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing channelsplit filter\n");
        return error;
    }

    return error;
}

int Decomposer::init_abuffersinks_ctx(AVFilter *abuffersink) {
    
    std::cout << "abuffersinks: ";
    int error = avfilter_graph_create_filter(&abuffersink_ctx, abuffersink,
                    "output_buffer", NULL, NULL, filter_graph);
        
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "unable to create abuffersink filter \n");
        return error;
    }
    
    std::cout << std::endl;
    
    return error;
}

