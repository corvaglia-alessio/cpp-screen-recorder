#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "Common.h"
#include "cMain.h"

using namespace std;


class CropFrame : public wxFrame
{
private:


public:

    wxBitmapButton* crop_btn = nullptr;
    wxBitmapButton* cancel_btn = nullptr;

    wxBitmap* close_btm = nullptr;
    wxBitmap* done_btm = nullptr;

    CropFrame(wxFrame* parent);

    void OnCropClicked(wxCommandEvent& evt);
    void OnCancelClicked(wxCommandEvent& evt);
    void OnClose(wxCloseEvent& evt);

    wxDECLARE_EVENT_TABLE();
};