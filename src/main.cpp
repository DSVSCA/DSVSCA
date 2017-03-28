#include <iostream>
#include "input/filter.h"

int process_filter_graph(Filter *in) {
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
            ret = av_read_frame(in->format_ctx, &packet);
            if(ret < 0) break;
            packet0 = packet;
        }
    
        if(packet.stream_index == in->audio_stream_index) {
            got_frame = 0;
            ret = avcodec_decode_audio4(in->decoder_ctx, frame, &got_frame, &packet);
            
            if(ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding audio\n");
                continue;
            }

            packet.size -= ret;
            packet.data += ret;
            
            std::cout << ".";
            if(got_frame) {
                /* push audio from decoded frame through filter graph */
                if(av_buffersrc_add_frame_flags(in->abuffer_ctx, frame, 0) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error feeding into filter graph\n");
                    break;
                }

                while(1) {
                    // This is where you will work with each processed frame. 
                    
                    ret = av_buffersink_get_frame(in->abuffersink_ctx_map["FR"], filt_frame); 
                    
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    std::cout << "FR"; 
                    av_frame_unref(filt_frame);
                    ret = av_buffersink_get_frame(in->abuffersink_ctx_map["FL"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    std::cout << "FL";
                    av_frame_unref(filt_frame);
                    ret = av_buffersink_get_frame(in->abuffersink_ctx_map["FC"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    std::cout << "FC";
                    av_frame_unref(filt_frame);
                    ret = av_buffersink_get_frame(in->abuffersink_ctx_map["LFE"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    std::cout << "LFE";
                    av_frame_unref(filt_frame);
                    ret = av_buffersink_get_frame(in->abuffersink_ctx_map["BR"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    std::cout << "BR";
                    av_frame_unref(filt_frame);
                    ret = av_buffersink_get_frame(in->abuffersink_ctx_map["BL"], filt_frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    std::cout << "BL\t";
                    if(ret < 0) {
                        av_frame_free(&frame);
                        av_frame_free(&filt_frame);
                        delete in;
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
    delete in;
}

int main(int argc, char *argv[]) {
    Filter *in = new Filter("sw.mp4", true);    
    process_filter_graph(in);    
    return 0; 
}


