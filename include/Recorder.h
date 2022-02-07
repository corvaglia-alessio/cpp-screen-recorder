#pragma once

#include <string>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "exception"
#include <cstdlib>
#include <vector>
#include <fstream>

#define UNIX

using namespace std;

#define PIX_FMT AV_PIX_FMT_YUV420P
#define VIDEO_ENCODER AV_CODEC_ID_MPEG4
#define AUDIO_ENCODER AV_CODEC_ID_AAC

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

class Recorder {

public:
    enum statuses {RECORDING, PAUSE, STOP};

    Recorder();

    /**
     * This function open the input devices, using the names passed.
     * This is the first function that you must call after having built the screen recorder object.
     * @param video_input
     * the string name of the video device that you want to capture, use recorder_get_video_devices_list() to get a list of available devices to choose from.
     * If the input is not compatible, an exception will be thrown.
     * @param frames_per_second
     * the number of frames per second to capture. It must be in the range [1, 30]. If the output is accelerated, the value passed is too high for the machine performances, try decreasing it. A standard value should be 30.
     * @param audio_input
     * If you do not want to capture the audio, just pass an empty string,
     * otherwise pass the string name of the audio device that you want to capture, use recorder_get_audio_devices_list() to get a list of available devices to choose from.
     * If the input is not compatible, or it does not contain an audio stream (e.g. webcam without a microphone), an exception will be thrown.
     * @return a vector of int where the first value is the video width and the second value is the video height
     */
    vector<int> recorder_open_inputs(const string& video_input, int frames_per_second, string audio_input);

    /**
     * Call this function, only if you want to crop the input video by removing some parts.
     * Call this function only after having opened the inputs with recorder_open_inputs(), otherwise you will get ad undefined behaviour.
     * Usage example: given a 1920x1080 screen, by calling recorder_crop_video(100, 100, 100, 100) you will obtain a video output with a 1720x880 resolution, with only the central screen part captured.
     * All the parameters, must be >=0, pass 0 if you do not want to crop that side. If you crop more than the width of the height of the input, an exception will be thrown.
     * @param left the number of pixels that you want to remove from the left side
     * @param right the number of pixels that you want to remove from the right side
     * @param top the number of pixels that you want to remove from the top side
     * @param bottom the number of pixels that you want to remove from the bottom side
     */
    void recorder_crop_video(int left, int right, int top, int bottom);

    /**
     * After having called the recorder_open_inputs() and optionally the recorder_crop_video(), you must call this function in order to initialize the output file.
     * @param path_name the path, with the file name, without the extension (it will be automatically appended)
     * Usage example: if you want to store your file in the path "/a/b" with the name "output", just pass the string "/a/b/output"
     * If the path does not exist, an exception will be thrown.
     * If the output file does not exist, it will be automatically created, otherwise it will be overwritten.
     */
    void recorder_init_output(const string& path_name);

    /**
     * After having called recorder_init_output() you can start the recording by calling this function.
     * You can start recording only if you are in the STOP state.
     * You can use this function only once during the life of the object, otherwise you will get an undefined behaviour.
     */
    void recorder_start_recording();

    /**
     * Use this function to temporarly pause the recording.
     * You can use this function only if you are in the RECORDING state.
     */
    void recorder_pause_recording();

    /**
    * Use this function to resume the recording after having paused it.
     * You can use this function only if you are in the PAUSE state.
     */
    void recorder_resume_recording();

    /**
     * Use this function to stop and conclude the recording.
     * You can call this function only if you are in RECORDING or PAUSE state.
     * You can use this function only once during the life of the object, otherwise you will get an undefined behaviour.
     */
    void recorder_stop_recording();

    /**
     * Use this function to get the actual state of the recording.
     * @return the actual enum status of the recording.
     */
    statuses recorder_get_state();

    /**
     * It probes for display devices, it may be possible that other kind of devices are returned (e.g. webcams), these devices may not be compatible with the library and an undefined behaviour may happen.
     * @return a std::vector<std::string> containing the list of found video input devices.
     */
    static vector<string> recorder_get_video_devices_list();

    /**
    * It probes for audio input devices, it may be possible that other kind of devices are returned (e.g. webcams that include a microphone), in such cases only the audio stream will be used and recorded.
    * @return a std::vector<std::string> containing the list of found audio input devices.
    */
    static vector<string> recorder_get_audio_devices_list();


    ~Recorder();

private:

    bool audio_on;
    string audio_in_device;
    AVFormatContext* audio_in_fmt_ctx;
    AVStream* audio_in_stream;
    AVCodecContext* audio_in_codec_ctx;
    SwrContext* audio_converter;
    AVAudioFifo* audio_fifo;
    AVStream* audio_out_stream;
    AVCodecContext* audio_out_codec_ctx;
    AVPacket* audio_tmp_pkt;
    int64_t audio_next_pts;

    bool crop;
    int c_left, c_right, c_top, c_bottom;
    AVFormatContext* video_in_fmt_ctx;
    AVStream* video_in_stream;
    AVCodecContext* video_in_codec_ctx;
    SwsContext* video_scaler;
    unsigned char* video_out_buf;
    AVStream* video_out_stream;
    AVCodecContext* video_out_codec_ctx;
    AVPacket* video_tmp_pkt;
    int64_t video_next_pts;
    AVFilterGraph* crop_video_filter_graph;
    AVFilterInOut* crop_video_inputs;
    AVFilterInOut* crop_video_outputs;
    AVFilterContext* crop_video_buffersink_ctx;
    AVFilterContext* crop_video_buffersrc_ctx;
    AVFrame* video_tmp_frame;

    AVFormatContext* out_fmt_ctx;
    int video_turn, audio_turn;
    int fps;

    condition_variable status_cv;
    mutex status_mutex;
    statuses status;
    thread recording_thread;

    int ret;

    int write_video_frame();
    int write_audio_frame();
    void record_loop();
    void open_audio_input();
    void open_audio_output();
    void destroy_audio_input();
    void destroy_audio_output();
    void destroy_video();
    void destroy_cropper();
    void write_file_trailer();
};
