#include <iostream>
#include "input/filter.h"
#include "input/format.h"
#include "sjoin/sjoin.h"
#include <ctime>

int process_filter_graph(Format *fmt, Filter *filter) {
    AVPacket packet, packet0;
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();
    
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
    process_filter_graph(format, filter);    
    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0; 
}


