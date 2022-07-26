/**
@file 	Si5351C_wxgui.h
@author Lime Microsystems
*/

#ifndef Si5351C_wxgui_H
#define Si5351C_wxgui_H

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/frame.h>
#include <wx/button.h>
#include <wx/radiobox.h>
#include <string>
#include "lime/LimeSuite.h"

class Si5351C_wxgui: public wxFrame
{
public:

    Si5351C_wxgui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &title = _(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int styles = wxDEFAULT_FRAME_STYLE, wxString idname = "");
    void Initialize(lms_device_t* pModule);
    virtual ~Si5351C_wxgui();
    void ModifyClocksGUI(const std::string &board);

    //(*Declarations(Si5351C_wxgui)
    wxCheckBox* chkEN_CLK5;
    wxCheckBox* chkInvert_CLK6;
    wxTextCtrl* txtFreq_CLK7;
    wxTextCtrl* txtFreq_CLK1;
    wxCheckBox* chkEN_CLK3;
    wxCheckBox* chkEN_CLK4;
    wxCheckBox* chkInvert_CLK3;
    wxTextCtrl* txtFreq_CLK5;
    wxStaticText* StaticText13;
    wxStaticText* StaticText2;
    wxButton* btnLoadFile;
    wxTextCtrl* txtFreq_CLK0;
    wxStaticText* lblCLK6;
    wxStaticText* lblCLK7;
    wxStaticText* lblCLK0;
    wxButton* btnResetToDefaults;
    wxStaticText* lblCLK5;
    wxStaticText* StaticText11;
    wxStaticText* StaticText1;
    wxButton* btnConfigure;
    wxCheckBox* chkEN_CLK6;
    wxStaticText* lblCLK4;
    wxStaticText* lblCLK2;
    wxTextCtrl* txtFreq_CLK3;
    wxButton* btnReadStatus;
    wxTextCtrl* txtFreq_CLK6;
    wxButton* btnClearStatus;
    wxTextCtrl* txtFreq_CLK2;
    wxCheckBox* chkInvert_CLK2;
    wxCheckBox* chkInvert_CLK0;
    wxCheckBox* chkEN_CLK1;
    wxStaticText* StaticText12;
    wxStaticText* lblStatus;
    wxStaticText* lblCLK3;
    wxCheckBox* chkEN_CLK7;
    wxCheckBox* chkInvert_CLK1;
    wxTextCtrl* txtFreq_CLK4;
    wxTextCtrl* txtCLKIN_MHz;
    wxStaticText* lblCLK1;
    wxCheckBox* chkInvert_CLK7;
    wxCheckBox* chkInvert_CLK5;
    wxCheckBox* chkEN_CLK2;
    wxCheckBox* chkEN_CLK0;
    wxCheckBox* chkInvert_CLK4;
    wxRadioBox* rgrClkSrc;
    wxRadioBox* rgrXTALfreq;
    //*)

protected:
    void ClockEnable(unsigned int i, bool enabled);
    //(*Identifiers(Si5351C_wxgui)
    static const long ID_BUTTON2;
    static const long ID_BUTTON4;
    static const long ID_STATICTEXT1;
    static const long ID_TEXTCTRL1;
    static const long ID_STATICTEXT14;
    static const long ID_BUTTON1;
    static const long ID_BUTTON5;
    static const long ID_STATICTEXT2;
    static const long ID_STATICTEXT11;
    static const long ID_STATICTEXT12;
    static const long ID_STATICTEXT13;
    static const long ID_STATICTEXT3;
    static const long ID_CHECKBOX1;
    static const long ID_TEXTCTRL2;
    static const long ID_CHECKBOX9;
    static const long ID_STATICTEXT4;
    static const long ID_CHECKBOX2;
    static const long ID_TEXTCTRL3;
    static const long ID_CHECKBOX10;
    static const long ID_STATICTEXT5;
    static const long ID_CHECKBOX3;
    static const long ID_TEXTCTRL4;
    static const long ID_CHECKBOX11;
    static const long ID_STATICTEXT6;
    static const long ID_CHECKBOX4;
    static const long ID_TEXTCTRL5;
    static const long ID_CHECKBOX12;
    static const long ID_STATICTEXT7;
    static const long ID_CHECKBOX5;
    static const long ID_TEXTCTRL6;
    static const long ID_CHECKBOX13;
    static const long ID_STATICTEXT8;
    static const long ID_CHECKBOX6;
    static const long ID_TEXTCTRL7;
    static const long ID_CHECKBOX14;
    static const long ID_STATICTEXT9;
    static const long ID_CHECKBOX7;
    static const long ID_TEXTCTRL8;
    static const long ID_CHECKBOX15;
    static const long ID_STATICTEXT10;
    static const long ID_CHECKBOX8;
    static const long ID_TEXTCTRL9;
    static const long ID_CHECKBOX16;
    static const long ID_BUTTON3;
    //*)

private:
    //(*Handlers(Si5351C_wxgui)
    void OnbtnLoadFileClick(wxCommandEvent& event);
    void OnbtnConfigureClockClick(wxCommandEvent& event);
    void OnbtnResetToDefaultsClick(wxCommandEvent& event);
    void OnButton1Click(wxCommandEvent& event);
    void OnbtnReadStatusClick(wxCommandEvent& event);
    void OnbtnClearStatusClick(wxCommandEvent& event);
    bool LoadRegValuesFromFile(std::string FName);
    unsigned char m_newConfiguration[255];
    lms_device_t* lmsControl;
    //*)

protected:

    void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

    DECLARE_EVENT_TABLE()
};

#endif
