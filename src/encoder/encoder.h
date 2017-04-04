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
    
    int buffer_size;
    
    int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt);
    int select_sample_rate(AVCodec *codec);
public:
    Encoder(enum AVCodecID encoder_id, int bit_rate, enum AVSampleFormat sample_fmt);
    ~Encoder();

    static AVFrame *fill_new_frame(AVCodecContext *codec_ctx, uint8_t *sampss);
    //AVFrame *fill_new_frame(uint8_t *samples, int channel_layout);
    AVCodecContext *codec_ctx = NULL;
};



#endif
