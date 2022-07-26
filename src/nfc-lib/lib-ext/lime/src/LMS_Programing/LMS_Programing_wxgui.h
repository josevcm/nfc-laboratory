/**
@file   LMS_Programing_wxgui.h
@author Lime Microsystems
*/
#ifndef LMS_Programing_wxgui_H
#define LMS_Programing_wxgui_H

#include <wx/frame.h>
#include <wx/wx.h>
class wxGauge;
class wxStaticText;
class wxFlexGridSizer;
class wxButton;
class wxChoice;

#include <thread>
#include <atomic>
#include <vector>
#include "lime/LimeSuite.h"

class LMS_Programing_wxgui : public wxFrame
{
public:
    LMS_Programing_wxgui(wxWindow* parent,wxWindowID id=wxID_ANY,const wxString& title =_(""), const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, int style=0, wxString name = wxEmptyString);
    virtual void SetConnection(lms_device_t* port);
    virtual ~LMS_Programing_wxgui();

protected:
    wxChoice* cmbDevice;
    wxStaticText* lblProgressPercent;
    wxStaticText* StaticText2;
    wxGauge* progressBar;
    wxStaticText* lblFilename;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxButton* btnStartStop;
    wxButton* btnOpen;
    bool btnOpenEnb;

    void DoProgramming();
    static bool OnProgrammingCallback(int bsent, int btotal, const char* progressMsg);
    static bool test(int bsent, int btotal, const char* progressMsg);
    std::vector<char> mProgramData;
    lms_device_t* lmsControl;
    std::atomic<bool> mProgrammingInProgress;
    std::atomic<bool> mAbortProgramming;
    std::thread mWorkerThread;
    static LMS_Programing_wxgui* obj_ptr;
    static const long ID_PROGRAMING_FINISHED_EVENT;
    static const long ID_PROGRAMING_STATUS_EVENT;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    static const long ID_GAUGE1;
    static const long ID_CHOICE2;
    static const long ID_CHOICE1;

private:
    void OnbtnOpenClick(wxCommandEvent& event);
    void OnbtnStartProgrammingClick(wxCommandEvent& event);
    void OnAbortProgramming(wxCommandEvent& event);
    void OnbtnProgFPGAClick(wxCommandEvent& event);
    void OncmbDeviceSelect(wxCommandEvent& event);
    void OnProgramingStatusUpdate(wxCommandEvent& event);
    void OnProgramingFinished(wxCommandEvent& event);

protected:
    DECLARE_EVENT_TABLE()
};

#endif
