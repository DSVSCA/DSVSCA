#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include "../encoder/encoder.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#ifndef TESTJOIN_H
#define TESTJOIN_H

class TestJoin {

private:
    static char strbuf[512];
   
    static Encoder *encoder;

    static AVFilterGraph *filter_graph;

    static int init_filter_graph();
    
    static int init_abuffer_ctx();
    static int init_abuffersink_ctx();

    //static AVFilterContext *amerge_ctx;
public:
    // TODO: Can read from abuffersink as abuffer?
    TestJoin(Encoder *enc);
    ~TestJoin();

    static AVFilterContext *abuffer_ctx;

    static AVFilterContext *abuffersink_ctx;
    
};



#endif
