#include "decomposer.h"
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
}


char Decomposer::strbuf[512];   // Send filter graph args through this

AVFormatContext *Decomposer::ic = NULL;  // Format I/O Context ????
AVStream *Decomposer::audio_stream = NULL;  // Stream structure 

AVFrame *Decomposer::oframe = NULL; // Describes decoded (raw) audio/video 

AVFilterGraph *Decomposer::filter_graph;  // Holds filter steps

AVFilterContext *Decomposer::abuffer_ctx = NULL; // Buffers audio frames to expose to filter chain
AVFilterContext *Decomposer::channelsplit_ctx = NULL;  // Splits channels into multiple output streams

// Buffer audio streams and make available at the end of a filter chain. 6 here for a 5.1
// channel audio setup
std::vector<AVFilterContext*> Decomposer::abuffersinks_ctx = std::vector<AVFilterContext*>(6); 



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
      av_log(NULL, AV_LOG_ERROR, "unable to guess chnnel layout\n");
    if(verbose) { 
        std::cout << "Channel layout was successfully found." << std::endl;
        std::cout << "Here is some more info about it: " << std::endl;
        std::cout << "Layout: " << avcctx->channel_layout << std::endl;
        std::cout << "# Channels: " << avcctx->channels << std::endl;
    }
    // at this point we should be able to init a filter graph
    init_filter_graph(avcctx);
}

int Decomposer::init_filter_graph(AVCodecContext *avcctx) {
    filter_graph = avfilter_graph_alloc();
    if(!filter_graph) {
        av_log(NULL, AV_LOG_ERROR, "FILTER GRAPH: unable to create filter graph: out of memory!\n");
        return -1;
    }
    
    std::cout << std::endl;
    std::cout << "Beginning creation of the filter graph" << std::endl;

    int error;
     
    error = init_abuffer_ctx();
    if(error >= 0) error = init_channelsplit_ctx();
    if(error >= 0) error = init_abuffersinks_ctx();
    
    if(error < 0) return error;
    
    /*
    // Split into all the channels
    // FL
    if(error >= 0) error = avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 0);
    
    // FR
    if(error >= 0) error = avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 1);

    // FC
    if(error >= 0) error = avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 2);

    // LFE 
    if(error >= 0) error = avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 3);

    // BL
    if(error >= 0) error = avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 4);

    // BR
    if(error >= 0) error = avfilter_link(abuffer_ctx, 0, channelsplit_ctx, 5);

    // Place back into our buffer sink
    // FL
    if(error >= 0) error = avfilter_link(channelsplit_ctx, 0, abuffersinks_ctx[0], 0);
    
    // FR 
    if(error >= 0) error = avfilter_link(channelsplit_ctx, 1, abuffersinks_ctx[1], 0);
    
    // FC
    if(error >= 0) error = avfilter_link(channelsplit_ctx, 2, abuffersinks_ctx[2], 0);
    
    // LFE
    if(error >= 0) error = avfilter_link(channelsplit_ctx, 3, abuffersinks_ctx[3], 0);
    
    // BL
    if(error >= 0) error = avfilter_link(channelsplit_ctx, 4, abuffersinks_ctx[4], 0);
    
    // BR
    if(error >= 0) error = avfilter_link(channelsplit_ctx, 5, abuffersinks_ctx[5], 0);

    */
    


    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error linking filter graph\n");
        return error;
    }
    
    error = avfilter_graph_config(filter_graph, NULL);
    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error configuring filter graph\n");
        return error;
    }
    std::cout << "Graph has been created!" << std::endl;
    
    std::cout << filter_graph->filters[0]->name << std::endl;

    // char *options;
    // std::string filter_dump = avfilter_graph_dump(filter_graph, options);
    // error = stream_packets(avcctx);
    return 0;
}


int Decomposer::init_abuffer_ctx() {
    AVFilter *abuffer = avfilter_get_by_name("abuffer");
    
    AVCodecContext *avcctx = audio_stream->codec;
    AVRational time_base = audio_stream->time_base;
    
    // Create an abuffer filter
    snprintf(strbuf, sizeof(strbuf),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64, 
             time_base.num, time_base.den, avcctx->sample_rate,
             av_get_sample_fmt_name(avcctx->sample_fmt),
             avcctx->channel_layout);

    fprintf(stderr, "abuffer: %s\n", strbuf);
    int error = avfilter_graph_create_filter(&abuffer_ctx, abuffer,
                NULL, strbuf, NULL, filter_graph);

    if (error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing abuffer filter\n");
        return error;
    }
    

    return error;
}

int Decomposer::init_channelsplit_ctx() {
    AVFilter *channelsplit = avfilter_get_by_name("channelsplit");
    
    snprintf(strbuf, sizeof(strbuf), 
             "channel_layout=FL+FR+FC+LFE+BL+BR");

    fprintf(stderr, "channelsplit: %s\n", strbuf);
    int error = avfilter_graph_create_filter(&channelsplit_ctx, channelsplit,
                NULL, strbuf, NULL, filter_graph);

    if(error < 0) {
        av_log(NULL, AV_LOG_ERROR, "error initializing channelsplit filter\n");
        return error;
    }

    return error;
}

