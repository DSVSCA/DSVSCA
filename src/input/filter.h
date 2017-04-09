#include <algorithm>
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

public:
    enum Channel {
        INVALID = -1,
        FL = 0,
        FR = 1,
        FC = 2,
        LFE = 3,
        BL = 4,
        BR = 5,
    };

    enum Coord_Type {
        Cartesian = 0,
        Spherical = 1
    };

    Filter(Format *fmt);
    ~Filter();
    static AVFilterContext *abuffer_ctx;
    static std::map<Channel, AVFilterContext*> abuffersink_ctx_map;
    
    static Channel str_to_channel(std::string channel_name);
    static void get_coords(Channel channel, float * x, float * y, float * z);

    static void filter_free();

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
};

#endif
