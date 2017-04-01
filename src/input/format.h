#include <iostream>
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
}

#ifndef FORMAT_H
#define FORMAT_H

class Format {

private:
    static int init_target_file(std::string fileName);
public:
    Format(std::string fileName); 
    ~Format();
    static AVFormatContext *format_ctx;   
    static AVCodecContext *decoder_ctx;
    static int audio_stream_index;
    static AVStream *audio_stream;
};

#endif
