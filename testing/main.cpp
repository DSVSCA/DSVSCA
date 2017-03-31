#include <cmath>
#include <fstream>
#include <iostream>
#include <string.h>
#include "FFTConvolver.h"

extern "C" {
#include <mysofa.h>
}

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

void interleave(bfloat * left, bfloat * right, bfloat * out, const size_t length) {
    size_t out_index = 0;
    for (size_t i = 0; i < length; i++) {
        for (size_t x = 0; x < 2; x++) {
            out[out_index].b[0] = left[i].b[x];
            out[out_index].b[1] = right[i].b[x];
            out_index++;
        }
    }
}

Wave * readFile(const char * fileName) {
    std::ifstream f(fileName, std::ios::in | std::ios::binary | std::ios::ate);
    if (!f.is_open()) return NULL;
    Wave * wave = new Wave();
    std::ifstream::pos_type fileSize = f.tellg();
    printf("File Size: %d\n", (int)fileSize);
    char * contents = new char[fileSize];
    f.seekg(0, std::ios::beg);
    f.read(contents, fileSize);
    f.close();
    unsigned int read = 0;
    memcpy(wave->chunk0_id, contents + read, 4);
    read += 4;
    memcpy(&wave->chunk0_size, contents + read, 4);
    read += 4;
    memcpy(&wave->format, contents + read, 4);
    read += 4;
    memcpy(wave->chunk1_id, contents + read, 4);
    read += 4;
    memcpy(&wave->chunk1_size, contents + read, 4);
    read += 4;
    memcpy(&wave->audio_format, contents + read, 2);
    read += 2;
    memcpy(&wave->num_channels, contents + read, 2);
    read += 2;
    memcpy(&wave->sample_rate, contents + read, 4);
    read += 4;
    memcpy(&wave->byte_rate, contents + read, 4);
    read += 4;
    memcpy(&wave->block_align, contents + read, 2);
    read += 2;
    memcpy(&wave->bits_per_sample, contents + read, 2);
    read += 2;
    memcpy(wave->chunk2_id, contents + read, 4);
    read += 4;
    memcpy(&wave->chunk2_size, contents + read, 4);
    read += 4;

    wave->data = new bfloat[wave->chunk2_size];
    memcpy(wave->data, contents + read, wave->chunk2_size);
    read += wave->chunk2_size;

    printf("Input:\nChunk0 ID: %.*s\nSize: %d\nFormat: %.*s\nChunk1 ID: %.*s\nSize: %d\nAudio Format: %d\nNum Channels: %d\nSample Rate: %d\nByte Rate: %d\nByte Align: %d\nBits Per Sample: %d\nChunk2 ID: %.*s\nSize: %d\n\n",
            4, wave->chunk0_id, wave->chunk0_size, 4, wave->format, 4, wave->chunk1_id, wave->chunk1_size, wave->audio_format, wave->num_channels, wave->sample_rate, wave->byte_rate, wave->block_align, wave->bits_per_sample, 4, wave->chunk2_id, wave->chunk2_size);

    return wave;
}

void writeFile(const char * fileName, Wave * wave, bfloat * left, bfloat * right, size_t length) {
    std::ofstream f(fileName, std::ios::out | std::ios::binary);
    if (!f.is_open()) return;

    uint16_t num_channels = 2;
    uint32_t chunk0_size = 4 + (8 + wave->chunk1_size) + (8 + (length * 2));
    uint32_t data_length = length * 2;
    uint32_t byte_rate = wave->byte_rate * 2;
    uint16_t block_align = wave->block_align * 2;
    char * contents = new char[chunk0_size + 8];
    uint32_t read = 0;

    memcpy(contents + read, wave->chunk0_id, 4);
    read += 4;
    memcpy(contents + read, &chunk0_size, 4);
    read += 4;
    memcpy(contents + read, &wave->format, 4);
    read += 4;
    memcpy(contents + read, wave->chunk1_id, 4);
    read += 4;
    memcpy(contents + read, &wave->chunk1_size, 4);
    read += 4;
    memcpy(contents + read, &wave->audio_format, 2);
    read += 2;
    memcpy(contents + read, &num_channels, 2);
    read += 2;
    memcpy(contents + read, &wave->sample_rate, 4);
    read += 4;
    memcpy(contents + read, &byte_rate, 4);
    read += 4;
    memcpy(contents + read, &block_align, 2);
    read += 2;
    memcpy(contents + read, &wave->bits_per_sample, 2);
    read += 2;
    memcpy(contents + read, wave->chunk2_id, 4);
    read += 4;
    memcpy(contents + read, &data_length, 4);
    read += 4;

    bfloat * out = new bfloat[data_length];
    interleave(left, right, out, length);
    memcpy(contents + read, out, data_length);
    read += length * 2;

    f.write(contents, chunk0_size + 8);
    f.close();
}

