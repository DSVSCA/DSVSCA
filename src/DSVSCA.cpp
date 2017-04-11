#include "DSVSCA.h"

int DSVSCA::process_filter_graph(process_info info) {
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
            info.format->decoder_ctx->bit_rate, AV_SAMPLE_FMT_FLTP);

    SJoin  *sjoin  = new SJoin(encoder);
    
    long total_duration = info.format->format_ctx->duration / (long)AV_TIME_BASE;
    uint64_t total_sample_count = 0;
    uint64_t samples_completed = 0;

    int ret = 0;

    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ofmt_ctx = NULL;

    size_t index_of_ext = info.video_file_name.find_last_of('.');
    std::string out_filename_str;
    if (index_of_ext == std::string::npos) out_filename_str = info.video_file_name + "-virtualized";
    else out_filename_str = info.video_file_name.substr(0, index_of_ext) + "-virtualized" + info.video_file_name.substr(index_of_ext);
    const char *out_filename = out_filename_str.c_str();

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if(!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context!\n");
        exit(1);
    }

    ofmt = ofmt_ctx->oformat;

    for(int i = 0; i < info.format->format_ctx->nb_streams; i++) {
        AVStream *in_stream = info.format->format_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
        if(!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Failed to allocate output stream!\n");
            exit(1);
        }
        
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        out_stream->codec->codec_tag = 0;
        if(ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to copy context from input to output stream codec context\n");
            exit(1);
        }
        
        if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    av_dump_format(ofmt_ctx, 0, out_filename, 1);
     
    if(!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if(ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Unable to open output file\n");
            exit(1);
        }
    }
    
    ret = avformat_write_header(ofmt_ctx, NULL);
    if(ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error opening file to write header\n");
        exit(1);
    }

    /* Read all of the packets */
    packet0.data = NULL;
    packet.data = NULL;
    while(1) {
        if(!packet0.data) {
            ret = av_read_frame(info.format->format_ctx, &packet);
            if(ret < 0) break;
            packet0 = packet;
        }
        
        //in_stream = ifmt_ctx->streams[packet.stream_index];
        //out_stream = ofmt_ctx->streams[packet.stream_index];
        
        if(packet.stream_index == 0) {
            ret = av_interleaved_write_frame(ofmt_ctx, &packet0);
            if(ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error muxing video packet\n");
            }
        }

        if(packet.stream_index == info.format->audio_stream_index) {
            got_frame = 0;
            ret = avcodec_decode_audio4(info.format->decoder_ctx, frame, &got_frame, &packet);

            if(ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding audio\n");
                continue;
            }

            packet.size -= ret;
            packet.data += ret;

            if(got_frame) {

                /* push audio from decoded frame through filter graph */
                if(av_buffersrc_add_frame_flags(info.filter->abuffer_ctx, frame, 0) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error feeding into filter graph\n");
                    break;
                }

                int i ;
                int frame_sample_count = 0;
            
                while(ret >= 0) {
                    // This is where you will work with each processed frame.

                    i = 0;

                    for (auto it = info.filter->abuffersink_ctx_map.begin();
                            it != info.filter->abuffersink_ctx_map.end(); it++) {
                        ret = av_buffersink_get_frame(it->second, filt_frame);
                        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;

                        int sample_count = filt_frame->nb_samples;
                        int sample_rate = filt_frame->sample_rate;
                        if (total_sample_count == 0) total_sample_count = total_duration * sample_rate;
                        if (frame_sample_count == 0) frame_sample_count = sample_count;

                        if (c2v_.count(it->first) == 0) {
                            float x_y_z[3];
                            if (info.coords.count(it->first) == 0) Filter::get_coords(it->first, &x_y_z[0], &x_y_z[1], &x_y_z[2]);
                            else {
                                x_y_z[0] = info.coords.at(it->first).x;
                                x_y_z[1] = info.coords.at(it->first).y;
                                x_y_z[2] = info.coords.at(it->first).z;

                                if (info.coord_type == Filter::Spherical) mysofa_s2c(x_y_z);
                            }

                            if (sofa_.hrtf == NULL) {
                                Virtualizer * virt = new Virtualizer(info.sofa_file_name.c_str(), 
                                        sample_rate, x_y_z[0], x_y_z[1], x_y_z[2], info.block_size);
                                c2v_.insert(std::make_pair(it->first, virt));
                                sofa_ = virt->get_hrtf();
                            }
                            else {
                                Virtualizer * virt = new Virtualizer(sofa_, sample_rate, 
                                        x_y_z[0], x_y_z[1], x_y_z[2], info.block_size);
                                c2v_.insert(std::make_pair(it->first, virt));
                            }
                        }

                        float * samples = Virtualizer::get_float_samples(filt_frame->extended_data[0],
                                info.format->decoder_ctx->sample_fmt, sample_count);

                        float ** float_results = c2v_[it->first]->process(samples, sample_count);

                        uint8_t * result_l = Virtualizer::get_short_samples(float_results[0],
                                info.format->decoder_ctx->sample_fmt, sample_count);

                        uint8_t * result_r = Virtualizer::get_short_samples(float_results[1],
                                info.format->decoder_ctx->sample_fmt, sample_count);

                        delete[] float_results[0];
                        delete[] float_results[1];
                        delete[] float_results;
                        delete[] samples;

                        AVFrame *virt_frame = encoder->new_frame(encoder->codec_ctx, result_r,
                                result_l);
                        
                        virt_frame->format = AV_SAMPLE_FMT_FLTP;
                        virt_frame->sample_rate = 48000;
                        virt_frame->channel_layout = 3;

                        //av_log(NULL, AV_LOG_INFO, "%d ", i);
                        
                        if(av_buffersrc_add_frame_flags(sjoin->abuffers_ctx[i], virt_frame, 0) < 0)
                            av_log(NULL, AV_LOG_ERROR, "Error feeding into filtergraph\n");
                    

                        av_frame_unref(filt_frame);
                        i++;
                    }
                    
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    ret = av_buffersink_get_frame(sjoin->abuffersink_ctx, comb_virt_frame);
                    
                    if(ret < 0) {
                        av_log(NULL, AV_LOG_ERROR, "No virtualization frame %d\n", ret);
                        continue;
                    }
                    av_init_packet(&comb_packet_out);
                    comb_packet_out.data = NULL;
                    comb_packet_out.size = 0;
                  
                    ret = avcodec_encode_audio2(encoder->codec_ctx, &comb_packet_out, 
                            comb_virt_frame, &got_output);
                    if(ret < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Error encoding comb frame %d\n", ret);  
                        exit(1);
                    }                   
                
                    uint8_t* data = comb_packet_out.data;

                    av_copy_packet(&comb_packet_out, &packet0);
                    
                    comb_packet_out.data = data;
                    ret = av_interleaved_write_frame(ofmt_ctx, &comb_packet_out);
                    if(ret < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Error muxing video packet\n");
                    }
                    av_free_packet(&comb_packet_out);
                    av_frame_unref(comb_virt_frame);
                
                }

                samples_completed += frame_sample_count;
            }
            if(packet.size <= 0) av_free_packet(&packet0);
        } else {
            av_free_packet(&packet0);
        }

        if (total_sample_count != 0) {
            int completion = (100 * samples_completed) / total_sample_count;
            if (completion > 100) completion = 100;
            info.progress->store(completion);
        }
    }

    for (auto it = c2v_.begin(); it != c2v_.end(); it++) delete it->second;
    
    av_write_trailer(ofmt_ctx);
    avformat_close_input(&info.format->format_ctx);
    if(ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    av_frame_free(&comb_virt_frame);

    if(ret < 0 && ret != AVERROR_EOF) {
        av_log(NULL, AV_LOG_ERROR, "Error occured while closing out: %d\n", ret);
        return ret;
    }
}
