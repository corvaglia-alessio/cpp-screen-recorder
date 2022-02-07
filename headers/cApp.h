#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "cMain.h"

const std::string APP_NAME = "Screen Recorder";

class cApp : public wxApp
{
private:
    cMain* main_frame = nullptr;

public:
	virtual bool OnInit();
};