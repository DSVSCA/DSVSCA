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
    
    static AVFrame *oframe;
public:
    Decomposer(std::string fileName, bool verbose);
    
    std::vector<char> *getBuffer();

};

#endif
