#include <iostream>
#include "input/filter.h"
#include "input/format.h"
#include "sjoin/sjoin.h"
#include "virtualizer/virtualizer.h"
#include <ctime>

int process_filter_graph(Format *fmt, Filter *filter, std::string sofa_file_name) {
    AVPacket packet, packet0;
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();

    std::unordered_map<Filter::Channel, Virtualizer*, std::hash<int>> c2v_;
    complete_sofa sofa_;
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

                    for (auto it = filter->abuffersink_ctx_map.begin(); it != filter->abuffersink_ctx_map.end(); it++) {
                        ret = av_buffersink_get_frame(it->second, filt_frame);
                        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;

                        uint8_t sample_count = filt_frame->nb_samples;
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

                        float * samples = Virtualizer::get_float_samples(filt_frame->extended_data[0], fmt->decoder_ctx->sample_fmt, sample_count);

                        float ** float_results = c2v_[it->first]->process(samples, sample_count);

                        uint8_t * result_l = Virtualizer::get_short_samples(float_results[0], fmt->decoder_ctx->sample_fmt, sample_count);
                        uint8_t * result_r = Virtualizer::get_short_samples(float_results[1], fmt->decoder_ctx->sample_fmt, sample_count);

                        // TODO: do something with the result

                        delete[] float_results[0];
                        delete[] float_results[1];
                        delete[] float_results;
                        delete[] samples;

                        //std::cout << "FR";
                        av_frame_unref(filt_frame);
                    }

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
    std::string videoFile = std::string(argv[1]);
    std::string sofaFile = std::string(argv[2]);

    clock_t begin = clock();
    Format *format = new Format("sw.mp4");
    Filter *filter = new Filter(format);
    SJoin  *sjoin  = new SJoin(format);
    clock_t end = clock();

    std::cout << "Filter initialization: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;

    begin = clock();
    process_filter_graph(format, filter, sofaFile);
    end = clock();

    std::cout << "Processing Time: " << (double)(end - begin) / CLOCKS_PER_SEC << " s" << std::endl;
    return 0;
}
