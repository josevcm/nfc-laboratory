/**
@file 	FPGAcontrols_wxgui.cpp
@author Lime Microsystems
@brief 	panel for uploading WFM files to Stream board
*/

#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/gauge.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "wx/checkbox.h"
#include "FPGAcontrols_wxgui.h"
#include <vector>
#include <fstream>
#include <wx/ffile.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <iostream>
#include "WFM_wcdma.h"
#include "Logger.h"

using std::vector;
using lime::complex16_t;

const long FPGAcontrols_wxgui::ID_BUTTON6 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON7 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON8 = wxNewId();
const long FPGAcontrols_wxgui::ID_BITMAPBUTTON1 = wxNewId();
const long FPGAcontrols_wxgui::ID_STATICTEXT2 = wxNewId();
const long FPGAcontrols_wxgui::ID_STATICTEXT5 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON3 = wxNewId();
const long FPGAcontrols_wxgui::ID_BUTTON4 = wxNewId();

wxDEFINE_EVENT(UPDATE_STATUS, wxCommandEvent);

BEGIN_EVENT_TABLE(FPGAcontrols_wxgui, wxFrame)
END_EVENT_TABLE()

FPGAcontrols_wxgui::FPGAcontrols_wxgui(wxWindow* parent,wxWindowID id,const wxString &title, const wxPoint& pos,const wxSize& size, long styles) :
    terminateStream(true),
    lmsControl(nullptr)
{
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, title, wxDefaultPosition, wxDefaultSize, styles, title);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 5);
    FlexGridSizer1->AddGrowableCol(0);
    cmbDevice = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE"));
    cmbDevice->Append(_T("LMS1"));
    cmbDevice->Append(_T("LMS2"));
    cmbDevice->Append(_T("ADC/DAC"));
    cmbDevice->SetSelection(0);
    FlexGridSizer1->Add(cmbDevice, 1, wxEXPAND, 5);
    mode = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE"));

    FlexGridSizer1->Add(mode, 1, wxEXPAND, 5);

    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("WFM loader"));
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 5, 0);
    FlexGridSizer6->AddGrowableCol(0);
    FlexGridSizer8 = new wxFlexGridSizer(0, 5, 0, 5);
    btnLoadOnetone = new wxToggleButton(this, ID_BUTTON6, _T("Onetone"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    btnLoadOnetone->SetToolTip(_T("Loads single tone waveform"));
    FlexGridSizer8->Add(btnLoadOnetone, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    btnLoadWCDMA = new wxToggleButton(this, ID_BUTTON7, _T("W-CDMA"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    btnLoadWCDMA->SetToolTip(_T("Loads WCDMA waveform"));
    FlexGridSizer8->Add(btnLoadWCDMA, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    btnLoadCustom = new wxToggleButton(this, ID_BUTTON8, _T("Custom"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    btnLoadCustom->SetToolTip(_T("Loads user selected custom file"));
    FlexGridSizer8->Add(btnLoadCustom, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    btnOpenWFM = new wxBitmapButton(this, ID_BITMAPBUTTON1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_OPEN")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    btnOpenWFM->SetToolTip("The file should contain interleaved 16bit I + 16bit Q sample values (IQIQIQ....)");
    FlexGridSizer8->Add(btnOpenWFM, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    txtFilename = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_START, _T("ID_STATICTEXT2"));
    FlexGridSizer8->Add(txtFilename, 1, wxALL|wxEXPAND, 5);
    chkMIMO = new wxCheckBox(this, wxNewId(), _("MIMO"));
    FlexGridSizer8->Add(chkMIMO, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_TOP, 5);
    FlexGridSizer6->Add(FlexGridSizer8, 1, wxEXPAND, 5);

    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 5);
    btnPlayWFM = new wxButton(this, ID_BUTTON3, _T("Play >"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer2->Add(btnPlayWFM, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    btnStopWFM = new wxButton(this, ID_BUTTON4, _T("Stop ||"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer2->Add(btnStopWFM, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer6->Add(FlexGridSizer2, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    StaticBoxSizer3->Add(FlexGridSizer6, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer1->Add(StaticBoxSizer3, 1, wxEXPAND | wxLEFT, 5);

    statusText = new wxStaticText(this, ID_STATICTEXT5, _T(""), wxDefaultPosition, wxSize(-1,-1), 0, _T("ID_STATICTEXT5"));
    FlexGridSizer1->Add(statusText, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Bind(UPDATE_STATUS, &FPGAcontrols_wxgui::OnStatus, this);
    mode->Connect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(FPGAcontrols_wxgui::OnModeChanged), NULL, this);
    Connect(ID_BUTTON6, wxEVT_TOGGLEBUTTON, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnLoadOnetoneClick);
    Connect(ID_BUTTON7, wxEVT_TOGGLEBUTTON, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnLoadWCDMAClick);
    Connect(ID_BUTTON8, wxEVT_TOGGLEBUTTON, (wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnLoadCustomClick);
    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnOpenFileClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnPlayWFMClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPGAcontrols_wxgui::OnbtnStopWFMClick);
}

void FPGAcontrols_wxgui::Initialize(lms_device_t* dataPort)
{
    lmsControl = dataPort;
    int numCh = LMS_GetNumChannels(lmsControl,LMS_CH_TX);
    if ( numCh > 2)
        cmbDevice->Show();
    else
        cmbDevice->Hide();

    mode->Clear();
    mode->SetSelection(mode->Append(_T("Stream Mode")));
    if (numCh != 1)
        mode->SetSelection(mode->Append(_T("WFM Mode")));
    wxCommandEvent evt;
    OnModeChanged(evt);
    Layout();
}

FPGAcontrols_wxgui::~FPGAcontrols_wxgui()
{
    if (txThread.joinable())
    {
        terminateStream.store(true, std::memory_order_relaxed);
        txThread.join();
    }
}

/** @brief Reads WFM file and outputs iq sample pairs
    @param filename source file
    @param iSamples I samples destination vector
    @param qSamples Q samples destination vector
    @return number of samples returned, -1 file not found
*/
int ReadWFM(const wxString filename, std::vector<int16_t> &iSamples, std::vector<int16_t> &qSamples)
{
	if (filename.length() <= 1)
		return 0;
    wxFFile fpin(filename, "rb");
    unsigned char inputBuffer[4];
    unsigned char c1, c2, c3, c4; // To read Agilent Signal Studio file
    double iin, qin; // IQ inputs
    int iint, qint;	// IQ integer versions
    int cnt = 0;

    if( fpin.IsOpened() == false)
    {
#ifndef NDEBUG
        lime::debug("WFM reading: Input file can not be opened (%s)", filename.mb_str().data());
#endif
        return -1;
    }

    while( fpin.Read(inputBuffer, 4) == 4 )
    {
        c1 = inputBuffer[0];
        c2 = inputBuffer[1];
        c3 = inputBuffer[2];
        c4 = inputBuffer[3];

        cnt++;

        c1 &= 0xFF;
        c2 &= 0xFF;
        c3 &= 0xFF;
        c4 &= 0xFF;

        if( c1&0x80 ) iin = (double)(-1*(1<<15) + ((c1&0x7F)<<8) + c2);
        else iin = (double)(((c1&0x7F)<<8) + c2);

        if( c3&0x80 ) qin = (double)(-1*(1<<15) + ((c3&0x7F)<<8) + c4);
        else qin = (double)(((c3&0x7F)<<8) + c4);

        iint = (int)(iin);
        qint = (int)(qin);

        //convert from 16 bit to 12 bit values
        //iint = iint >> 4;
        //qint = qint >> 4;

        iSamples.push_back(iint);
        qSamples.push_back(qint);
    };
    return iSamples.size();
}

void FPGAcontrols_wxgui::OnbtnOpenFileClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Open file"), _(""), _(""), _("wfm (*.wfm)|*.wfm"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    txtFilename->SetLabelText( dlg.GetPath());
    Layout();
}

void FPGAcontrols_wxgui::OnbtnPlayWFMClick(wxCommandEvent& event)
{
    LMS_EnableTxWFM(lmsControl, cmbDevice->GetSelection()*2, true);
    btnPlayWFM->Enable(false);
    btnStopWFM->Enable(true);
    statusText->SetLabel("Status: Running");
}

void FPGAcontrols_wxgui::OnbtnStopWFMClick(wxCommandEvent& event)
{
    if (txThread.joinable())
    {
        terminateStream.store(true, std::memory_order_relaxed);
        txThread.join();
    }
    LMS_EnableTxWFM(lmsControl, cmbDevice->GetSelection()*2, false);
    statusText->SetLabel("Status: Stopped");
    if (mode->GetSelection())
        btnPlayWFM->Enable(true);
    btnStopWFM->Enable(false);
}

int FPGAcontrols_wxgui::UploadFile(const wxString &filename)
{
    if (!lmsControl)
    {
        wxMessageBox(_("Device not connected"), _("Error"));
        return -2;
    }

    if (filename.length() == 0)
    {
        wxMessageBox(_("File not selected"), _("Error"));
        return -1;
    }
    vector<int16_t> isamples;
    vector<int16_t> qsamples;
    isamples.clear();
    qsamples.clear();
    if( ReadWFM(filename, isamples, qsamples) < 0)
    {
        wxMessageBox(_("File not found ") + filename, _("Error"));
        return -1;
    }
    return UploadFile(isamples, qsamples);
}

int FPGAcontrols_wxgui::UploadFile(std::vector<int16_t> isamples, std::vector<int16_t> qsamples)
{
    if (!lmsControl)
    {
        wxMessageBox(_("Device not connected"), _("Error"));
        return -2;
    }
    btnPlayWFM->Enable(false);
    btnStopWFM->Enable(false);

    const uint8_t chCount = 2;
    bool MIMO = chkMIMO->IsChecked();

    vector<vector<complex16_t>> buffers(chCount,vector<complex16_t>(isamples.size()));

    for(size_t i=0; i<isamples.size(); ++i)
    {
        for(int c=0; c<chCount; ++c)
        {
            if(c == 1 && !MIMO)
            {
                buffers[c][i].i = 0;
                buffers[c][i].q = 0;
            }
            else
            {
                buffers[c][i].i = isamples[i];
                buffers[c][i].q = qsamples[i];
            }
        }
    }

    void* src[chCount];
    for (int i = 0; i < chCount; i++)
        src[i] = buffers[i].data();

    int status = 0;

    auto StreamPlayer = [this](const vector<complex16_t> buffer) {
        wxCommandEvent* evt = new wxCommandEvent(UPDATE_STATUS);
        constexpr unsigned fifoSize = 4*1024*1024;
        const unsigned send_cnt = buffer.size() > fifoSize/2 ? fifoSize/2 : buffer.size();
        lms_stream_t tx_stream = {0, true, uint32_t(cmbDevice->GetSelection()*2), fifoSize, 0.5, lms_stream_t::LMS_FMT_I16};
        if (LMS_SetupStream(lmsControl, &tx_stream)!= 0)
        {
            evt = new wxCommandEvent(UPDATE_STATUS);
            evt->SetString("Status: Failure");
            this->QueueEvent(evt);
            return;
        }
        LMS_StartStream(&tx_stream);
        int ind = 0;
        evt->SetString("Status: Running");
        this->QueueEvent(evt);
        while (!terminateStream.load(std::memory_order_relaxed)){
            int cnt = (send_cnt > buffer.size()- ind) ? buffer.size()- ind : send_cnt;
            int ret = LMS_SendStream(&tx_stream, &buffer[ind], cnt, nullptr, 1000);
            if (ret <= 0)
                break;
            ind = (ind + ret)% buffer.size();
        }
        LMS_StopStream(&tx_stream);
        LMS_DestroyStream(lmsControl, &tx_stream);
        evt = new wxCommandEvent(UPDATE_STATUS);
        evt->SetString("Status: Stopped");
        this->QueueEvent(evt);
    };

    if (mode->GetSelection())
    {
        status = LMS_UploadWFM(lmsControl, (const void**)src, 2+cmbDevice->GetSelection()*2, isamples.size(), 1);
        LMS_EnableTxWFM(lmsControl, cmbDevice->GetSelection()*2, true);
        statusText->SetLabel("Status: Running");
    }
    else
    {
        if (txThread.joinable())
        {
            terminateStream.store(true, std::memory_order_relaxed);
            txThread.join();
        }
        terminateStream.store(false, std::memory_order_relaxed);
        txThread = std::thread(StreamPlayer, std::move(buffers[0]));
    }
    btnPlayWFM->Enable(false);
    btnStopWFM->Enable(true);

    if (status != 0)
    {
        statusText->SetLabel("Status: Failure");
        wxMessageBox(_("Failed to upload WFM file"), _("Error"));
        return -3;
    }
    else
        return 0;
}

void FPGAcontrols_wxgui::OnbtnLoadOnetoneClick(wxCommandEvent& event)
{
    const int samplesPerPeriod = 64;
    vector<int16_t> isamples;
    vector<int16_t> qsamples;
    isamples.resize(samplesPerPeriod);
    qsamples.resize(samplesPerPeriod);
    for (int i = 0; i < samplesPerPeriod; i++)
    {
        const double PI  = 3.141592653589793238463;
        isamples[i] = 32767.0*cos(2.0*PI*i/samplesPerPeriod)+0.5;
        qsamples[i] = 32767.0*sin(2.0*PI*i/samplesPerPeriod)+0.5;;
    }

    if (UploadFile(isamples, qsamples) < 0)
        btnLoadOnetone->SetValue(0);
    else
    {
        btnLoadOnetone->SetValue(1);
        btnLoadWCDMA->SetValue(0);
        btnLoadCustom->SetValue(0);
    }
}

void FPGAcontrols_wxgui::OnbtnLoadWCDMAClick(wxCommandEvent& event)
{
    const int samplesCnt = wcdma_wfm_len/4;
    vector<int16_t> isamples;
    vector<int16_t> qsamples;
    isamples.resize(samplesCnt);
    qsamples.resize(samplesCnt);
    for (int i = 0; i < samplesCnt; i++)
    {
        //TODO: generate WCDMA waveform instead of storing wfm data in header
        isamples[i] = ((int16_t)wcdma_wfm[4*i]<<8)| (wcdma_wfm[4*i+1]);
        qsamples[i] = ((int16_t)wcdma_wfm[4*i+2]<<8)|(wcdma_wfm[4*i+3]);
    }

    if (UploadFile(isamples, qsamples) < 0)
        btnLoadWCDMA->SetValue(0);
    else
    {
        btnLoadOnetone->SetValue(0);
        btnLoadWCDMA->SetValue(1);
        btnLoadCustom->SetValue(0);
    }
}

void FPGAcontrols_wxgui::OnbtnLoadCustomClick(wxCommandEvent& event)
{
    if( UploadFile(txtFilename->GetLabel()) < 0)
    {
        btnLoadCustom->SetValue(0);
    }
    else
    {
        btnLoadOnetone->SetValue(0);
        btnLoadWCDMA->SetValue(0);
        btnLoadCustom->SetValue(1);
    }
}

void FPGAcontrols_wxgui::OnModeChanged(wxCommandEvent& event)
{
    if (mode->GetSelection())
    {
        uint16_t regAddr = 0x000D;
        uint16_t regValue = 0;
        LMS_WriteFPGAReg(lmsControl, 0xFFFF, 1<<(cmbDevice->GetSelection()));
        LMS_ReadFPGAReg(lmsControl, regAddr, &regValue);
        if (regValue&2)
        {
            btnPlayWFM->Enable(false);
            btnStopWFM->Enable(true);
            statusText->SetLabel("Status: Running");
        }
        else
        {
            btnPlayWFM->Enable(true);
            btnStopWFM->Enable(false);
            statusText->SetLabel("Status: Stopped");
        }
        chkMIMO->Enable(true);
    }
    else
    {
        btnPlayWFM->Enable(false);
        chkMIMO->Enable(false);
        btnStopWFM->Enable(txThread.joinable());
        statusText->SetLabel(txThread.joinable() ? "Status: Running" : "Status: Stopped");
    }
}

void FPGAcontrols_wxgui::OnStatus(wxCommandEvent& event)
{
    statusText->SetLabel(event.GetString());
}
