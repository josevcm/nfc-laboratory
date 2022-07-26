#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>

#include "pnlQSpark.h"
#include <ciso646>
#include <map>

#include <FPGA_common.h>
#include <lms7_device.h>
#include "lms7suiteEvents.h"

using namespace lime;
using namespace std;

BEGIN_EVENT_TABLE(pnlQSpark,wxPanel)
END_EVENT_TABLE()

pnlQSpark::Register::Register()
    : address(0), msb(0), lsb(0), defaultValue(0)
{
}


pnlQSpark::Register::Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue)
    : address(address), msb(msb), lsb(lsb), defaultValue(defaultValue)
{
}

pnlQSpark::pnlQSpark(wxWindow* parent,wxWindowID id, const wxString &title, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, wxDefaultPosition, wxDefaultSize, style, _T("id"));
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 5, 5);

    SetSizer(mainSizer);
    {
        auto groupSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("External loopback controls") ), wxVERTICAL );
        {
            auto controlsSizer = new wxFlexGridSizer(0, 1, 5, 5);
            chkLB_1A = new wxCheckBox(this, wxNewId(), _("RF loopback ch.A"));
            chkLB_1A->SetToolTip(_("[RFLB_A_EN] External RF loopback TxBAND2->RxLNAH channel A"));
            Connect(chkLB_1A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_1A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkLB_1B = new wxCheckBox(this, wxNewId(), _("RF loopback ch.B"));
            chkLB_1B->SetToolTip(_("[RFLB_B_EN] External RF loopback TxBAND2->RxLNAH channel B"));
            Connect(chkLB_1B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_1B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

            chkSH_1A = new wxCheckBox(this, wxNewId(), _("Ch.A shunt"));
            chkSH_1A->SetToolTip(_("[TX1_2_LB_SH]"));
            Connect(chkSH_1A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_1A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkSH_1B = new wxCheckBox(this, wxNewId(), _("Ch.B shunt"));
            chkSH_1B->SetToolTip(_("[TX2_2_LB_SH]"));
            Connect(chkSH_1B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_1B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_1A = new wxCheckBox(this, wxNewId(), _("Ch.A attenuator"));
            chkAT_1A->SetToolTip(_("[TX1_2_LB_AT]"));
            Connect(chkAT_1A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_1A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_1B = new wxCheckBox(this, wxNewId(), _("Ch.B attenuator"));
            chkAT_1B->SetToolTip(_("[TX2_2_LB_AT]"));
            Connect(chkAT_1B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_1B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            auto lms1sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LMS#1") ), wxVERTICAL );
            lms1sizer->Add(controlsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
            groupSizer->Add(lms1sizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
        }
        {
            auto controlsSizer = new wxFlexGridSizer(0, 1, 5, 5);
            chkLB_2A = new wxCheckBox(this, wxNewId(), _("RF loopback ch.A"));
            chkLB_2A->SetToolTip(_("[RFLB_A_EN] External RF loopback TxBAND2->RxLNAH channel A"));
            Connect(chkLB_2A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_2A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkLB_2B = new wxCheckBox(this, wxNewId(), _("RF loopback ch.B"));
            chkLB_2B->SetToolTip(_("[RFLB_B_EN] External RF loopback TxBAND2->RxLNAH channel B"));
            Connect(chkLB_2B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkLB_2B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

            chkSH_2A = new wxCheckBox(this, wxNewId(), _("Ch.A shunt"));
            chkSH_2A->SetToolTip(_("[TX1_2_LB_SH]"));
            Connect(chkSH_2A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_2A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkSH_2B = new wxCheckBox(this, wxNewId(), _("Ch.B shunt"));
            chkSH_2B->SetToolTip(_("[TX2_2_LB_SH]"));
            Connect(chkSH_2B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkSH_2B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_2A = new wxCheckBox(this, wxNewId(), _("Ch.A attenuator"));
            chkAT_2A->SetToolTip(_("[TX1_2_LB_AT]"));
            Connect(chkAT_2A->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_2A, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            chkAT_2B = new wxCheckBox(this, wxNewId(), _("Ch.B attenuator"));
            chkAT_2B->SetToolTip(_("[TX2_2_LB_AT]"));
            Connect(chkAT_2B->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
            controlsSizer->Add(chkAT_2B, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
            auto lms1sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LMS#2") ), wxVERTICAL );
            lms1sizer->Add(controlsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
            groupSizer->Add(lms1sizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP |  wxALL, 5);
        }
        mainSizer->Add(groupSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    }



    wxSize freqTextfieldSize(64, -1);
    mPanelStreamPLL = new wxPanel(this, wxNewId());
    wxFlexGridSizer* sizerPllControls = new wxFlexGridSizer(0, 3, 5, 5);
    sizerPllControls->Add(new wxStaticText(mPanelStreamPLL, wxID_ANY, _("DAC :")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    txtPllFreqTxMHz = new wxTextCtrl(mPanelStreamPLL, wxNewId(), _("160"), wxDefaultPosition, freqTextfieldSize);
    sizerPllControls->Add(txtPllFreqTxMHz, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblRealFreqTx = new wxStaticText(mPanelStreamPLL, wxID_ANY, _("MHz"));
    sizerPllControls->Add(lblRealFreqTx, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    sizerPllControls->Add(new wxStaticText(mPanelStreamPLL, wxID_ANY, _("ADC :")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    txtPllFreqRxMHz = new wxTextCtrl(mPanelStreamPLL, wxNewId(), _("160"), wxDefaultPosition, freqTextfieldSize);
    sizerPllControls->Add(txtPllFreqRxMHz, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    lblRealFreqRx = new wxStaticText(mPanelStreamPLL, wxID_ANY, _("MHz"));
    sizerPllControls->Add(lblRealFreqRx, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    btnConfigurePLL = new wxButton(mPanelStreamPLL, wxNewId(), _("Configure"));
    sizerPllControls->Add(btnConfigurePLL, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    Connect(btnConfigurePLL->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(pnlQSpark::OnConfigurePLL), NULL, this);

    wxStaticBoxSizer* streamPllGroup = new wxStaticBoxSizer(wxHORIZONTAL, mPanelStreamPLL, _T("Digital Interface Clock"));
    mPanelStreamPLL->SetSizer(streamPllGroup);
    streamPllGroup->Add(sizerPllControls, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    streamPllGroup->Fit(mPanelStreamPLL);
    FlexGridSizer1->Add(mPanelStreamPLL, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxStaticBoxSizer* chBox = new wxStaticBoxSizer(wxVERTICAL, this, _T("ADC/DAC channel controls"));
    wxFlexGridSizer* chSizer = new wxFlexGridSizer(6, 0, 0, 0);
    wxFlexGridSizer* chSelect = new wxFlexGridSizer(0, 2, 0, 0);
    rbChannelA = new wxRadioButton(this, wxNewId(), wxT("A CHANNEL"), wxDefaultPosition, wxDefaultSize, 0);
    rbChannelA->SetValue(true);
    rbChannelA->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlQSpark::OnSwitchToChannelA), NULL, this);
    chSelect->Add(rbChannelA, 0, wxEXPAND, 5);

    rbChannelB = new wxRadioButton(this, wxNewId(), wxT("B CHANNEL"), wxDefaultPosition, wxDefaultSize, 0);
    chSelect->Add(rbChannelB, 0, wxEXPAND, 5);
    rbChannelB->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(pnlQSpark::OnSwitchToChannelB), NULL, this);
    chSizer->Add(chSelect, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    chBox->Add(chSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);
    FlexGridSizer1->Add(chBox, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxStaticBoxSizer* moduleEnables = new wxStaticBoxSizer(wxHORIZONTAL, this, _T(""));
    chkEN_RXTSP = new wxCheckBox(this, wxNewId(), _("Enable RxTSP"));
    moduleEnables->Add(chkEN_RXTSP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkEN_RXTSP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    chkEN_TXTSP = new wxCheckBox(this, wxNewId(), _("Enable TxTSP"));
    moduleEnables->Add(chkEN_TXTSP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkEN_TXTSP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    chSizer->Add(moduleEnables, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);

    wxStaticBoxSizer* bypassesGroup = new wxStaticBoxSizer(wxVERTICAL, this, _T("Bypass"));
    wxFlexGridSizer* bypasses = new wxFlexGridSizer(0, 2, 0, 0);
    chkRX_GCORR_BYP = new wxCheckBox(this, wxNewId(), _("Rx Gain corrector"));
    bypasses->Add(chkRX_GCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkRX_GCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkTX_GCORR_BYP = new wxCheckBox(this, wxNewId(), _("Tx Gain corrector"));
    bypasses->Add(chkTX_GCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkTX_GCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkRX_PHCORR_BYP = new wxCheckBox(this, wxNewId(), _("Rx Phase corrector"));
    bypasses->Add(chkRX_PHCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkRX_PHCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkTX_PHCORR_BYP = new wxCheckBox(this, wxNewId(), _("Tx Phase corrector"));
    bypasses->Add(chkTX_PHCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkTX_PHCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkRX_DCCORR_BYP = new wxCheckBox(this, wxNewId(), _("Rx DC corrector"));
    bypasses->Add(chkRX_DCCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkRX_DCCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    chkTX_DCCORR_BYP = new wxCheckBox(this, wxNewId(), _("Tx DC corrector"));
    bypasses->Add(chkTX_DCCORR_BYP, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(chkTX_DCCORR_BYP->GetId(), wxEVT_CHECKBOX, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    bypassesGroup->Add(bypasses , 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    chSizer->Add(bypassesGroup, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxFlexGridSizer* tx_correctorsSizer = new wxFlexGridSizer(0, 2, 0, 5);
    wxSize spinBoxSize(64, -1);
    long spinBoxStyle = wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER;
    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx GCORRQ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_GCORRQ = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    tx_correctorsSizer->Add(spinTX_GCORRQ, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_GCORRQ->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRQ->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRQ->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx GCORRI")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_GCORRI = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    tx_correctorsSizer->Add(spinTX_GCORRI, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_GCORRI->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRI->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_GCORRI->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx PHCORR")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_PHCORR = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -2048, 2047, 0);
    tx_correctorsSizer->Add(spinTX_PHCORR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_PHCORR->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_PHCORR->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_PHCORR->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx DCCORRI")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_DCCORRI = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -128, 127, 0);
    tx_correctorsSizer->Add(spinTX_DCCORRI, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_DCCORRI->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRI->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRI->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    tx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx DCCORRQ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinTX_DCCORRQ = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -128, 127, 0);
    tx_correctorsSizer->Add(spinTX_DCCORRQ, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinTX_DCCORRQ->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRQ->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinTX_DCCORRQ->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);


    wxFlexGridSizer* rx_correctorsSizer = new wxFlexGridSizer(0, 2, 0, 5);
    rx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Rx GCORRQ")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinRX_GCORRQ = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    rx_correctorsSizer->Add(spinRX_GCORRQ, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinRX_GCORRQ->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRQ->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRQ->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    rx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Rx GCORRI")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinRX_GCORRI = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, 0, 2047, 2047);
    rx_correctorsSizer->Add(spinRX_GCORRI, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinRX_GCORRI->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRI->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_GCORRI->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    rx_correctorsSizer->Add(new wxStaticText(this, wxID_ANY, _("Rx PHCORR")), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    spinRX_PHCORR = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, spinBoxSize, spinBoxStyle, -2048, 2047, 0);
    rx_correctorsSizer->Add(spinRX_PHCORR, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    Connect(spinRX_PHCORR->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_PHCORR->GetId(), wxEVT_SPINCTRL, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);
    Connect(spinRX_PHCORR->GetId(), wxEVT_SPIN, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    wxFlexGridSizer* correctorsSizer = new wxFlexGridSizer(0, 2, 0, 5);
    correctorsSizer->Add(rx_correctorsSizer, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);
    correctorsSizer->Add(tx_correctorsSizer, 1, wxALIGN_LEFT | wxALIGN_TOP, 5);

    wxStaticBoxSizer* correctorsGroup = new wxStaticBoxSizer(wxVERTICAL, this, _T("Correctors"));
    correctorsGroup->Add(correctorsSizer, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);
    chSizer->Add(correctorsGroup, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    txtNcoFreq = new wxTextCtrl(this, wxNewId(), _("1.0"), wxDefaultPosition, freqTextfieldSize);
    wxArrayString insel_choices;
    insel_choices.push_back(_("Rx chain"));
    insel_choices.push_back(_("FPGA NCO"));
    cmbInsel = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, insel_choices, 1);
    cmbInsel->SetSelection(0);
    Connect(cmbInsel->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlQSpark::RegisterParameterChangeHandler), NULL, this);

    wxStaticBoxSizer* loopbackGroup = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Loopback controls"));
    wxFlexGridSizer* loopbackSizer = new wxFlexGridSizer(0, 2, 0, 0);
    loopbackSizer->Add(new wxStaticText(this, wxID_ANY, _("Tx chain input source:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    loopbackSizer->Add(cmbInsel, 1, wxALIGN_LEFT | wxEXPAND, 5);
    loopbackSizer->Add(new wxStaticText(this, wxID_ANY, _("NCO (MHz):")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    loopbackSizer->Add(txtNcoFreq, 1, wxALIGN_LEFT | wxEXPAND, 5);
    Connect(txtNcoFreq->GetId(), wxEVT_TEXT, wxCommandEventHandler(pnlQSpark::OnNcoFrequencyChanged), NULL, this);
    Connect(txtNcoFreq->GetId(), wxEVT_TEXT_ENTER, wxCommandEventHandler(pnlQSpark::OnNcoFrequencyChanged), NULL, this);

    loopbackGroup->Add(loopbackSizer, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);
    chSizer->Add(loopbackGroup, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);

    mainSizer->Add(FlexGridSizer1, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);

    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();

    controlsPtr2Registers.clear();
    controlsPtr2Registers[chkEN_RXTSP] = Register(0x00A0, 0, 0, 0);
    controlsPtr2Registers[chkEN_TXTSP] = Register(0x0080, 0, 0, 0);

    controlsPtr2Registers[chkRX_DCCORR_BYP] = Register(0x00AC, 2, 2, 0);
    controlsPtr2Registers[chkRX_PHCORR_BYP] = Register(0x00AC, 0, 0, 0);
    controlsPtr2Registers[chkRX_GCORR_BYP] = Register(0x00AC, 1, 1, 0);
    controlsPtr2Registers[chkTX_PHCORR_BYP] = Register(0x0088, 0, 0, 0);
    controlsPtr2Registers[chkTX_GCORR_BYP] = Register(0x0088, 1, 1, 0);
    controlsPtr2Registers[chkTX_DCCORR_BYP] = Register(0x0088, 3, 3, 0);

    controlsPtr2Registers[spinTX_GCORRQ] = Register(0x0081, 10, 0, 2047);
    controlsPtr2Registers[spinTX_GCORRI] = Register(0x0082, 10, 0, 2047);
    controlsPtr2Registers[spinTX_PHCORR] = Register(0x0083, 11, 0, 0);
    controlsPtr2Registers[spinTX_DCCORRI] = Register(0x0084, 15, 8, 0);
    controlsPtr2Registers[spinTX_DCCORRQ] = Register(0x0084, 7, 0, 0);

    controlsPtr2Registers[spinRX_GCORRQ] = Register(0x00A1, 10, 0, 2047);
    controlsPtr2Registers[spinRX_GCORRI] = Register(0x00A2, 10, 0, 2047);
    controlsPtr2Registers[spinRX_PHCORR] = Register(0x00A3, 11, 0, 0);

    controlsPtr2Registers[chkLB_1A] = Register(0x0017, 1, 0, 2);
    controlsPtr2Registers[chkLB_1B] = Register(0x0017, 5, 4, 2);
    controlsPtr2Registers[chkLB_2A] = Register(0x0017, 9, 8, 2);
    controlsPtr2Registers[chkLB_2B] = Register(0x0017, 13, 12, 2);
    controlsPtr2Registers[chkSH_1A] = Register(0x0017, 3, 3, 1);
    controlsPtr2Registers[chkSH_1B] = Register(0x0017, 7, 7, 1);
    controlsPtr2Registers[chkSH_2A] = Register(0x0017, 11, 11, 1);
    controlsPtr2Registers[chkSH_2B] = Register(0x0017, 15, 15, 1);
    controlsPtr2Registers[chkAT_1A] = Register(0x0017, 2, 2, 0);
    controlsPtr2Registers[chkAT_1B] = Register(0x0017, 6, 6, 0);
    controlsPtr2Registers[chkAT_2A] = Register(0x0017, 10, 10, 0);
    controlsPtr2Registers[chkAT_2B] = Register(0x0017, 14, 14, 0);


    controlsPtr2Registers[cmbInsel] = Register(0x0080, 2, 2, 0);

    Bind(READ_ALL_VALUES, &pnlQSpark::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlQSpark::OnWriteAll, this, this->GetId());
}

void pnlQSpark::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    LMS_WriteFPGAReg(lmsControl, 0xFFFF, rbChannelB->GetValue() ? 0x2 : 0x1);
    double freqHz;
    LMS_GetSampleRate(lmsControl, LMS_CH_RX, 4, &freqHz,nullptr);
    txtPllFreqRxMHz->SetValue(wxString::Format("%1.3f", freqHz/1e6));
    LMS_GetSampleRate(lmsControl, LMS_CH_TX, 4, &freqHz,nullptr);
    txtPllFreqTxMHz->SetValue(wxString::Format("%1.3f", freqHz/1e6));

}

void pnlQSpark::RegisterParameterChangeHandler(wxCommandEvent& event)
{
    if (controlsPtr2Registers.find(event.GetEventObject()) == controlsPtr2Registers.end())
        return; //control not found in the table

    Register reg = controlsPtr2Registers[event.GetEventObject()];
    int mac = (reg.address!=0x17) && (rbChannelB->GetValue()) ? 0x2 : 0x1;
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF,  mac)!=0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    unsigned short mask = (~(~0u << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask

    uint16_t regValue;
    LMS_ReadFPGAReg(lmsControl,reg.address,&regValue);

    regValue &= ~mask;
    int evtVal = event.GetInt();
    if (event.GetEventObject() == chkLB_1A || event.GetEventObject() == chkLB_1B || event.GetEventObject() == chkLB_2A || event.GetEventObject() == chkLB_2B)
        evtVal++;
    regValue |= ( evtVal << reg.lsb) & mask;

    if(LMS_WriteFPGAReg(lmsControl, reg.address, regValue) != 0)
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
}

pnlQSpark::~pnlQSpark()
{
    mPanelStreamPLL->Disconnect(wxEVT_BUTTON, btnConfigurePLL->GetId(), wxCommandEventHandler(pnlQSpark::OnConfigurePLL), 0, this);
}

void pnlQSpark::OnbtnUpdateAll(wxCommandEvent& event)
{
    map<wxObject*, Register>::iterator iter;
    wxClassInfo* spinctr = wxClassInfo::FindClass("wxSpinCtrl");
    wxClassInfo* checkboxctr = wxClassInfo::FindClass("wxCheckBox");
    wxClassInfo* choicectr = wxClassInfo::FindClass("wxChoice");
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, rbChannelB->GetValue() ? 0x2 : 0x1)!= 0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    for (iter = controlsPtr2Registers.begin(); iter != controlsPtr2Registers.end(); ++iter)
    {
        Register reg = iter->second;
        unsigned short mask = (~(~0u << (reg.msb - reg.lsb + 1))) << reg.lsb; // creates bit mask
        uint16_t value;

        LMS_ReadFPGAReg(lmsControl,reg.address,&value);

        value = value & mask;
        value = value >> reg.lsb;

        if (iter->first == chkLB_1A || iter->first == chkLB_1B || iter->first == chkLB_2A || iter->first == chkLB_2B)
            value = value == 2 ? 1 : 0;
        if (iter->first->IsKindOf(spinctr))
            reinterpret_cast<wxSpinCtrl*>(iter->first)->SetValue(value);
        else if(iter->first->IsKindOf(checkboxctr))
            reinterpret_cast<wxCheckBox*>(iter->first)->SetValue(value);
        else if(iter->first->IsKindOf(choicectr))
            reinterpret_cast<wxComboBox*>(iter->first)->SetSelection(value);
    }

    double refClk_MHz, ncoFreq_MHz;
    txtPllFreqTxMHz->GetValue().ToDouble(&refClk_MHz);
    uint32_t fcw = 0;
    vector<uint32_t> addrs = { 0x008E, 0x008F };

    for (size_t i = 0; i <addrs.size(); i++)
    {
        uint16_t value;
        if (LMS_ReadFPGAReg(lmsControl, addrs[i], &value) != 0)
        {
            wxMessageBox(_("Read FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
            return;
        }
        fcw <<= 16;
        fcw += value;
    }
    ncoFreq_MHz = fcw * refClk_MHz / 4294967296;
    txtNcoFreq->SetValue(wxString::Format("%1.3f", ncoFreq_MHz));
}

void pnlQSpark::OnConfigurePLL(wxCommandEvent &event)
{

    auto conn = ((LMS7_Device*)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        wxMessageBox(_("device not connected"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    double FreqTxMHz, FreqRxMHz;
    txtPllFreqTxMHz->GetValue().ToDouble(&FreqTxMHz);
    txtPllFreqRxMHz->GetValue().ToDouble(&FreqRxMHz);

    if (((LMS7_Device*)lmsControl)->SetRate(4, FreqRxMHz*1e6, FreqTxMHz*1e6)!=0)
        wxMessageBox(_("PLL configuration failed"), _("Error"), wxICON_ERROR | wxOK);
    else
        OnNcoFrequencyChanged(event);
}

void pnlQSpark::OnNcoFrequencyChanged(wxCommandEvent& event)
{
    LMS_WriteFPGAReg(lmsControl, 0xFFFF, rbChannelB->GetValue() ? 0x2 : 0x1);
    double refClk_MHz, ncoFreq_MHz;
    txtPllFreqTxMHz->GetValue().ToDouble(&refClk_MHz);
    txtNcoFreq->GetValue().ToDouble(&ncoFreq_MHz);
    uint32_t fcw = (uint32_t)((ncoFreq_MHz / refClk_MHz) * 4294967296);
    vector<uint32_t> addrs = { 0x008E, 0x008F };
    vector<uint32_t> values = { (fcw >> 16) & 0xFFFF, fcw & 0xFFFF };

    for (size_t i = 0; i <values.size();i++)
    {
        if (LMS_WriteFPGAReg(lmsControl,addrs[i],values[i]) !=0)
        {
            wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
            return;
        }
    }
}

void pnlQSpark::OnReadAll(wxCommandEvent &event)
{
     OnbtnUpdateAll(event);
}

void pnlQSpark::OnWriteAll(wxCommandEvent &event)
{
    OnNcoFrequencyChanged(event);
    OnConfigurePLL(event);
}

void pnlQSpark::OnSwitchToChannelA(wxCommandEvent& event)
{
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, 0x1) != 0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    wxCommandEvent evt;
    OnbtnUpdateAll(evt);
}

void pnlQSpark::OnSwitchToChannelB(wxCommandEvent& event)
{
    if (LMS_WriteFPGAReg(lmsControl, 0xFFFF, 0x2) != 0)
    {
        wxMessageBox(_("Write FPGA register failed"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    wxCommandEvent evt;
    OnbtnUpdateAll(evt);
}
