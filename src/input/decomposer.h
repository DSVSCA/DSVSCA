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


#ifndef DECOMPOSER_H
#define DECOMPOSER_H

class Decomposer {

private:
    std::vector<char> buffer;
    
    static AVCodecContext *decoder_ctx;

    static AVFilterGraph *filter_graph;

    static AVFilterContext *abuffer_ctx;
    static AVFilterContext *channelsplit_ctx;
    
    static AVFormatContext *format_ctx;   
    static AVStream *audio_stream;

    static AVFrame *oframe;

    static int init_target_file(std::string fileName);
    static int init_filter_graph(); 
    static int init_abuffer_ctx();
    static int init_abuffersink_ctx();
    static int init_channelsplit_ctx(); 
   
    static int audio_stream_index;
    
    static void stream();
    static void print_frame(const AVFrame *frame);
    static char strbuf[512];

public:
    Decomposer(std::string fileName, bool verbose);

    std::vector<char> *getBuffer();
    
    static std::map<std::string, AVFilterContext*> abuffersink_ctx_map;
};

#endif
