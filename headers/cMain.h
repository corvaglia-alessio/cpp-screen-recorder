#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "wx/filepicker.h"
#include "../include/Recorder.h"

using namespace std;

enum {
    REC_BTN_ID,
    PAUSE_BTN_ID,
    SETTINGS_BTN_ID,
    MIC_BTN_ID,
    LOGGER_ID,
    PICKER_ID,
    FILE_LABEL_ID,
    UP_CROP_ID,
    DOWN_CROP_ID,
    LEFT_CROP_ID,
    RIGHT_CROP_ID,
    VIDEO_COMBOBOX_ID,
    AUDIO_COMBOBOX_ID,
    FPS_ID
};

const int BTN_BORDER = 50;
const int WIDTH = 600, HEIGHT = 400;
const int POS_X = 100, POS_Y = 100;

class cMain : public wxFrame
{
private:
    bool recording;
    bool mic_enabled;
    bool paused;
    bool in_settings;

    string path;

    wxArrayString wx_video_sources;
    wxArrayString wx_audio_sources;
    wxArrayString wx_rec_fps;

    Recorder* r = nullptr;
public:
    //BOXES
    wxBoxSizer* main_sizer = nullptr;
    wxBoxSizer* rec_controls_box = nullptr;
    wxBoxSizer* rec_settings_box = nullptr;

    //BUTTONS
    wxBitmapButton* rec_btn = nullptr;
    wxBitmapButton* pause_btn = nullptr;
    wxBitmapButton* mic_btn = nullptr;
    wxBitmapButton* settings_btn = nullptr;

    //BITMAPS
    wxBitmap* settings_btm = nullptr;
    wxBitmap* rec_btm = nullptr;
    wxBitmap* stop_btm = nullptr;
    wxBitmap* play_btm = nullptr;
    wxBitmap* pause_btm = nullptr;
    wxBitmap* mic_on_btm = nullptr;
    wxBitmap* mic_off_btm = nullptr;

    //CROP TEXT CTRL
    wxTextCtrl* up_margin = nullptr;
    wxTextCtrl* down_margin = nullptr;
    wxTextCtrl* left_margin = nullptr;
    wxTextCtrl* right_margin = nullptr;

    wxChoice* fps = nullptr;
    wxChoice* video_source = nullptr;
    wxChoice* audio_source = nullptr;

    wxStaticText* logger = nullptr;

    wxStaticText* file_label = nullptr;
    wxFilePickerCtrl* file_picker = nullptr;

    cMain(const string title);

    void OnRecClicked(wxCommandEvent& evt);
    void OnPauseClicked(wxCommandEvent& evt);
    void OnMicClicked(wxCommandEvent& evt);
    void OnVideoInputChanged(wxCommandEvent& evt);
    void ToggleSettings(wxCommandEvent& evt);
    void OnAudioInputChanged(wxCommandEvent& evt);
    void OnFileChanged(wxFileDirPickerEvent& evt);
    void OnFPSChanged(wxCommandEvent& evt);
    void crop();

    wxDECLARE_EVENT_TABLE();
};