#include "filter.h"
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


char Filter::strbuf[512];   // Send filter graph args through this

Format *Filter::format = NULL;

AVFilterGraph *Filter::filter_graph;  // Holds filter steps

AVFilterContext *Filter::abuffer_ctx = NULL; // Buffers audio frames to expose to filter chain
AVFilterContext *Filter::channelsplit_ctx = NULL;  // Splits channels into multiple output streams

// Buffer audio streams and make available at the end of a filter chain. 6 here for a 5.1
// channel audio setup
std::map<Filter::Channel, AVFilterContext*> Filter::abuffersink_ctx_map;

Filter::Filter(Format *fmt) {  
    std::cout << "Initializing libav* " << std::endl;
    av_register_all();
    avfilter_register_all();
    
    format = fmt;

    init_filter_graph();
}

Filter::~Filter() {
    avfilter_graph_free(&filter_graph);
}

// TODO: remove this method
void Filter::print_frame(const AVFrame *frame) {
    const int n = frame->nb_samples * av_get_channel_layout_nb_channels(av_frame_get_channel_layout(frame));
    const uint16_t *p     = (uint16_t*)frame->data[0];
    const uint16_t *p_end = p + n;
    
    while(p < p_end) {
        fputc(*p    & 0xff, stdout);
        fputc(*p>>8 & 0xff, stdout);
        p++;
    }
    fflush(stdout);
}


int Filter::init_filter_graph() {
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

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing abuffer context\n");
        return error;
    }

    error = init_channelsplit_ctx();

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing channelsplit context\n");
        return error;
    }

    error = init_abuffersink_ctx();
    
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error initializing abuffersink context\n"); 
        return error;
    }
  
    /* connect all of the components of the filter graph */
    std::cout << "  * Linking Filters" << std::endl;
    error = avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 0);
    if(error >= 0)
        error = avfilter_link(channelsplit_ctx, 0, abuffersink_ctx_map[FL], 0);
    if(error >= 0)
        error = avfilter_link(channelsplit_ctx, 1, abuffersink_ctx_map[FR], 0);
    if(error >= 0)
        error = avfilter_link(channelsplit_ctx, 2, abuffersink_ctx_map[FC], 0);
    if(error >= 0)
        error = avfilter_link(channelsplit_ctx, 3, abuffersink_ctx_map[LFE], 0);
    if(error >= 0)
        error = avfilter_link(channelsplit_ctx, 4, abuffersink_ctx_map[BL], 0);
    if(error >= 0)
        error = avfilter_link(channelsplit_ctx, 5, abuffersink_ctx_map[BR], 0);
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
    /*
    std::cout << "More filter graph info:   " << std::endl;

    outlink = abuffersink_ctx->inputs[0];
    av_get_channel_layout_string(strbuf, sizeof(strbuf), -1, outlink->channel_layout);
    av_log(NULL, AV_LOG_INFO, "Output: streamrate:%dHz fmt:? chlayout:%s\n",
           (int)outlink->sample_rate, strbuf);
    */
    // char *options;
    //std::string filter_dump = avfilter_graph_dump(filter_graph, options);
    return error;
}


int Filter::init_abuffer_ctx() {
    std::cout << "  * Initialzing input buffer" << std::endl;
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    AVRational time_base = format->audio_stream->time_base;
   
    if(!format->decoder_ctx->channel_layout) {
        std::cout << "  ***  Decoder channel layout not found, finding channel layout ***" << std::endl;
        format->decoder_ctx->channel_layout = av_get_default_channel_layout(format->decoder_ctx->channels);
    }
    if(!format->decoder_ctx->channel_layout)
      av_log(NULL, AV_LOG_ERROR, "unable to guess chnnel layout\n");
    
    std::cout << "  * Channel layout was successfully found." << std::endl;
    std::cout << "    Here is some more info about it: " << std::endl;
    std::cout << "      - Layout: " << format->decoder_ctx->channel_layout << std::endl;
    std::cout << "      - # Channels: " << format->decoder_ctx->channels << std::endl;
    

    // Create an abuffer filter
    snprintf(strbuf, sizeof(strbuf),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64, 
             time_base.num, time_base.den, format->decoder_ctx->sample_rate,
             av_get_sample_fmt_name(format->decoder_ctx->sample_fmt),
             format->decoder_ctx->channel_layout);

    int error = avfilter_graph_create_filter(&abuffer_ctx, abuffer,
                "input_buffer", strbuf, NULL, filter_graph);

    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffer filter\n");
        return error;
    }
    

    return error;
}

int Filter::init_channelsplit_ctx() {
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

int Filter::init_abuffersink_ctx() {
    std::cout << "  * Initializing output buffer" << std::endl;
        
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    
    int error;
    std::string f_name;

    for(int i = 0; i < channelsplit_ctx->nb_outputs; i++) {
        AVFilterContext *sink_ctx = NULL;
        const char *name = channelsplit_ctx->output_pads[i].name;
        error = avfilter_graph_create_filter(&sink_ctx, abuffersink,
                name, NULL, NULL, filter_graph);

         if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "error creating filter sink \n");
            return error;
         }
        
        f_name.assign(name);
        Channel ch = str_to_channel(f_name);
        abuffersink_ctx_map[ch] = sink_ctx;
        std::cout << f_name << " => " << abuffersink_ctx_map[ch] << std::endl;
    }
    
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "unable to create abuffersink filter \n");
        return error;
    }
    
    return error;
}

Filter::Channel Filter::str_to_channel(std::string channel_name) {
    std::transform(channel_name.begin(), channel_name.end(), channel_name.begin(), ::toupper);

    if (channel_name == "FL") return FL;
    else if (channel_name == "FR") return FR;
    else if (channel_name == "FC") return FC;
    else if (channel_name == "LFE") return LFE;
    else if (channel_name == "BR") return BR;
    else if (channel_name == "BL") return BL;
    else return INVALID;
}

// The x-axis (1 0 0) is the listening direction. The y-axis (0 1 0) is the left side of the listener. The z-axis (0 0 1) is upwards.
void Filter::get_coords(Channel channel, float * x, float * y, float * z) {
    *z = 0;

    switch (channel) {
        case FL:
            *x = 1;
            *y = 1;
            break;
        case FR:
            *x = 1;
            *y = -1;
            break;
        case FC:
        case LFE:
            *x = 1;
            *y = 0;
            break;
        case BL:
            *x = -1;
            *y = 1;
            break;
        case BR:
            *x = -1;
            *y = -1;
            break;
    }
}
