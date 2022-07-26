#include "lms7002_pnlTRF_view.h"
#include <map>
#include <vector>
#include "lms7002_gui_utilities.h"
#include "wx/msgdlg.h"
#include "lms7suiteEvents.h"
using namespace lime;
using namespace LMS7002_WXGUI;
static indexValueMap en_amphf_pdet_trfIndexValuePairs;

lms7002_pnlTRF_view::lms7002_pnlTRF_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer33;
    fgSizer33 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer33->SetFlexibleDirection( wxBOTH );
    fgSizer33->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer35;
    fgSizer35 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer35->SetFlexibleDirection( wxBOTH );
    fgSizer35->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerPowerDowns;
    sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer38;
    fgSizer38 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer38->SetFlexibleDirection( wxBOTH );
    fgSizer38->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkPD_PDET_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PDET_TRF, wxT("Power detector"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_PDET_TRF->SetToolTip( wxT("Powerdown signal for Power Detector") );
    
    fgSizer38->Add( chkPD_PDET_TRF, 0, flags, 0 );
    
    chkPD_TLOBUF_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TLOBUF_TRF, wxT("TX LO buffer"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_TLOBUF_TRF->SetToolTip( wxT("Powerdown signal for TX LO buffer") );
    
    fgSizer38->Add( chkPD_TLOBUF_TRF, 0, flags, 0 );
    
    chkPD_TXPAD_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TXPAD_TRF, wxT("TXPAD"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_TXPAD_TRF->SetToolTip( wxT("Powerdown signal for TXPAD") );
    
    fgSizer38->Add( chkPD_TXPAD_TRF, 0, flags, 0 );
    
    chkEN_G_TRF = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_TRF, wxT("Enable TRF module"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_G_TRF->SetToolTip( wxT("Enable control for all the TRF_1 power downs") );
    
    fgSizer38->Add( chkEN_G_TRF, 0, flags, 0 );
    
    wxStaticBoxSizer* sbSizer20;
    sbSizer20 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
    
    chkEN_DIR_TRF = new wxCheckBox( sbSizer20->GetStaticBox(), ID_EN_DIR_TRF, wxT("Direct control of PDs and ENs"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_DIR_TRF->SetToolTip( wxT("Enables direct control of PDs and ENs for TRF module") );
    
    sbSizer20->Add( chkEN_DIR_TRF, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    fgSizer38->Add( sbSizer20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizerPowerDowns->Add( fgSizer38, 0, flags, 0 );
    
    
    fgSizer35->Add( sbSizerPowerDowns, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerPowerDetector;
    sbSizerPowerDetector = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power detector") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer36;
    fgSizer36 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer36->AddGrowableCol( 1 );
    fgSizer36->SetFlexibleDirection( wxBOTH );
    fgSizer36->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT11 = new wxStaticText( sbSizerPowerDetector->GetStaticBox(), wxID_ANY, wxT("Resistive load"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT11->Wrap( -1 );
    fgSizer36->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOADR_PDET_TRF = new wxComboBox( sbSizerPowerDetector->GetStaticBox(), ID_LOADR_PDET_TRF, wxEmptyString, wxDefaultPosition, wxSize( 160,-1 ), 0, NULL, 0 );
    cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K||2.5K||1.25K") );
    cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K||1.25K") );
    cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K||2.5K") );
    cmbLOADR_PDET_TRF->Append( wxT("R_DIFF 5K") );
    cmbLOADR_PDET_TRF->Append( wxEmptyString );
    cmbLOADR_PDET_TRF->SetToolTip( wxT("Controls the resistive load of the Power detector") );
    
    fgSizer36->Add( cmbLOADR_PDET_TRF, 0, wxEXPAND, 0 );
    
    
    sbSizerPowerDetector->Add( fgSizer36, 0, flags, 0 );
    
    
    fgSizer35->Add( sbSizerPowerDetector, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerBiasCurrent;
    sbSizerBiasCurrent = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bias current") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer37;
    fgSizer37 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer37->AddGrowableCol( 1 );
    fgSizer37->SetFlexibleDirection( wxBOTH );
    fgSizer37->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT12 = new wxStaticText( sbSizerBiasCurrent->GetStaticBox(), wxID_ANY, wxT("Linearization section"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT12->Wrap( -1 );
    fgSizer37->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LIN_TXPAD_TRF = new wxComboBox( sbSizerBiasCurrent->GetStaticBox(), ID_ICT_LIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LIN_TXPAD_TRF->SetToolTip( wxT("Control the bias current of the linearization section of the TXPAD") );
    
    fgSizer37->Add( cmbICT_LIN_TXPAD_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT13 = new wxStaticText( sbSizerBiasCurrent->GetStaticBox(), wxID_ANY, wxT("Main gm section"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT13->Wrap( -1 );
    fgSizer37->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_MAIN_TXPAD_TRF = new wxComboBox( sbSizerBiasCurrent->GetStaticBox(), ID_ICT_MAIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_MAIN_TXPAD_TRF->SetToolTip( wxT("Control the bias current of the main gm section of the TXPAD") );
    
    fgSizer37->Add( cmbICT_MAIN_TXPAD_TRF, 0, wxEXPAND, 0 );
    
    
    sbSizerBiasCurrent->Add( fgSizer37, 0, wxEXPAND, 0 );
    
    
    fgSizer35->Add( sbSizerBiasCurrent, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerTrimDuty;
    sbSizerTrimDuty = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Trim duty cycle") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer39;
    fgSizer39 = new wxFlexGridSizer( 2, 2, 0, 0 );
    fgSizer39->AddGrowableCol( 1 );
    fgSizer39->SetFlexibleDirection( wxBOTH );
    fgSizer39->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT14 = new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("I channel:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT14->Wrap( -1 );
    fgSizer39->Add( ID_STATICTEXT14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbCDC_I_TRF = new wxComboBox( sbSizerTrimDuty->GetStaticBox(), ID_CDC_I_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    fgSizer39->Add( cmbCDC_I_TRF, 1, wxEXPAND, 5 );
    
    ID_STATICTEXT15 = new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("Q channel:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT15->Wrap( -1 );
    fgSizer39->Add( ID_STATICTEXT15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbCDC_Q_TRF = new wxComboBox( sbSizerTrimDuty->GetStaticBox(), ID_CDC_Q_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    fgSizer39->Add( cmbCDC_Q_TRF, 1, wxEXPAND, 5 );
    
    
    sbSizerTrimDuty->Add( fgSizer39, 1, wxEXPAND, 5 );
    
    
    fgSizer35->Add( sbSizerTrimDuty, 0, wxEXPAND, 5 );
    
    wxString rgrGCAS_GNDREF_TXPAD_TRFChoices[] = { wxT("VDD"), wxT("GNDS") };
    int rgrGCAS_GNDREF_TXPAD_TRFNChoices = sizeof( rgrGCAS_GNDREF_TXPAD_TRFChoices ) / sizeof( wxString );
    rgrGCAS_GNDREF_TXPAD_TRF = new wxRadioBox( this, ID_GCAS_GNDREF_TXPAD_TRF, wxT("TXPAD cascode transistor gate bias"), wxDefaultPosition, wxDefaultSize, rgrGCAS_GNDREF_TXPAD_TRFNChoices, rgrGCAS_GNDREF_TXPAD_TRFChoices, 2, wxRA_SPECIFY_COLS );
    rgrGCAS_GNDREF_TXPAD_TRF->SetSelection( 1 );
    rgrGCAS_GNDREF_TXPAD_TRF->SetToolTip( wxT("Controls if the TXPAD cascode transistor gate bias is referred to VDD or GND") );
    
    fgSizer35->Add( rgrGCAS_GNDREF_TXPAD_TRF, 0, wxEXPAND, 5 );
    
    
    fgSizer33->Add( fgSizer35, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxFlexGridSizer* fgSizer34;
    fgSizer34 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer34->SetFlexibleDirection( wxBOTH );
    fgSizer34->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkEN_NEXTTX_TRF = new wxCheckBox( this, ID_EN_NEXTTX_TRF, wxT("Enable Tx MIMO mode"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer34->Add( chkEN_NEXTTX_TRF, 0, wxEXPAND, 5 );
    
    chkEN_LOOPB_TXPAD_TRF = new wxCheckBox( this, ID_EN_LOOPB_TXPAD_TRF, wxT("Enable TXPAD loopback path"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOOPB_TXPAD_TRF->SetToolTip( wxT("Enables the TXPAD loopback path") );
    
    fgSizer34->Add( chkEN_LOOPB_TXPAD_TRF, 0, wxALIGN_LEFT|wxALIGN_TOP, 0 );
    
    ID_STATICTEXT16 = new wxStaticText( this, wxID_ANY, wxT("TXFE output selection"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT16->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT16, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbTXFEoutput = new wxComboBox( this, ID_TXFEoutput, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbTXFEoutput->Append( wxT("Band1") );
    cmbTXFEoutput->Append( wxT("Band2") );
    cmbTXFEoutput->Append( wxT("Disable") );
    cmbTXFEoutput->SetSelection( 0 );
    cmbTXFEoutput->SetToolTip( wxT("Enable signal for TXFE, Band 1 or Band 2.\n0x0103 [11] SEL_BAND1_TRF\n0x0103 [10] SEL_BAND2_TRF") );
    
    fgSizer34->Add( cmbTXFEoutput, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("EN_LOWBWLOMX_TMX_TRF"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbEN_LOWBWLOMX_TMX_TRF = new wxComboBox( this, ID_EN_LOWBWLOMX_TMX_TRF, wxEmptyString, wxDefaultPosition, wxSize( 201,-1 ), 0, NULL, 0 );
    cmbEN_LOWBWLOMX_TMX_TRF->Append( wxT("High band - bias resistor 3K") );
    cmbEN_LOWBWLOMX_TMX_TRF->Append( wxT("Low band -bias resistor 30K") );
    cmbEN_LOWBWLOMX_TMX_TRF->SetToolTip( wxT("Controls the high pass pole frequency of the RC biasing the gate of the mixer switches") );
    
    fgSizer34->Add( cmbEN_LOWBWLOMX_TMX_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("TXPAD power detector preamplifier gain"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbEN_AMPHF_PDET_TRF = new wxComboBox( this, ID_EN_AMPHF_PDET_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbEN_AMPHF_PDET_TRF->SetToolTip( wxT("Enables the TXPAD power detector preamplifier") );
    
    fgSizer34->Add( cmbEN_AMPHF_PDET_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT4 = new wxStaticText( this, wxID_ANY, wxT("Switched capacitor at TXPAD output"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbF_TXPAD_TRF = new wxComboBox( this, ID_F_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbF_TXPAD_TRF->SetToolTip( wxT("Controls the switched capacitor at the TXPAD output. Is used for fine tuning of the TXPAD output") );
    
    fgSizer34->Add( cmbF_TXPAD_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT5 = new wxStaticText( this, wxID_ANY, wxT("Loss of the loopback path at the TX side"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT5->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbL_LOOPB_TXPAD_TRF = new wxComboBox( this, ID_L_LOOPB_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbL_LOOPB_TXPAD_TRF->Append( wxT("0 dB") );
    cmbL_LOOPB_TXPAD_TRF->Append( wxT("13.9 dB") );
    cmbL_LOOPB_TXPAD_TRF->Append( wxT("20.8 dB") );
    cmbL_LOOPB_TXPAD_TRF->Append( wxT("24 dB") );
    cmbL_LOOPB_TXPAD_TRF->SetToolTip( wxT("Controls the loss of the of the loopback path at the TX side") );
    
    fgSizer34->Add( cmbL_LOOPB_TXPAD_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT6 = new wxStaticText( this, wxID_ANY, wxT("TXPAD linearizing part gain"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT6->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOSS_LIN_TXPAD_TRF = new wxComboBox( this, ID_LOSS_LIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbLOSS_LIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain of the linearizing part of of the TXPAD") );
    
    fgSizer34->Add( cmbLOSS_LIN_TXPAD_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT7 = new wxStaticText( this, wxID_ANY, wxT("TXPAD gain control"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT7->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOSS_MAIN_TXPAD_TRF = new wxComboBox( this, ID_LOSS_MAIN_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbLOSS_MAIN_TXPAD_TRF->SetToolTip( wxT("Controls the gain  output power of the TXPAD") );
    
    fgSizer34->Add( cmbLOSS_MAIN_TXPAD_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT8 = new wxStaticText( this, wxID_ANY, wxT("Bias voltage at gate of TXPAD cascade"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT8->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbVGCAS_TXPAD_TRF = new wxComboBox( this, ID_VGCAS_TXPAD_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbVGCAS_TXPAD_TRF->SetToolTip( wxT("Controls the bias voltage at the gate of TXPAD cascade") );
    
    fgSizer34->Add( cmbVGCAS_TXPAD_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT9 = new wxStaticText( this, wxID_ANY, wxT("Bias at gate of mixer NMOS"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT9->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOBIASN_TXM_TRF = new wxComboBox( this, ID_LOBIASN_TXM_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbLOBIASN_TXM_TRF->SetToolTip( wxT("Controls the bias at the gate of the mixer NMOS") );
    
    fgSizer34->Add( cmbLOBIASN_TXM_TRF, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT10 = new wxStaticText( this, wxID_ANY, wxT("Bias at gate of mixer PMOS"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT10->Wrap( -1 );
    fgSizer34->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOBIASP_TXX_TRF = new wxComboBox( this, ID_LOBIASP_TXX_TRF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbLOBIASP_TXX_TRF->SetToolTip( wxT("Controls the bias at the gate of the mixer PMOS") );
    
    fgSizer34->Add( cmbLOBIASP_TXX_TRF, 0, wxEXPAND, 0 );
    
    
    fgSizer33->Add( fgSizer34, 0, 0, 0 );
    
    
    this->SetSizer( fgSizer33 );
    this->Layout();
    fgSizer33->Fit( this );
    
    // Connect Events
    chkPD_PDET_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    chkPD_TLOBUF_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    chkPD_TXPAD_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    chkEN_G_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    chkEN_DIR_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbLOADR_PDET_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbICT_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbCDC_I_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbCDC_Q_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    rgrGCAS_GNDREF_TXPAD_TRF->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    chkEN_NEXTTX_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOOPB_TXPAD_TRF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbTXFEoutput->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::OnBandChange ), NULL, this );
    cmbEN_LOWBWLOMX_TMX_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbEN_AMPHF_PDET_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbF_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbL_LOOPB_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbLOSS_LIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbLOSS_MAIN_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbVGCAS_TXPAD_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbLOBIASN_TXM_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );
    cmbLOBIASP_TXX_TRF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTRF_view::ParameterChangeHandler ), NULL, this );

    wndId2Enum[cmbEN_AMPHF_PDET_TRF] = LMS7param(EN_AMPHF_PDET_TRF);
    wndId2Enum[chkEN_G_TRF] = LMS7param(EN_G_TRF);
    wndId2Enum[chkEN_LOOPB_TXPAD_TRF] = LMS7param(EN_LOOPB_TXPAD_TRF);
    wndId2Enum[cmbEN_LOWBWLOMX_TMX_TRF] = LMS7param(EN_LOWBWLOMX_TMX_TRF);
    wndId2Enum[chkEN_NEXTTX_TRF] = LMS7param(EN_NEXTTX_TRF);
    wndId2Enum[cmbF_TXPAD_TRF] = LMS7param(F_TXPAD_TRF);
    wndId2Enum[rgrGCAS_GNDREF_TXPAD_TRF] = LMS7param(GCAS_GNDREF_TXPAD_TRF);
    wndId2Enum[cmbICT_LIN_TXPAD_TRF] = LMS7param(ICT_LIN_TXPAD_TRF);
    wndId2Enum[cmbICT_MAIN_TXPAD_TRF] = LMS7param(ICT_MAIN_TXPAD_TRF);
    wndId2Enum[cmbLOADR_PDET_TRF] = LMS7param(LOADR_PDET_TRF);
    wndId2Enum[cmbLOBIASN_TXM_TRF] = LMS7param(LOBIASN_TXM_TRF);
    wndId2Enum[cmbLOBIASP_TXX_TRF] = LMS7param(LOBIASP_TXX_TRF);
    wndId2Enum[cmbLOSS_LIN_TXPAD_TRF] = LMS7param(LOSS_LIN_TXPAD_TRF);
    wndId2Enum[cmbLOSS_MAIN_TXPAD_TRF] = LMS7param(LOSS_MAIN_TXPAD_TRF);
    wndId2Enum[cmbL_LOOPB_TXPAD_TRF] = LMS7param(L_LOOPB_TXPAD_TRF);
    wndId2Enum[chkPD_PDET_TRF] = LMS7param(PD_PDET_TRF);
    wndId2Enum[chkPD_TLOBUF_TRF] = LMS7param(PD_TLOBUF_TRF);
    wndId2Enum[chkPD_TXPAD_TRF] = LMS7param(PD_TXPAD_TRF);
    wndId2Enum[cmbVGCAS_TXPAD_TRF] = LMS7param(VGCAS_TXPAD_TRF);
    wndId2Enum[cmbCDC_I_TRF] = LMS7param(CDC_I_TRF);
    wndId2Enum[cmbCDC_Q_TRF] = LMS7param(CDC_Q_TRF);
    wndId2Enum[chkEN_DIR_TRF] = LMS7param(EN_DIR_TRF);

    wxArrayString temp;
    temp.clear();
    temp.push_back(_("-10 db"));
    en_amphf_pdet_trfIndexValuePairs.push_back(indexValuePair(0,0));
    temp.push_back(_("7 db"));
    en_amphf_pdet_trfIndexValuePairs.push_back(indexValuePair(1,1));
    temp.push_back(_("25 db"));
    en_amphf_pdet_trfIndexValuePairs.push_back(indexValuePair(2,3));
    cmbEN_AMPHF_PDET_TRF->Set(temp);

    temp.clear();
    for(int i=0; i<8; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbF_TXPAD_TRF->Append(temp);

    temp.clear();
    float IbiasNominal = 20; //uA
    for(int i=0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.1f uA"), IbiasNominal*(i/12.0)));
    cmbICT_LIN_TXPAD_TRF->Append(temp);

    temp.clear();
    for(int i=0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbICT_MAIN_TXPAD_TRF->Append(temp);
    cmbLOBIASN_TXM_TRF->Append(temp);
    cmbLOBIASP_TXX_TRF->Append(temp);
    cmbLOSS_LIN_TXPAD_TRF->Append(temp);
    cmbLOSS_MAIN_TXPAD_TRF->Append(temp);
    cmbVGCAS_TXPAD_TRF->Append(temp);

    temp.clear();
    for(int i=0; i<16; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbCDC_I_TRF->Append(temp);
    cmbCDC_Q_TRF->Append(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlTRF_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlTRF_view::ParameterChangeHandler(wxCommandEvent& event)
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
    if(event.GetEventObject() == cmbEN_AMPHF_PDET_TRF)
        value = index2value(value, en_amphf_pdet_trfIndexValuePairs);

    LMS_WriteParam(lmsControl,parameter,value);
}

void lms7002_pnlTRF_view::OnBandChange( wxCommandEvent& event )
{
    switch (cmbTXFEoutput->GetSelection())
    {
    case 0:
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND1_TRF),true);
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND2_TRF),false);
        break;
    case 1:
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND1_TRF),false);
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND2_TRF),true);
        break;
    case 2:
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND1_TRF),false);
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND2_TRF),false);
        break;
    default:
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND1_TRF),false);
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND2_TRF),false);
    }
}

void lms7002_pnlTRF_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(EN_AMPHF_PDET_TRF),&value);
    cmbEN_AMPHF_PDET_TRF->SetSelection(value2index(value, en_amphf_pdet_trfIndexValuePairs));

    uint16_t SEL_BAND1_TRFvalue;
    LMS_ReadParam(lmsControl,LMS7param(SEL_BAND1_TRF),&SEL_BAND1_TRFvalue);
    uint16_t SEL_BAND2_TRFvalue;
    LMS_ReadParam(lmsControl,LMS7param(SEL_BAND2_TRF),&SEL_BAND2_TRFvalue);
    long TXFEoutputValue = 0;
    if (!SEL_BAND1_TRFvalue && !SEL_BAND2_TRFvalue)
        TXFEoutputValue = 2;
    else if (SEL_BAND1_TRFvalue && !SEL_BAND2_TRFvalue)
        TXFEoutputValue = 0;
    else if (!SEL_BAND1_TRFvalue && SEL_BAND2_TRFvalue)
        TXFEoutputValue = 1;
    else
    {
        TXFEoutputValue = 2;
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND1_TRF),false);
        LMS_WriteParam(lmsControl,LMS7param(SEL_BAND2_TRF),false);
    }
    cmbTXFEoutput->SetSelection(TXFEoutputValue);

    //check if B channel is enabled
    uint16_t macBck;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&macBck);
    if (macBck >= 2)
        chkEN_NEXTTX_TRF->Hide();
    else
        chkEN_NEXTTX_TRF->Show();

    LMS_ReadParam(lmsControl,LMS7param(TRX_GAIN_SRC),&value);
    cmbLOSS_LIN_TXPAD_TRF->Enable(!value);
    cmbLOSS_MAIN_TXPAD_TRF->Enable(!value);
}
