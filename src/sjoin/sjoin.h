#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include "../input/format.h"
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
   
    static Format *format;

    static AVFilterGraph *filter_graph;

    static int init_filter_graph();
    static int init_left_abuffers_ctx(); 
    static int init_right_abuffers_ctx();
    static int init_left_amix_ctx();
    static int init_right_amix_ctx();
    static int init_amerge_ctx();
    static int init_abuffersink_ctx();
    
    static std::vector<AVFilterContext*> left_abuffers_ctx;
    static std::vector<AVFilterContext*> right_abuffers_ctx;
    static AVFilterContext *left_amix_ctx;
    static AVFilterContext *right_amix_ctx;
    static AVFilterContext *amerge_ctx;
    static AVFilterContext *abuffersink_ctx;
public:
    // TODO: Can read from abuffersink as abuffer?
    SJoin(Format *fmt);
    ~SJoin();

    
    //static AVFilterContext *abuffer_ctx; 
    
};



#endif
