/**
@file FPGAcontrols_wxgui.h
@author Lime Microsystems
*/
#ifndef FPGA_CONTROLS_WXGUI_H
#define FPGA_CONTROLS_WXGUI_H

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/wx.h>
class wxStaticText;
class wxBitmapButton;
class wxButton;
class wxStaticBoxSizer;
class wxToggleButton;
class wxCheckBox;

#include "lime/LimeSuite.h"
#include "dataTypes.h"
#include <vector>
#include <atomic>
#include <thread>


class FPGAcontrols_wxgui: public wxFrame
{
public:
        FPGAcontrols_wxgui(wxWindow* parent,wxWindowID id=wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long styles = 0);
        virtual void Initialize(lms_device_t* dataPort);
        virtual ~FPGAcontrols_wxgui();
private:
        int UploadFile(const wxString &filename);
        int UploadFile(std::vector<int16_t> isamples, std::vector<int16_t> qsamples);
        wxButton* btnPlayWFM;
        wxButton* btnStopWFM;
        wxBitmapButton* btnOpenWFM;
        wxToggleButton* btnLoadOnetone;
        wxStaticText* txtFilename;
        wxStaticText* statusText;
        wxToggleButton* btnLoadCustom;
        wxToggleButton* btnLoadWCDMA;
        wxCheckBox* chkMIMO;
        static const long ID_BUTTON6;
        static const long ID_BUTTON7;
        static const long ID_BUTTON8;
        static const long ID_BITMAPBUTTON1;
        static const long ID_STATICTEXT2;
        static const long ID_STATICTEXT5;
        static const long ID_GAUGE1;
        static const long ID_BUTTON3;
        static const long ID_BUTTON4;
        wxChoice* cmbDevice;
        wxChoice* mode;
        void OnbtnUploadClick(wxCommandEvent& event);
        void OnbtnOpenFileClick(wxCommandEvent& event);
        void OnbtnMifClick(wxCommandEvent& event);
        void OnbtnHexClick(wxCommandEvent& event);
        void OnbtnPlayWFMClick(wxCommandEvent& event);
        void OnbtnStopWFMClick(wxCommandEvent& event);
        void OnbtnLoadOnetoneClick(wxCommandEvent& event);
        void OnbtnLoadWCDMAClick(wxCommandEvent& event);
        void OnbtnLoadCustomClick(wxCommandEvent& event);
        void OnModeChanged(wxCommandEvent& event);
        void OnStatus(wxCommandEvent& event);
        std::atomic<bool> terminateStream;
        lms_device_t* lmsControl;
        std::thread txThread;
        DECLARE_EVENT_TABLE()
};

#endif
