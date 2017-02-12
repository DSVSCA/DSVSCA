#include "decomposer.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}
Decomposer::Decomposer(std::string fileName) {
    // TODO: Add error checking for fileName

    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    avfilter_register_all();

    AVFormatContext *ic = NULL;
    char *filename = new char[fileName.length() + 1];
    strcpy(filename, fileName.c_str());
    if(avformat_open_input(&ic, filename, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "error opening %s\n", filename);
        // TODO: throw an exception
    }
    
    if (avformat_find_stream_info(ic, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: could not find codec parameters\n", filename);
        // throw an error
    }
    
}
