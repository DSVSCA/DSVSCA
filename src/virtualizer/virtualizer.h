#include <iostream>
#include <vector>
#include <fstream>
extern "C" {
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

typedef struct SofaFile {  /* contains data of one SOFA file */
    int ncid;            /* netCDF ID of the opened SOFA file */
    int n_samples;       /* length of one impulse response (IR) */
    int m_dim;           /* number of measurement positions */
    int *data_delay;     /* broadband delay of each IR */
                         /* all measurement positions for each receiver (i.e. ear): */
    float *sp_a;         /* azimuth angles */
    float *sp_e;         /* elevation angles */
    float *sp_r;         /* radii */
                         /* data at each measurement position for each receiver: */
    float *data_ir;      /* IRs (time-domain) */
} SofaFile; 

typedef struct SOFAContext {
    const AVClass *class;
    char *filename;             /* name of SOFA file */
    SofaFile sofa;              /* contains data of the SOFA file */
    int sample_rate;            /* sample rate from SOFA file */
    float *speaker_azim;        /* azimuth of the virtual loudspeakers */
    float *speaker_elev;        /* elevation of the virtual loudspeakers */
    float gain_lfe;             /* gain applied to LFE channel */
    int lfe_channel;            /* LFE channel position in channel layout */
    int n_conv;                 /* number of channels to convolute */
                                /* buffer variables (for convolution) */
    float *ringbuffer[2];       /* buffers input samples, length of one buffer: */
                                /* no. input ch. (incl. LFE) x buffer_length */
    int write[2];               /* current write position to ringbuffer */
    int buffer_length;          /* is: longest IR plus max. delay in all SOFA files */
                                /* then choose next power of 2 */
    int n_fft;                  /* number of samples in one FFT block */
                                /* netCDF variables */
    int *delay[2];              /* broadband delay for each channel/IR to be convolved */
    float *data_ir[2];          /* IRs for all channels to be convolved */
                                /* (this excludes the LFE) */
    float *temp_src[2];
    FFTComplex *temp_fft[2];
                         /* control variables */
    float gain;          /* filter gain (in dB) */
    float rotation;      /* rotation of virtual loudspeakers (in degrees)  */
    float elevation;     /* elevation of virtual loudspeakers (in deg.) */
    float radius;        /* distance virtual loudspeakers to listener (in metres) */
    int type;            /* processing type */
    FFTContext *fft[2], *ifft[2];
    FFTComplex *data_hrtf[2];
    AVFloatDSPContext *fdsp;
} SOFAContext;

typedef struct ConvoluteData {
    AVFrame * in;
    AVFrame * out;
    int * write;
    int ** delay;
    float ** ir;
    int * num_clippings;
    float ** ring_buffer;
    float ** temp_src;
    FFTComplex ** temp_fft;
} ConvoluteData;

class Virtualizer {
public:
    static void close_sofa(struct SofaFile * file);
    static int load_sofa(AVFilterContext * context, char * fileName); // returns the sample rate
    static void filter_frame(AVFilter * link, AVFrame * in);

private:
    Virtualizer();

    static void convolute(AVFilterContext * context, ConvoluteData * arg, int jobNum, int numJobs);
    static void fast_convolute(AVFilterContext * context, ConvoluteData * arg, int jobNum, int numJobs);
    static void get_speaker_position(AVFilterContext * context, float * azim, float * elev);
    static int max_delay(struct SofaFile * file);
    static int get_closest_measurement(SOFAContext * context, int azim, int elev, float radius);
    static void compensate_volume(AVFilterContext * context);
    static void load_data(AVFilterContext * context, int azim, int elev, float radius);
}

#endif
