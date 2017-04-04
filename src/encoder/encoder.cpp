#include "encoder.h"

Encoder::Encoder(enum AVCodecID encoder_id, int bit_rate, enum AVSampleFormat sample_fmt) {
   codec = avcodec_find_encoder(encoder_id);
   if(!codec) av_log(NULL, AV_LOG_ERROR, "Unable to find codec\n");

   codec_ctx = avcodec_alloc_context3(codec);
   if(!codec_ctx) av_log(NULL, AV_LOG_ERROR, "Failed to allocate codec context\n");

   codec_ctx->bit_rate = bit_rate;
   codec_ctx->sample_fmt = sample_fmt;
    
   if(!check_sample_fmt(codec, codec_ctx->sample_fmt))
       av_log(NULL, AV_LOG_ERROR, "Codec does not support this sample format!\n");

   codec_ctx->sample_rate = select_sample_rate(codec);
   codec_ctx->channel_layout = 3;
   codec_ctx->channels = 2;

   if(avcodec_open2(codec_ctx, codec, NULL) < 0) 
       av_log(NULL, AV_LOG_ERROR, "Could not open codec\n"); 
    
   buffer_size = av_samples_get_buffer_size(NULL, codec_ctx->channels,
           codec_ctx->frame_size, codec_ctx->sample_fmt, 0);

   if(buffer_size < 0)
       av_log(NULL, AV_LOG_ERROR, "Could not determine buffer size\n");
    
   av_malloc(buffer_size);
}

int Encoder::check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;
    while(*p != AV_SAMPLE_FMT_NONE) {
        if(*p == sample_fmt) 
            return 1;
        p++;
    }
    return 0;
}


int Encoder::select_sample_rate(AVCodec *codec) {
    const int *p;
    int best_samplerate = 0;

    if(!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while(*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    
    return best_samplerate;
}

AVFrame *Encoder::fill_new_frame(AVCodecContext *codec_ctx, uint8_t *samps) { 
    uint16_t *samples;    
    AVFrame *frame;

    frame = av_frame_alloc();

    int buffer_size = av_samples_get_buffer_size(NULL, codec_ctx->channels, codec_ctx->frame_size,
            codec_ctx->sample_fmt, 0);
    if(buffer_size < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not get samples buffer size\n");
        return NULL;
    }
  

    frame->nb_samples = codec_ctx->frame_size;
    frame->format = codec_ctx->sample_fmt;
    frame->channel_layout = 1551;
    frame->channels = 6;

    av_malloc(buffer_size);

    int ret = avcodec_fill_audio_frame(frame, codec_ctx->channels, codec_ctx->sample_fmt,
            (const uint8_t*)samples, buffer_size, 0);

    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not setup audio frame\n");
        return NULL;
    }

    float t, tincr;
    t = 0;
    tincr = 2 * M_PI * 440.0 / codec_ctx->sample_rate;
    for(int i = 0; i < 200; i++) {
        for(int j = 0; j < codec_ctx->frame_size; j++) {
            samples[2*j] = (int)sin(t) * 10000;
            for(int k = 1; k < codec_ctx->channels; k++) 
                samples[2*j + k] = samples[2*j];
            t += tincr;
        }
    }
    
    //frame->channels = 2;
    return frame;
}

/*
AVFrame *Encoder::fill_new_frame(uint8_t *samples, int channel_layout) { 
    AVFrame *frame;

    frame = av_frame_alloc();
    if(!frame)
        av_log(NULL, AV_LOG_ERROR, "Unable to allocate frame\n");

    frame->nb_samples = codec_ctx->frame_size;
    frame->format = codec_ctx->sample_fmt;
    frame->channel_layout = channel_layout;

    std::cout << "Frame info!: " << frame->nb_samples << " " << frame->format << " " << frame->channel_layout << std::endl;

    int ret = avcodec_fill_audio_frame(frame, 1, codec_ctx->sample_fmt,
            (const uint8_t*)samples, buffer_size, 0);
    
    std::cout << av_frame_get_channels(frame) << std::endl;
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not fill frame\n");
        return NULL;
    } else {
        return frame;
    }
}

*/
