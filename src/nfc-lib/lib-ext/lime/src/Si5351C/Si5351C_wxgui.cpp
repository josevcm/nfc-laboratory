/**
@file	Si5351C_wxgui.cpp
@author	Lime Microsystems
@brief 	panel for configuring Si5351C
*/

#include "Si5351C_wxgui.h"
#include "Si5351C.h"
#include "lms7_device.h"
#include <LMSBoards.h>

//(*InternalHeaders(Si5351C_wxgui)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/filedlg.h>
#include <wx/wx.h>

using namespace lime;

//(*IdInit(Si5351C_wxgui)
const long Si5351C_wxgui::ID_BUTTON2 = wxNewId();
const long Si5351C_wxgui::ID_BUTTON4 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT1 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL1 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT14 = wxNewId();
const long Si5351C_wxgui::ID_BUTTON1 = wxNewId();
const long Si5351C_wxgui::ID_BUTTON5 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT2 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT11 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT12 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT13 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT3 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX1 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL2 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX9 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT4 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX2 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL3 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX10 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT5 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX3 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL4 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX11 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT6 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX4 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL5 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX12 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT7 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX5 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL6 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX13 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT8 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX6 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL7 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX14 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT9 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX7 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL8 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX15 = wxNewId();
const long Si5351C_wxgui::ID_STATICTEXT10 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX8 = wxNewId();
const long Si5351C_wxgui::ID_TEXTCTRL9 = wxNewId();
const long Si5351C_wxgui::ID_CHECKBOX16 = wxNewId();
const long Si5351C_wxgui::ID_BUTTON3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(Si5351C_wxgui, wxFrame)
//(*EventTable(Si5351C_wxgui)
//*)
END_EVENT_TABLE()
#include <vector>

