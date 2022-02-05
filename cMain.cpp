#include "headers/cMain.h"

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(REC_BTN_ID, cMain::OnRecClicked)
	EVT_BUTTON(PAUSE_BTN_ID, cMain::OnPauseClicked)
	EVT_BUTTON(MIC_BTN_ID, cMain::OnMicClicked)
	EVT_BUTTON(SETTINGS_BTN_ID, cMain::ToggleSettings)
	EVT_CHOICE(VIDEO_COMBOBOX_ID, cMain::OnVideoInputChanged)
	EVT_CHOICE(AUDIO_COMBOBOX_ID, cMain::OnAudioInputChanged)
	EVT_CHOICE(FPS_ID, cMain::OnFPSChanged)
	EVT_FILEPICKER_CHANGED(PICKER_ID, cMain::OnFileChanged)
wxEND_EVENT_TABLE()


std::wstring ExePath() {
#ifdef UNIX
    char pBuf[MAX_BYTES];
    size_t len = size(pBuf);
    int bytes = MIN(readlink("/proc/self/exe", pBuf, len), len-1);
    if(bytes >= 0)
        pBuf[bytes] = '\0';
    int last_slash = 0;
    for(int i=0; i<bytes; i++) {
        if (pBuf[i] == '/')
            last_slash = i;
    }
    return std::wstring (&pBuf[0], &pBuf[last_slash]);
#else
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    std::wstring exe_path = std::wstring(buffer).substr(0, pos);
    std::replace(exe_path.begin(), exe_path.end(), '\\', '/');
    return exe_path;
#endif

}


