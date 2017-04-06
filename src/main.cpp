#include <iostream>
#include "input/filter.h"
#include "input/format.h"
#include "encoder/encoder.h"
#include "sjoin/sjoin.h"
//#include "sjoin/testjoin.h"
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
    FILE *fl;
    FILE *fr;
    FILE *fc;
    FILE *lfe;
    FILE *bl;
    FILE *br;
    FILE *vf;
    /*
    const char *fl_filename = "FL.aac";
    const char *fr_filename = "FR.aac";
    const char *fc_filename = "FC.aac";
    const char *lfe_filename = "LFE.aac";
    const char *bl_filename = "BL.aac";
    const char *br_filename= "BR.aac";
    */
    const char *vf_filename = "virtualize.aac";
    AVPacket packet, packet0;
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();
    AVFrame *comb_virt_frame = av_frame_alloc();
    int got_frame;


    std::unordered_map<Filter::Channel, Virtualizer*, std::hash<int>> c2v_;
    complete_sofa sofa_;


    AVPacket packet_out;
    AVPacket comb_packet_out;
    int got_output;

    Encoder *encoder = new Encoder(AV_CODEC_ID_AC3,
            fmt->decoder_ctx->bit_rate, AV_SAMPLE_FMT_FLTP);

    SJoin  *sjoin  = new SJoin(encoder);
    //TestJoin *testjoin = new TestJoin(encoder);
/*
    fl = fopen(fl_filename, "wb");
    fr = fopen(fr_filename, "wb");
    fc = fopen(fc_filename, "wb");
    lfe = fopen(lfe_filename, "wb");
    bl = fopen(bl_filename, "wb");
    br = fopen(br_filename, "wb");
    vf = fopen(vf_filename, "wb");
    if(!fl || !fr || !fc || !lfe || !bl || !br) {
        std::cout << "Error opening file" << std::endl;
    }
*/
    vf = fopen(vf_filename, "wb");
    if(!vf) {
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

                int i ;
            
                while(ret >= 0) {
                    // This is where you will work with each processed frame.

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
                                Virtualizer * virt = new Virtualizer(sofa_file_name.c_str(), 
                                        sample_rate, x, y, z);
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

                        float ** float_results = c2v_[it->first]->process(samples, sample_count);

                        uint8_t * result_l = Virtualizer::get_short_samples(float_results[0],
                                fmt->decoder_ctx->sample_fmt, sample_count);

                        uint8_t * result_r = Virtualizer::get_short_samples(float_results[1],
                                fmt->decoder_ctx->sample_fmt, sample_count);

                        delete[] float_results[0];
                        delete[] float_results[1];
                        delete[] float_results;
                        delete[] samples;

                        AVFrame *virt_frame = encoder->new_frame(encoder->codec_ctx, result_r,
                                result_l);
                        
                        virt_frame->format = AV_SAMPLE_FMT_FLTP;
                        virt_frame->sample_rate = 48000;
                        virt_frame->channel_layout = 3;


                        av_log(NULL, AV_LOG_INFO, "%d ", i);
                        
                        if(av_buffersrc_add_frame_flags(sjoin->abuffers_ctx[i], virt_frame, 0) < 0)
                            av_log(NULL, AV_LOG_ERROR, "Error feeding into filtergraph\n");
                        //if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    
                             
                       
                        // TODO: do something with the result

                        //std::cout << "FR";
                        av_frame_unref(virt_frame);
                        av_frame_unref(filt_frame);
                        //av_frame_unref(r_frame);
                        i++;
                    }
                    
                    av_log(NULL, AV_LOG_INFO, "Try get frame! \n");
                    ret = -1;
                    ret = av_buffersink_get_frame(sjoin->abuffersink_ctx, comb_virt_frame);
                    if(ret < 0) {
                        av_log(NULL, AV_LOG_ERROR, "No virtualization frame %d\n", ret);
                        continue;
                    }
                    av_init_packet(&comb_packet_out);
                    comb_packet_out.data = NULL;
                    comb_packet_out.size = 0;
                   
                    ret = avcodec_encode_audio2(encoder->codec_ctx, &comb_packet_out, comb_virt_frame, &got_output);
                    if(ret < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Error encoding comb frame %d\n", ret);  
                        exit(1);
                    }                   
                    
                    fwrite(comb_packet_out.data, 1, comb_packet_out.size, vf);
                    
                    av_free_packet(&comb_packet_out);
                    av_frame_unref(comb_virt_frame);
                }
            }
            if(packet.size <= 0) av_free_packet(&packet0);
        } else {
            av_free_packet(&packet0);
        }
    }
    /*
    fclose(fl);
    fclose(fr);
    fclose(fc);
    fclose(lfe);
    fclose(br);
    fclose(bl);*/
    fclose(vf); 
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    av_frame_free(&comb_virt_frame);
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
