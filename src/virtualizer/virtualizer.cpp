#include "virtualizer.h"

// The x-axis (1 0 0) is the listening direction. The y-axis (0 1 0) is the left side of the listener. The z-axis (0 0 1) is upwards.
Virtualizer::Virtualizer(const char * sofa_file_name, int sample_rate, float x, float y, float z, int block_size) {
    this->open_sofa(sofa_file_name, sample_rate); 
    this->init(hrtf, sample_rate, x, y, z, block_size);
}

Virtualizer::Virtualizer(sofa_file * hrtf, int filter_length, int sample_rate, float x, float y, float z, int block_size) {
    this->hrtf = hrtf;
    this->filter_length = filter_length;

    this->init(hrtf, sample_rate, x, y, z, block_size);
}

Virtualizer::~Virtualizer() {
    this->close_sofa();
    delete[] this->right_delay;
    delete[] this->left_delay;
    delete[] this->overflow_audio;
    delete this->left_conv;
    delete this->right_conv;
}

void Virtualizer::init(sofa_file * hrtf, int sample_rate, float x, float y, float z, int block_size) {
    left_ir = new float[this->filter_length];
    right_ir = new float[this->filter_length];
    float left_delay_f = -1;
    float right_delay_f = -1;
    mysofa_getfilter_float(this->hrtf, x, y, z, left_ir, right_ir, &left_delay_f, &right_delay_f);

    this->right_delay = 0;
    this->left_delay = 0;
    if (left_delay_f < right_delay_f) this->right_delay = std::round((right_delay_f - left_delay_f) * sample_rate);
    else if (left_delay_f > right_delay_f) this->left_delay = std::round((left_delay_f - right_delay_f) * sample_rate);
    this->overall_delay = this->left_delay + this->right_delay;

    this->overflow_audio = new float[this->overall_delay];
    memset(this->overflow_audio, 0, this->overall_delay);

    this->left_conv = new fftconvolver::FFTConvolver();
    this->right_conv = new fftconvolver::FFTConvolver();
    bool left_success = this->left_conv.init(block_size, this->left_ir, this->filter_length);
    bool right_success = this->right_conv.init(block_size, this->right_ir, this->filter_length);
    
    if (!left_success) printf("Left Convolution failed during init.\n");
    if (!right_success) printf("Right Convolution failed during init.\n");
    //TODO: do something if this fails
}

float ** process(const float * source, size_t data_length) {
    float ** out = new float*[2];
    float * out_conv_left = new float[data_length];
    float * out_conv_right = new float[data_length];

    left_conv.process(source, out_conv_left, data_length);
    right_conv.process(source, out_conv_right, data_length);

    // TODO: right now, we just drop the audio at the end of the file for the delayed channel. So we need to figure out how to extend the audio by the amount of the overall delay.
    // add in the delay
    float * new_overflow = new float[this->overall_delay];
    if (right_delay > left_delay) {
        // move the end of the current float array to our new overflow audio array
        memmove(new_overflow, out_conv_right + data_length - this->overall_delay, this->overall_delay);
        // shift everything in the array back to make room at the front to place the audio in our overflow_audio buffer
        memmove(out_conv_right + this->overall_delay, out_conv_right, data_length - this->overall_delay);
        // copy our overflow audio buffer to the front of the array
        memmove(out_conv_right, this->overflow_audio, this->overall_delay);
    }
    else if (left_delay > right_delay) {
        memmove(new_overflow, out_conv_left + data_length - this->overall_delay, this->overall_delay);
        memmove(out_conv_left + this->overall_delay, out_conv_left, data_length - this->overall_delay);
        memmove(out_conv_left, this->overflow_audio, this->overall_delay);
    }

    delete[] this->overflow_audio;
    this->overflow_audio = new_overflow;

    return out;
}

void open_sofa(const char * file_name, int sample_rate) {
    if (hrtf != NULL || filter_length > 0) return;

    int err;
    hrtf = mysofa_open(file_name, sample_rate, &filter_length, &err);
    if (hrtf == NULL) printf("HRTF is null.\n");
    if (err != 0) printf("Error code: %d\n", err);
    if (filter_length < 1) printf("No FIR information is available for this HRTF.\n");
}