cMain::cMain(const string title) : wxFrame(nullptr, wxID_ANY, title, wxPoint(POS_X, POS_Y), wxSize(WIDTH, HEIGHT), wxMINIMIZE_BOX | wxCAPTION | wxCLOSE_BOX)
{
	this->SetBackgroundColour(wxColour(255,255,255));

	r = new Recorder();

	path = ExePath() + "/out";

	recording = false;
	mic_enabled = false;
	paused = false;

	in_settings = false;

	//	INIT COMPONENTS
		wxFont font_for_desc = GetFont().Italic();
		wxFont font_for_header = GetFont().Larger().Bold();

		//	LOAD BITMAPS
		settings_btm = new wxBitmap(ExePath() + "/sources/settings.png", wxBITMAP_TYPE_PNG);
		rec_btm = new wxBitmap(ExePath() + "/sources/record.png", wxBITMAP_TYPE_PNG);
		stop_btm = new wxBitmap(ExePath() + "/sources/stop.png", wxBITMAP_TYPE_PNG);
		play_btm = new wxBitmap(ExePath() + "/sources/play.png", wxBITMAP_TYPE_PNG);
		pause_btm = new wxBitmap(ExePath() + "/sources/pause.png", wxBITMAP_TYPE_PNG);
		mic_on_btm = new wxBitmap(ExePath() + "/sources/mic_on.png", wxBITMAP_TYPE_PNG);
		mic_off_btm = new wxBitmap(ExePath() + "/sources/mic_off.png", wxBITMAP_TYPE_PNG);

		//	BUTTONS
		settings_btn = new wxBitmapButton(this, SETTINGS_BTN_ID, *settings_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		settings_btn->SetBackgroundColour(wxColour(255, 255, 255));

		rec_btn = new wxBitmapButton(this, REC_BTN_ID, *rec_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		rec_btn->SetBackgroundColour(wxColour(255, 255, 255));

		pause_btn = new wxBitmapButton(this, PAUSE_BTN_ID, *pause_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		pause_btn->SetBackgroundColour(wxColour(255, 255, 255));

		mic_btn = new wxBitmapButton(this, MIC_BTN_ID, *mic_off_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		mic_btn->SetBackgroundColour(wxColour(255, 255, 255));

		//	LOGGER
		logger = new wxStaticText(this, LOGGER_ID, "Ready", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		logger->SetFont(font_for_header);

		pause_btn->Enable(false);

		//	FILE PICKER
		file_picker = new wxFilePickerCtrl(this, PICKER_ID, wxEmptyString, wxString::FromAscii(wxFileSelectorPromptStr), wxString::FromAscii(wxFileSelectorDefaultWildcardStr), wxDefaultPosition, wxDefaultSize, wxFLP_SAVE);
		file_label = new wxStaticText(this, FILE_LABEL_ID, path + ".mp4", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);

		//	CROP MARGIN
		up_margin = new wxTextCtrl(this, UP_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
		up_margin->SetHint("Up");
		down_margin = new wxTextCtrl(this, DOWN_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
		down_margin->SetHint("Down");
		left_margin = new wxTextCtrl(this, LEFT_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
		left_margin->SetHint("Left");
		right_margin = new wxTextCtrl(this, RIGHT_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
		right_margin->SetHint("Right");

		//	FPS COMBOBOX
		std::vector<string> rec_fps = { "15", "24", "30" };
		wx_rec_fps = wxArrayString();
		for (string s : rec_fps)
		{
			wx_rec_fps.Add(s);
		}
		fps = new wxChoice(this, FPS_ID, wxDefaultPosition, wxDefaultSize, wx_rec_fps);
		fps->SetSelection(0);

		//	VIDEO DEVICES COMBOBOX
		vector<string> video_sources = Recorder::recorder_get_video_devices_list();
		wx_video_sources = wxArrayString();
		for (string s : video_sources)
		{
			wx_video_sources.Add(s);
		}
		video_source = new wxChoice(this, VIDEO_COMBOBOX_ID, wxDefaultPosition, wxDefaultSize, wx_video_sources);
		video_source->SetSelection(0);

		//	AUDIO DEVICES COMBOBOX
		vector<string> audio_sources = Recorder::recorder_get_audio_devices_list();
		wx_audio_sources = wxArrayString();
		for (string s : audio_sources)
		{
			wx_audio_sources.Add(s);
		}
		audio_source = new wxChoice(this, AUDIO_COMBOBOX_ID, wxDefaultPosition, wxDefaultSize, wx_audio_sources);
		audio_source->SetSelection(0);


		//	LABELS

		wxStaticText* path_label = new wxStaticText(this, wxID_ANY, "Saving location", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		path_label->SetFont(font_for_header);
		wxStaticText* video_source_label = new wxStaticText(this, wxID_ANY, "Video Source", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		video_source_label->SetFont(font_for_header);
		wxStaticText* audio_source_label = new wxStaticText(this, wxID_ANY, "Audio Source", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		audio_source_label->SetFont(font_for_header);
		wxStaticText* fps_label = new wxStaticText(this, wxID_ANY, "FPS", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		fps_label->SetFont(font_for_header);
		wxStaticText* crop_label = new wxStaticText(this, wxID_ANY, "Crop Pixels x Direction", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		crop_label->SetFont(font_for_header);

		rec_label = new wxStaticText(this, wxID_ANY, "Record", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		rec_label->SetFont(font_for_desc);
		pause_label = new wxStaticText(this, wxID_ANY, "Pause", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		pause_label->SetFont(font_for_desc);
		mic_label = new wxStaticText(this, wxID_ANY, "Enable Mic", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		mic_label->SetFont(font_for_desc);



	//	SIZERS

	wxBoxSizer* file_picker_label_sizer = new wxBoxSizer(wxVERTICAL);

		wxBoxSizer* file_picker_sizer = new wxBoxSizer(wxHORIZONTAL);
		file_picker_sizer->Add(file_label, 4, wxRIGHT | wxLEFT, 10);
		file_picker_sizer->Add(file_picker, 1, wxRIGHT | wxLEFT, 10);

	file_picker_label_sizer->Add(path_label, 0, wxEXPAND | wxBOTTOM, 10);
	file_picker_label_sizer->Add(file_picker_sizer, 0, wxEXPAND | wxBOTTOM, 25);


	wxBoxSizer* rec_settings_sizer = new wxBoxSizer(wxHORIZONTAL);

		wxBoxSizer* video_source_label_sizer = new wxBoxSizer(wxVERTICAL);
		video_source_label_sizer->Add(video_source_label, 0, wxEXPAND, 0);
		video_source_label_sizer->Add(video_source, 0, wxEXPAND | wxTOP, 10);

		wxBoxSizer* audio_source_label_sizer = new wxBoxSizer(wxVERTICAL);
		audio_source_label_sizer->Add(audio_source_label, 0, wxEXPAND, 0);
		audio_source_label_sizer->Add(audio_source, 0, wxEXPAND | wxTOP, 10);

		wxBoxSizer* fps_label_sizer = new wxBoxSizer(wxVERTICAL);
		fps_label_sizer->Add(fps_label, 0, wxEXPAND, 0);
		fps_label_sizer->Add(fps, 0, wxEXPAND | wxTOP, 10);

	rec_settings_sizer->Add(video_source_label_sizer, 2, wxRIGHT | wxLEFT, 10);
	rec_settings_sizer->Add(audio_source_label_sizer, 2, wxRIGHT | wxLEFT, 10);
	rec_settings_sizer->Add(fps_label_sizer, 1, wxRIGHT | wxLEFT, 10);

	wxBoxSizer* rec_settings_label_sizer = new wxBoxSizer(wxVERTICAL);
	rec_settings_label_sizer->Add(rec_settings_sizer, 0, wxEXPAND | wxBOTTOM, 25);
	

	wxBoxSizer* crop_label_sizer = new wxBoxSizer(wxVERTICAL);

		wxBoxSizer* crop_sizer = new wxBoxSizer(wxHORIZONTAL);
		crop_sizer->Add(up_margin, 1, 0, 0);
		crop_sizer->Add(down_margin, 1, 0, 0);
		crop_sizer->Add(left_margin, 1, 0, 0);
		crop_sizer->Add(right_margin, 1, 0, 0);

	crop_label_sizer->Add(crop_label, 0, wxEXPAND | wxBOTTOM, 10);
	crop_label_sizer->Add(crop_sizer, 0, wxEXPAND | wxBOTTOM, 25);

	wxBoxSizer* rec_controls_sizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* rec_label_sizer = new wxBoxSizer(wxVERTICAL);
		rec_label_sizer->Add(rec_btn, 0, wxEXPAND | wxBOTTOM, 10);
		rec_label_sizer->Add(rec_label, 0, wxEXPAND, 0);

		wxBoxSizer* pause_label_sizer = new wxBoxSizer(wxVERTICAL);
		pause_label_sizer->Add(pause_btn, 0, wxEXPAND | wxBOTTOM, 10);
		pause_label_sizer->Add(pause_label, 0, wxEXPAND, 0);

		wxBoxSizer* mic_label_sizer = new wxBoxSizer(wxVERTICAL);
		mic_label_sizer->Add(mic_btn, 0, wxEXPAND | wxBOTTOM, 10);
		mic_label_sizer->Add(mic_label, 0, wxEXPAND, 0);

	rec_controls_sizer->Add(rec_label_sizer, 1, wxRIGHT | wxLEFT, 20);
	rec_controls_sizer->Add(pause_label_sizer, 1, wxRIGHT | wxLEFT, 20);
	rec_controls_sizer->Add(mic_label_sizer, 1, wxRIGHT | wxLEFT, 20);

	rec_controls_box = new wxBoxSizer(wxVERTICAL);
	rec_controls_box->Add(rec_controls_sizer, 0, wxEXPAND | wxRIGHT | wxLEFT, 20);
	rec_controls_box->Add(logger, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);

	rec_settings_box = new wxBoxSizer(wxVERTICAL);
	rec_settings_box->Add(file_picker_label_sizer, 0, wxEXPAND | wxRIGHT | wxLEFT, 20);
	rec_settings_box->Add(rec_settings_label_sizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);
	rec_settings_box->Add(crop_label_sizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);

	main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(settings_btn, 0, wxALIGN_RIGHT | wxTOP | wxRIGHT, 10);
	main_sizer->Add(rec_controls_box, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT | wxBOTTOM, 20);
	main_sizer->Add(rec_settings_box, 0, wxEXPAND | wxRIGHT | wxLEFT, 20);
	

	rec_settings_box->Show(false);

	SetSizerAndFit(main_sizer);
}

void cMain::OnRecClicked(wxCommandEvent& evt)
{
	if (recording)
	{
		try {
			r->recorder_stop_recording();
		}
		catch (const std::exception& e) {
            logger->SetLabel(e.what());
            r = new Recorder();
        }
		
		r = new Recorder();

		rec_btn->SetBitmap(*rec_btm);
		rec_label->SetLabel("Record");

        pause_btn->SetBitmap(*pause_btm);
		pause_label->SetLabel("Pause");
        recording = false;

		logger->SetLabel("Ready");
	}
	else
	{
		try {
			int video_choice = video_source->GetSelection();
			int fps_choice = fps->GetSelection();

			if (mic_enabled) {
				int audio_choice = audio_source->GetSelection();
				r->recorder_open_inputs(wx_video_sources[video_choice].ToStdString(), stoi(wx_rec_fps[fps_choice].ToStdString()), wx_audio_sources[audio_choice].ToStdString());
			}
			else {
				r->recorder_open_inputs(wx_video_sources[video_choice].ToStdString(), stoi(wx_rec_fps[fps_choice].ToStdString()), "");
			}

			crop();
			r->recorder_init_output(path);
			r->recorder_start_recording();

			rec_btn->SetBitmap(*stop_btm);
			rec_label->SetLabel("Stop");
			recording = true;

			logger->SetLabel("Recording in progress");
		}
		catch (const std::exception& e) {
            logger->SetLabel(e.what());
            r = new Recorder();
        }
	}

	pause_btn->Enable(recording);
	mic_btn->Enable(!recording);
	evt.Skip();
}

void cMain::OnPauseClicked(wxCommandEvent& evt)
{
	if (paused)
	{
		try {
			r->recorder_resume_recording();
		}
		catch (const std::exception& e) {
            logger->SetLabel(e.what());
            r = new Recorder();
        }

		pause_btn->SetBitmap(*pause_btm);
		pause_label->SetLabel("Pause");
		paused = false;

		logger->SetLabel("Recording in progress");
	}
	else
	{
		try {
			r->recorder_pause_recording();
		}
		catch (const std::exception& e) {
            logger->SetLabel(e.what());
            r = new Recorder();
        }

		pause_btn->SetBitmap(*play_btm);
		pause_label->SetLabel("Play");
		paused = true;

		logger->SetLabel("Recording Paused");
	}
	evt.Skip();
}

void cMain::OnMicClicked(wxCommandEvent& evt)
{
	if (mic_enabled) 
	{
		mic_btn->SetBitmap(*mic_off_btm);
		mic_label->SetLabel("Enable Mic");
		mic_enabled = false;
	}
	else
	{
		mic_btn->SetBitmap(*mic_on_btm);
		mic_label->SetLabel("Dis Mic");
		mic_enabled = true;
	}
	
	evt.Skip();
}

void cMain::crop()
{
	string up_margin_str = up_margin->GetValue().ToStdString();
	string down_margin_str = down_margin->GetValue().ToStdString();
	string left_margin_str = left_margin->GetValue().ToStdString();
	string right_margin_str = right_margin->GetValue().ToStdString();


	if (!up_margin_str.empty() || !down_margin_str.empty() || !left_margin_str.empty() || !right_margin_str.empty())
	{
        int up_margin_value, down_margin_value, left_margin_value, right_margin_value;
        try {

            if (up_margin_str.empty())
                up_margin_value = 0;
            else
                up_margin_value = stoi(up_margin_str);

            if (down_margin_str.empty())
                down_margin_value = 0;
            else
                down_margin_value = stoi(down_margin_str);

            if (left_margin_str.empty())
                left_margin_value = 0;
            else
                left_margin_value = stoi(left_margin_str);

            if (right_margin_str.empty())
                right_margin_value = 0;
            else
                right_margin_value = stoi(right_margin_str);
        }
        catch (const invalid_argument &ia){
            logger->SetLabel("Type valid input into crop fields");
        }

        r->recorder_crop_video(left_margin_value, right_margin_value, up_margin_value, down_margin_value);
	}
}

void cMain::OnVideoInputChanged(wxCommandEvent& evt)
{
	evt.Skip();
}

void cMain::OnAudioInputChanged(wxCommandEvent& evt)
{
	evt.Skip();
}

void cMain::OnFPSChanged(wxCommandEvent& evt)
{
	evt.Skip();
}

void cMain::OnFileChanged(wxFileDirPickerEvent& evt)
{
	path = file_picker->GetPath().ToStdString();
	file_label->SetLabel(path + ".mp4");
	evt.Skip();
}

void cMain::ToggleSettings(wxCommandEvent& evt)
{
	if (in_settings)
	{
		settings_btn->SetBackgroundColour(wxColour(255, 255, 255));
		in_settings = false;
	}
	else
	{
		settings_btn->SetBackgroundColour(wxColour(33, 150, 243));
		in_settings = true;
	}

	rec_controls_box->Show(!in_settings);
	rec_controls_box->Layout();
	rec_settings_box->Show(in_settings);
	rec_settings_box->Layout();

	main_sizer->Layout();
	Fit();
	evt.Skip();
}