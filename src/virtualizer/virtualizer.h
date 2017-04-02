#include <iostream>
#include <vector>
#include <fstream>
#include <FFTConvolver.h>

extern "C" {
#include <mysofa.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

#ifndef VIRTUALIZER_H
#define VIRTUALIZER_H

typedef struct MYSOFA_EASY sofa_file;
typedef struct complete_sofa {
    sofa_file * hrtf = NULL;
    int filter_length;
};

class Virtualizer {
public:
    Virtualizer(const char * sofa_file_name, int sample_rate, float x, float y, float z, int block_size = 8);
    Virtualizer(sofa_file * hrtf, int filter_length, int sample_rate, float x, float y, float z, int block_size = 8);
    ~Virtualizer();
    
    float ** process();
    complete_sofa get_hrtf();

    static uint8_t * get_short_samples(float * buffer, AVSampleFormat format, uint8_t sample_count);
    static float * get_float_samples(uint8_t * buffer, AVSampleFormat format, uint8_t sample_count);

private:
    Virtualizer();
    void init(sofa_file * hrtf, int sample_rate, float x, float y, float z, int block_size);
    void open_sofa(const char * file_name, int sample_rate);
    void close_sofa();

    fftconvolver::FFTConvolver * left_conv;
    fftconvolver::FFTConvolver * right_conv;

    sofa_file * hrtf;
    float * left_ir;
    float * right_ir;
    float * overflow_audio;
    int filter_length = 0;
    uint32_t left_delay = 0;
    uint32_t right_delay = 0;
    uint32_t overall_delay = 0;
}

#endif