void close_sofa() {
    mysofa_close(hrtf);
    hrtf = NULL;
    filter_length = 0;
}

complete_sofa get_hrtf() {
    complete_sofa sofa;
    sofa.hrtf = this->hrtf;
    sofa.filter_length = this->filter_length;

    return sofa;
}

uint8_t * Virtualizer::get_short_samples(float * buffer, AVSampleFormat format, uint8_t sample_count) {
    // based on implementation here: https://www.targodan.de/post/decoding-audio-files-with-ffmpeg/
    uint8_t * out = new uint8_t[sample_count];
    int sample_size = av_get_bytes_per_sample(format);
    int64_t num_signed_bits = sample_size * 8 - 1;
    int64_t max_val = (1 << num_signed_bits) - 1;

    for (uint8_t i = 0; i < sample_count; i++) {
        int64_t current_val;
        switch(format) {
            case AV_SAMPLE_FMT_U8:
            case AV_SAMPLE_FMT_S16:
            case AV_SAMPLE_FMT_S32:
            case AV_SAMPLE_FMT_U8P:
            case AV_SAMPLE_FMT_U8P:
            case AV_SAMPLE_FMT_S16P:
            case AV_SAMPLE_FMT_S32P:
                current_val = (int64_t)(buffer[i] * max_val);
                break;
            case AV_SAMPLE_FMT_FLT:
            case AV_SAMPLE_FMT_FLTP:
            case AV_SAMPLE_FMT_DBL:
            case AV_SAMPLE_FMT_DBLP:
                current_val = (int64_t)current_val;
                break;
        }

        switch (sample_size) {
            case 1:
                // add the minimum value of int8_t since we are going from back from signed to unsigned
                ((uint8_t*)out)[i] = (uint8_t)(buffer[i] - SCHAR_MIN);
                break;

            case 2:
                ((int16_t*)out)[i] = (int16_t)current_val;
                break;

            case 4:
                ((int32_t*)out)[i] = (int32_t)current_val;
                break;

            case 8:
                ((int64_t*)out)[i] = current_val;
                break;
        }
    }

    return out;
} 

float * Virtualizer::get_float_samples(uint8_t * buffer, AVSampleFormat format, uint8_t sample_count) {
    // based on implementation here: https://www.targodan.de/post/decoding-audio-files-with-ffmpeg/
    float * out = new float[sample_count];
    int sample_size = av_get_bytes_per_sample(format);
    int64_t num_signed_bits = sample_size * 8 - 1;
    int64_t max_val = (1 << num_signed_bits) - 1;

    for (uint8_t i = 0; i < sample_count; i++) {
        int64_t current_val;
        switch (sample_size) {
            case 1:
                // subtract the minimum value of int8_t since we are going from unsigned to signed
                current_val = buffer[i] + SCHAR_MIN;
                break;

            case 2:
                current_val = ((int16_t*)buffer)[i];
                break;

            case 4:
                current_val = ((int32_t*)buffer)[i];
                break;

            case 8:
                current_val = ((int64_t*)buffer)[i];
                break;
        }

        switch(format) {
            case AV_SAMPLE_FMT_U8:
            case AV_SAMPLE_FMT_S16:
            case AV_SAMPLE_FMT_S32:
            case AV_SAMPLE_FMT_U8P:
            case AV_SAMPLE_FMT_U8P:
            case AV_SAMPLE_FMT_S16P:
            case AV_SAMPLE_FMT_S32P:
                out[i] = current_val / (float)max_val;
                break;
            case AV_SAMPLE_FMT_FLT:
            case AV_SAMPLE_FMT_FLTP:
            case AV_SAMPLE_FMT_DBL:
            case AV_SAMPLE_FMT_DBLP:
                out[i] = (float)current_val;
                break;
        }
    }

    return out;
}
