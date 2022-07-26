#include "lms7002_pnlGains_view.h"
#include "numericSlider.h"
#include <map>
#include "lms7002_gui_utilities.h"
#include "lms7_device.h"
using namespace lime;
using namespace LMS7002_WXGUI;

static indexValueMap g_lna_rfe_IndexValuePairs;
static indexValueMap g_tia_rfe_IndexValuePairs;

lms7002_pnlGains_view::lms7002_pnlGains_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    wxFlexGridSizer* fgSizer309;
    fgSizer309 = new wxFlexGridSizer( 0, 3, 5, 5 );
    fgSizer309->AddGrowableCol( 2 );
    fgSizer309->SetFlexibleDirection( wxBOTH );
    fgSizer309->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkTRX_GAIN_SRC = new wxCheckBox( this, ID_EN_NEXTRX_RFE, wxT("Alternative TRX gain source"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTRX_GAIN_SRC->SetToolTip( wxT("Enables the daisy chain LO buffer going from RXFE1  to RXFE2") );
    
    fgSizer309->Add( chkTRX_GAIN_SRC, 0, wxALL, 5 );
    
    
    fgSizer309->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    fgSizer309->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer159;
    sbSizer159 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Receiver") ), wxHORIZONTAL );
    
    wxFlexGridSizer* rxSizer;
    rxSizer = new wxFlexGridSizer( 0, 2, 0, 5 );
    rxSizer->AddGrowableCol( 0 );
    rxSizer->SetFlexibleDirection( wxBOTH );
    rxSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
    
    wxStaticText* ID_STATICTEXT16;
    ID_STATICTEXT16 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("LNA"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT16->Wrap( -1 );
    rxSizer->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbG_LNA_RFE = new wxComboBox( sbSizer159->GetStaticBox(), ID_G_LNA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbG_LNA_RFE->SetToolTip( wxT("Controls the gain of the LNA") );
    
    rxSizer->Add( cmbG_LNA_RFE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
    
    wxStaticText* ID_STATICTEXT18;
    ID_STATICTEXT18 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("TIA"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT18->Wrap( -1 );
    rxSizer->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbG_TIA_RFE = new wxComboBox( sbSizer159->GetStaticBox(), ID_G_TIA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbG_TIA_RFE->SetToolTip( wxT("Controls the Gain of the TIA") );
    
    rxSizer->Add( cmbG_TIA_RFE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
    
    wxStaticText* ID_STATICTEXT2;
    ID_STATICTEXT2 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("PGA gain"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    rxSizer->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbG_PGA_RBB = new wxComboBox( sbSizer159->GetStaticBox(), ID_G_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbG_PGA_RBB->SetToolTip( wxT("This is the gain of the PGA") );
    
    rxSizer->Add( cmbG_PGA_RBB, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
    
    wxStaticText* ID_STATICTEXT3;
    ID_STATICTEXT3 = new wxStaticText( sbSizer159->GetStaticBox(), wxID_ANY, wxT("PGA Feedback capacitor"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    rxSizer->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbC_CTL_PGA_RBB = new NumericSlider( sbSizer159->GetStaticBox(), ID_C_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 3, 0 );
    rxSizer->Add( cmbC_CTL_PGA_RBB, 0, wxALIGN_LEFT, 5 );
    
    
    sbSizer159->Add( rxSizer, 1, 0, 5 );
    
    
    fgSizer309->Add( sbSizer159, 1, 0, 5 );
    
    wxStaticBoxSizer* sbSizer148;
    sbSizer148 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Transmitter") ), wxHORIZONTAL );
    
    wxFlexGridSizer* txSizer;
    txSizer = new wxFlexGridSizer( 0, 2, 0, 5 );
    txSizer->SetFlexibleDirection( wxBOTH );
    txSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
    
    wxStaticText* ID_STATICTEXT6;
    ID_STATICTEXT6 = new wxStaticText( sbSizer148->GetStaticBox(), wxID_ANY, wxT("TXPAD linearizing part gain"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT6->Wrap( -1 );
    txSizer->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOSS_LIN_TXPAD_TRF = new wxComboBox( sbSizer148->GetStaticBox(), ID_LOSS_LIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbLOSS_LIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain of the linearizing part of of the TXPAD") );
    
    txSizer->Add( cmbLOSS_LIN_TXPAD_TRF, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
    
    wxStaticText* ID_STATICTEXT7;
    ID_STATICTEXT7 = new wxStaticText( sbSizer148->GetStaticBox(), wxID_ANY, wxT("TXPAD gain control"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT7->Wrap( -1 );
    txSizer->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOSS_MAIN_TXPAD_TRF = new wxComboBox( sbSizer148->GetStaticBox(), ID_LOSS_MAIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbLOSS_MAIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain  output power of the TXPAD") );
    
    txSizer->Add( cmbLOSS_MAIN_TXPAD_TRF, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 0 );
    
    wxStaticText* ID_STATICTEXT21;
    ID_STATICTEXT21 = new wxStaticText( sbSizer148->GetStaticBox(), wxID_ANY, wxT("Frontend gain"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT21->Wrap( -1 );
    txSizer->Add( ID_STATICTEXT21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCG_IAMP_TBB = new NumericSlider( sbSizer148->GetStaticBox(), ID_CG_IAMP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
    txSizer->Add( cmbCG_IAMP_TBB, 0, wxALIGN_LEFT|wxEXPAND, 5 );
    
    
    sbSizer148->Add( txSizer, 1, 0, 5 );
    
    
    fgSizer309->Add( sbSizer148, 1, 0, 5 );
    
    
    fgSizer309->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer135;
    sbSizer135 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("MCU AGC") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer256;
    fgSizer256 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer256->SetFlexibleDirection( wxBOTH );
    fgSizer256->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkAGC = new wxCheckBox( sbSizer135->GetStaticBox(), wxID_ANY, wxT("Enable AGC"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer256->Add( chkAGC, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    m_staticText360 = new wxStaticText( sbSizer135->GetStaticBox(), wxID_ANY, wxT("IN crest factor"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText360->Wrap( -1 );
    fgSizer256->Add( m_staticText360, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
    
    txtCrestFactor = new wxTextCtrl( sbSizer135->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer256->Add( txtCrestFactor, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizer135->Add( fgSizer256, 1, wxEXPAND, 5 );
    
    
    fgSizer309->Add( sbSizer135, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( fgSizer309 );
    this->Layout();
    fgSizer309->Fit( this );
    
    // Connect Events
    chkTRX_GAIN_SRC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    cmbG_LNA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    cmbG_TIA_RFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    cmbG_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    cmbC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    cmbLOSS_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    cmbLOSS_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    cmbCG_IAMP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlGains_view::ParameterChangeHandler ), NULL, this );
    chkAGC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlGains_view::OnAGCStateChange ), NULL, this );

    wndId2Enum[chkTRX_GAIN_SRC] = LMS7param(TRX_GAIN_SRC);
    wndId2Enum[cmbG_PGA_RBB] = LMS7param(G_PGA_RBB_R3);
    wndId2Enum[cmbG_LNA_RFE] = LMS7param(G_LNA_RFE_R3);
    wndId2Enum[cmbG_TIA_RFE] = LMS7param(G_TIA_RFE_R3);
    wndId2Enum[cmbCG_IAMP_TBB] = LMS7param(CG_IAMP_TBB_R3);
    wndId2Enum[cmbLOSS_LIN_TXPAD_TRF] = LMS7param(LOSS_LIN_TXPAD_R3);
    wndId2Enum[cmbLOSS_MAIN_TXPAD_TRF] = LMS7param(LOSS_MAIN_TXPAD_R3);
    wndId2Enum[cmbC_CTL_PGA_RBB] = LMS7param(C_CTL_PGA_RBB_R3);

    wxArrayString temp;

    temp.clear();
    for(int i=0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbLOSS_LIN_TXPAD_TRF->Append(temp);
    cmbLOSS_MAIN_TXPAD_TRF->Append(temp);

    temp.clear();
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i dB"), -12 + i));
    cmbG_PGA_RBB->Set(temp);

    for (int i = 0; i < 15; ++i)
        g_lna_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-30"));
    temp.push_back(_("Gmax-27"));
    temp.push_back(_("Gmax-24"));
    temp.push_back(_("Gmax-21"));
    temp.push_back(_("Gmax-18"));
    temp.push_back(_("Gmax-15"));
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-9"));
    temp.push_back(_("Gmax-6"));
    temp.push_back(_("Gmax-5"));
    temp.push_back(_("Gmax-4"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax-2"));
    temp.push_back(_("Gmax-1"));
    temp.push_back(_("Gmax"));
    cmbG_LNA_RFE->Set(temp);

    for (int i = 0; i < 3; ++i)
        g_tia_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax"));
    cmbG_TIA_RFE->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlGains_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)==0 && value == 0)
    {
        chkTRX_GAIN_SRC->Enable(false);
        chkTRX_GAIN_SRC->SetValue(false);
    }
    else
        chkTRX_GAIN_SRC->Enable(true);
}

void lms7002_pnlGains_view::ParameterChangeHandler(wxSpinEvent& event)
{
    ParameterChangeHandler(static_cast<wxCommandEvent&>(event));
}

void lms7002_pnlGains_view::ParameterChangeHandler(wxCommandEvent& event)
{
    assert(lmsControl != nullptr);
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }
    long value = event.GetInt();
    if (event.GetEventObject() == cmbG_LNA_RFE)
    {
        value = index2value(value, g_lna_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbG_TIA_RFE)
    {
        value = index2value(value, g_tia_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == chkTRX_GAIN_SRC)
    {
        cmbG_LNA_RFE->Enable(value);
        cmbG_TIA_RFE->Enable(value);
        cmbG_PGA_RBB->Enable(value);
        cmbC_CTL_PGA_RBB->Enable(value);
        cmbLOSS_LIN_TXPAD_TRF->Enable(value);
        cmbLOSS_MAIN_TXPAD_TRF->Enable(value);
        cmbCG_IAMP_TBB->Enable(value);
    }
    LMS_WriteParam(lmsControl,parameter,value);
}

void lms7002_pnlGains_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(G_LNA_RFE_R3),&value);
    cmbG_LNA_RFE->SetSelection( value2index(value, g_lna_rfe_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(G_TIA_RFE_R3),&value);
    cmbG_TIA_RFE->SetSelection( value2index(value, g_tia_rfe_IndexValuePairs));

    value = chkTRX_GAIN_SRC->GetValue();
    cmbG_LNA_RFE->Enable(value);
    cmbG_TIA_RFE->Enable(value);
    cmbG_PGA_RBB->Enable(value);
    cmbC_CTL_PGA_RBB->Enable(value);
    cmbLOSS_LIN_TXPAD_TRF->Enable(value);
    cmbLOSS_MAIN_TXPAD_TRF->Enable(value);
    cmbCG_IAMP_TBB->Enable(value);

}

void lms7002_pnlGains_view::OnAGCStateChange(wxCommandEvent& event)
{
    if(chkAGC->GetValue() != 0)
    {
        LMS7_Device* lms = (LMS7_Device*)lmsControl;
        double crestFactor;
        txtCrestFactor->GetValue().ToDouble(&crestFactor);
        uint32_t wantedRSSI = 87330 / pow(10.0, (3+crestFactor)/20);
        lms->MCU_AGCStart(wantedRSSI);
        txtCrestFactor->Disable();
    }
    else
    {
        LMS7_Device* lms = (LMS7_Device*)lmsControl;
        lms->MCU_AGCStop();
        txtCrestFactor->Enable();
    }
}
