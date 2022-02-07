#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "wx/filepicker.h"
#include "../include/Recorder.h"

#include "CropFrame.h"
#include "Common.h"

using namespace std;

class cMain : public wxFrame
{
private:

    bool recording;
    bool mic_enabled;
    bool paused;
    bool in_settings;

    bool perform_crop;
    int crop_w, crop_h, crop_x, crop_y;

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

    wxBitmapButton* crop_btn = nullptr;

    //BITMAPS
    wxBitmap* settings_btm = nullptr;
    wxBitmap* rec_btm = nullptr;
    wxBitmap* stop_btm = nullptr;
    wxBitmap* play_btm = nullptr;
    wxBitmap* pause_btm = nullptr;
    wxBitmap* mic_on_btm = nullptr;
    wxBitmap* mic_off_btm = nullptr;
    wxBitmap* crop_btm = nullptr;
    wxBitmap* cancel_btm = nullptr;

    wxChoice* fps = nullptr;
    wxChoice* video_source = nullptr;
    wxChoice* audio_source = nullptr;

    // DYNAMIC LABELS
    wxStaticText* rec_label = nullptr;
    wxStaticText* pause_label = nullptr;
    wxStaticText* mic_label = nullptr;
    wxStaticText* crop_label = nullptr;

    wxStaticText* logger = nullptr;

    wxStaticText* file_label = nullptr;
    wxFilePickerCtrl* file_picker = nullptr;

    cMain(wxFrame* parent, const string title);

    void OnRecClicked(wxCommandEvent& evt);
    void OnPauseClicked(wxCommandEvent& evt);
    void OnMicClicked(wxCommandEvent& evt);
    void OnVideoInputChanged(wxCommandEvent& evt);
    void ToggleSettings(wxCommandEvent& evt);
    void OnAudioInputChanged(wxCommandEvent& evt);
    void OnFileChanged(wxFileDirPickerEvent& evt);
    void OnFPSChanged(wxCommandEvent& evt);
    void OnCropClicked(wxCommandEvent& evt);
    void SaveCropValues(int w, int h, int x, int y);
    void crop(vector<int> screen_dim);
    void reset_gui();

    wxDECLARE_EVENT_TABLE();
};