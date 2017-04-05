#include "testjoin.h"
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


char TestJoin::strbuf[512];   // Send filter graph args through this

Encoder *TestJoin::encoder = NULL;

AVFilterGraph *TestJoin::filter_graph = NULL;  // Holds filter steps

AVFilterContext *TestJoin::abuffer_ctx = NULL;
AVFilterContext *TestJoin::abuffersink_ctx = NULL;

TestJoin::TestJoin(Encoder *enc) {
    std::cout << "Begin joining streams" << std::endl;
    
    encoder = enc;
    
    std::cout << "Encoder information:" << std::endl;
    std::cout << "Channel Layout: " << encoder->codec_ctx->channel_layout << " Channels: " << 
        encoder->codec_ctx->channels << std::endl;

    std::cout << "Initialize filter graph" << std::endl;    
    init_filter_graph();
}

int TestJoin::init_filter_graph() { 

    filter_graph = avfilter_graph_alloc();

    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "Filter graph: unable to create filter graph: out of memory!\n");
    }


    int error = init_abuffer_ctx();
    if(error < 0) av_log(NULL, AV_LOG_ERROR, "Error init abuffer\n");
    error = init_abuffersink_ctx();
    if(error < 0) av_log(NULL, AV_LOG_ERROR, "Error init abuffersink\n");

    
    std::cout << " * Linking Filters" << std::endl;

    
    std::cout << "  - Link abuffers to amix" << std::endl;
    error = avfilter_link(abuffer_ctx, 0, abuffersink_ctx, 0);
    if(error < 0)
        av_log(NULL, AV_LOG_ERROR, "Error linking abuffer_ctx to amix_ctx\n");;
    
    std::cout << "Configuring" << std::endl;
    error = avfilter_graph_config(filter_graph, NULL);
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error configuring filter graph\n");
        return error;
    }

    std::cout << "Filter graph initialized successfully" << std::endl;

    return error;
}

int TestJoin::init_abuffer_ctx() {
    std::cout << " * Initializing input buffers" << std::endl;

    AVRational time_base = encoder->codec_ctx->time_base;
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    
    std::cout << "  Here is some more info about the left channel buffers: " << std::endl;
    std::cout << "      - Layout: " << encoder->codec_ctx->channel_layout << std::endl;
    std::cout << "      - #channels: " << encoder->codec_ctx->channels << std::endl;
    
    snprintf(strbuf, sizeof(strbuf),
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=2c",
            time_base.num, time_base.den, encoder->codec_ctx->sample_rate,
            av_get_sample_fmt_name(encoder->codec_ctx->sample_fmt));

    std::cout << strbuf << std::endl;

    int error;
    avfilter_graph_create_filter(&abuffer_ctx, abuffer, NULL, strbuf, NULL, filter_graph);
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffer fitler \n");
    }
}


int TestJoin::init_abuffersink_ctx() {
    std::cout << "  * Initializing output buffer" << std::endl;
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");

    int error = avfilter_graph_create_filter(&abuffersink_ctx, abuffersink, NULL, NULL, NULL, filter_graph);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffersink filter\n");
    }

    return error;
}


