#include "headers/cApp.h"

wxIMPLEMENT_APP(cApp);

bool cApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    main_frame = new cMain(APP_NAME);
    main_frame->Show();

    return true;
}