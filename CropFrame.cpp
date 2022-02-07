#include "headers/CropFrame.h"


wxBEGIN_EVENT_TABLE(CropFrame, wxFrame)
	EVT_BUTTON(CROP_FRAME_CROP_BTN_ID, CropFrame::OnCropClicked)
	EVT_BUTTON(CROP_FRAME_CANCEL_BTN_ID, CropFrame::OnCancelClicked)
	EVT_CLOSE(CropFrame::OnClose)
wxEND_EVENT_TABLE()



CropFrame::CropFrame(wxFrame* parent) : wxFrame(parent, wxID_ANY, "", wxPoint(300, 300), wxSize(500, 500), wxCAPTION | wxMAXIMIZE_BOX | wxRESIZE_BORDER)
{
	this->SetBackgroundColour(wxColour(255, 255, 255));

    //	LOAD BITMAPS
    close_btm = new wxBitmap(ExePath() + "/sources/close.png", wxBITMAP_TYPE_PNG);
    done_btm = new wxBitmap(ExePath() + "/sources/done.png", wxBITMAP_TYPE_PNG);

    crop_btn = new wxBitmapButton(this, CROP_FRAME_CROP_BTN_ID, *done_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    crop_btn->SetBackgroundColour(wxColour(255, 255, 255));

    cancel_btn = new wxBitmapButton(this, CROP_FRAME_CANCEL_BTN_ID, *close_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    cancel_btn->SetBackgroundColour(wxColour(255, 255, 255));


	wxBoxSizer* btns_sizer = new wxBoxSizer(wxHORIZONTAL);
	btns_sizer->Add(crop_btn, 1);
	btns_sizer->Add(cancel_btn, 1);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(btns_sizer, 0, wxEXPAND);

	SetSizer(main_sizer);
}

void CropFrame::OnCropClicked(wxCommandEvent& evt) 
{
	int w, h, x, y;
	this->GetSize(&w, &h);
	this->GetScreenPosition(&x, &y);

	if (this->m_parent != nullptr)
	{
		((cMain*)this->m_parent)->SaveCropValues(w, h, x, y);
	}
	Close();
	evt.Skip();
}

void CropFrame::OnCancelClicked(wxCommandEvent& evt)
{
	Close();
	evt.Skip();
}

void CropFrame::OnClose(wxCloseEvent& evt)
{
	if (this->m_parent != nullptr)
	{
		this->m_parent->Enable(true);
	}
	evt.Skip();
}