#include "pnluLimeSDR.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"
#include "pnlGPIO.h"

using namespace std;

BEGIN_EVENT_TABLE(pnluLimeSDR, wxPanel)
END_EVENT_TABLE()

pnluLimeSDR::pnluLimeSDR(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    SetSizer(mainSizer);
    wxFlexGridSizer* lbSizer = new wxFlexGridSizer(0, 2, 5, 5);
    lbSizer->Add(new wxStaticText(this, wxID_ANY, _("RX RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString rxChoices;
    rxChoices.push_back(_("LNAH"));
    rxChoices.push_back(_("LNAW"));
    cmbRxPath = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, rxChoices, 1);
    cmbRxPath->SetSelection(0);
    Connect(cmbRxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), NULL, this);
    lbSizer->Add(cmbRxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    
    lbSizer->Add(new wxStaticText(this, wxID_ANY, _("TX RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString txChoices;
    txChoices.push_back(_("Band 1"));
    txChoices.push_back(_("Band 2"));
    cmbTxPath = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, txChoices, 1);
    cmbTxPath->SetSelection(0);
    Connect(cmbTxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), NULL, this);
    lbSizer->Add(cmbTxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    lbSizer->Add(new wxStaticText(this, wxID_ANY, _("Loopback:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    txtLB = new wxStaticText(this, wxNewId(), _("TX Band 1 -> RX LNAH"));
    lbSizer->Add(txtLB, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    
    chkTxLBSH = new wxCheckBox(this, wxNewId(), _("Loopback shunt"));
    Connect(chkTxLBSH->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), NULL, this);
    lbSizer->Add(chkTxLBSH, 1, wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chkTxLBAT = new wxCheckBox(this, wxNewId(), _("Loopback attenuator"));
    Connect(chkTxLBAT->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), NULL, this);
    lbSizer->Add(chkTxLBAT, 1, wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    mainSizer->Add(lbSizer, 1, wxEXPAND | wxALL, 5);
    pnl_gpio = new pnlGPIO(this, wxNewId());
    mainSizer->Add(pnl_gpio, 1, wxEXPAND | wxALL, 5);
    Bind(READ_ALL_VALUES, &pnluLimeSDR::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnluLimeSDR::OnLoopbackChange, this, this->GetId());
}

void pnluLimeSDR::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    pnl_gpio->Initialize(lmsControl);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();
}

pnluLimeSDR::~pnluLimeSDR()
{
    chkTxLBSH->Disconnect(wxEVT_CHECKBOX, chkTxLBSH->GetId(), wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), 0, this);
    chkTxLBAT->Disconnect(wxEVT_CHECKBOX, chkTxLBAT->GetId(), wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), 0, this);
    cmbRxPath->Disconnect(wxEVT_CHOICE, cmbRxPath->GetId(), wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), 0, this);
    cmbTxPath->Disconnect(wxEVT_CHOICE, cmbTxPath->GetId(), wxCommandEventHandler(pnluLimeSDR::OnLoopbackChange), 0, this);
}

void pnluLimeSDR::OnLoopbackChange(wxCommandEvent& event)
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    value |= chkTxLBSH->GetValue() << 2;
    value |= chkTxLBAT->GetValue() << 1;
    value |= cmbRxPath->GetSelection() == 1 ? 1<<9 : 1<<8;
    value |= cmbTxPath->GetSelection() == 1 ? 1<<13 : 1<<12;
    if(LMS_WriteFPGAReg(lmsControl,addr, value))
        wxMessageBox(_("Failed to write FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
    txtLB->SetLabel(wxString::Format(_("TX Band %c -> RX LNA%c"), ((value>>13)&1)?'1':'2',((value>>9)&1)?'H':'W'));
}

void pnluLimeSDR::UpdatePanel()
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    if(LMS_ReadFPGAReg(lmsControl,addr, &value))
    {
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    chkTxLBSH->SetValue((value >> 2) & 0x1);
    chkTxLBAT->SetValue((value >> 1) & 0x1);
    cmbRxPath->SetSelection((value >> 9) & 0x1);
    cmbTxPath->SetSelection((value >> 13) & 0x1);
    txtLB->SetLabel(wxString::Format(_("TX Band %c -> RX LNA%c"), ((value>>13)&1)?'1':'2',((value>>9)&1)?'H':'W'));
    pnl_gpio->UpdatePanel();
}

void pnluLimeSDR::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnluLimeSDR::OnWriteAll(wxCommandEvent &event)
{
    OnLoopbackChange(event);
    pnl_gpio->OnUsrGPIODirChange(event);
    pnl_gpio->OnUsrGPIOChange(event);
}