int Decomposer::init_abuffersinks_ctx() {
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    
    std::cout << "abuffersinks: ";
    for(int i = 0; i < 6; i++) {
        int error = avfilter_graph_create_filter(&abuffersinks_ctx[i], abuffersink,
                    NULL, NULL, NULL, filter_graph);
        
        std::cout << i;
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "unable to create abuffersink filter %d\n", i);
            return error;
        }
    }
    
    std::cout << std::endl;
    
    return 0;
}

int Decomposer::audio_decode_frame(AVPacket *packet, AVFrame *frame) {
    AVPacket packet_temp_; 
    memset(&packet_temp_, 0, sizeof(packet_temp_));
    AVPacket *packet_temp = &packet_temp_;

    *packet_temp = *packet;
    
    std::cout << "-------" << std::endl << std::endl;
    std::cout << "New packet!" << std::endl;
    std::cout << "Packet Size: " << packet_temp->size << std::endl;

    int length, got_frame;
    int new_packet = 1;
    while(packet_temp->size > 0 || (!packet_temp->data && new_packet)) {
        av_frame_unref(frame);
        new_packet = 0;

        std::cout << "Begin decoding" << std::endl;
        std::cout << "Codec Info" << std::endl;
        std::cout << "  -Codec ID: " << audio_stream->codec->codec_id << std::endl;
        std::cout << "  -Bit rate: " << audio_stream->codec->bit_rate << std::endl;
       

        length = avcodec_decode_audio4(audio_stream->codec, frame, &got_frame, packet_temp);
        
        std::cout << "Frame Info" << std::endl;
        std::cout << "  -Audio Channel Count: " << frame->channels << std::endl << std::endl;
         
        std::cout << length << std::endl;
        if(length < 0) {
            // if error skip frame
            packet_temp->size = 0;
            return -1;
        }

        packet_temp->data += length;
        packet_temp->size -= length;

        if(!got_frame) {
        // stop if finished
            if(!packet_temp->data && 
                audio_stream->codec->codec->capabilities&CODEC_CAP_DELAY) {
                return 0;
            }
            continue;
        }

        // push audio data from decoded frame into the filter graph

        int error = av_buffersrc_write_frame(abuffer_ctx, frame);
        if(error < 0) {
            av_log(NULL, AV_LOG_ERROR, "Decode: error writing frame to buffersrc\n");
            return -1;
        }

        // pull filtered audio from FL channel filter graph (this is so cool)
        for(;;) {
            int error = av_buffersink_get_frame(abuffersinks_ctx[0], oframe);
            if((error = AVERROR_EOF) || (error = AVERROR(EAGAIN))) {
                break;
            }
            if(error < 0) {
                av_log(NULL, AV_LOG_ERROR, "Decode: error reading buffer from buffersink!\n");
            }

            int nb_channels = av_get_channel_layout_nb_channels(oframe->channel_layout);
            //int bytes_per_sample = av_get_bytes_per_sample(oframe->format);
            //int data_size = oframe->nb_samples * nb_channels * bytes_per_sample;
            //std::cout << (void*)oframe->data[0] << std::endl;
            std::cout << "HI";
        }
    }


    return 0;
}


int Decomposer::stream_packets(AVCodecContext *avcctx) {
    AVPacket audio_packet;
    memset(&audio_packet, 0, sizeof(audio_packet));
    AVPacket *packet = &audio_packet;
    AVFrame *frame = av_frame_alloc();

    oframe = av_frame_alloc();
    if(!oframe) {
        av_log(NULL, AV_LOG_ERROR, "AVFrame: error allocating oframe\n");
        return -1;
    }
    
    int eof = 0;

    for(;;) {
        if(eof) {
            /*if(avcctx->codec->capabilities & CODEC_CAP_DELAY) {
                av_init_packet(packet);
                packet->data = NULL;
                packet->size = 0;
                packet->stream_index = 1;
                std::cout << packet << std::endl;
                if(audio_decode_frame(packet, frame) > 0) {
                    // keep flushing packets?
                    continue;
                }                
            }*/
            break;
        }
        int error = av_read_frame(ic, packet);
        if(error < 0) {
            if(error != AVERROR_EOF)
              av_log(NULL, AV_LOG_WARNING, "AVFrame: error reading frames\n");
            eof = 1;
            continue;
        }
        if(packet->stream_index != 1) {
            av_free_packet(packet);
            continue;
        }
        //std::cout << packet << std::endl;
        audio_decode_frame(packet, frame);
        av_free_packet(packet);
    }

    avformat_network_deinit();
    return 0;
}
