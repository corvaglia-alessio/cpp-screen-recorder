#include "../include/Recorder.h"

using namespace std;

Recorder::Recorder(){
    audio_on = false;
    audio_in_fmt_ctx = nullptr;
    audio_in_stream = nullptr;
    audio_in_codec_ctx = nullptr;
    audio_converter = nullptr;
    audio_fifo = nullptr;
    audio_out_stream = nullptr;
    audio_out_codec_ctx = nullptr;
    audio_tmp_pkt = nullptr;
    crop = false;
    c_left = c_right = c_top = c_bottom = 0;
    video_in_fmt_ctx = nullptr;
    video_in_stream = nullptr;
    video_in_codec_ctx = nullptr;
    video_scaler = nullptr;
    video_out_buf = nullptr;
    video_out_stream = nullptr;
    video_out_codec_ctx = nullptr;
    video_tmp_pkt = nullptr;
    crop_video_filter_graph = nullptr;
    crop_video_inputs = nullptr;
    crop_video_outputs = nullptr;
    crop_video_buffersink_ctx = nullptr;
    crop_video_buffersrc_ctx = nullptr;
    video_tmp_frame = nullptr;
    out_fmt_ctx = nullptr;
    video_next_pts = audio_next_pts = 0;
    video_turn = 1;
    audio_turn = 0;
    fps = 0;
    status = STOP;
    ret = 0;

    avdevice_register_all();
}

Recorder::~Recorder() {
    if(audio_on){
        destroy_audio_input();
        destroy_audio_output();
    }
    if(crop)
        destroy_cropper();
    destroy_video();
    avformat_close_input(&out_fmt_ctx);
    avformat_free_context(out_fmt_ctx);
    out_fmt_ctx = nullptr;
}

