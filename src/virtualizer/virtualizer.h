#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cmath>
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

#define BLOCK_SIZE 256

typedef struct MYSOFA_EASY sofa_file;
struct complete_sofa {
    sofa_file * hrtf = NULL;
    int filter_length;
};

class Virtualizer {
public:
    // The x-axis (1 0 0) is the listening direction. The y-axis (0 1 0) is the left side of the listener. The z-axis (0 0 1) is upwards.
    Virtualizer(const char * sofa_file_name, int sample_rate, float x, float y, float z, int block_size);
    Virtualizer(complete_sofa sofa_, int sample_rate, float x, float y, float z, int block_size);
    ~Virtualizer();

    float ** process(const float * source, size_t data_length);
    complete_sofa get_hrtf();

    static uint8_t * get_short_samples(float * buffer, AVSampleFormat format, int sample_count);
    static float * get_float_samples(uint8_t * buffer, AVSampleFormat format, int sample_count);
    static void get_peak(float ** float_results, int sample_count, float * max_peak);
    static void get_peak(uint8_t * buffer, AVSampleFormat format, int sample_count, float * max_peak);
    template<typename T>
    static void store_value(uint8_t * output, int64_t value, int64_t max_val, AVSampleFormat format, int index);

private:
    Virtualizer();
    void init(int sample_rate, float x, float y, float z, int block_size);
    void open_sofa(const char * file_name, int sample_rate);
    void close_sofa();

    fftconvolver::FFTConvolver * left_conv;
    fftconvolver::FFTConvolver * right_conv;

    bool created_hrtf;
    sofa_file * hrtf = NULL;
    float * left_ir = NULL;
    float * right_ir = NULL;
    float * overflow_audio = NULL;
    int filter_length = 0;
    size_t left_delay = 0;
    size_t right_delay = 0;
    size_t overall_delay = 0;
};

#endif
