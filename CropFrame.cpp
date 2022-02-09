#include "headers/CropFrame.h"


wxBEGIN_EVENT_TABLE(CropFrame, wxFrame)
	EVT_BUTTON(CROP_FRAME_CROP_BTN_ID, CropFrame::OnCropClicked)
	EVT_BUTTON(CROP_FRAME_CANCEL_BTN_ID, CropFrame::OnCancelClicked)
	EVT_CLOSE(CropFrame::OnClose)
wxEND_EVENT_TABLE()



CropFrame::CropFrame(wxFrame* parent) : wxFrame(parent, wxID_ANY, "", wxPoint(100, 100), wxSize(500, 500), wxCAPTION | wxMAXIMIZE_BOX | wxRESIZE_BORDER)
{
	this->SetBackgroundColour(wxColour(255, 255, 255));
	SetTransparent(160);

    //	LOAD BITMAPS
    close_btm = new wxBitmap(ExePath() + "/sources/close.png", wxBITMAP_TYPE_PNG);
    done_btm = new wxBitmap(ExePath() + "/sources/done.png", wxBITMAP_TYPE_PNG);

	ne_arrow_btm = new wxBitmap(ExePath() + "/sources/north_east_arrow.png", wxBITMAP_TYPE_PNG);
	nw_arrow_btm = new wxBitmap(ExePath() + "/sources/north_west_arrow.png", wxBITMAP_TYPE_PNG);
	se_arrow_btm = new wxBitmap(ExePath() + "/sources/south_east_arrow.png", wxBITMAP_TYPE_PNG);
	sw_arrow_btm = new wxBitmap(ExePath() + "/sources/south_west_arrow.png", wxBITMAP_TYPE_PNG);

	ne_arrow_btn = new wxBitmapButton(this, CROP_FRAME_NE_ARROW_BTN_ID, *ne_arrow_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	ne_arrow_btn->SetBackgroundColour(wxColour(255, 255, 255));
	nw_arrow_btn = new wxBitmapButton(this, CROP_FRAME_NW_ARROW_BTN_ID, *nw_arrow_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	nw_arrow_btn->SetBackgroundColour(wxColour(255, 255, 255));
	se_arrow_btn = new wxBitmapButton(this, CROP_FRAME_SE_ARROW_BTN_ID, *se_arrow_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	se_arrow_btn->SetBackgroundColour(wxColour(255, 255, 255));
	sw_arrow_btn = new wxBitmapButton(this, CROP_FRAME_SW_ARROW_BTN_ID, *sw_arrow_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	sw_arrow_btn->SetBackgroundColour(wxColour(255, 255, 255));

    crop_btn = new wxBitmapButton(this, CROP_FRAME_CROP_BTN_ID, *done_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    crop_btn->SetBackgroundColour(wxColour(76, 175, 80));

    cancel_btn = new wxBitmapButton(this, CROP_FRAME_CANCEL_BTN_ID, *close_btm, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    cancel_btn->SetBackgroundColour(wxColour(244, 67, 54));


	wxBoxSizer* btns_sizer = new wxBoxSizer(wxVERTICAL);
	btns_sizer->Add(crop_btn, 1, wxTOP | wxBOTTOM | wxEXPAND, 10);
	btns_sizer->Add(cancel_btn, 1, wxTOP | wxBOTTOM | wxEXPAND, 10);


	wxBoxSizer* north_arrow_sizer = new wxBoxSizer(wxHORIZONTAL);
	north_arrow_sizer->Add(nw_arrow_btn, 0, wxALIGN_TOP);
	north_arrow_sizer->AddStretchSpacer();
	north_arrow_sizer->Add(ne_arrow_btn), 0, wxALIGN_TOP;

	wxBoxSizer* south_arrow_sizer = new wxBoxSizer(wxHORIZONTAL);
	south_arrow_sizer->Add(sw_arrow_btn, 0, wxALIGN_BOTTOM);
	south_arrow_sizer->AddStretchSpacer();
	south_arrow_sizer->Add(se_arrow_btn, 0, wxALIGN_BOTTOM);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(north_arrow_sizer, 1, wxEXPAND);
	main_sizer->Add(btns_sizer, 3, wxEXPAND);
	main_sizer->Add(south_arrow_sizer, 1, wxEXPAND);

	SetSizerAndFit(main_sizer);
	SetSize(wxSize(300, 300));
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