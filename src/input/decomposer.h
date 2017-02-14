#include <iostream>
#include <vector>
#include <fstream>
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}


#ifndef DECOMPOSER_H
#define DECOMPOSER_H

class Decomposer {

private:
    std::vector<char> buffer;

    static AVFilterGraph *filter_graph;

    static AVFilterContext *abuffer_ctx;
    static AVFilterContext *channelsplit_ctx;
    
    
    // FL, FR, FC, LFE, BL, BR
    static std::vector<AVFilterContext*> abuffersinks_ctx;
    
    static AVFilterContext *abuffersink_ctx; // temp


    static AVFormatContext *ic;   
    static AVStream *audio_stream;

    static AVFrame *oframe;

    static int init_filter_graph(AVCodecContext *avcctx);
    
    static int init_abuffer_ctx(AVFilter *abuffer);
    static int init_channelsplit_ctx(AVFilter *channelsplit);
    static int init_abuffersinks_ctx(AVFilter *abuffersink);
    

    static char strbuf[512];

public:
    Decomposer(std::string fileName, bool verbose);
    
    std::vector<char> *getBuffer();
    
};

#endif
