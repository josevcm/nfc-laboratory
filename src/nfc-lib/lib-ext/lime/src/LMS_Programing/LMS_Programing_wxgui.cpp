/**
@file   LMS_Programing_wxgui.cpp
@author Lime Microsystems
@brief  panel for uploading data to FPGA
*/
#include "LMS_Programing_wxgui.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/gauge.h>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>

#include "LMSBoards.h"

const long LMS_Programing_wxgui::ID_PROGRAMING_FINISHED_EVENT = wxNewId();
const long LMS_Programing_wxgui::ID_PROGRAMING_STATUS_EVENT = wxNewId();
const long LMS_Programing_wxgui::ID_BUTTON1 = wxNewId();
const long LMS_Programing_wxgui::ID_BUTTON2 = wxNewId();
const long LMS_Programing_wxgui::ID_GAUGE1 = wxNewId();
const long LMS_Programing_wxgui::ID_CHOICE2 = wxNewId();
const long LMS_Programing_wxgui::ID_CHOICE1 = wxNewId();

BEGIN_EVENT_TABLE(LMS_Programing_wxgui, wxFrame)
END_EVENT_TABLE()

LMS_Programing_wxgui::LMS_Programing_wxgui(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, int styles, wxString idname)
{
    mProgrammingInProgress.store(false);
    mAbortProgramming.store(false);
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;

    wxFrame::Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, _T("id"));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 5, 5);
    btnOpen = new wxButton(this, ID_BUTTON1, _T("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(btnOpen, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, wxID_ANY, _T("File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblFilename = new wxStaticText(this, wxID_ANY, _T("\?"), wxDefaultPosition, wxSize(400, -1), wxST_ELLIPSIZE_START, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(lblFilename, 1, wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer6, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    btnStartStop = new wxButton(this, ID_BUTTON2, _T("Program"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(btnStartStop, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer8->AddGrowableCol(0);
    lblProgressPercent = new wxStaticText(this, wxID_ANY, _T(""), wxDefaultPosition, wxSize(48, -1), 0, _T("ID_STATICTEXT5"));

    progressBar = new wxGauge(this, ID_GAUGE1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GAUGE1"));
    FlexGridSizer8->Add(progressBar, 1, wxEXPAND, 0);
    FlexGridSizer8->AddGrowableRow(0);
    FlexGridSizer2->Add(FlexGridSizer8, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer2->Add(lblProgressPercent, 1, wxEXPAND | wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 5);
    StaticText2 = new wxStaticText(this, wxID_ANY, _T("Programming mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer7->Add(StaticText2, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    cmbDevice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer7->Add(cmbDevice, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer7, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnOpenClick);
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnStartProgrammingClick);
    Connect(ID_CHOICE2, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&LMS_Programing_wxgui::OncmbDeviceSelect);
    Connect(ID_PROGRAMING_FINISHED_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&LMS_Programing_wxgui::OnProgramingFinished);
    Connect(ID_PROGRAMING_STATUS_EVENT, wxEVT_COMMAND_THREAD, (wxObjectEventFunction)&LMS_Programing_wxgui::OnProgramingStatusUpdate);
}

LMS_Programing_wxgui::~LMS_Programing_wxgui()
{
    //make sure the thread has stopped before destroying data
    if(mProgrammingInProgress.load() == true)
    {
        mAbortProgramming.store(true);
        mWorkerThread.join();
    }
}

void LMS_Programing_wxgui::OnbtnOpenClick(wxCommandEvent& event)
{
    wxString wildcards;
    wxString deviceSelection = cmbDevice->GetStringSelection();
    auto info = LMS_GetDeviceInfo(lmsControl);
    if (info)
    {
        if (strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDR)))
        {
            if (deviceSelection.find("FPGA") != wxString::npos)
                wildcards = "rbf(*.rbf)|*.rbf|All files(*.*)|*.*";
            else
                wildcards = "img(*.img)|*.img|All files(*.*)|*.*";
        }
        else if (strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI_V2)))
            wildcards = "bit(*.bit)|*.bit|All files(*.*)|*.*";
        else if (strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDRMINI)))
            wildcards = "rpd(*.rpd)|*.rpd|All files(*.*)|*.*";
        else if (strstr(info->deviceName, lime::GetDeviceName(lime::LMS_DEV_LIMESDR_QPCIE)))
            wildcards = "rbf(*.rbf)|*.rbf|All files(*.*)|*.*";
    }
    else if (deviceSelection.find("FPGA") != wxString::npos)
        wildcards = "rbf(*.rbf)|*.rbf|bin(*.bin)|*.bin|rpd(*.rpd)|*.rpd|img(*.img)|*.img|All files(*.*)|*.*";
    else
         wildcards = "img(*.img)|*.img|rbf(*.rbf)|*.rbf|bin(*.bin)|*.bin|rpd(*.rpd)|*.rpd|All files(*.*)|*.*";

    wxFileDialog dlg(this, _("Select file"), _(""), _(""), wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_CANCEL)
        return;

    lblFilename->SetLabel(dlg.GetPath());
}

void LMS_Programing_wxgui::OnbtnStartProgrammingClick(wxCommandEvent& event)
{
    //if needed load program data from file
    wxString deviceSelection = cmbDevice->GetStringSelection();

    if(
        (deviceSelection.find("Reset") == wxString::npos) &&
        (deviceSelection.find("Auto") == wxString::npos))
    {
        if (lblFilename->GetLabel().length() <= 1)
        {
            wxMessageBox(_("Program file not selected"), _("Warning"));
            return;
        }

        //using wxWidgets to read file, to support nonascii characters in path
        wxFFileInputStream fin(lblFilename->GetLabel());

        if(!fin.IsOk())
        {
            wxMessageBox(_("Error loading program file"), _("Error"));
            return;
        }

        fin.SeekI(0, wxFromEnd);
        unsigned int m_data_size = fin.TellI();

        mProgramData.resize(m_data_size, 0);
        fin.SeekI(0, wxFromStart);
        fin.Read(mProgramData.data(), m_data_size);
    }

    Disconnect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnStartProgrammingClick);
    btnOpen->Disable();
    btnStartStop->SetLabel(_("Abort"));

    mAbortProgramming.store(false);
    //run programming in separate thread, to prevent GUI freeze
    mWorkerThread = std::thread(&LMS_Programing_wxgui::DoProgramming, this);
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnAbortProgramming);
}

/** @brief Change programming modes according to selected device
*/
void LMS_Programing_wxgui::OncmbDeviceSelect(wxCommandEvent& event)
{
    wxString deviceSelection = cmbDevice->GetStringSelection();
    if(
        (deviceSelection.find("Reset") == wxString::npos) &&
        (deviceSelection.find("Auto") == wxString::npos))
        btnOpenEnb = true;
    else
        btnOpenEnb = false;
    btnOpen->Enable(btnOpenEnb);
    StaticText1->Enable(btnOpenEnb);
    lblFilename->Enable(btnOpenEnb);
}

void LMS_Programing_wxgui::OnProgramingFinished(wxCommandEvent& event)
{
    mWorkerThread.join();
    wxMessageBox(event.GetString(), _("INFO"), wxICON_INFORMATION | wxOK);
    btnOpen->Enable(btnOpenEnb);
    Disconnect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnAbortProgramming);
    Connect(btnStartStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LMS_Programing_wxgui::OnbtnStartProgrammingClick);
    btnStartStop->SetLabel(_("Program"));
}

