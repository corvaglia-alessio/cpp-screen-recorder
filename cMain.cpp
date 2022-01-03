#include "headers/cMain.h"


wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(REC_BTN_ID, cMain::OnRecClicked)
	EVT_BUTTON(PAUSE_BTN_ID, cMain::OnPauseClicked)
	EVT_BUTTON(MIC_BTN_ID, cMain::OnMicClicked)
	EVT_BUTTON(CROP_BTN_ID, cMain::OnCropClicked)
	EVT_CHOICE(VIDEO_COMBOBOX_ID, cMain::OnVideoInputChanged)
	EVT_CHOICE(AUDIO_COMBOBOX_ID, cMain::OnAudioInputChanged)
	EVT_FILEPICKER_CHANGED(PICKER_ID, cMain::OnFileChanged)
wxEND_EVENT_TABLE()

cMain::cMain(const string title) : wxFrame(nullptr, wxID_ANY, title, wxPoint(POS_X, POS_Y), wxSize(WIDTH, HEIGHT))
{
	this->SetBackgroundColour(wxColour(255,255,255));

	//r = new Recorder();

	path = "./out";

	//CaptureMouse();
	//SetTransparent();
	recording = false;
	mic_enabled = false;
	paused = false;

	//	INIT COMPONENTS
	rec_btn = new wxButton(this, REC_BTN_ID, "Start Recording");
	pause_btn = new wxButton(this, PAUSE_BTN_ID, "Pause Recording");
	mic_btn = new wxButton(this, MIC_BTN_ID, "Enable Microphone");
	logger = new wxStaticText(this, LOGGER_ID, "Ready", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);


	file_picker = new wxFilePickerCtrl(this, PICKER_ID, wxEmptyString, wxString::FromAscii(wxFileSelectorPromptStr), wxString::FromAscii(wxFileSelectorDefaultWildcardStr), wxDefaultPosition, wxDefaultSize, wxFLP_SAVE);
	file_label = new wxStaticText(this, FILE_LABEL_ID, "Select output file", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);

	wxBoxSizer* fileSizer = new wxBoxSizer(wxHORIZONTAL);
	fileSizer->Add(file_label, 4, wxRIGHT | wxLEFT, 10);
	fileSizer->Add(file_picker, 1, wxRIGHT | wxLEFT, 10);


	crop_btn = new wxButton(this, CROP_BTN_ID, "Crop Margin");
	up_margin = new wxTextCtrl(this, UP_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	up_margin->SetHint("Up");
	down_margin = new wxTextCtrl(this, DOWN_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	down_margin->SetHint("Down");
	left_margin = new wxTextCtrl(this, LEFT_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	left_margin->SetHint("Left");
	right_margin = new wxTextCtrl(this, RIGHT_CROP_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	right_margin->SetHint("Right");


	fps = new wxTextCtrl(this, FPS_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	fps->SetHint("FPS");

	
	//vector<string> video_sources = Recorder::recorder_get_video_devices_list();
	wx_video_sources = wxArrayString();
/*
	for (string s : video_sources)
	{
		wx_video_sources.Add(s);
	}
 */
	video_source = new wxChoice(this, VIDEO_COMBOBOX_ID, wxDefaultPosition, wxDefaultSize, wx_video_sources);
	video_source->SetSelection(0);

	//vector<string> audio_sources = Recorder::recorder_get_audio_devices_list();
	wx_audio_sources = wxArrayString();
/*
	for (string s : audio_sources)
	{
		wx_audio_sources.Add(s);
	}
 */
	audio_source = new wxChoice(this, AUDIO_COMBOBOX_ID, wxDefaultPosition, wxDefaultSize, wx_audio_sources);
	audio_source->SetSelection(0);

	wxBoxSizer* inputSizer = new wxBoxSizer(wxHORIZONTAL);
	inputSizer->Add(video_source, 3, wxRIGHT | wxLEFT, 10);
	inputSizer->Add(audio_source, 3, wxRIGHT | wxLEFT, 10);
	inputSizer->Add(fps, 2, wxRIGHT | wxLEFT, 10);
	
	wxBoxSizer* cropSizer = new wxBoxSizer(wxHORIZONTAL);
	cropSizer->Add(crop_btn, 1, 0, 0);
	cropSizer->Add(up_margin, 1, 0, 0);
	cropSizer->Add(down_margin, 1, 0, 0);
	cropSizer->Add(left_margin, 1, 0, 0);
	cropSizer->Add(right_margin, 1, 0, 0);

	pause_btn->Enable(false);

	wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
	hSizer->Add(rec_btn, 1, 0, 0);
	hSizer->Add(pause_btn, 1, 0, 0);
	hSizer->Add(mic_btn, 1, 0, 0);

	vSizer->Add(fileSizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);
	vSizer->Add(cropSizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);
	vSizer->Add(inputSizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);
	vSizer->Add(hSizer, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);
	vSizer->Add(logger, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 20);
	
	vSizer->SetSizeHints(this);
	SetSizer(vSizer);
}

void cMain::OnRecClicked(wxCommandEvent& evt)
{
	if (recording)
	{
		//r->recorder_stop_recording();

		//r = new Recorder();

		rec_btn->SetLabel("Start Recording");
		recording = false;

		logger->SetLabel("Ready");
	}
	else
	{
		int video_choice = video_source->GetSelection();

		if (mic_enabled) {
			int audio_choice = audio_source->GetSelection();
			//r->recorder_open_inputs(wx_video_sources[video_choice].ToStdString(), stoi(fps->GetValue().ToStdString()), wx_audio_sources[audio_choice].ToStdString());
		}
		else {
			//r->recorder_open_inputs(wx_video_sources[video_choice].ToStdString(), stoi(fps->GetValue().ToStdString()), "");
		}
		crop();
		//r->recorder_init_output(path);
		//r->recorder_start_recording();
		
		rec_btn->SetLabel("Stop Recording");
		recording = true;

		logger->SetLabel("Recording in progress");
	}

	pause_btn->Enable(recording);
	mic_btn->Enable(!recording);
	crop_btn->Enable(!recording);
	evt.Skip();
}

void cMain::OnPauseClicked(wxCommandEvent& evt)
{
	if (paused)
	{
		//r->recorder_resume_recording();

		pause_btn->SetLabel("Pause Recording");
		paused = false;

		logger->SetLabel("Recording in progress");
	}
	else
	{
		//r->recorder_pause_recording();

		pause_btn->SetLabel("Resume Recording");
		paused = true;

		logger->SetLabel("Paused");
	}
	evt.Skip();
}

void cMain::OnMicClicked(wxCommandEvent& evt)
{
	if (mic_enabled) 
	{
		mic_btn->SetLabel("Enable Microphone");
		mic_enabled = false;
	}
	else
	{
		mic_btn->SetLabel("Disable Microphone");
		mic_enabled = true;
	}
	
	evt.Skip();
}

void cMain::OnCropClicked(wxCommandEvent& evt)
{
	string up_margin_str = up_margin->GetValue().ToStdString();
	string down_margin_str = down_margin->GetValue().ToStdString();
	string left_margin_str = left_margin->GetValue().ToStdString();
	string right_margin_str = right_margin->GetValue().ToStdString();

	int up_margin_value = stoi(up_margin_str);
	int down_margin_value = stoi(up_margin_str);
	int left_margin_value = stoi(up_margin_str);
	int right_margin_value = stoi(up_margin_str);

	//r->recorder_crop_video(left_margin_value, right_margin_value, up_margin_value, down_margin_value);

	logger->SetLabel("Cropped " + up_margin_str + ":" + down_margin_str + ":" + left_margin_str + ":" + right_margin_str);
	evt.Skip();
}

void cMain::crop()
{
	string up_margin_str = up_margin->GetValue().ToStdString();
	string down_margin_str = down_margin->GetValue().ToStdString();
	string left_margin_str = left_margin->GetValue().ToStdString();
	string right_margin_str = right_margin->GetValue().ToStdString();

	int up_margin_value = stoi(up_margin_str);
	int down_margin_value = stoi(up_margin_str);
	int left_margin_value = stoi(up_margin_str);
	int right_margin_value = stoi(up_margin_str);

	//r->recorder_crop_video(left_margin_value, right_margin_value, up_margin_value, down_margin_value);
}

void cMain::OnVideoInputChanged(wxCommandEvent& evt)
{
	int choice = video_source->GetSelection();
	logger->SetLabel(wx_video_sources[choice]);
	evt.Skip();
}

void cMain::OnAudioInputChanged(wxCommandEvent& evt)
{
	int choice = audio_source->GetSelection();
	logger->SetLabel(wx_audio_sources[choice]);
	evt.Skip();
}

void cMain::OnFileChanged(wxFileDirPickerEvent& evt)
{
	path = file_picker->GetPath().ToStdString();
	file_label->SetLabel(path);
	evt.Skip();
}