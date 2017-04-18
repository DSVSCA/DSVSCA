#include <cmath>
#include <climits>
#include <fstream>
#include <iostream>
#include <string.h>
#include <FFTConvolver.h>
#include "filter.h"

extern "C" {
#include <mysofa.h>
}

class WaveUtil {
private:
    union bfloat {
        int16_t b[2];
        float f;
    };

    struct Wave {
        char chunk0_id[4];
        uint32_t chunk0_size;
        char format[4];
        char chunk1_id[4];
        uint32_t chunk1_size;
        uint16_t audio_format;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t byte_rate;
        uint16_t block_align;
        uint16_t bits_per_sample;
        char chunk2_id[4];
        uint32_t chunk2_size;
        bfloat * data;
    };

    static float * unpack_wave(bfloat * source, size_t source_length, size_t * output_size);

    static bfloat * repack_wave(float * source, size_t source_length, uint32_t current_delay, uint32_t overall_delay, size_t * output_size);

    static void interleave(bfloat * left, bfloat * right, bfloat * out, const size_t length);

    static Wave * readFile(const char * fileName);

    static void writeFile(const char * fileName, const Wave * wave, bfloat * left, bfloat * right, size_t length);

    static bfloat** virtualize(const Wave * source, const char * sofaFile, size_t * data_length, float x, float y, float z, size_t block_size);

public:
    static void virtualize(const char * fileName, const char * output_fileName, const char * sofa_file, float x, float y, float z, Filter::Coord_Type coord_type, size_t block_size);
};
