#include "headers/cMain.h"

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(REC_BTN_ID, cMain::OnRecClicked)
	EVT_BUTTON(PAUSE_BTN_ID, cMain::OnPauseClicked)
	EVT_BUTTON(MIC_BTN_ID, cMain::OnMicClicked)
	EVT_BUTTON(SETTINGS_BTN_ID, cMain::ToggleSettings)
	EVT_BUTTON(CROP_BTN_ID, cMain::OnCropClicked)
	EVT_CHOICE(VIDEO_COMBOBOX_ID, cMain::OnVideoInputChanged)
	EVT_CHOICE(AUDIO_COMBOBOX_ID, cMain::OnAudioInputChanged)
	EVT_CHOICE(FPS_ID, cMain::OnFPSChanged)
	EVT_FILEPICKER_CHANGED(PICKER_ID, cMain::OnFileChanged)
wxEND_EVENT_TABLE()

cMain::cMain(wxFrame* parent, const string title) : wxFrame(parent, wxID_ANY, title, wxPoint(300, 300), wxDefaultSize, wxMINIMIZE_BOX | wxCAPTION | wxCLOSE_BOX)
{
	this->SetBackgroundColour(wxColour(255,255,255));
	
	r = new Recorder();

	path = ExePath() + "/out";

	recording = false;
	mic_enabled = false;
	paused = false;
	perform_crop = false;

	in_settings = false;

	//	INIT COMPONENTS
		wxFont font_for_desc = GetFont().Italic();
		wxFont font_for_header = GetFont().Bold();

		//	LOAD BITMAPS
		settings_btm = new wxBitmap(ExePath() + "/sources/settings.png", wxBITMAP_TYPE_PNG);
		rec_btm = new wxBitmap(ExePath() + "/sources/record.png", wxBITMAP_TYPE_PNG);
		stop_btm = new wxBitmap(ExePath() + "/sources/stop.png", wxBITMAP_TYPE_PNG);
		play_btm = new wxBitmap(ExePath() + "/sources/play.png", wxBITMAP_TYPE_PNG);
		pause_btm = new wxBitmap(ExePath() + "/sources/pause.png", wxBITMAP_TYPE_PNG);
		mic_on_btm = new wxBitmap(ExePath() + "/sources/mic_on.png", wxBITMAP_TYPE_PNG);
		mic_off_btm = new wxBitmap(ExePath() + "/sources/mic_off.png", wxBITMAP_TYPE_PNG);
		crop_btm = new wxBitmap(ExePath() + "/sources/crop.png", wxBITMAP_TYPE_PNG);
		cancel_btm = new wxBitmap(ExePath() + "/sources/close.png", wxBITMAP_TYPE_PNG);

		//	BUTTONS
		settings_btn = new wxBitmapButton(this, SETTINGS_BTN_ID, *settings_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		settings_btn->SetBackgroundColour(wxColour(255, 255, 255));

		crop_btn = new wxBitmapButton(this, CROP_BTN_ID, *crop_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		crop_btn->SetBackgroundColour(wxColour(255, 255, 255));

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

		rec_label = new wxStaticText(this, wxID_ANY, "Record", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		rec_label->SetFont(font_for_desc);
		pause_label = new wxStaticText(this, wxID_ANY, "Pause", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		pause_label->SetFont(font_for_desc);
		mic_label = new wxStaticText(this, wxID_ANY, "Mic off", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		mic_label->SetFont(font_for_desc);
		crop_label = new wxStaticText(this, wxID_ANY, "Crop Screen", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		crop_label->SetFont(font_for_desc);



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

		wxBoxSizer* crop_label_sizer = new wxBoxSizer(wxVERTICAL);
		crop_label_sizer->Add(crop_btn, 0, wxEXPAND | wxBOTTOM, 10);
		crop_label_sizer->Add(crop_label, 0, wxEXPAND, 0);

	rec_controls_sizer->Add(rec_label_sizer, 1, wxRIGHT | wxLEFT, 20);
	rec_controls_sizer->Add(pause_label_sizer, 1, wxRIGHT | wxLEFT, 20);
	rec_controls_sizer->Add(mic_label_sizer, 1, wxRIGHT | wxLEFT, 20);
	rec_controls_sizer->Add(crop_label_sizer, 1, wxRIGHT | wxLEFT, 20);

	rec_controls_box = new wxBoxSizer(wxVERTICAL);
	rec_controls_box->Add(rec_controls_sizer, 0, wxEXPAND | wxRIGHT | wxLEFT, 20);
	rec_controls_box->Add(logger, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);

	rec_settings_box = new wxBoxSizer(wxVERTICAL);
	rec_settings_box->Add(file_picker_label_sizer, 0, wxEXPAND | wxRIGHT | wxLEFT, 20);
	rec_settings_box->Add(rec_settings_label_sizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);
	//rec_settings_box->Add(crop_label_sizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);

	main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(settings_btn, 0, wxALIGN_RIGHT | wxTOP | wxRIGHT, 10);
	main_sizer->Add(rec_controls_box, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT | wxBOTTOM, 20);
	main_sizer->Add(rec_settings_box, 0, wxEXPAND | wxRIGHT | wxLEFT, 20);
	

	rec_settings_box->Show(false);

	SetSizerAndFit(main_sizer);
    Layout();
}

void cMain::OnRecClicked(wxCommandEvent& evt)
{
	if (recording)
	{
		try {
			r->recorder_stop_recording();
		}
		catch (const std::exception& e) {
            //logger->SetLabel(e.what());
			wxMessageBox(e.what(), "", wxICON_ERROR);
            r = new Recorder();
			reset_gui();
        }
		
		r = new Recorder();

		rec_btn->SetBitmap(*rec_btm);
		rec_label->SetLabel("Record");

        pause_btn->SetBitmap(*pause_btm);
		pause_label->SetLabel("Pause");
        
		logger->SetLabel("Ready");
		recording = false;
	}
	else
	{
		logger->SetLabel("Recording is starting");
		try {
			int video_choice = video_source->GetSelection();
			int fps_choice = fps->GetSelection();

			vector<int> screen_dim;

			if (mic_enabled) 
			{
				int audio_choice = audio_source->GetSelection();
				screen_dim = r->recorder_open_inputs(wx_video_sources[video_choice].ToStdString(), stoi(wx_rec_fps[fps_choice].ToStdString()), wx_audio_sources[audio_choice].ToStdString());
			}
			else 
			{
				screen_dim = r->recorder_open_inputs(wx_video_sources[video_choice].ToStdString(), stoi(wx_rec_fps[fps_choice].ToStdString()), "");
			}

			if (perform_crop)
			{
				crop(screen_dim);
			}
			
			r->recorder_init_output(path);
			r->recorder_start_recording();

			rec_btn->SetBitmap(*stop_btm);
			rec_label->SetLabel("Stop");

			logger->SetLabel("Recording in progress");
			recording = true;
		}
		catch (const std::exception& e) {
            //logger->SetLabel(e.what());
			wxMessageBox(e.what(), "", wxICON_ERROR);
            r = new Recorder();
			reset_gui();
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
            //logger->SetLabel(e.what());
			wxMessageBox(e.what(), "", wxICON_ERROR);
            r = new Recorder();
			reset_gui();
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
            //logger->SetLabel(e.what());
			wxMessageBox(e.what(), "", wxICON_ERROR);
            r = new Recorder();
			reset_gui();
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
		mic_label->SetLabel("Mic off");
		mic_enabled = false;
	}
	else
	{
		mic_btn->SetBitmap(*mic_on_btm);
		mic_label->SetLabel("Mic on");
		mic_enabled = true;
	}
	
	evt.Skip();
}

void cMain::crop(vector<int> screen_dim)
{
	int up_margin, bottom_margin, left_margin, right_margin;
	if (crop_x < 0)
		left_margin = 0;
	else
	{
		if (crop_x > screen_dim[0])
			left_margin = screen_dim[0];
		else
			left_margin = crop_x;
	}

	if (crop_y < 0)
		up_margin = 0;
	else
	{
		if (crop_y > screen_dim[1])
			up_margin = screen_dim[1];
		else
			up_margin = crop_y;
	}
	
	int end_x = crop_x + crop_w;
	if (end_x < 0)
		right_margin = screen_dim[0];
	else
	{
		if (end_x > screen_dim[0])
			right_margin = 0;
		else
			right_margin = screen_dim[0] - end_x;
	}

	int end_y = crop_y + crop_h;
	if (end_y < 0)
		bottom_margin = screen_dim[1];
	else
	{
		if (end_y > screen_dim[1])
			bottom_margin = 0;
		else
			bottom_margin = screen_dim[1] - end_y;
	}
	r->recorder_crop_video(left_margin, right_margin, up_margin, bottom_margin);
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

void cMain::reset_gui()
{
	logger->SetLabel("Ready");

	recording = false;
	rec_btn->SetBitmap(*rec_btm);
	rec_label->SetLabel("Record");

	paused = false;
	pause_btn->SetBitmap(*pause_btm);
	pause_label->SetLabel("Pause");

	pause_btn->Enable(recording);
	mic_btn->Enable(!recording);
}

void cMain::OnCropClicked(wxCommandEvent& evt)
{
	if (perform_crop)
	{
		perform_crop = false;
		crop_btn->SetBitmap(*crop_btm);
		crop_label->SetLabel("Crop Screen");
	}
	else
	{
		CropFrame* crop_frame = new CropFrame(this);
		crop_frame->Show();
		this->Enable(false);
	}
	
	evt.Skip();
}

void cMain::SaveCropValues(int w, int h, int x, int y)
{
	crop_w = w;
	crop_h = h;
	crop_x = x;
	crop_y = y;

	perform_crop = true;
	crop_btn->SetBitmap(*cancel_btm);
	crop_label->SetLabel("Reset");
}