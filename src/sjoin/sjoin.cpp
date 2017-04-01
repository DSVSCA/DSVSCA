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

Format *SJoin::format = NULL;

AVFilterGraph *SJoin::filter_graph;  // Holds filter steps

std::vector<AVFilterContext*> SJoin::left_abuffers_ctx(6);
std::vector<AVFilterContext*> SJoin::right_abuffers_ctx(6);
AVFilterContext *SJoin::left_amix_ctx = NULL;
AVFilterContext *SJoin::right_amix_ctx = NULL;
AVFilterContext *SJoin::amerge_ctx = NULL;
AVFilterContext *SJoin::abuffersink_ctx = NULL;

SJoin::SJoin(Format *fmt) {
    std::cout << "Begin joining streams" << std::endl;
    
    format = fmt;
    
    // Fix the format context's channel layout to mirror the new channels
    format->decoder_ctx->channel_layout = 3;
    format->decoder_ctx->channels = 2; 

    avcodec_register_all();
    av_register_all();
    avfilter_register_all();
    std::cout << "Initialize filter graph" << std::endl;    
    init_filter_graph();
}

int SJoin::init_filter_graph() { 

    filter_graph = avfilter_graph_alloc();

    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "Filter graph: unable to create filter graph: out of memory!\n");
    }

    int error = init_left_abuffers_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing left abuffers\n");
    }

    error = init_right_abuffers_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing right abuffers\n");
    }

    error = init_left_amix_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing left amix context\n");
    }

    error = init_right_amix_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing right amix context\n");
    }

    error = init_amerge_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing channelmap context\n");
    }

    error = init_abuffersink_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing abuffersink context\n");
    }

    std::cout << " * Linking Filters" << std::endl;

    std::cout << "      - Link left abuffers to left amix" << std::endl;
    for(int i = 0; i < 6; i++) {
        error = avfilter_link(left_abuffers_ctx[i], 0, left_amix_ctx, i);
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "error linking left abuffer ctx to left amix ctx %d\n", i);
        }
    }
    
    std::cout << "      - Link right abuffers to right amix" << std::endl;;
    for(int j = 0; j < 6; j++) {
        error = avfilter_link(right_abuffers_ctx[j], 0, right_amix_ctx, j);
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "error linking right abuffer ctx to right amix ctx %d\n", j);
        }
    }
   
    std::cout << "      - Link left amix to amerge" << std::endl;

    error = avfilter_link(left_amix_ctx, 0, amerge_ctx, 0);

    if(error < 0)
        av_log(NULL, AV_LOG_ERROR, "Error linking left_amix_ctx to amerge_ctx\n");

    std::cout << "      - Link right amix to amerge" << std::endl;
    
    error = avfilter_link(right_amix_ctx, 0, amerge_ctx, 1);

    if(error < 0)
        av_log(NULL, AV_LOG_ERROR, "Error linking right_amix_ctx to amerge_ctx\n");

    std::cout << "      - Link amerge to abuffersink" << std::endl;
   
    error = avfilter_link(amerge_ctx, 0, abuffersink_ctx, 0);

    if(error < 0) 
        av_log(NULL, AV_LOG_ERROR, "Error linking amerge_ctx to abuffersink_ctx\n");

    return 0;
}

int SJoin::init_left_abuffers_ctx() {
    std::cout << "  * Initializing left input buffers" << std::endl;
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    AVRational time_base = format->audio_stream->time_base;
    
    std::cout << "  Here is some more info about the left channel buffers: " << std::endl;
    std::cout << "      - Layout: " << 1 << std::endl;
    std::cout << "      - #channels: " << 1 << std::endl;
    
    // Create the abuffer filter argument
    snprintf(strbuf, sizeof(strbuf), 
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
            time_base.num, time_base.den, format->decoder_ctx->sample_rate,
            av_get_sample_fmt_name(format->decoder_ctx->sample_fmt),
            1);  

    
    std::cout << strbuf << std::endl;
    
    int error;
    
    for(int i = 0; i < 6; i++) {
        avfilter_graph_create_filter(&left_abuffers_ctx[i], abuffer,
                NULL, strbuf, NULL, filter_graph);
        
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "error initializing abuffer filter %d\n", i);
            break;
        }
    }

    return error;
}

int SJoin::init_right_abuffers_ctx() {
    std::cout << "  * Initializing right input buffers" << std::endl;
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    AVRational time_base = format->audio_stream->time_base;
    
    std::cout << "  Here is some more info about the right channel buffers: " << std::endl;
    std::cout << "      - Layout: " << 1 << std::endl;
    std::cout << "      - #channels: " << 1 << std::endl;
    
    // Create the abuffer filter argument
    snprintf(strbuf, sizeof(strbuf), 
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
            time_base.num, time_base.den, format->decoder_ctx->sample_rate,
            av_get_sample_fmt_name(format->decoder_ctx->sample_fmt),
            2);  

    
    std::cout << strbuf << std::endl;
    
    int error;
    
    for(int i = 0; i < 6; i++) {
        error = avfilter_graph_create_filter(&right_abuffers_ctx[i], abuffer,
                NULL, strbuf, NULL, filter_graph);
        
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "error initializing abuffer filter %d\n", i);
            break;
        }
    }

    return error;
}

int SJoin::init_left_amix_ctx() {
    std::cout << "  * Initializing left amix context" << std::endl;
    AVFilter *amix = avfilter_get_by_name("amix");
    
    snprintf(strbuf, sizeof(strbuf), "inputs=6:duration=first");
    std::cout << strbuf << std::endl;

    int error = avfilter_graph_create_filter(&left_amix_ctx, amix, NULL, strbuf, NULL, filter_graph);

    if(error < 0) 
        av_log(NULL, AV_LOG_ERROR, "error initializing left amix filter\n");

    return error;
}

int SJoin::init_right_amix_ctx() {
    std::cout << "  * Initializing right amix context" << std::endl;
    AVFilter *amix = avfilter_get_by_name("amix");
    
    snprintf(strbuf, sizeof(strbuf), "inputs=6:duration=first");
    std::cout << strbuf << std::endl;

    int error = avfilter_graph_create_filter(&right_amix_ctx, amix, NULL, strbuf, NULL, filter_graph);

    if(error < 0) 
        av_log(NULL, AV_LOG_ERROR, "error initializing right amix filter\n");

    return error;
}


int SJoin::init_amerge_ctx() {
    std::cout << "  * Initializing amerge context" << std::endl;
    AVFilter *amerge = avfilter_get_by_name("amerge");

    snprintf(strbuf, sizeof(strbuf), "inputs=2");
    std::cout << strbuf << std::endl;

    int error = avfilter_graph_create_filter(&amerge_ctx, amerge, NULL, strbuf, NULL, filter_graph);
    
    if(error < 0)
        av_log(NULL, AV_LOG_ERROR, "error initializing amerge filter\n");

    return error;
}

int SJoin::init_abuffersink_ctx() {
    std::cout << "  * Initializing output buffer" << std::endl;
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");

    int error = avfilter_graph_create_filter(&abuffersink_ctx, abuffersink, NULL, NULL, NULL, filter_graph);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffersink filter\n");
    }

    return error;
}


