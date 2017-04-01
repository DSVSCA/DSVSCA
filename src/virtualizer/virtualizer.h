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

class Virtualizer {
public:
    static float ** virtualize();

private:
    Virtualizer();
}

#endif
