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


AVFormatContext *Decomposer::format_ctx = NULL;  // Format I/O Context ????
AVStream *Decomposer::audio_stream = NULL;  // Stream structure 
AVCodecContext *Decomposer::decoder_ctx = NULL;  // Main API structure


AVFrame *Decomposer::oframe = NULL; // Describes decoded (raw) audio/video 

AVFilterGraph *Decomposer::filter_graph;  // Holds filter steps

AVFilterContext *Decomposer::abuffer_ctx = NULL; // Buffers audio frames to expose to filter chain
AVFilterContext *Decomposer::channelsplit_ctx = NULL;  // Splits channels into multiple output streams
AVFilterContext *Decomposer::abuffersink_ctx = NULL;
// Buffer audio streams and make available at the end of a filter chain. 6 here for a 5.1
// channel audio setup
std::vector<AVFilterContext*> Decomposer::abuffersinks_ctx = std::vector<AVFilterContext*>(6); 

Decomposer::Decomposer(std::string fileName, bool verbose) {  
    std::cout << "Initializing libav* " << std::endl;
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    avfilter_register_all();
    
    std::cout << "Initializing input file" << fileName << std::endl;
    
    init_target_file(fileName);
    init_filter_graph();
}

int Decomposer::init_target_file(std::string fileName) { 
    AVCodec *decoder = NULL;    // Provides codec information and decoder info
    char *filename = new char[fileName.length() + 1];
    strcpy(filename, fileName.c_str());

    int error;
    error = avformat_open_input(&format_ctx, filename, NULL, NULL);
    
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error opening %s\n", filename);
        return error;
    }

    error = avformat_find_stream_info(format_ctx, NULL);

    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: could not find codec parameters\n", filename);
        return error;
    }
    
      int audio_stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
   
    if(audio_stream_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "No audio stream found in the input file\n");
        error = audio_stream_index;
        return error;
    }

    audio_stream = format_ctx->streams[audio_stream_index];
    decoder_ctx = format_ctx->streams[audio_stream_index]->codec;
    
    error = avcodec_open2(decoder_ctx, decoder, NULL);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: unable to open decoder/decoder not found\n", format_ctx->filename);
        return error; 
    } 
    
    std::cout << "  * Format Context Info" << std::endl;
    std::cout << "      - Video file duration: " << format_ctx->duration << std::endl;
    std::cout << "      - Stream bitrate:      " << format_ctx->bit_rate << std::endl;
    std::cout << "      - audio stream index:  " << audio_stream_index << std::endl; 
    
    return 0;
}


int Decomposer::init_filter_graph() {
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    const AVFilterLink *outlink;

    std::cout << std::endl;
    std::cout << "Initialize filter graph" << std::endl;
    

    filter_graph = avfilter_graph_alloc();
    
    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "FILTER GRAPH: unable to create filter graph: out of memory!\n");
        return -1;
    }
    
    int error;
  
    error = init_abuffer_ctx();

    if(error) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing abuffer context\n");
        return error;
    }

    error = init_abuffersink_ctx();
    
    if(error < 0) {
     av_log(NULL, AV_LOG_ERROR, "Error initializing abuffersink context\n"); 
     return error;
    }
    
    std::cout << "  * Linking Filters" << std::endl;
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

    error = avfilter_graph_config(filter_graph, NULL);
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error configuring filter graph\n");
        return error;
    }
   
    std::cout << "Filter graph successfully initialized!" << std::endl;
    std::cout << "More filter graph info:   " << std::endl;

    outlink = abuffersink_ctx->inputs[0];
    av_get_channel_layout_string(strbuf, sizeof(strbuf), -1, outlink->channel_layout);
    av_log(NULL, AV_LOG_INFO, "Output: streamrate:%dHz fmt:? chlayout:%s\n",
           (int)outlink->sample_rate, strbuf);

    // char *options;
    //std::string filter_dump = avfilter_graph_dump(filter_graph, options);
    return 0;
}


int Decomposer::init_abuffer_ctx() {
    std::cout << "  * Initialzing input buffer" << std::endl;
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    AVABufferSinkParams *abuffersink_params;
    AVRational time_base = audio_stream->time_base;
   
    if(!decoder_ctx->channel_layout) {
        std::cout << "  ***  Decoder channel layout not found, finding channel layout ***" << std::endl;
        decoder_ctx->channel_layout = av_get_default_channel_layout(decoder_ctx->channels);
    }
    if(!decoder_ctx->channel_layout)
      av_log(NULL, AV_LOG_ERROR, "unable to guess chnnel layout\n");
    
    std::cout << "  * Channel layout was successfully found." << std::endl;
    std::cout << "    Here is some more info about it: " << std::endl;
    std::cout << "      - Layout: " << decoder_ctx->channel_layout << std::endl;
    std::cout << "      - # Channels: " << decoder_ctx->channels << std::endl;
    

    // Create an abuffer filter
    snprintf(strbuf, sizeof(strbuf),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64, 
             time_base.num, time_base.den, decoder_ctx->sample_rate,
             av_get_sample_fmt_name(decoder_ctx->sample_fmt),
             decoder_ctx->channel_layout);

    int error = avfilter_graph_create_filter(&abuffer_ctx, abuffer,
                "input_buffer", strbuf, NULL, filter_graph);

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
                NULL, strbuf, NULL, filter_graph);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing channelsplit filter\n");
        return error;
    }

    return error;
}

int Decomposer::init_abuffersink_ctx() {
    std::cout << "  * Initializing output buffer" << std::endl;
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");

    int error = avfilter_graph_create_filter(&abuffersink_ctx, abuffersink,
                    "output_buffer", NULL, NULL, filter_graph);
    
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "unable to create abuffersink filter \n");
        return error;
    }
    
    
    return error;
}