bfloat** virtualize(const Wave * source, const char * sofaFile, size_t * data_length, float x, float y, float z) {
    uint32_t sample_rate = source->sample_rate;
    *data_length = source->chunk2_size;

    int filter_length;
    int err;
    struct MYSOFA_EASY * hrtf = mysofa_open(sofaFile, sample_rate, &filter_length, &err);
    if (hrtf == NULL) printf("HRTF is null.\n");
    if (err != 0) printf("Error code: %d\n", err);
    if (filter_length < 1) {
        printf("No FIR information is available for this HRTF.\n");
        return NULL;
    }

    // get IR and delay for front left (1 1 0)
    float * leftIR = new float[filter_length];
    float * rightIR = new float[filter_length];
    float left_delay_f = -1;
    float right_delay_f = -1;
    while (left_delay_f < 0 || right_delay_f < 0) mysofa_getfilter_float(hrtf, x, y, z, leftIR, rightIR, &left_delay_f, &right_delay_f);

    uint32_t left_delay = 0;
    uint32_t right_delay = 0;
    if (left_delay_f < right_delay_f) right_delay = std::round((right_delay_f - left_delay_f) * sample_rate);
    else if (left_delay_f > right_delay_f) left_delay = std::round((left_delay_f - right_delay_f) * sample_rate);
    printf("Old Left Delay: %f, Old Right Delay: %f\n", left_delay_f, right_delay_f);
    printf("Left Delay: %d, Right Delay: %d\n", left_delay, right_delay);
    *data_length += left_delay + right_delay;

    bfloat ** out = new bfloat*[2];
    for (int i = 0; i < 2; i++) {
        out[i] = new bfloat[*data_length];

        for (int x = 0; x < *data_length; x++) out[i][x].f = 0;
    }

    fftconvolver::FFTConvolver left_conv;
    fftconvolver::FFTConvolver right_conv;
    bool left_success = left_conv.init(2048, leftIR, filter_length);
    bool right_success = right_conv.init(2048, rightIR, filter_length);

    if (!left_success) printf("Left Convolution failed during init.\n");
    if (!right_success) printf("Right Convolution failed during init.\n");
    if (!left_success || !right_success) return out;

    left_conv.process((float*)source->data, (float*)(out[0] + left_delay), source->chunk2_size);
    right_conv.process((float*)source->data, (float*)(out[1] + right_delay), source->chunk2_size);
    //memcpy((float*)(out[0] + left_delay), (float*)source->data, source->chunk2_size);
    //memcpy((float*)(out[1] + right_delay), (float*)source->data, source->chunk2_size);

    mysofa_close(hrtf);

    return out;
}

int main(int argc, const char ** argv) {
    std::string fileName = std::string(argv[1]);
    std::string sofaFile = std::string(argv[2]);
    std::string outputFile = "out." + fileName;

    Wave * wave = readFile(fileName.c_str());

    size_t data_length;
    bfloat ** audio = virtualize(wave, sofaFile.c_str(), &data_length, 1, 1, 0);
    if (audio == NULL) return 1;

    writeFile(outputFile.c_str(), wave, audio[0], audio[1], data_length);

    return 0;
}
