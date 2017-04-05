#include <iostream>
#include "input/filter.h"
#include "input/format.h"
#include "encoder/encoder.h"
//#include "sjoin/sjoin.h"
#include "sjoin/testjoin.h"
#include "virtualizer/virtualizer.h"
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

int process_filter_graph(Format *fmt, Filter *filter, std::string sofa_file_name) {
    FILE *f;
    const char *filename = "FL.aac";
    
    AVPacket packet, packet0;
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();
    int got_frame;

  
    std::unordered_map<Filter::Channel, Virtualizer*, std::hash<int>> c2v_;
    complete_sofa sofa_;

    
    AVPacket packet_out;
    int got_output;

    Encoder *encoder = new Encoder(AV_CODEC_ID_AC3,
            fmt->decoder_ctx->bit_rate, AV_SAMPLE_FMT_FLTP);

    encoder->codec_ctx->channel_layout = 2;

    TestJoin *testjoin  = new TestJoin(encoder);
   
    BufferSourceContext *s = (BufferSourceContext*)testjoin->abuffer_ctx->priv;

    f = fopen(filename, "wb");
    if(!f) {
        std::cout << "Error opening file" << std::endl;
    }
   
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
                
                while(ret >= 0) {
                    // This is where you will work with each processed frame.
                    
                    int i;
                    
                    i = 0;
                    for (auto it = filter->abuffersink_ctx_map.begin(); 
                            it != filter->abuffersink_ctx_map.end(); it++) {
                   
                        ret = av_buffersink_get_frame(it->second, filt_frame);
                        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                        

                        int sample_count = filt_frame->nb_samples;
                        int sample_rate = filt_frame->sample_rate;

                        if (c2v_.count(it->first) == 0) {
                            float x;
                            float y;
                            float z;
                            Filter::get_coords(it->first, &x, &y, &z);

                            if (sofa_.hrtf == NULL) {
                                //TODO: delete these after execution
                                Virtualizer * virt = new Virtualizer(sofa_file_name.c_str(), sample_rate, x, y, z);
                                c2v_.insert(std::make_pair(it->first, virt));
                                sofa_ = virt->get_hrtf();
                            }
                            else {
                                //TODO: delete these after execution
                                Virtualizer * virt = new Virtualizer(sofa_, sample_rate, x, y, z);
                                c2v_.insert(std::make_pair(it->first, virt));
                            }
                        }

                        float * samples = Virtualizer::get_float_samples(filt_frame->extended_data[0], 
                                fmt->decoder_ctx->sample_fmt, sample_count);

                       // float ** float_results = c2v_[it->first]->process(samples, sample_count);
                        
                        uint8_t * result_l = Virtualizer::get_short_samples(samples, 
                                fmt->decoder_ctx->sample_fmt, sample_count);
                        
                        uint8_t * result_r = Virtualizer::get_short_samples(samples, 
                                fmt->decoder_ctx->sample_fmt, sample_count);
                   
                        AVFrame *virt_frame = encoder->new_frame(encoder->codec_ctx, filt_frame->extended_data[1],
                                filt_frame->extended_data[0]);

                        //memcpy(filt_frame->extended_data[1], filt_frame->extended_data[0],
                        //        av_get_bytes_per_sample(encoder->codec_ctx->sample_fmt) * sample_count);
                       
                        // memcpy(filt_frame->extended_data[0], result_r, 
                       //         av_get_bytes_per_sample(encoder->codec_ctx->sample_fmt) * sample_count);
                        
                        virt_frame->format = AV_SAMPLE_FMT_FLTP;
                        virt_frame->sample_rate = 48000;
                        virt_frame->channel_layout = 3;
                        //s->channel_layout = (uint64_t)3;
                        //s->channels = 2;
                        memcpy((BufferSourceContext*)testjoin->abuffer_ctx->priv, s, sizeof(BufferSourceContext));
                        
                        BufferSourceContext *d = (BufferSourceContext*)testjoin->abuffer_ctx->priv;
                        int nb_channels = av_frame_get_channels(virt_frame);
                        int layout = av_get_channel_layout_nb_channels(virt_frame->channel_layout);
                        if(av_buffersrc_add_frame_flags(testjoin->abuffer_ctx, virt_frame, 0) < 0) {
                            av_log(NULL, AV_LOG_ERROR, "Error feeding into filter graph\n");
                        }
                        
                       
                        AVFrame *test_virt_frame = av_frame_alloc();
                        
                        ret = av_buffersink_get_frame(testjoin->abuffersink_ctx, test_virt_frame);
                        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
    
                        if(it->first == 0) { 
                           //std::cout << l_frame->nb_samples << std::endl;
                           //std::cout << l_frame->format << std::endl;
                           // This will move once virtualizaiton works
                        av_init_packet(&packet_out);
                        packet_out.data = NULL;
                        packet_out.size = 0;
                    

                        ret = avcodec_encode_audio2(encoder->codec_ctx, &packet_out, test_virt_frame, &got_output);
                        if(ret < 0) exit(1);
                        if(got_output) {
                          fwrite(packet_out.data, 1, packet_out.size, f);
                          av_free_packet(&packet_out);
                        }   
                       }
                    
                       
                        //AVFrame *r_frame = encoder->fill_new_frame(result_r, 2);
                        
                        /*
                        if(av_buffersrc_add_frame_flags(sjoin->right_abuffers_ctx[i], r_frame, 0) < 0) {
                            av_log(NULL, AV_LOG_ERROR, "Error feeding into filter graph\n");
                            break;
                        }
                        */
                        // TODO: do something with the result

                       // delete[] float_results[0];
                       // delete[] float_results[1];
                       // delete[] float_results;
                        delete[] samples;
                        
                        //std::cout << "FR";
                        av_frame_unref(filt_frame);
                        //av_frame_unref(r_frame);
                        i++;
                    }
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
    std::string videoFile = std::string(argv[1]);
    std::string sofaFile = std::string(argv[2]);

    clock_t begin = clock();
    Format *format = new Format(videoFile);
    Filter *filter = new Filter(format);
    clock_t end = clock();

    std::cout << "Filter initialization: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;

    begin = clock();
    process_filter_graph(format, filter, sofaFile);
    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0;
}
