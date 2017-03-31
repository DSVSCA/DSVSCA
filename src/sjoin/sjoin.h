#include <iostream>
#include <vector>
#include <fstream>
#include <map>

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#ifndef SJOIN_H
#define SJOIN_H

class SJoin {

private:
    static char strbuf[512];
    
    static AVFilterGraph *filter_graph;
    static AVStream *audio_stream;
    static AVFrame *oframe;
    static AVCodec *decoder;

    static int init_audio_stream();
    static int init_decoder();
    
    static int init_filter_graph();
    static int init_abuffers_ctx(); 
    static int init_channelmap_ctx();
    static int init_abuffersink_ctx();
    
    static int audio_stream_index; //potentially not needed since it HAS to be an audio stream
    
    static AVCodecContext *decoder_ctx;
    
    static std::pair <AVFilterContext*, AVFilterContext*> abuffer_ctx_pair;
    static AVFilterContext *channelmap_ctx;
    static AVFilterContext *abuffersink_ctx;
public:
    // TODO: Can read from abuffersink as abuffer?
    SJoin();
    ~SJoin();

    
    //static AVFilterContext *abuffer_ctx; 
    
};



#endif
