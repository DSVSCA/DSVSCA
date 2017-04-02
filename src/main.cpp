#include <iostream>
#include "input/filter.h"
#include "input/format.h"
#include "sjoin/sjoin.h"
#include <ctime>
#include "mysofa.h"
#include <stdio.h>

int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;
    while(*p != AV_SAMPLE_FMT_NONE) {
        if(*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

int select_sample_rate(AVCodec *codec) {
    const int *p;
    int best_samplerate = 0;

    if(!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while(*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;

}


int process_filter_graph(Format *fmt, Filter *filter, SJoin *sj) {
    
    FILE *f;
    const char *filename = "FL.aac";

    AVPacket pkt;
    int got_output;
    
    AVPacket packet, packet0;
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();

    AVCodec *codec;
    AVCodecContext *c = NULL;

    codec = avcodec_find_encoder(AV_CODEC_ID_AC3);
    if(!codec) {
        std::cout << "Codec not found!" << std::endl;
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if(!c) {
        std::cout << "Failed to allocate audio codec context" << std::endl;
    }

    int buffer_size;

    c->bit_rate = fmt->decoder_ctx->bit_rate;
    c->sample_fmt = AV_SAMPLE_FMT_FLTP;
    if(!check_sample_fmt(codec, c->sample_fmt)) {
        std::cout << "Does not support sample format" << std::endl;
        exit(1);
    }

    c->sample_rate = select_sample_rate(codec);
    c->channel_layout = 3;
    c->channels = 2;

    if(avcodec_open2(c, codec, NULL) < 0) {
        std::cout << "Could not open codec" << std::endl;
        exit(1);
    }

    f = fopen(filename, "wb");
    if(!f) {
        std::cout << "Error opening file" << std::endl;
    }

    buffer_size = av_samples_get_buffer_size(NULL, c->channels, c->frame_size, c->sample_fmt, 0);

    if(buffer_size < 0) {
        std::cout << buffer_size << " COULD NOT GET SAMPLE BUFFER SIZE" << std::endl;
        exit(1);
    }

    av_malloc(buffer_size);
    std::cout << "BUFFER SIZE: " << buffer_size << std::endl;


    int got_frame;
    int ret = 0;

    /* Read all of the packets */
    packet0.data = NULL;
    packet.data = NULL;

    while(1) {

        if(!packet0.data) {
            ret = av_read_frame(fmt->format_ctx, &packet);
            if(ret < 0) break;
            packet0 = packet;
        }
    
        if(packet.stream_index == fmt->audio_stream_index) {
            got_frame = 0;
            ret = avcodec_decode_audio4(fmt->decoder_ctx, frame, &got_frame, &packet);
            
            if(ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding audio\n");
                continue;
            }

            packet.size -= ret;
            packet.data += ret;

            if(got_frame) {

                /* push audio from decoded frame through filter graph */
                if(av_buffersrc_add_frame_flags(filter->abuffer_ctx, frame, 0) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error feeding into filter graph\n");
                    break;
                }
                
                while(1) {
                
                    // This is where you will work with each processed frame. 
                    
                    ret = av_buffersink_get_frame(filter->abuffersink_ctx_map["FR"], filt_frame); 
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    //std::cout << "FR";
                    
                    av_init_packet(&pkt);
                    pkt.data = NULL;
                    pkt.size = 0;
                    
                    ret = avcodec_encode_audio2(c, &pkt, filt_frame, &got_output);
                    if(ret < 0) exit(1);
                    if(got_output) {
                        fwrite(pkt.data, 1, pkt.size, f);
                        av_free_packet(&pkt);
                    }    
                                      
                    av_frame_unref(filt_frame);
                    
                    ret = av_buffersink_get_frame(filter->abuffersink_ctx_map["FL"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
     
                   
                    

                    
                    //std::cout << "FL";
                    av_frame_unref(filt_frame);

                    ret = av_buffersink_get_frame(filter->abuffersink_ctx_map["FC"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    //std::cout << "FC";
                    
                    
                    av_frame_unref(filt_frame);
                    
                    ret = av_buffersink_get_frame(filter->abuffersink_ctx_map["LFE"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    
                    //std::cout << "LFE";
                    av_frame_unref(filt_frame);
                    
                    ret = av_buffersink_get_frame(filter->abuffersink_ctx_map["BR"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    
                    
                    //std::cout << "BR";
                    av_frame_unref(filt_frame);
                    
                    ret = av_buffersink_get_frame(filter->abuffersink_ctx_map["BL"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    //std::cout << "BL\t";
                
                    if(ret < 0) {
                        av_frame_free(&frame);
                        av_frame_free(&filt_frame);
                        delete filter;
                        delete fmt;
                        break; 

                    }
                    //std::cout << "Preparing to print" << std::endl;
                    //print_frame(filt_frame);
                    av_frame_unref(filt_frame);
                }
            }
            if(packet.size <= 0) av_free_packet(&packet0);
        } else {
            av_free_packet(&packet0);
        }
    }
    fclose(f);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    delete filter;
    delete fmt;
}

int main(int argc, char *argv[]) {
    clock_t begin = clock(); 
    Format *format = new Format("sw.mp4");
    Filter *filter = new Filter(format);    
    SJoin  *sjoin  = new SJoin(format);
    clock_t end = clock();
    
    std::cout << "Filter initialization: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;

    begin = clock();
    process_filter_graph(format, filter, sjoin);    
    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0; 

}


