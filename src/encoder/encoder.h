#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/log.h>
}

#ifndef ENCODER_H
#define ENCODER_H

class Encoder {

private:
    AVCodec *codec;
    
    
    int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt);
    int select_sample_rate(AVCodec *codec);
public:
    Encoder(enum AVCodecID encoder_id, int bit_rate, enum AVSampleFormat sample_fmt);
    ~Encoder();

    int buffer_size;
    AVFrame *new_frame(AVCodecContext *codec_ctx, uint8_t *extended_data_l, uint8_t *extended_data_r);
    static AVCodecContext *codec_ctx;
};



#endif
