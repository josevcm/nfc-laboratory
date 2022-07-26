/**
@file 	ADF4002_wxgui.h
@author	Lime Microsystems
@brief 	ADF4002 configuring panel
*/

#ifndef ADF4002_wxgui_H
#define ADF4002_wxgui_H

//(*Headers(ADF4002_wxgui)
#include <wx/frame.h>
#include <wx/wx.h>
#include "lime/LimeSuite.h"
class wxSpinEvent;
class wxTextCtrl;
class wxComboBox;
class wxRadioBox;
class wxStaticText;
class wxFlexGridSizer;
class wxSpinCtrl;
class wxButton;
class wxStaticBoxSizer;
//*)

namespace lime{
class ADF4002;

}

class ADF4002_wxgui: public wxFrame
{
public:

    ADF4002_wxgui(wxWindow* parent,wxWindowID id=wxID_ANY, const wxString &title=_(""), const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, int styles=wxDEFAULT_FRAME_STYLE, wxString idname="");
    void Initialize(lms_device_t* pSerPort);
    virtual ~ADF4002_wxgui();

    void SetGuiDefaults();

    //(*Declarations(ADF4002_wxgui)
    wxStaticText* StaticText10;
    wxButton* btnUpload;
    wxStaticText* StaticText9;
    wxRadioBox* rgrPDP_i;
    wxComboBox* cmbCS1_f;
    wxComboBox* cmbCS2_f;
    wxStaticText* StaticText13;
    wxStaticText* StaticText2;
    wxSpinCtrl* spinNCnt;
    wxStaticText* StaticText14;
    wxComboBox* cmbFL_i;
    wxStaticText* StaticText6;
    wxStaticText* StaticText19;
    wxStaticText* StaticText8;
    wxStaticText* StaticText11;
    wxComboBox* cmbLDP;
    wxStaticText* StaticText18;
    wxRadioBox* rgrPD1_f;
    wxButton* btnCalcSend;
    wxStaticText* StaticText1;
    wxRadioBox* rgrCPS_f;
    wxComboBox* cmbMOC_f;
    wxComboBox* cmbTC_i;
    wxStaticText* StaticText3;
    wxTextCtrl* txtFref;
    wxRadioBox* rgrPDP_f;
    wxComboBox* cmbFL_f;
    wxSpinCtrl* spinRCnt;
    wxComboBox* cmbMOC_i;
    wxRadioBox* rgrCPS_i;
    wxComboBox* cmbCS1_i;
    wxRadioBox* rgrPD2_f;
    wxStaticText* StaticText5;
    wxStaticText* StaticText7;
    wxComboBox* cmbCPG;
    wxStaticText* StaticText15;
    wxStaticText* StaticText12;
    wxRadioBox* rgrPD1_i;
    wxStaticText* lblFcomp;
    wxRadioBox* rgrCR_f;
    wxTextCtrl* txtFvco;
    wxStaticText* StaticText17;
    wxStaticText* StaticText4;
    wxComboBox* cmbTC_f;
    wxRadioBox* rgrCR_i;
    wxStaticText* lblFvco;
    wxStaticText* StaticText16;
    wxRadioBox* rgrPD2_i;
    wxComboBox* cmbCS2_i;
    wxComboBox* cmbABW;
    //*)

protected:

    //(*Identifiers(ADF4002_wxgui)
    static const long ID_STATICTEXT1;
    static const long ID_STATICTEXT2;
    static const long ID_STATICTEXT3;
    static const long ID_COMBOBOX1;
    static const long ID_COMBOBOX2;
    static const long ID_SPINCTRL1;
    static const long ID_STATICTEXT4;
    static const long ID_STATICTEXT5;
    static const long ID_COMBOBOX3;
    static const long ID_SPINCTRL2;
    static const long ID_STATICTEXT6;
    static const long ID_STATICTEXT7;
    static const long ID_COMBOBOX4;
    static const long ID_COMBOBOX5;
    static const long ID_STATICTEXT8;
    static const long ID_STATICTEXT9;
    static const long ID_COMBOBOX6;
    static const long ID_COMBOBOX7;
    static const long ID_STATICTEXT10;
    static const long ID_COMBOBOX8;
    static const long ID_RADIOBOX1;
    static const long ID_RADIOBOX2;
    static const long ID_RADIOBOX3;
    static const long ID_RADIOBOX4;
    static const long ID_RADIOBOX5;
    static const long ID_STATICTEXT17;
    static const long ID_STATICTEXT18;
    static const long ID_COMBOBOX9;
    static const long ID_COMBOBOX10;
    static const long ID_STATICTEXT19;
    static const long ID_STATICTEXT20;
    static const long ID_COMBOBOX11;
    static const long ID_COMBOBOX12;
    static const long ID_STATICTEXT21;
    static const long ID_COMBOBOX13;
    static const long ID_RADIOBOX6;
    static const long ID_RADIOBOX7;
    static const long ID_RADIOBOX8;
    static const long ID_RADIOBOX9;
    static const long ID_RADIOBOX10;
    static const long ID_STATICTEXT11;
    static const long ID_STATICTEXT12;
    static const long ID_TEXTCTRL1;
    static const long ID_TEXTCTRL2;
    static const long ID_STATICTEXT13;
    static const long ID_STATICTEXT14;
    static const long ID_STATICTEXT15;
    static const long ID_STATICTEXT16;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    //*)

private:
    lime::ADF4002* m_pModule;
    lms_device_t* lmsControl;
    //(*Handlers(ADF4002_wxgui)
    void OnbtnCalcSendClick(wxCommandEvent& event);
    void OnbtnUploadClick(wxCommandEvent& event);
    //*)

protected:
    DECLARE_EVENT_TABLE()
};

#endif
