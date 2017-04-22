#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include "encoder.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#define __STDC_FORMAT_MACROS

#ifndef SJOIN_H
#define SJOIN_H

class SJoin {

private:
    static char strbuf[512];
   
    static Encoder *encoder;

    static AVFilterGraph *filter_graph;

    static int init_filter_graph();
    
    static int init_abuffers_ctx();
    static int init_amix_ctx();
    static int init_bass_ctx();
    static int init_abuffersink_ctx();

    
    static AVFilterContext *amix_ctx;
    static AVFilterContext *bass_ctx;
public:
    SJoin(Encoder *enc);
    ~SJoin();

    static std::vector<AVFilterContext*> abuffers_ctx;

    static AVFilterContext *abuffersink_ctx;
    
};



#endif
