#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include "format.h"
extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}


#ifndef FILTER_H
#define FILTER_H

class Filter {

private:
    static Format *format;

    static AVFilterGraph *filter_graph;
    static AVFilterContext *channelsplit_ctx;
    
    static int init_filter_graph(); 
    static int init_abuffer_ctx();
    static int init_abuffersink_ctx();
    static int init_channelsplit_ctx(); 
   
    static void print_frame(const AVFrame *frame);
    static char strbuf[512];

public:
    Filter(Format *fmt); 
    ~Filter();
    static AVFilterContext *abuffer_ctx;
    static std::map<std::string, AVFilterContext*> abuffersink_ctx_map;
    static void filter_free();
};

#endif
