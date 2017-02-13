#include "decomposer.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

AVFormatContext *Decomposer::ic = NULL;
AVStream *Decomposer::audio_stream = NULL;
AVFilterGraph *Decomposer::filter_graph;
char Decomposer::strbuf[512];
AVFilterContext *Decomposer::abuffer_ctx = NULL;
AVFilterContext *Decomposer::channelsplit_ctx = NULL;
//AVFilterContext *Decomposer::abuffersink_ctx = NULL;
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
    init_filter_graph();
}

int Decomposer::init_filter_graph() {
    filter_graph = avfilter_graph_alloc();
    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "FILTER GRAPH: unable to create filter graph: out of memory!\n");
        return -1;
    }
    
    std::cout << std::endl;
    std::cout << "Beginning creation of the filter graph" << std::endl;

    int error;
    
    init_abuffer_ctx();
    init_channelsplit_ctx();
    init_abuffersinks_ctx();
    

    // Split into all the channels
    // FL
    avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 0);
    
    // FR
    avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 1);

    // FC
    avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 2);

    // LFE 
    avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 3);

    // BL
    avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 4);

    // BR
    avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 5);

    // Place back into our buffer sink
    // FL
    avfilter_link(channelsplit_ctx, 0, abuffersinks_ctx[0], 0);
    
    // FR 
    avfilter_link(channelsplit_ctx, 1, abuffersinks_ctx[1], 0);
    
    // FC
    avfilter_link(channelsplit_ctx, 2, abuffersinks_ctx[2], 0);
    
    // LFE
    avfilter_link(channelsplit_ctx, 3, abuffersinks_ctx[3], 0);
    
    // BL
    avfilter_link(channelsplit_ctx, 4, abuffersinks_ctx[4], 0);
    
    // BR
    avfilter_link(channelsplit_ctx, 5, abuffersinks_ctx[5], 0);

    
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error linking filter graph\n");
        return error;
    }

    error = avfilter_graph_config(filter_graph, NULL);
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error configuring filter graph\n");
        return error;
    }
    std::cout << "Graph has been created!" << std::endl;
    return 0;
}


int Decomposer::init_abuffer_ctx() {
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    
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
                NULL, strbuf, NULL, filter_graph);

    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffer filter\n");
        return error;
    }

    return error;
}

int Decomposer::init_channelsplit_ctx() {
    AVFilter *channelsplit = avfilter_get_by_name("channelsplit");
    
    snprintf(strbuf, sizeof(strbuf), 
             "channel_layout=FL+FR+FC+LFE+BL+BR");

    fprintf(stderr, "channelsplit: %s\n", strbuf);
    int error = avfilter_graph_create_filter(&channelsplit_ctx, channelsplit,
                "csplit", strbuf, NULL, filter_graph);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing channelsplit filter\n");
        return error;
    }

    return error;
}

int Decomposer::init_abuffersinks_ctx() {
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    
    std::cout << "abuffersinks: ";
    for(int i = 0; i < 6; i++) {
        int error = avfilter_graph_create_filter(&abuffersinks_ctx[i], abuffersink,
                    NULL, NULL, NULL, filter_graph);
        
        std::cout << i;
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "unable to create abuffersink filter %d\n", i);
            return error;
        }
    }
    
    std::cout << std::endl;
    
    return 0;
}
