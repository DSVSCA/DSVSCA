#include "format.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/log.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

AVFormatContext *Format::format_ctx = NULL;  // Format I/O Context ????
AVStream *Format::audio_stream = NULL;  // Stream structure 
int Format::audio_stream_index = 0;
AVCodecContext *Format::decoder_ctx = NULL;  // Main API structure

Format::Format(std::string fileName) {  
    std::cout << "Initializing libav* " << std::endl;
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    avfilter_register_all();
    
    std::cout << "Initializing input file" << fileName << std::endl;
    
    init_target_file(fileName);
}

Format::~Format() {
    avcodec_close(decoder_ctx);
    avformat_close_input(&format_ctx);
}

int Format::init_target_file(std::string fileName) { 
    AVCodec *decoder = NULL;    // Provides codec information and decoder info
    char *filename = new char[fileName.length() + 1];
    strcpy(filename, fileName.c_str());

    int error;
    error = avformat_open_input(&format_ctx, filename, NULL, NULL);
    
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error opening %s\n", filename);
        return error;
    }

    error = avformat_find_stream_info(format_ctx, NULL);

    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: could not find codec parameters\n", filename);
        return error;
    }
    
    audio_stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
   
    if(audio_stream_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "No audio stream found in the input file\n");
        error = audio_stream_index;
        return error;
    }

    audio_stream = format_ctx->streams[audio_stream_index];
    decoder_ctx = format_ctx->streams[audio_stream_index]->codec;
    
    error = avcodec_open2(decoder_ctx, decoder, NULL);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: unable to open decoder/decoder not found\n", format_ctx->filename);
        return error; 
    } 
    
    std::cout << "  * Format Context Info" << std::endl;
    std::cout << "      - Video file duration: " << format_ctx->duration << std::endl;
    std::cout << "      - Stream bitrate:      " << format_ctx->bit_rate << std::endl;
    std::cout << "      - audio stream index:  " << audio_stream_index << std::endl; 
    
    return 0;
}




