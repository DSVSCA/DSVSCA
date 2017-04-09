#ifndef DSVSCA_H
#define DSVSCA_H

#include <atomic>
#include <iostream>
#include "filter.h"
#include "format.h"
#include "encoder.h"
#include "sjoin.h"
#include "virtualizer.h"
#include <ctime>
#include <stdio.h>
#include <libavutil/fifo.h>

typedef struct {
    const AVClass    *classs;
    AVFifoBuffer     *fifo;
    AVRational        time_base;     ///< time_base to set in the output link
    AVRational        frame_rate;    ///< frame_rate to set in the output link
    unsigned          nb_failed_requests;
    unsigned          warning_limit;
 
    /* video only */
    int               w, h;
    enum AVPixelFormat  pix_fmt;
    AVRational        pixel_aspect;
    char              *sws_param;
 
    /* audio only */
    int sample_rate;
    enum AVSampleFormat sample_fmt;
    char               *sample_fmt_str;
    int channels;
    uint64_t channel_layout;
    char    *channel_layout_str;
 
    int eof;
} BufferSourceContext;

struct coordinate {
    float x;
    float y;
    float z;
};

struct process_info {
    Format * format;
    Filter * filter;
    std::string video_file_name;
    std::string sofa_file_name;
    Filter::Coord_Type coord_type;
    std::unordered_map<Filter::Channel, coordinate, std::hash<int>> coords;
    int block_size = BLOCK_SIZE;
    std::atomic_int * progress;
};

class DSVSCA {
public:
    static int process_filter_graph(process_info info);

private:
};

#endif