void Recorder::open_audio_input(){
    audio_in_fmt_ctx = avformat_alloc_context();
    if(!audio_in_fmt_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate audio input video_format context");

    string audio_format, audio_device_name;
    #ifdef WINDOWS
    audio_format = "dshow";
    #endif
    #ifdef MACOS
    audio_format = "avfoundation";
    #endif
    #ifdef UNIX
    audio_format = "alsa";
    #endif

    AVInputFormat* audio_in_fmt = av_find_input_format(audio_format.c_str());
    if(!audio_in_fmt)
        throw runtime_error("[RUNTIME_ERROR] cannot find the audio input video_format");

    #ifdef WINDOWS
    audio_device_name = "audio=" + audio_in_device;
    #endif
    #ifdef MACOS
    audio_device_name = "none:" + audio_in_device;
    #endif
    #ifdef UNIX
    audio_device_name = audio_in_device;
    #endif

    #ifdef MACOS
    int count = 0;
    while(count < 100){ //in MACOS the device may need more than one try in order to be detected
    #endif

    ret = avformat_open_input(&audio_in_fmt_ctx, audio_device_name.c_str(), audio_in_fmt, nullptr);

    #ifdef MACOS
        if(ret<0)
            count++;
        else
            break;
    }
    #endif

    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot open the audio input, invalid index/name or busy device");

    ret = avformat_find_stream_info(audio_in_fmt_ctx, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot find the audio stream info ");

    for(int i=0; i < audio_in_fmt_ctx->nb_streams; i++){
        if(audio_in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_in_stream = audio_in_fmt_ctx->streams[i];
            break;
        }
    }
    if(!audio_in_stream)
        throw runtime_error("[RUNTIME_ERROR] no audio stream found");


    AVCodec* audio_in_codec = avcodec_find_decoder(audio_in_stream->codecpar->codec_id);
    if(!audio_in_codec)
        throw runtime_error("[RUNTIME_ERROR] audio decoder not found");

    audio_in_codec_ctx = avcodec_alloc_context3(audio_in_codec);
    if(!audio_in_codec_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate the audio input codec context");

    ret = avcodec_parameters_to_context(audio_in_codec_ctx, audio_in_stream->codecpar);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot create audio codec context from parameters");

    ret = avcodec_open2(audio_in_codec_ctx, audio_in_codec, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot open audio decoder");
}

void Recorder::open_audio_output() {
    AVCodec* audio_out_codec = avcodec_find_encoder(AUDIO_ENCODER);
    if(!audio_out_codec)
        throw runtime_error("[RUNTIME_ERROR] cannot find the audio encoder");

    audio_tmp_pkt = av_packet_alloc();
    if(!audio_tmp_pkt)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate audio packet");

    audio_out_stream = avformat_new_stream(out_fmt_ctx, nullptr);
    if(!audio_out_stream)
        throw runtime_error("[RUNTIME_ERROR] cannot create the audio output stream");

    audio_out_stream->id = (int) out_fmt_ctx->nb_streams - 1;

    audio_out_codec_ctx = avcodec_alloc_context3(audio_out_codec);
    if(!audio_out_codec_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate the audio output video_format context");

    audio_out_codec_ctx->sample_fmt = audio_out_codec->sample_fmts[0];
    audio_out_codec_ctx->bit_rate = 128000;
    audio_out_codec_ctx->sample_rate = audio_in_stream->codecpar->sample_rate;
    audio_out_codec_ctx->channels = audio_in_stream->codecpar->channels;
    audio_out_codec_ctx->channel_layout = av_get_default_channel_layout(audio_in_stream->codecpar->channels);

    audio_out_stream->time_base.num = 1;
    audio_out_stream->time_base.den = audio_out_codec_ctx->sample_rate;

    if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        audio_out_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    ret = avcodec_open2(audio_out_codec_ctx, audio_out_codec, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot open the audio encoder");

    ret = avcodec_parameters_from_context(audio_out_stream->codecpar, audio_out_codec_ctx);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot create stream parameters from audio output context");

    audio_converter = swr_alloc_set_opts(nullptr, av_get_default_channel_layout(audio_in_codec_ctx->channels), AV_SAMPLE_FMT_FLTP, audio_in_codec_ctx->sample_rate,
                                         av_get_default_channel_layout(audio_in_codec_ctx->channels), (AVSampleFormat)audio_in_stream->codecpar->format, audio_in_stream->codecpar->sample_rate, 0,
                                         nullptr );
    if(!audio_converter)
        throw runtime_error("[RUNTIME_ERROR] cannot configure audio converter");

    ret = swr_init(audio_converter);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot init audio converter");

    audio_fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLTP, audio_in_codec_ctx->channels, audio_in_codec_ctx->sample_rate * 2);
    if(!audio_fifo)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate the audio fifo queue");
}

vector<int> Recorder::recorder_open_inputs(const string& video_input, int frames_per_second, string audio_input) {

    if(frames_per_second <= 0 || frames_per_second > 30)
        throw invalid_argument("[INVALID ARGUMENT] frames_per_second must be in the range [1, 30]");
    fps = frames_per_second;

    if(!audio_input.empty()){
        audio_in_device = std::move(audio_input);
        audio_on = true;
        audio_turn = 1;
    }

    video_in_fmt_ctx = avformat_alloc_context();
    if(!video_in_fmt_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate video input video_format context");

    string video_format, video_device_name;

    #ifdef WINDOWS
    video_format = "gdigrab";
    #endif
    #ifdef MACOS
    video_format = "avfoundation";
    #endif
    #ifdef UNIX
    video_format = "x11grab";
    #endif

    AVInputFormat* video_in_fmt = av_find_input_format(video_format.c_str());
    if(!video_in_fmt)
        throw runtime_error("[RUNTIME_ERROR] cannot find the video input video_format");

    #ifdef WINDOWS
    video_device_name = video_input;
    #endif
    #ifdef MACOS
    video_device_name = video_input + ":none";
    #endif
    #ifdef UNIX
    video_device_name = video_input;
    #endif

    AVDictionary* opts = nullptr;
    ret = av_dict_set(&opts, "framerate", to_string(frames_per_second).c_str(), 0);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot set options");

    ret = av_dict_set(&opts, "probesize", "50M", 0);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot set options");
    
    #ifdef MACOS
        ret = av_dict_set(&opts, "pixel_format", "0rgb", 0);
        if(ret<0)
            throw runtime_error("[RUNTIME_ERROR] cannot set options");
    
        ret = av_dict_set(&opts, "capture_cursor", "1", 0);
        if(ret<0)
            throw runtime_error("[RUNTIME_ERROR] cannot set options");
    #endif

    ret = avformat_open_input(&video_in_fmt_ctx, video_device_name.c_str(), video_in_fmt, &opts);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot open the video input");

    ret = avformat_find_stream_info(video_in_fmt_ctx, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot find the video stream info ");

    for(int i=0; i < video_in_fmt_ctx->nb_streams; i++){
        if(video_in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_in_stream = video_in_fmt_ctx->streams[i];
            break;
        }
    }
    if(!video_in_stream)
        throw runtime_error("[RUNTIME_ERROR] no video stream found");

    video_in_codec_ctx = avcodec_alloc_context3(nullptr);
    if(!video_in_codec_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate the video input codec context");

    ret = avcodec_parameters_to_context(video_in_codec_ctx, video_in_stream->codecpar);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot create video codec context from parameters");

    AVCodec* video_in_codec = avcodec_find_decoder(video_in_codec_ctx->codec_id);
    if(!video_in_codec)
        throw runtime_error("[RUNTIME_ERROR] video decoder not found");

    ret = avcodec_open2(video_in_codec_ctx, video_in_codec, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot open video decoder");

    if(audio_on){
        open_audio_input();
    }

    av_dict_free(&opts);

    vector<int> v;
    v.push_back(video_in_codec_ctx->width);
    v.push_back(video_in_codec_ctx->height);
    return v;
}

void Recorder::recorder_crop_video(int left, int right, int top, int bottom) {
    if(left<0 || right<0 || top<0 || bottom<0)
        throw invalid_argument("Crop values must be valid");
    if(left+right >= video_in_codec_ctx->width)
        throw invalid_argument("Crop values must be valid");
    if(bottom+top >= video_in_codec_ctx->height)
        throw invalid_argument("Crop values must be valid");

    this->c_left = left;
    this->c_right = right;
    this->c_top = top;
    this->c_bottom = bottom;
    crop = true;

    crop_video_filter_graph = avfilter_graph_alloc();
    crop_video_inputs = avfilter_inout_alloc();
    crop_video_outputs = avfilter_inout_alloc();
    if(!crop_video_filter_graph || !crop_video_inputs || !crop_video_outputs)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate structures for video cropping");

    char args[512];
    ret = snprintf(args, sizeof(args), "buffer=video_size=%dx%d:pix_fmt=%d:time_base=1/1:pixel_aspect=0/1[in];"
                                 "[in]crop=x=%d:y=%d:out_w=in_w-%d-%d:out_h=in_h-%d-%d[out];"
                                 "[out]buffersink", video_in_codec_ctx->width, video_in_codec_ctx->height, video_in_codec_ctx->pix_fmt, c_left, c_top, c_left, c_right, c_top, c_bottom);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot initalize crop structures");

    ret = avfilter_graph_parse2(crop_video_filter_graph, args, &crop_video_inputs, &crop_video_outputs);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot parse crop graph filter");

    ret = avfilter_graph_config(crop_video_filter_graph, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot configure crop graph filter");

    crop_video_buffersrc_ctx = avfilter_graph_get_filter(crop_video_filter_graph, "Parsed_buffer_0");
    if(!crop_video_buffersrc_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot get buffer src of the crop filter");

    crop_video_buffersink_ctx = avfilter_graph_get_filter(crop_video_filter_graph, "Parsed_buffersink_2");
    if(!crop_video_buffersink_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot get buffer sink of the crop filter");
}

void Recorder::recorder_init_output(const string& path_name){

    string s;
    if(!path_name.empty())
        s = path_name + ".mp4";
    else
        throw invalid_argument("[INVALID ARGUMENT] the path_name is empty");

    ret = avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, s.c_str());
    if(ret<0 || !out_fmt_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate the output video_format context");

    AVOutputFormat* out_fmt = out_fmt_ctx->oformat;

    AVCodec* video_out_codec = avcodec_find_encoder(VIDEO_ENCODER);
    if(!video_out_codec)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate the video encoder");

    video_tmp_pkt = av_packet_alloc();
    if(!video_tmp_pkt)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate video packet");

    video_out_stream = avformat_new_stream(out_fmt_ctx, video_out_codec);
    if(!video_out_stream)
        throw runtime_error("[RUNTIME_ERROR] cannot create the video output stream");

    video_out_stream->id = (int) out_fmt_ctx->nb_streams - 1;

    video_out_codec_ctx = avcodec_alloc_context3(video_out_codec);
    if(!video_out_codec_ctx)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate the video output video_format context");

    video_out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    video_out_codec_ctx->codec_id = video_out_codec->id;
    video_out_codec_ctx->bit_rate = 5000000;
    video_out_codec_ctx->width = video_in_codec_ctx->width - c_left - c_right;
    video_out_codec_ctx->height = video_in_codec_ctx->height - c_top - c_bottom;
    video_out_codec_ctx->max_b_frames = 0;
    video_out_codec_ctx->pix_fmt = PIX_FMT;

    video_out_codec_ctx->time_base.num = 1;
    video_out_codec_ctx->time_base.den = fps;

    if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        video_out_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    ret = avcodec_open2(video_out_codec_ctx, video_out_codec, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot open the video encoder");

    video_tmp_frame = av_frame_alloc();
    if(!video_tmp_frame)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate video frame");

    video_tmp_frame->format = PIX_FMT;
    video_tmp_frame->width = video_in_codec_ctx->width - c_left - c_right;
    video_tmp_frame->height = video_in_codec_ctx->height - c_bottom - c_top;

    ret = av_frame_get_buffer(video_tmp_frame, 0);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot get video frame buffers");

    ret = avcodec_parameters_from_context(video_out_stream->codecpar, video_out_codec_ctx);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot create stream parameters from video output context");

    int nb = av_image_get_buffer_size(video_out_codec_ctx->pix_fmt, video_out_codec_ctx->width, video_out_codec_ctx->height, 32);
    if(nb<0)
        throw runtime_error("[RUNTIME_ERROR] cannot get size of the video buffer for the scaler");

    video_out_buf = (uint8_t*) av_malloc(nb);
    if(!video_out_buf)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate video buffer for the scaler");

    ret = av_image_fill_arrays(video_tmp_frame->data, video_tmp_frame->linesize, video_out_buf, PIX_FMT, video_out_codec_ctx->width, video_out_codec_ctx->height, 1);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot configure the video scaler");

    video_scaler = sws_getContext(video_out_codec_ctx->width, video_out_codec_ctx->height, video_in_codec_ctx->pix_fmt, video_out_codec_ctx->width, video_out_codec_ctx->height, video_out_codec_ctx->pix_fmt, SWS_BICUBIC,
                                  nullptr, nullptr, nullptr);

    if(!video_scaler)
        throw runtime_error("[RUNTIME_ERROR] cannot obtain the video scaler");

    if(audio_on){
        open_audio_output();
    }

    if(!(out_fmt->flags & AVFMT_NOFILE)){
        avio_open(&out_fmt_ctx->pb, s.c_str(), AVIO_FLAG_WRITE);
    }

    ret = avformat_write_header(out_fmt_ctx, nullptr);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot  write the output file header");
}

void Recorder::record_loop() {
    while(true){
        unique_lock<mutex> ul(status_mutex);
        try {
            while (status == PAUSE) {
                status_cv.wait(ul);
            }
            if (status == STOP) {
                break;
            }
            ul.unlock();
            if (audio_on) {
                if (video_turn && (!audio_turn ||
                                   av_compare_ts(video_next_pts, video_out_codec_ctx->time_base, audio_next_pts,
                                                 audio_out_codec_ctx->time_base) <= 0))
                    video_turn = !write_video_frame();
                else
                    audio_turn = !write_audio_frame();
            } else {
                write_video_frame();
            }
        }
        catch(exception &e){
            ul.lock();
            status = STOP;
            ul.unlock();
        }
    }
    write_file_trailer();
}

int Recorder::write_video_frame() {
    AVPacket* video_in_packet = av_packet_alloc();
    if(!video_in_packet)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate video input packet");

    AVFrame* video_in_frame = av_frame_alloc();
    if(!video_in_frame)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate video input frame");

    ret = av_read_frame(video_in_fmt_ctx, video_in_packet);
    if(ret<0)
        return 0;

    ret = avcodec_send_packet(video_in_codec_ctx, video_in_packet);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot send video packet to the decoder");

    ret = avcodec_receive_frame(video_in_codec_ctx, video_in_frame);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot receive video frame from the decoder");

    if(crop){
        ret = av_buffersrc_add_frame(crop_video_buffersrc_ctx, video_in_frame);
        if (ret<0)
            throw runtime_error("[RUNTIME_ERROR] cannot send the video frame to the cropper");

        ret = av_buffersink_get_frame(crop_video_buffersink_ctx, video_in_frame);
        if(ret<0)
            throw runtime_error("[RUNTIME_ERROR] cannot receive video frame from the cropper");
    }

    video_tmp_frame->pts = video_next_pts++;
    ret = avcodec_send_frame(video_out_codec_ctx, video_tmp_frame);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot send video frame to the encoder");

    ret = sws_scale(video_scaler, video_in_frame->data, video_in_frame->linesize, 0, video_out_codec_ctx->height, video_tmp_frame->data, video_tmp_frame->linesize);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot scale the video frame");

    ret = avcodec_receive_packet(video_out_codec_ctx, video_tmp_pkt);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot receive video packet from the encoder");

    av_frame_unref(video_in_frame);
    av_packet_unref(video_in_packet);

    av_packet_rescale_ts(video_tmp_pkt, video_out_codec_ctx->time_base, video_out_stream->time_base);

    video_tmp_pkt->stream_index = video_out_stream->index;

    ret = av_interleaved_write_frame(out_fmt_ctx, video_tmp_pkt);

    return ret < 0 ? 1 : 0;
}

int Recorder::write_audio_frame() {
    AVFrame* audio_in_frame = av_frame_alloc();
    if(!audio_in_frame)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate audio input frame");

    AVPacket* audio_in_packet = av_packet_alloc();
    if(!audio_in_packet)
        throw runtime_error("[RUNTIME_ERROR] cannot allocate audio input packet");

    ret = av_read_frame(audio_in_fmt_ctx, audio_in_packet);
    if(ret<0)
        return 0;
    
    ret = avcodec_send_packet(audio_in_codec_ctx, audio_in_packet);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot send audio packet to the decoder");

    ret = avcodec_receive_frame(audio_in_codec_ctx, audio_in_frame);
    if(ret==AVERROR(EINVAL))
        throw runtime_error("[RUNTIME_ERROR] cannot receive audio frame from the decoder");

    uint8_t** samples = nullptr;
    ret = av_samples_alloc_array_and_samples(&samples, nullptr, audio_out_codec_ctx->channels, audio_in_frame->nb_samples, AV_SAMPLE_FMT_FLTP, 0);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot alloc structure for the audio converter");

    ret = swr_convert(audio_converter, samples, audio_in_frame->nb_samples, (const uint8_t**)audio_in_frame->extended_data, audio_in_frame->nb_samples);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot convert audio");

    if(av_audio_fifo_space(audio_fifo) < audio_in_frame->nb_samples)
        throw runtime_error("[RUNTIME_ERROR] no space in the audio fifo queue");


    ret = av_audio_fifo_write(audio_fifo, (void**)samples, audio_in_frame->nb_samples);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot write in the audio fifo queue");

    av_freep(&samples[0]);
    av_frame_unref(audio_in_frame);
    av_packet_unref(audio_in_packet);

    while(av_audio_fifo_size(audio_fifo) >= audio_out_codec_ctx->frame_size) {
        int pts;

        AVFrame* audio_out_frame = av_frame_alloc();
        if(!audio_out_frame)
            throw runtime_error("[RUNTIME_ERROR] cannot allocate audio input frame");

        audio_out_frame->nb_samples = audio_out_codec_ctx->frame_size;
        audio_out_frame->channels = audio_in_codec_ctx->channels;
        audio_out_frame->channel_layout = av_get_default_channel_layout(audio_in_codec_ctx->channels);
        audio_out_frame->format = AV_SAMPLE_FMT_FLTP;
        audio_out_frame->sample_rate = audio_out_codec_ctx->sample_rate;

        ret = av_frame_get_buffer(audio_out_frame, 0);
        if(ret<0)
            throw runtime_error("[RUNTIME_ERROR] cannot get audio buffer");

        ret = av_audio_fifo_read(audio_fifo, (void**)audio_out_frame->data, audio_out_codec_ctx->frame_size);
        if(ret<0)
            throw runtime_error("[RUNTIME_ERROR] cannot read from the audio fifo queue");

        audio_out_frame->pts = audio_next_pts;
        pts = (int) audio_out_frame->pts;
        audio_next_pts += audio_out_frame->nb_samples;

        ret = avcodec_send_frame(audio_out_codec_ctx, audio_out_frame);
        if(ret<0)
            throw runtime_error("[RUNTIME_ERROR] cannot send audio frame to the encoder");

        av_frame_free(&audio_out_frame);

        ret = avcodec_receive_packet(audio_out_codec_ctx, audio_tmp_pkt);
        if(ret==AVERROR(EINVAL))
            throw runtime_error("[RUNTIME_ERROR] cannot receive audio packet from the encoder");

        audio_tmp_pkt->stream_index = audio_out_stream->index;
        audio_tmp_pkt->duration = audio_out_stream->time_base.den * 1024 / audio_out_codec_ctx->sample_rate;
        audio_tmp_pkt->pts = audio_tmp_pkt->dts = pts;

        ret = av_interleaved_write_frame(out_fmt_ctx, audio_tmp_pkt);
    }

    return ret < 0 ? 1 : 0;
}

void Recorder::recorder_stop_recording(){
    unique_lock<mutex> ul(status_mutex);
    if(status == RECORDING || status == PAUSE) {
        status = STOP;
        ul.unlock();
        status_cv.notify_one();
        recording_thread.join();
        destroy_audio_input();
    }
    else
        throw logic_error("[LOGIC_ERROR] already in STOP state");
}

void Recorder::recorder_start_recording() {
    lock_guard<mutex> lg(status_mutex);
    if(status == STOP){
        if(audio_on) {
            av_audio_fifo_reset(audio_fifo);
            destroy_audio_input();
            open_audio_input();
        }
        status = RECORDING;
        recording_thread = thread([this] { this->record_loop();});
    }
    else
        throw logic_error("[LOGIC_ERROR] not possible to start recording. Either because you are already recording or because you are in PAUSE state, in the latter case use recorder_resume_recording() instead");
}

void Recorder::recorder_pause_recording() {
    lock_guard<mutex> lg(status_mutex);
    if(status == RECORDING) {
        status = PAUSE;
    }
    else
        throw logic_error("[LOGIC_ERROR] cannot pause, not in RECORDING state.");
}

void Recorder::recorder_resume_recording() {
    lock_guard<mutex> lg(status_mutex);
    if(status == PAUSE) {
        status = RECORDING;
        if(audio_on) {
            av_audio_fifo_reset(audio_fifo);
            destroy_audio_input();
            open_audio_input();
        }
        status_cv.notify_one();
    }
    else
        throw logic_error("[LOGIC_ERROR] cannot resume, not in PAUSE state.");
}

void Recorder::destroy_audio_input() {
    avformat_close_input(&audio_in_fmt_ctx);
    avformat_free_context(audio_in_fmt_ctx);
    audio_in_fmt_ctx = nullptr;
    avcodec_close(audio_in_codec_ctx);
    avcodec_free_context(&audio_in_codec_ctx);
    audio_in_codec_ctx = nullptr;
    audio_in_stream = nullptr;
}

void Recorder::destroy_audio_output() {
    swr_free(&audio_converter);
    audio_converter = nullptr;
    av_audio_fifo_free(audio_fifo);
    audio_fifo = nullptr;
    avcodec_close(audio_out_codec_ctx);
    avcodec_free_context(&audio_out_codec_ctx);
    audio_out_codec_ctx = nullptr;
    av_packet_free(&audio_tmp_pkt);
    audio_out_stream = nullptr;
}

void Recorder::destroy_cropper() {
    avfilter_free(crop_video_buffersink_ctx);
    crop_video_buffersink_ctx = nullptr;
    avfilter_free(crop_video_buffersrc_ctx);
    crop_video_buffersrc_ctx = nullptr;
    avfilter_graph_free(&crop_video_filter_graph);
    crop_video_filter_graph = nullptr;
    avfilter_inout_free(&crop_video_inputs);
    crop_video_inputs = nullptr;
    avfilter_inout_free(&crop_video_outputs);
    crop_video_outputs = nullptr;
}

void Recorder::destroy_video() {
    avformat_close_input(&video_in_fmt_ctx);
    avformat_free_context(video_in_fmt_ctx);
    video_in_fmt_ctx = nullptr;
    avcodec_close(video_in_codec_ctx);
    avcodec_free_context(&video_in_codec_ctx);
    video_in_codec_ctx = nullptr;
    video_in_stream = nullptr;
    av_free(video_out_buf);
    sws_freeContext(video_scaler);
    video_scaler = nullptr;
    avcodec_close(video_out_codec_ctx);
    avcodec_free_context(&video_out_codec_ctx);
    video_out_codec_ctx = nullptr;
    video_out_stream = nullptr;
    av_frame_free(&video_tmp_frame);
    video_tmp_frame = nullptr;
    av_packet_free(&video_tmp_pkt);
    video_tmp_pkt = nullptr;
}

Recorder::statuses Recorder::recorder_get_state() {
    lock_guard<mutex> lg(status_mutex);
    return status;
}

vector<string> Recorder::recorder_get_video_devices_list() {
    vector<string> v;
    #ifdef WINDOWS
        v.emplace_back("desktop");
    #endif
    #ifdef UNIX
        v.emplace_back(getenv("DISPLAY"));
    #endif
    #ifdef MACOS
        system("ffmpeg -f avfoundation -list_devices true -i '' 2> list.tmp");
        string s;
        string line;
        ifstream file ("list.tmp");
        if(file.is_open()){
            while(getline(file, line)){
                if(line.find("AVFoundation audio devices:")!=-1)
                    break;
                s.append(line);
            }
            file.close();
            system("rm list.tmp");
        }
        else
            throw runtime_error("[RUNTIME_ERROR] cannot get the list of video devices");
        for(int i=0; i<10; i++){
            string expr;
            expr.append("[").append(to_string(i)).append("]");
            if(s.find(expr)!=-1)
                v.emplace_back(to_string(i));
        }
    #endif
    return v;
}

vector<string> Recorder::recorder_get_audio_devices_list() {
    vector<string> v;
    string line;
    #ifdef WINDOWS
        system("ffmpeg -list_devices true -f dshow -i dummy 2> list.tmp");
        ifstream file ("list.tmp");
        if(file.is_open()){
            while (getline(file, line)){
                unsigned long long start = line.find('"');
                if(start!=-1){
                    unsigned long long end = line.rfind('"');
                    string s = line.substr(start+1, end-start-1);
                    if (s.find('@')==-1)
                        v.emplace_back(s);
                }
            }
            file.close();
            system("del list.tmp");
        }
        else
            throw runtime_error("[RUNTIME_ERROR] cannot get the list audio of devices");
    #endif
    #ifdef UNIX
        system("arecord -l > list.tmp");
        ifstream file ("list.tmp");
        if(file.is_open()){
            while (getline(file, line)){
                unsigned long long card = line.find("card");
                unsigned long long device = line.find("device");
                if(card != -1 && device != -1){
                    string c = line.substr(card+5, 1);
                    string d = line.substr(device+7, 1);
                    string entry;
                    entry.append("hw:").append(c).append(",").append(d);
                    v.emplace_back(entry);
                }
            }
            file.close();
            system("rm list.tmp");
        }
        else
            throw runtime_error("[RUNTIME_ERROR] cannot get the list audio of devices");
    #endif
    #ifdef MACOS
        system("ffmpeg -f avfoundation -list_devices true -i '' 2> list.tmp");
        string s;
        ifstream file ("list.tmp");
        if(file.is_open()){
            while(getline(file, line)){
                s.append(line);
            }
            file.close();
            system("rm list.tmp");
        }
        else
            throw runtime_error("[RUNTIME_ERROR] cannot get the list audio of devices");
        for(int i=0; i<10; i++){
            string expr;
            expr.append("[").append(to_string(i)).append("]");
            if(s.rfind(expr)!=-1)
                v.emplace_back(to_string(i));
        }
    #endif
    return v;
}

void Recorder::write_file_trailer(){
    ret = av_write_trailer(out_fmt_ctx);
    if(ret<0)
        throw runtime_error("[RUNTIME_ERROR] cannot write the output file trailer. The output file may be corrupted!");
}