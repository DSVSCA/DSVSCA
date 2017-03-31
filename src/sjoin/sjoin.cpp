#include "sjoin.h"
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
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}


char SJoin::strbuf[512];   // Send filter graph args through this

AVFilterGraph *SJoin::filter_graph;  // Holds filter steps
AVFrame *SJoin::oframe = NULL; // Describes decoded (raw) audio/video 
AVCodec *SJoin::decoder = NULL;

AVStream *SJoin::audio_stream = NULL;  // Stream structure 

int SJoin::audio_stream_index = 0;

AVCodecContext *SJoin::decoder_ctx = NULL;  // Main API structure

std::pair <AVFilterContext*, AVFilterContext*> abuffer_ctx_pair;
AVFilterContext *SJoin::channelmap_ctx = NULL;
AVFilterContext *SJoin::abuffersink_ctx = NULL;

SJoin::SJoin() {
    std::cout << "Begin joining streams" << std::endl;

    avcodec_register_all();
    av_register_all();
    avfilter_register_all();
    
    init_filter_graph();
}

int SJoin::init_filter_graph() { 

    filter_graph = avfilter_graph_alloc();

    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "Filter graph: unable to create filter graph: out of memory!\n");
    }

    int error = init_abuffers_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing abuffers\n");
    }

    error = init_channelmap_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing channelmap context\n");
    }

    error = init_abuffersink_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing abuffersink context\n");
    }

    std::cout << " * Linking Filters" << std::endl;

    return 0;
}

int SJoin::init_abuffers_ctx() {
    std::cout << "  * Initializing input buffers" << std::endl;
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    AVRational time_base = audio_stream->time_base;

    //if(!decoder_ctx->

    return 0;
}

int SJoin::init_channelmap_ctx() {


    return 0;
}

int SJoin::init_abuffersink_ctx() {

}


