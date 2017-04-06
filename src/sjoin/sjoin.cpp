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

Encoder *SJoin::encoder = NULL;

AVFilterGraph *SJoin::filter_graph;  // Holds filter steps

std::vector<AVFilterContext*> SJoin::abuffers_ctx(6);
AVFilterContext *SJoin::amix_ctx = NULL;
AVFilterContext *SJoin::abuffersink_ctx = NULL;

SJoin::SJoin(Encoder *enc) {
    std::cout << "Begin joining streams" << std::endl;
    
    encoder = enc;
    
    // Fix the format context's channel layout to mirror the new channels
    //format->decoder_ctx->channel_layout = 3;
    //format->decoder_ctx->channels = 2; 
    std::cout << "Encoder information:" << std::endl;
    std::cout << "Channel Layout: " << encoder->codec_ctx->channel_layout << " Channels: " << 
        encoder->codec_ctx->channels << std::endl;

    //avcodec_register_all();
    //av_register_all();
    //avfilter_register_all();
    std::cout << "Initialize filter graph" << std::endl;    
    init_filter_graph();
}

int SJoin::init_filter_graph() { 

    filter_graph = avfilter_graph_alloc();

    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "Filter graph: unable to create filter graph: out of memory!\n");
    }


    int error = init_abuffers_ctx();
    if(error < 0) av_log(NULL, AV_LOG_ERROR, "Error init abuffers\n");
    error = init_amix_ctx();
    if(error < 0) av_log(NULL, AV_LOG_ERROR, "Error init amix\n");
    error = init_abuffersink_ctx();
    if(error < 0) av_log(NULL, AV_LOG_ERROR, "Error init abuffersink\n");

    
    std::cout << " * Linking Filters" << std::endl;

    std::cout << "  - Link abuffers to amix" << std::endl;
    for(int i = 0; i < 6; i++) {
        error = avfilter_link(abuffers_ctx[i], 0, amix_ctx, i);
        if(error < 0)
            av_log(NULL, AV_LOG_ERROR, "Error linking abuffer_ctx[%d] to amix_ctx\n", i);
    }
    
    std::cout << "      - Link amix to abuffersink" << std::endl;
   
    error = avfilter_link(amix_ctx, 0, abuffersink_ctx, 0);
    if(error < 0) 
        av_log(NULL, AV_LOG_ERROR, "Error linking amix_ctx to abuffersink_ctx\n");

    std::cout << "Configuring" << std::endl;
    error = avfilter_graph_config(filter_graph, NULL);
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error configuring filter graph\n");
        return error;
    }

    std::cout << "Filter graph initialized successfully" << std::endl;

    return error;
}

int SJoin::init_abuffers_ctx() {
    std::cout << " * Initializing input buffers" << std::endl;

    AVRational time_base = encoder->codec_ctx->time_base;
   
    std::cout << "  Here is some more info about the left channel buffers: " << std::endl;
    std::cout << "      - Layout: " << encoder->codec_ctx->channel_layout << std::endl;
    std::cout << "      - #channels: " << encoder->codec_ctx->channels << std::endl;
    
    snprintf(strbuf, sizeof(strbuf),
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
            time_base.num, time_base.den, encoder->codec_ctx->sample_rate,
            av_get_sample_fmt_name(encoder->codec_ctx->sample_fmt),
            encoder->codec_ctx->channel_layout);

    std::cout << strbuf << std::endl;

    int error;

    for(int i = 0; i < 6; i++) {
        AVFilter *abuffer = avfilter_get_by_name("abuffer");
        avfilter_graph_create_filter(&abuffers_ctx[i], abuffer,
                NULL, strbuf, NULL, filter_graph);
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "error initializing abuffer fitler %d\n", i);
        }
    }

}

int SJoin::init_amix_ctx() {
    std::cout << "  * Initializing amix context" << std::endl;
    AVFilter *amix = avfilter_get_by_name("amix");
    
    snprintf(strbuf, sizeof(strbuf), "inputs=6:duration=first");
    std::cout << strbuf << std::endl;

    int error = avfilter_graph_create_filter(&amix_ctx, amix, NULL, strbuf, NULL, filter_graph);

    if(error < 0) 
        av_log(NULL, AV_LOG_ERROR, "error initializing left amix filter\n");

    return error;
}
/*

int SJoin::init_left_abuffers_ctx() {
    std::cout << "  * Initializing left input buffers" << std::endl;
    AVRational time_base = format->audio_stream->time_base;
    
    std::cout << "  Here is some more info about the left channel buffers: " << std::endl;
    std::cout << "      - Layout: " << format->decoder_ctx->channel_layout << std::endl;
    std::cout << "      - #channels: " << format->decoder_ctx->channels << std::endl;
    
    // Create the abuffer filter argument
    snprintf(strbuf, sizeof(strbuf), 
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
            time_base.num, time_base.den, format->decoder_ctx->sample_rate,
            av_get_sample_fmt_name(format->decoder_ctx->sample_fmt),
            format->decoder_ctx->channel_layout);  

    std::cout << strbuf << std::endl;
    
    int error;
    
    for(int i = 0; i < 6; i++) {
        AVFilter *abuffer = avfilter_get_by_name("abuffer");
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
    std::cout << "      - Layout: " << 2 << std::endl;
    std::cout << "      - #channels: " << 1 << std::endl;
    
    // Create the abuffer filter argument
    snprintf(strbuf, sizeof(strbuf), 
            "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%d",
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
*/
int SJoin::init_abuffersink_ctx() {
    std::cout << "  * Initializing output buffer" << std::endl;
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");

    int error = avfilter_graph_create_filter(&abuffersink_ctx, abuffersink, NULL, NULL, NULL, filter_graph);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffersink filter\n");
    }

    return error;
}


