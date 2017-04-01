#include "virtualizer.h"

// What I need:
// Sample Rate, data, data length, position of the source, the sofa file
// What I return:
// Two channels of float audio and the length of the channels
float** virtualize(const Wave * source, const char * sofaFile, size_t * data_length, float x, float y, float z) {
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

    // get IR and delay
    float leftIR[filter_length];
    float rightIR[filter_length];
    float left_delay_f = -1;
    float right_delay_f = -1;
    mysofa_getfilter_float(hrtf, x, y, z, (float*)leftIR, (float*)rightIR, &left_delay_f, &right_delay_f);

    uint32_t left_delay = 0;
    uint32_t right_delay = 0;
    if (left_delay_f < right_delay_f) right_delay = std::round((right_delay_f - left_delay_f) * sample_rate);
    else if (left_delay_f > right_delay_f) left_delay = std::round((left_delay_f - right_delay_f) * sample_rate);
    uint32_t overall_delay = left_delay + right_delay;
    *data_length += (overall_delay / 2);

    float ** out = new float*[2];

    fftconvolver::FFTConvolver left_conv;
    fftconvolver::FFTConvolver right_conv;
    bool left_success = left_conv.init(8, leftIR, filter_length);
    bool right_success = right_conv.init(8, rightIR, filter_length);

    if (!left_success) printf("Left Convolution failed during init.\n");
    if (!right_success) printf("Right Convolution failed during init.\n");
    if (!left_success || !right_success) return out;

    size_t to_conv_size;
    float * to_conv = unpack_wave(source->data, source->chunk2_size, &to_conv_size);
    float * out_conv_left = new float[to_conv_size];
    float * out_conv_right = new float[to_conv_size];

    left_conv.process(to_conv, out_conv_left, to_conv_size);
    right_conv.process(to_conv, out_conv_right, to_conv_size);

    size_t output_size_left;
    size_t output_size_right;
    out[0] = repack_wave(out_conv_left, to_conv_size, left_delay, overall_delay, &output_size_left);
    out[1] = repack_wave(out_conv_right, to_conv_size, right_delay, overall_delay, &output_size_right);

    delete[] to_conv;
    delete[] out_conv_left;
    delete[] out_conv_right;
    mysofa_close(hrtf);

    return out;
}