Si5351C_wxgui::Si5351C_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, int styles, wxString idname)
{
    lmsControl = NULL;

    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, styles, _T("wxID_ANY"));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Si5351C"));
    btnLoadFile = new wxButton(this, ID_BUTTON2, _("Upload register map from file"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    StaticBoxSizer1->Add(btnLoadFile, 1, wxEXPAND, 5);
    btnResetToDefaults = new wxButton(this, ID_BUTTON4, _("Reset register map to default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    StaticBoxSizer1->Add(btnResetToDefaults, 1, wxEXPAND, 5);
    FlexGridSizer3->Add(StaticBoxSizer1, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 5, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("CLKIN Frequency (MHz)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxTOP | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtCLKIN_MHz = new wxTextCtrl(this, ID_TEXTCTRL1, _("30.72"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer2->Add(txtCLKIN_MHz, 1, wxTOP | wxEXPAND, 5);

    const wxString clksrcChoices[] = { _("XTAL"), _("CLKIN") };
    rgrClkSrc = new wxRadioBox(this, wxNewId(), _("PLL src"), wxDefaultPosition, wxDefaultSize, 2, clksrcChoices, 2, wxRA_SPECIFY_COLS, wxDefaultValidator, _T("ID_RADIOBOX1"));
    FlexGridSizer2->Add(rgrClkSrc, 1, wxALL | wxEXPAND, 0);

    const wxString xtalfreqChoices[] = { _("25 MHz"), _("27 MHz") };
    rgrXTALfreq = new wxRadioBox(this, wxNewId(), _("XTAL freq"), wxDefaultPosition, wxDefaultSize, 2, xtalfreqChoices, 2, wxRA_SPECIFY_COLS, wxDefaultValidator, _T("ID_RADIOBOX1"));
    FlexGridSizer2->Add(rgrXTALfreq, 1, wxALL | wxEXPAND, 0);

    FlexGridSizer3->Add(FlexGridSizer2, 1, wxLEFT | wxEXPAND, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    lblStatus = new wxStaticText(this, ID_STATICTEXT14, _("SYS_INIT:\t\? \t SYS_INIT_STKY:\t\?\nLOL_B:\t\?  \tLOL_B_STKY:\t\?\nLOL_A:\t\?  \tLOL_A_STKY:\t\?\nLOS:\t\?  \tLOS_STKY:\t\?"), wxDefaultPosition, wxSize(282, 59), 0, _T("ID_STATICTEXT14"));
    FlexGridSizer6->Add(lblStatus, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
    btnReadStatus = new wxButton(this, ID_BUTTON1, _("Read Status"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer5->Add(btnReadStatus, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    btnClearStatus = new wxButton(this, ID_BUTTON5, _("Clear"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer5->Add(btnClearStatus, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(FlexGridSizer5, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer6, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 5, 0, 0);
    FlexGridSizer9 = new wxFlexGridSizer(0, 4, 0, 0);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Pin Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer9->Add(StaticText2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Enable channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer9->Add(StaticText11, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Output frequency (MHz)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer9->Add(StaticText12, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Invert output"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer9->Add(StaticText13, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK0 = new wxStaticText(this, ID_STATICTEXT3, _("CLK0"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer9->Add(lblCLK0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK0 = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    chkEN_CLK0->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK0 = new wxTextCtrl(this, ID_TEXTCTRL2, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer9->Add(txtFreq_CLK0, 1, wxEXPAND, 5);
    chkInvert_CLK0 = new wxCheckBox(this, ID_CHECKBOX9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX9"));
    chkInvert_CLK0->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK1 = new wxStaticText(this, ID_STATICTEXT4, _("CLK1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer9->Add(lblCLK1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK1 = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    chkEN_CLK1->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK1 = new wxTextCtrl(this, ID_TEXTCTRL3, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    FlexGridSizer9->Add(txtFreq_CLK1, 1, wxEXPAND, 5);
    chkInvert_CLK1 = new wxCheckBox(this, ID_CHECKBOX10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX10"));
    chkInvert_CLK1->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK2 = new wxStaticText(this, ID_STATICTEXT5, _("CLK2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer9->Add(lblCLK2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK2 = new wxCheckBox(this, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    chkEN_CLK2->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK2 = new wxTextCtrl(this, ID_TEXTCTRL4, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    FlexGridSizer9->Add(txtFreq_CLK2, 1, wxEXPAND, 5);
    chkInvert_CLK2 = new wxCheckBox(this, ID_CHECKBOX11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX11"));
    chkInvert_CLK2->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK3 = new wxStaticText(this, ID_STATICTEXT6, _("CLK3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer9->Add(lblCLK3, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK3 = new wxCheckBox(this, ID_CHECKBOX4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    chkEN_CLK3->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK3, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK3 = new wxTextCtrl(this, ID_TEXTCTRL5, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
    FlexGridSizer9->Add(txtFreq_CLK3, 1, wxEXPAND, 5);
    chkInvert_CLK3 = new wxCheckBox(this, ID_CHECKBOX12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX12"));
    chkInvert_CLK3->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK3, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK4 = new wxStaticText(this, ID_STATICTEXT7, _("CLK4"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer9->Add(lblCLK4, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK4 = new wxCheckBox(this, ID_CHECKBOX5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    chkEN_CLK4->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK4, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK4 = new wxTextCtrl(this, ID_TEXTCTRL6, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL6"));
    FlexGridSizer9->Add(txtFreq_CLK4, 1, wxEXPAND, 5);
    chkInvert_CLK4 = new wxCheckBox(this, ID_CHECKBOX13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX13"));
    chkInvert_CLK4->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK4, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK5 = new wxStaticText(this, ID_STATICTEXT8, _("CLK5"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer9->Add(lblCLK5, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK5 = new wxCheckBox(this, ID_CHECKBOX6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
    chkEN_CLK5->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK5, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK5 = new wxTextCtrl(this, ID_TEXTCTRL7, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL7"));
    FlexGridSizer9->Add(txtFreq_CLK5, 1, wxEXPAND, 5);
    chkInvert_CLK5 = new wxCheckBox(this, ID_CHECKBOX14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX14"));
    chkInvert_CLK5->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK5, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK6 = new wxStaticText(this, ID_STATICTEXT9, _("CLK6"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer9->Add(lblCLK6, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK6 = new wxCheckBox(this, ID_CHECKBOX7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
    chkEN_CLK6->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK6, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK6 = new wxTextCtrl(this, ID_TEXTCTRL8, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL8"));
    FlexGridSizer9->Add(txtFreq_CLK6, 1, wxEXPAND, 5);
    chkInvert_CLK6 = new wxCheckBox(this, ID_CHECKBOX15, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX15"));
    chkInvert_CLK6->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK6, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    lblCLK7 = new wxStaticText(this, ID_STATICTEXT10, _("CLK7"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer9->Add(lblCLK7, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    chkEN_CLK7 = new wxCheckBox(this, ID_CHECKBOX8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX8"));
    chkEN_CLK7->SetValue(true);
    FlexGridSizer9->Add(chkEN_CLK7, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    txtFreq_CLK7 = new wxTextCtrl(this, ID_TEXTCTRL9, _("27.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL9"));
    FlexGridSizer9->Add(txtFreq_CLK7, 1, wxEXPAND, 5);
    chkInvert_CLK7 = new wxCheckBox(this, ID_CHECKBOX16, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX16"));
    chkInvert_CLK7->SetValue(false);
    FlexGridSizer9->Add(chkInvert_CLK7, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer9, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    btnConfigure = new wxButton(this, ID_BUTTON3, _("Configure Clocks"), wxDefaultPosition, wxSize(113, 61), 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer4->Add(btnConfigure, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL | wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&Si5351C_wxgui::OnbtnLoadFileClick);
    Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&Si5351C_wxgui::OnbtnResetToDefaultsClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&Si5351C_wxgui::OnbtnReadStatusClick);
    Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&Si5351C_wxgui::OnbtnClearStatusClick);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&Si5351C_wxgui::OnbtnConfigureClockClick);
}

void Si5351C_wxgui::Initialize(lms_device_t* pModule)
{
    lmsControl = pModule;
}

void Si5351C_wxgui::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
}

Si5351C_wxgui::~Si5351C_wxgui()
{
}

void Si5351C_wxgui::OnbtnLoadFileClick(wxCommandEvent& event)
{
   wxFileDialog openFileDialog(this, _("Open project file"), "", "", "Register Files (*.h)|*.h|Text files (*.txt)|*.TXT", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    lime::Si5351C obj;
    obj.Initialize(((LMS7_Device*)lmsControl)->GetConnection());
    obj.LoadRegValuesFromFile(std::string(openFileDialog.GetPath().ToStdString()));
    if (obj.UploadConfiguration()!=0)
        wxMessageBox(wxString::Format(_("Configuration failed"), _("Error")));
}

void Si5351C_wxgui::OnbtnConfigureClockClick(wxCommandEvent& event)
{
    double refFreq;
    if (rgrClkSrc->GetSelection() == 0)
        refFreq = (rgrXTALfreq->GetSelection() == 0 ? 25 : 27);
    else
        txtCLKIN_MHz->GetValue().ToDouble(&refFreq);
    double freq[8];
    double clkin = refFreq * 1000000;
    txtFreq_CLK0->GetValue().ToDouble(&freq[0]);
    txtFreq_CLK1->GetValue().ToDouble(&freq[1]);
    txtFreq_CLK2->GetValue().ToDouble(&freq[2]);
    txtFreq_CLK3->GetValue().ToDouble(&freq[3]);
    txtFreq_CLK4->GetValue().ToDouble(&freq[4]);
    txtFreq_CLK5->GetValue().ToDouble(&freq[5]);
    txtFreq_CLK6->GetValue().ToDouble(&freq[6]);
    txtFreq_CLK7->GetValue().ToDouble(&freq[7]);

    for (int i = 0; i < 8 ; i++)
        freq[i] *= 1e6;

    if (!chkEN_CLK0->GetValue()) freq[0]  = 0;
    if (!chkEN_CLK1->GetValue()) freq[1]  = 0;
    if (!chkEN_CLK2->GetValue()) freq[2]  = 0;
    if (!chkEN_CLK3->GetValue()) freq[3]  = 0;
    if (!chkEN_CLK4->GetValue()) freq[4]  = 0;
    if (!chkEN_CLK5->GetValue()) freq[5]  = 0;
    if (!chkEN_CLK6->GetValue()) freq[6]  = 0;
    if (!chkEN_CLK7->GetValue()) freq[7]  = 0;

    if (chkInvert_CLK0->GetValue()) freq[0] *= -1;
    if (chkInvert_CLK1->GetValue()) freq[1] *= -1;
    if (chkInvert_CLK2->GetValue()) freq[2] *= -1;
    if (chkInvert_CLK3->GetValue()) freq[3] *= -1;
    if (chkInvert_CLK4->GetValue()) freq[4] *= -1;
    if (chkInvert_CLK5->GetValue()) freq[5] *= -1;
    if (chkInvert_CLK6->GetValue()) freq[6] *= -1;
    if (chkInvert_CLK7->GetValue()) freq[7] *= -1;

    lime::Si5351C obj;
    obj.Initialize(((LMS7_Device*)lmsControl)->GetConnection());

    obj.SetPLL(0,clkin,rgrClkSrc->GetSelection());
    obj.SetPLL(1,clkin,rgrClkSrc->GetSelection());

    for (int i = 0; i < 8;i++)
    {
        unsigned clock = abs(freq[i]);
        obj.SetClock(i,clock,clock!=0,freq[i]<0);
    }

    if (obj.ConfigureClocks()!=0 || obj.UploadConfiguration()!=0)
       wxMessageBox(wxString::Format(_("Configuration failed"), _("Error")));
}

void Si5351C_wxgui::OnbtnResetToDefaultsClick(wxCommandEvent& event)
{
    lime::Si5351C obj;
    obj.Initialize(((LMS7_Device*)lmsControl)->GetConnection());
    obj.Reset();
    obj.UploadConfiguration();
}

void Si5351C_wxgui::ModifyClocksGUI(const std::string &board)
{
    for(int i=0; i<8; ++i)
        ClockEnable(i, true);
    if(board == GetDeviceName(LMS_DEV_STREAM))
    {
        lblCLK0->SetLabel(_("CLK0 - CLK_IN"));
        lblCLK1->SetLabel(_("CLK1"));
        ClockEnable(1, false);
        lblCLK2->SetLabel(_("CLK2 - FCLK2"));
        lblCLK3->SetLabel(_("CLK3"));
        ClockEnable(3, false);
        lblCLK4->SetLabel(_("CLK4 - FCLK1"));
        lblCLK5->SetLabel(_("CLK5"));
        ClockEnable(5, false);
        lblCLK6->SetLabel(_("CLK6 - CLK_FPGA0"));
        lblCLK7->SetLabel(_("CLK7 - CLK_FPGA1"));
        rgrClkSrc->SetSelection(1);
    }
    else
    {
        lblCLK0->SetLabel(_("CLK0"));
        lblCLK1->SetLabel(_("CLK1"));
        lblCLK2->SetLabel(_("CLK2"));
        lblCLK3->SetLabel(_("CLK3"));
        lblCLK4->SetLabel(_("CLK4"));
        lblCLK5->SetLabel(_("CLK5"));
        lblCLK6->SetLabel(_("CLK6"));
        lblCLK7->SetLabel(_("CLK7"));
        rgrClkSrc->SetSelection(0);
    }
    Layout();
}

void Si5351C_wxgui::ClockEnable(unsigned int i, bool enabled)
{
    switch(i)
    {
    case 0:
        lblCLK0->Enable(enabled);
        chkEN_CLK0->Enable(enabled);
        txtFreq_CLK0->Enable(enabled);
        chkInvert_CLK0->Enable(enabled);
        break;
    case 1:
        lblCLK1->Enable(enabled);
        chkEN_CLK1->Enable(enabled);
        txtFreq_CLK1->Enable(enabled);
        chkInvert_CLK1->Enable(enabled);
        break;
    case 2:
        lblCLK2->Enable(enabled);
        chkEN_CLK2->Enable(enabled);
        txtFreq_CLK2->Enable(enabled);
        chkInvert_CLK2->Enable(enabled);
        break;
    case 3:
        lblCLK3->Enable(enabled);
        chkEN_CLK3->Enable(enabled);
        txtFreq_CLK3->Enable(enabled);
        chkInvert_CLK3->Enable(enabled);
        break;
    case 4:
        lblCLK4->Enable(enabled);
        chkEN_CLK4->Enable(enabled);
        txtFreq_CLK4->Enable(enabled);
        chkInvert_CLK4->Enable(enabled);
        break;
    case 5:
        lblCLK5->Enable(enabled);
        chkEN_CLK5->Enable(enabled);
        txtFreq_CLK5->Enable(enabled);
        chkInvert_CLK5->Enable(enabled);
        break;
    case 6:
        lblCLK6->Enable(enabled);
        chkEN_CLK6->Enable(enabled);
        txtFreq_CLK6->Enable(enabled);
        chkInvert_CLK6->Enable(enabled);
        break;
    case 7:
        lblCLK7->Enable(enabled);
        chkEN_CLK7->Enable(enabled);
        txtFreq_CLK7->Enable(enabled);
        chkInvert_CLK7->Enable(enabled);
        break;
    default:
        break;
    }
}

void Si5351C_wxgui::OnbtnReadStatusClick(wxCommandEvent& event)
{
    lime::Si5351C obj;
    obj.Initialize(((LMS7_Device*)lmsControl)->GetConnection());
    lime::Si5351C::StatusBits stat = obj.GetStatusBits();
    wxString text = wxString::Format("\
SYS_INIT:	%i 	 SYS_INIT_STKY:	%i\n\
LOL_B:	%i  	LOL_B_STKY:	%i\n\
LOL_A:	%i  	LOL_A_STKY:	%i\n\
LOS:	%i  	LOS_STKY:	%i", stat.sys_init, stat.sys_init_stky, stat.lol_b, stat.lol_b_stky, stat.lol_a, stat.lol_a_stky, stat.los, stat.los_stky);
lblStatus->SetLabel(text);
}

void Si5351C_wxgui::OnbtnClearStatusClick(wxCommandEvent& event)
{
    lime::Si5351C obj;
    obj.Initialize(((LMS7_Device*)lmsControl)->GetConnection());
    obj.ClearStatus();
    wxString text = wxString::Format("\
SYS_INIT:	%i 	 SYS_INIT_STKY:	%i\n\
LOL_B:	%i  	LOL_B_STKY:	%i\n\
LOL_A:	%i  	LOL_A_STKY:	%i\n\
LOS:	%i  	LOS_STKY:	%i", 0,0,0,0,0,0,0,0);
    lblStatus->SetLabel(text);
}
