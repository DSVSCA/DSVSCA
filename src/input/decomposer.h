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

    static AVFilterGraph *f_graph;
    static AVFilterContext *abuffer_ctx; 
    
    static AVFormatContext *ic;   
    static AVStream *audio_stream;

    static AVFrame *oframe;

    static int init_filter_graph();

public:
    Decomposer(std::string fileName, bool verbose);
    
    std::vector<char> *getBuffer();

};

#endif
