#include "decomposer.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

AVFormatContext *Decomposer::ic = NULL;
AVStream *Decomposer::audio_stream = NULL;
AVFilterGraph *Decomposer::f_graph;
Decomposer::Decomposer(std::string fileName, bool verbose) {
    // TODO: Add error checking for fileName
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    avfilter_register_all();
    
    if(verbose)
      std::cout << "Loading " << fileName << std::endl;

    char *filename = new char[fileName.length() + 1];
    strcpy(filename, fileName.c_str());
    if(avformat_open_input(&ic, filename, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "error opening %s\n", filename);
        // TODO: throw an exception
    }

    if (avformat_find_stream_info(ic, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: could not find codec parameters\n", filename);
        //TODO: throw an error
        std::cout << "Error on stream info";
    }
    
    // discard all the streams, we don't care about the video and will save audio stream later
    for(int i = 0; i < ic->nb_streams; i++)
        ic->streams[i]->discard = AVDISCARD_ALL;

    AVCodec *decoder = NULL;
    int audio_stream_index = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
    
    // There should be 1 audio stream? 
    if(verbose) 
        std::cout << "Index of audio stream: " << audio_stream_index << std::endl;

    if(audio_stream_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s: no audio stream found\n", ic->filename);
        //TODO throw exception
    }    

    if(!decoder) {
        av_log(NULL, AV_LOG_ERROR, "%s: no decoder found\n", ic->filename);
    }
    
    audio_stream = ic->streams[audio_stream_index];
    audio_stream->discard = AVDISCARD_DEFAULT;
    
    AVCodecContext *avcctx = audio_stream->codec; 

    if(!avcctx->channel_layout)
      avcctx->channel_layout = av_get_default_channel_layout(avcctx->channels);
    if(!avcctx->channel_layout)   
      av_log(NULL, AV_LOG_ERROR, "unable to guess cahnnel layout\n");
    if(verbose) { 
        std::cout << "Channel layout was successfully found." << std::endl;
        std::cout << "Here is some more info about it: " << std::endl;
        std::cout << "Layout: " << avcctx->channel_layout << std::endl;
        std::cout << "# Channels: " << avcctx->channels << std::endl;
    }
    // at this point we should be able to init a filter graph
    init_filter_graph();
}

int Decomposer::init_filter_graph() {
    f_graph = avfilter_graph_alloc();
    if(!f_graph) {
        av_log(NULL, AV_LOG_ERROR, "FILTER GRAPH: unable to create filter graph: out of memory!\n");
        return -1;
    }
    
    

    return 0;
}