void LMS_Programing_wxgui::OnAbortProgramming(wxCommandEvent& event)
{
    mAbortProgramming.store(true);
}

void LMS_Programing_wxgui::SetConnection(lms_device_t* port)
{
    lmsControl = port;
    if (lmsControl)
    {
        cmbDevice->Clear();
        lms_name_t modes[16];
        int count = LMS_GetProgramModes(lmsControl, modes);
        for (int i = 0; i < count; i ++)
            if (strstr(modes[i],"MCU") == nullptr)
                cmbDevice->Append(wxString(modes[i]));
        cmbDevice->SetSelection(0);
        wxCommandEvent evt;
        OncmbDeviceSelect(evt);
        Layout();
    }
}

LMS_Programing_wxgui* LMS_Programing_wxgui::obj_ptr=nullptr;
bool LMS_Programing_wxgui::OnProgrammingCallback(int bsent, int btotal, const char* progressMsg)
{
    wxCommandEvent evt;
    evt.SetEventObject(obj_ptr);
    evt.SetInt(100.0 * bsent / btotal); //round to int
    evt.SetString(wxString::From8BitData(progressMsg));
    evt.SetEventType(wxEVT_COMMAND_THREAD);
    evt.SetId(ID_PROGRAMING_STATUS_EVENT);
    wxPostEvent(obj_ptr, evt);
    return obj_ptr->mAbortProgramming.load();
}


void LMS_Programing_wxgui::DoProgramming()
{
    mProgrammingInProgress.store(true);
    obj_ptr = this;
    wxString device = cmbDevice->GetStringSelection();
    int status = LMS_Program(lmsControl, mProgramData.data(), mProgramData.size(), device.c_str(), OnProgrammingCallback);
    wxCommandEvent evt;
    evt.SetEventObject(this);
    evt.SetId(ID_PROGRAMING_FINISHED_EVENT);
    evt.SetEventType(wxEVT_COMMAND_THREAD);
    evt.SetString(status == 0 ? _("Programming Completed!") : _("Programming failed!\n"));

    wxPostEvent(this, evt);
    mProgrammingInProgress.store(false);
    return;
}

/** Updates GUI elements with programming status
*/
void LMS_Programing_wxgui::OnProgramingStatusUpdate(wxCommandEvent& event)
{
    progressBar->SetValue(event.GetInt());
    lblProgressPercent->SetLabel(event.GetString());
}
