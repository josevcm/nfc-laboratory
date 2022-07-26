#include "lms7002_pnlCLKGEN_view.h"
#include <map>
#include <wx/msgdlg.h>
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include "lms7002_dlgVCOfrequencies.h"
#include "lms7_device.h"
#include "FPGA_common.h"
#include "Logger.h"
using namespace lime;

lms7002_pnlCLKGEN_view::lms7002_pnlCLKGEN_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer244;
    fgSizer244 = new wxFlexGridSizer( 0, 3, 5, 5 );
    fgSizer244->SetFlexibleDirection( wxBOTH );
    fgSizer244->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer247;
    fgSizer247 = new wxFlexGridSizer( 0, 1, 0, 5 );
    fgSizer247->SetFlexibleDirection( wxBOTH );
    fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer66;
    sbSizer66 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
    
    chkPD_CP_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_CP_CGEN, wxT("Charge pump"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_CP_CGEN->SetToolTip( wxT("Power down for Charge Pump") );
    
    sbSizer66->Add( chkPD_CP_CGEN, 0, flags, 0 );
    
    chkPD_FDIV_FB_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_FDIV_FB_CGEN, wxT("Feedback frequency divider"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_FDIV_FB_CGEN->SetToolTip( wxT("Power down for feedback frequency divider") );
    
    sbSizer66->Add( chkPD_FDIV_FB_CGEN, 0, flags, 0 );
    
    chkPD_FDIV_O_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_FDIV_O_CGEN, wxT("Frequency divider"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_FDIV_O_CGEN->SetToolTip( wxT("Power down for forward frequency divider of the CGEN block") );
    
    sbSizer66->Add( chkPD_FDIV_O_CGEN, 0, flags, 0 );
    
    chkPD_SDM_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_SDM_CGEN, wxT("SDM"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_SDM_CGEN->SetToolTip( wxT("Power down for SDM") );
    
    sbSizer66->Add( chkPD_SDM_CGEN, 0, flags, 0 );
    
    chkPD_VCO_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_VCO_CGEN, wxT("VCO"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_VCO_CGEN->SetToolTip( wxT("Power down for VCO") );
    
    sbSizer66->Add( chkPD_VCO_CGEN, 0, flags, 0 );
    
    chkPD_VCO_COMP_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_PD_VCO_COMP_CGEN, wxT("VCO comparator"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_VCO_COMP_CGEN->SetToolTip( wxT("Power down for VCO comparator") );
    
    sbSizer66->Add( chkPD_VCO_COMP_CGEN, 0, flags, 0 );
    
    chkEN_G_CGEN = new wxCheckBox( sbSizer66->GetStaticBox(), ID_EN_G_CGEN, wxT("Enable CLKGEN module"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_G_CGEN->SetToolTip( wxT("Enable control for all the CGEN power downs") );
    
    sbSizer66->Add( chkEN_G_CGEN, 0, flags, 0 );
    
    
    fgSizer247->Add( sbSizer66, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer65;
    sbSizer65 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("UNGROUPED") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer81;
    fgSizer81 = new wxFlexGridSizer( 0, 1, 0, 5 );
    fgSizer81->SetFlexibleDirection( wxBOTH );
    fgSizer81->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkSPDUP_VCO_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_SPDUP_VCO_CGEN, wxT("Bypass noise filter resistor"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_VCO_CGEN->SetToolTip( wxT("Bypasses the noise filter resistor for fast setlling time. It should be connected to a 1us pulse") );
    
    fgSizer81->Add( chkSPDUP_VCO_CGEN, 0, flags, 0 );
    
    chkRESET_N_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_RESET_N_CGEN, wxT("Pulse used in start-up to reset"), wxDefaultPosition, wxDefaultSize, 0 );
    chkRESET_N_CGEN->SetToolTip( wxT("A pulse should be used in the start-up to reset ( 1-normal operation)") );
    
    fgSizer81->Add( chkRESET_N_CGEN, 0, flags, 0 );
    
    chkEN_COARSE_CKLGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_EN_COARSE_CKLGEN, wxT("Enable coarse tuning block"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_COARSE_CKLGEN->SetToolTip( wxT("Enable signal for coarse tuning block") );
    
    fgSizer81->Add( chkEN_COARSE_CKLGEN, 0, flags, 0 );
    
    chkEN_INTONLY_SDM_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_EN_INTONLY_SDM_CGEN, wxT("Enable INTEGER-N mode"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_INTONLY_SDM_CGEN->SetToolTip( wxT("Enables INTEGER-N mode of the SX ") );
    
    fgSizer81->Add( chkEN_INTONLY_SDM_CGEN, 0, flags, 0 );
    
    chkEN_SDM_CLK_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_EN_SDM_CLK_CGEN, wxT("Enable SDM clock"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_SDM_CLK_CGEN->SetToolTip( wxT("Enables/Disables SDM clock. In INT-N mode or for noise testing, SDM clock can be disabled") );
    
    fgSizer81->Add( chkEN_SDM_CLK_CGEN, 0, flags, 0 );
    
    chkREV_SDMCLK_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REV_SDMCLK_CGEN, wxT("Reverse SDM clock"), wxDefaultPosition, wxDefaultSize, 0 );
    chkREV_SDMCLK_CGEN->SetToolTip( wxT("Reverses the SDM clock") );
    
    fgSizer81->Add( chkREV_SDMCLK_CGEN, 0, flags, 0 );
    
    chkSX_DITHER_EN_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_SX_DITHER_EN_CGEN, wxT("Enable dithering in SDM"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSX_DITHER_EN_CGEN->SetToolTip( wxT("Enabled dithering in SDM") );
    
    fgSizer81->Add( chkSX_DITHER_EN_CGEN, 0, flags, 0 );
    
    chkREV_CLKDAC_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REV_CLKDAC_CGEN, wxT("Invert DAC F_CLKL"), wxDefaultPosition, wxDefaultSize, 0 );
    chkREV_CLKDAC_CGEN->SetToolTip( wxT("Inverts the clock F_CLKL") );
    
    fgSizer81->Add( chkREV_CLKDAC_CGEN, 0, flags, 0 );
    
    chkREV_CLKADC_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REV_CLKADC_CGEN, wxT("Invert ADC F_CLKL"), wxDefaultPosition, wxDefaultSize, 0 );
    chkREV_CLKADC_CGEN->SetToolTip( wxT("Inverts the clock F_CLKL") );
    
    fgSizer81->Add( chkREV_CLKADC_CGEN, 0, flags, 0 );
    
    chkREVPH_PFD_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_REVPH_PFD_CGEN, wxT("REVPH_PFD_CGEN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkREVPH_PFD_CGEN->SetToolTip( wxT("Reverse the pulses of PFD. It can be used to reverse the polarity of the PLL loop (positive feedback to negative feedback)") );
    
    fgSizer81->Add( chkREVPH_PFD_CGEN, 0, flags, 0 );
    
    chkCOARSE_START_CGEN = new wxCheckBox( sbSizer65->GetStaticBox(), ID_COARSE_START_CGEN, wxT("Signal coarse tuning algorithm"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCOARSE_START_CGEN->SetToolTip( wxT("Control signal for coarse tuning algorithm (SX_SWC_calibration)") );
    
    fgSizer81->Add( chkCOARSE_START_CGEN, 0, flags, 0 );
    
    
    sbSizer65->Add( fgSizer81, 0, flags, 0 );
    
    
    fgSizer247->Add( sbSizer65, 1, wxEXPAND, 5 );
    
    wxString rgrSEL_SDMCLK_CGENChoices[] = { wxT("Feedback divider"), wxT("Fref") };
    int rgrSEL_SDMCLK_CGENNChoices = sizeof( rgrSEL_SDMCLK_CGENChoices ) / sizeof( wxString );
    rgrSEL_SDMCLK_CGEN = new wxRadioBox( this, ID_SEL_SDMCLK_CGEN, wxT("Output for SDM"), wxDefaultPosition, wxDefaultSize, rgrSEL_SDMCLK_CGENNChoices, rgrSEL_SDMCLK_CGENChoices, 1, wxRA_SPECIFY_COLS );
    rgrSEL_SDMCLK_CGEN->SetSelection( 0 );
    rgrSEL_SDMCLK_CGEN->SetToolTip( wxT("Selects between the feedback divider output and Fref for SDM") );
    
    fgSizer247->Add( rgrSEL_SDMCLK_CGEN, 0, flags, 0 );
    
    
    fgSizer244->Add( fgSizer247, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer245;
    fgSizer245 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer245->SetFlexibleDirection( wxBOTH );
    fgSizer245->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer246;
    fgSizer246 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer246->AddGrowableCol( 1 );
    fgSizer246->SetFlexibleDirection( wxBOTH );
    fgSizer246->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT4 = new wxStaticText( this, wxID_ANY, wxT("Test mode of SX"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer246->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbTST_CGEN = new wxComboBox( this, ID_TST_CGEN, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
    fgSizer246->Add( cmbTST_CGEN, 0, wxEXPAND, 0 );
    
    
    fgSizer245->Add( fgSizer246, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer248;
    fgSizer248 = new wxFlexGridSizer( 0, 3, 0, 5 );
    fgSizer248->AddGrowableCol( 2 );
    fgSizer248->SetFlexibleDirection( wxBOTH );
    fgSizer248->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer68;
    sbSizer68 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("PLL loop filter") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer86;
    fgSizer86 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer86->SetFlexibleDirection( wxBOTH );
    fgSizer86->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT13 = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("CP2"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    ID_STATICTEXT13->Wrap( -1 );
    fgSizer86->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCP2_CGEN = new wxComboBox( sbSizer68->GetStaticBox(), ID_CP2_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbCP2_CGEN->SetToolTip( wxT("Controls the value of CP2 (cap from CP output to GND) in the PLL filter") );
    
    fgSizer86->Add( cmbCP2_CGEN, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT14 = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("CP3"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT14->Wrap( -1 );
    fgSizer86->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCP3_CGEN = new wxComboBox( sbSizer68->GetStaticBox(), ID_CP3_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbCP3_CGEN->SetToolTip( wxT("Controls the value of CP3 (cap from VCO Vtune input to GND) in the PLL filter") );
    
    fgSizer86->Add( cmbCP3_CGEN, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT15 = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("CZ"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT15->Wrap( -1 );
    fgSizer86->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCZ_CGEN = new wxComboBox( sbSizer68->GetStaticBox(), ID_CZ_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbCZ_CGEN->SetToolTip( wxT("Controls the value of CZ (Zero capacitor) in the PLL filter") );
    
    fgSizer86->Add( cmbCZ_CGEN, 0, wxEXPAND, 0 );
    
    
    sbSizer68->Add( fgSizer86, 0, wxEXPAND|wxALL, 5);
    
    
    fgSizer248->Add( sbSizer68, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer254;
    fgSizer254 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer254->SetFlexibleDirection( wxBOTH );
    fgSizer254->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString cmbEN_ADCCLKH_CLKGNChoices[] = { wxT("ADC"), wxT("DAC") };
    int cmbEN_ADCCLKH_CLKGNNChoices = sizeof( cmbEN_ADCCLKH_CLKGNChoices ) / sizeof( wxString );
    cmbEN_ADCCLKH_CLKGN = new wxRadioBox( this, ID_EN_ADCCLKH_CLKGN, wxT("FCLKH to"), wxDefaultPosition, wxDefaultSize, cmbEN_ADCCLKH_CLKGNNChoices, cmbEN_ADCCLKH_CLKGNChoices, 2, wxRA_SPECIFY_COLS );
    cmbEN_ADCCLKH_CLKGN->SetSelection( 1 );
    fgSizer254->Add( cmbEN_ADCCLKH_CLKGN, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer140;
    sbSizer140 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TSP frequency") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer255;
    fgSizer255 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer255->SetFlexibleDirection( wxBOTH );
    fgSizer255->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText369 = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("RxTSP (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText369->Wrap( -1 );
    fgSizer255->Add( m_staticText369, 0, 0, 5 );
    
    lblRxTSPfreq = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("??????"), wxDefaultPosition, wxDefaultSize, 0 );
    lblRxTSPfreq->Wrap( -1 );
    fgSizer255->Add( lblRxTSPfreq, 0, 0, 5 );
    
    m_staticText367 = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("TxTSP (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText367->Wrap( -1 );
    fgSizer255->Add( m_staticText367, 0, 0, 5 );
    
    lblTxTSPfreq = new wxStaticText( sbSizer140->GetStaticBox(), wxID_ANY, wxT("??????"), wxDefaultPosition, wxDefaultSize, 0 );
    lblTxTSPfreq->Wrap( -1 );
    fgSizer255->Add( lblTxTSPfreq, 0, 0, 5 );
    
    
    sbSizer140->Add( fgSizer255, 1, wxEXPAND|wxALL, 5 );
    
    
    fgSizer254->Add( sbSizer140, 1, wxEXPAND, 5 );
    
    
    fgSizer248->Add( fgSizer254, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer70;
    sbSizer70 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Frequency controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer89;
    fgSizer89 = new wxFlexGridSizer( 0, 3, 0, 10 );
    fgSizer89->AddGrowableCol( 0 );
    fgSizer89->AddGrowableCol( 2 );
    fgSizer89->SetFlexibleDirection( wxBOTH );
    fgSizer89->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT10 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("CLK_H (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT10->Wrap( -1 );
    fgSizer89->Add( ID_STATICTEXT10, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    ID_STATICTEXT18 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("CLKH_OV_CLKL"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT18->Wrap( -1 );
    fgSizer89->Add( ID_STATICTEXT18, 1, wxALIGN_CENTER_HORIZONTAL, 10 );
    
    ID_STATICTEXT3 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("CLK_L (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer89->Add( ID_STATICTEXT3, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    txtFrequency = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), 0 );
    fgSizer89->Add( txtFrequency, 1, wxEXPAND, 5 );
    
    cmbCLKH_OV_CLKL_CGEN = new wxComboBox( sbSizer70->GetStaticBox(), ID_CLKH_OV_CLKL_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbCLKH_OV_CLKL_CGEN->SetToolTip( wxT("FCLKL here is ADC clock. FCLKH is the clock to the DAC and if no division is added to the ADC as well") );
    
    fgSizer89->Add( cmbCLKH_OV_CLKL_CGEN, 1, wxEXPAND, 5 );
    
    txtFrequencyCLKL = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 48,-1 ), wxTE_READONLY );
    txtFrequencyCLKL->Enable( false );
    
    fgSizer89->Add( txtFrequencyCLKL, 1, wxEXPAND, 5 );
    
    
    sbSizer70->Add( fgSizer89, 0, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer11;
    bSizer11 = new wxBoxSizer( wxHORIZONTAL );
    
    ID_STATICTEXT101 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Rx phase"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT101->Wrap( -1 );
    bSizer11->Add( ID_STATICTEXT101, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    rxPhase = new wxSpinCtrl( sbSizer70->GetStaticBox(), ID_GFIR1_N_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 360, 120 );
    rxPhase->Enable( false );
    rxPhase->SetToolTip( wxT("LML interface phase offset for Rx") );
    
    bSizer11->Add( rxPhase, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT102 = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Tx phase"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT102->Wrap( -1 );
    bSizer11->Add( ID_STATICTEXT102, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    txPhase = new wxSpinCtrl( sbSizer70->GetStaticBox(), ID_GFIR1_N_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 360, 120 );
    txPhase->Enable( false );
    txPhase->SetToolTip( wxT("LML interface phase offset for Tx") );
    
    bSizer11->Add( txPhase, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    chkAutoPhase = new wxCheckBox( sbSizer70->GetStaticBox(), ID_AUTO_PHASE, wxT("Auto phase"), wxDefaultPosition, wxDefaultSize, 0 );
    chkAutoPhase->SetValue(true); 
    chkAutoPhase->SetToolTip( wxT("Configure FPGA PLL phase for LML interface automatically") );
    
    bSizer11->Add( chkAutoPhase, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizer70->Add( bSizer11, 1, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer( wxHORIZONTAL );
    
    btnCalculate = new wxButton( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Calculate"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer1->Add( btnCalculate, 1, 0, 5 );
    
    btnTune = new wxButton( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Tune"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer1->Add( btnTune, 1, 0, 5 );
    
    
    sbSizer70->Add( bSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 5 );
    
    
    fgSizer248->Add( sbSizer70, 1, wxEXPAND, 5 );
    
    
    fgSizer245->Add( fgSizer248, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer88;
    fgSizer88 = new wxFlexGridSizer( 0, 3, 5, 5 );
    fgSizer88->AddGrowableCol( 0 );
    fgSizer88->AddGrowableCol( 1 );
    fgSizer88->AddGrowableCol( 2 );
    fgSizer88->SetFlexibleDirection( wxBOTH );
    fgSizer88->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer71;
    sbSizer71 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Calculated Values for Fractional Mode") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer90;
    fgSizer90 = new wxFlexGridSizer( 5, 4, 5, 10 );
    fgSizer90->SetFlexibleDirection( wxBOTH );
    fgSizer90->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT7 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("N Integer:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT7->Wrap( -1 );
    fgSizer90->Add( ID_STATICTEXT7, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    lblINT_SDM_CGEN = new wxStaticText( sbSizer71->GetStaticBox(), ID_INT_SDM_CGEN, wxT("???"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    lblINT_SDM_CGEN->Wrap( 0 );
    lblINT_SDM_CGEN->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer90->Add( lblINT_SDM_CGEN, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT23 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Ref. clock (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT23->Wrap( -1 );
    fgSizer90->Add( ID_STATICTEXT23, 1, wxALIGN_CENTER_VERTICAL, 5 );
    
    lblRefClk_MHz = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    lblRefClk_MHz->Wrap( -1 );
    lblRefClk_MHz->SetToolTip( wxT("Reference clock is received from SXR") );
    lblRefClk_MHz->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer90->Add( lblRefClk_MHz, 1, wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT21 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("N Fractional:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT21->Wrap( -1 );
    fgSizer90->Add( ID_STATICTEXT21, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    lblFRAC_SDM_CGEN = new wxStaticText( sbSizer71->GetStaticBox(), ID_FRAC_SDM_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblFRAC_SDM_CGEN->Wrap( 0 );
    fgSizer90->Add( lblFRAC_SDM_CGEN, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT2 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Divider:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer90->Add( ID_STATICTEXT2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    lblDivider = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblDivider->Wrap( 0 );
    fgSizer90->Add( lblDivider, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT25 = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Output Freq (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT25->Wrap( -1 );
    fgSizer90->Add( ID_STATICTEXT25, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    lblRealOutFrequency = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblRealOutFrequency->Wrap( 0 );
    fgSizer90->Add( lblRealOutFrequency, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizer71->Add( fgSizer90, 1, wxALL|wxEXPAND, 5 );
    
    
    fgSizer88->Add( sbSizer71, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer76;
    sbSizer76 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCO Comparators") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer97;
    fgSizer97 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer97->AddGrowableRow( 0 );
    fgSizer97->SetFlexibleDirection( wxBOTH );
    fgSizer97->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT161 = new wxStaticText( sbSizer76->GetStaticBox(), wxID_ANY, wxT("High:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT161->Wrap( -1 );
    fgSizer97->Add( ID_STATICTEXT161, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    lblVCO_CMPHO_CGEN = new wxStaticText( sbSizer76->GetStaticBox(), ID_VCO_CMPHO_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblVCO_CMPHO_CGEN->Wrap( -1 );
    fgSizer97->Add( lblVCO_CMPHO_CGEN, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT171 = new wxStaticText( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Low:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT171->Wrap( -1 );
    fgSizer97->Add( ID_STATICTEXT171, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10 );
    
    lblVCO_CMPLO_CGEN = new wxStaticText( sbSizer76->GetStaticBox(), ID_VCO_CMPLO_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblVCO_CMPLO_CGEN->Wrap( -1 );
    fgSizer97->Add( lblVCO_CMPLO_CGEN, 0, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizer76->Add( fgSizer97, 1, wxEXPAND|wxALL, 5 );
    
    btnUpdateValues1 = new wxButton( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer76->Add( btnUpdateValues1, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    fgSizer88->Add( sbSizer76, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer139;
    sbSizer139 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Coarse comparator") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer252;
    fgSizer252 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer252->SetFlexibleDirection( wxBOTH );
    fgSizer252->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT5 = new wxStaticText( sbSizer139->GetStaticBox(), wxID_ANY, wxT("STEPDONE:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT5->Wrap( -1 );
    ID_STATICTEXT5->SetToolTip( wxT("COARSE_STEPDONE_CGEN") );
    
    fgSizer252->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    lblCOARSE_STEPDONE_CGEN = new wxStaticText( sbSizer139->GetStaticBox(), ID_COARSE_STEPDONE_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblCOARSE_STEPDONE_CGEN->Wrap( -1 );
    fgSizer252->Add( lblCOARSE_STEPDONE_CGEN, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    ID_STATICTEXT6 = new wxStaticText( sbSizer139->GetStaticBox(), wxID_ANY, wxT("COMPO:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT6->Wrap( -1 );
    ID_STATICTEXT6->SetToolTip( wxT("COARSEPLL_COMPO_CGEN") );
    
    fgSizer252->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    lblCOARSEPLL_COMPO_CGEN = new wxStaticText( sbSizer139->GetStaticBox(), ID_COARSEPLL_COMPO_CGEN, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblCOARSEPLL_COMPO_CGEN->Wrap( -1 );
    fgSizer252->Add( lblCOARSEPLL_COMPO_CGEN, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    
    sbSizer139->Add( fgSizer252, 0, wxEXPAND|wxALL, 5 );
    
    btnUpdateCoarse = new wxButton( sbSizer139->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer139->Add( btnUpdateCoarse, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    fgSizer88->Add( sbSizer139, 1, wxEXPAND, 5 );
    
    
    fgSizer245->Add( fgSizer88, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer67;
    sbSizer67 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCO") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer85;
    fgSizer85 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer85->AddGrowableCol( 1 );
    fgSizer85->SetFlexibleDirection( wxBOTH );
    fgSizer85->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT20 = new wxStaticText( sbSizer67->GetStaticBox(), wxID_ANY, wxT("CSW_VCO_CGEN"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT20->Wrap( -1 );
    fgSizer85->Add( ID_STATICTEXT20, 1, wxALIGN_LEFT|wxALIGN_BOTTOM, 5 );
    
    cmbCSW_VCO_CGEN = new NumericSlider( sbSizer67->GetStaticBox(), ID_CSW_VCO_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    fgSizer85->Add( cmbCSW_VCO_CGEN, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT9 = new wxStaticText( sbSizer67->GetStaticBox(), wxID_ANY, wxT("Scales VCO bias current"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT9->Wrap( -1 );
    fgSizer85->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_VCO_CGEN = new NumericSlider( sbSizer67->GetStaticBox(), ID_ICT_VCO_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
    fgSizer85->Add( cmbICT_VCO_CGEN, 1, wxEXPAND, 5 );
    
    btnShowVCO = new wxButton( sbSizer67->GetStaticBox(), wxID_ANY, wxT("VCO params"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer85->Add( btnShowVCO, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizer67->Add( fgSizer85, 1, wxEXPAND, 0 );
    
    
    fgSizer245->Add( sbSizer67, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer69;
    sbSizer69 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Charge pump scales") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer87;
    fgSizer87 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer87->AddGrowableCol( 1 );
    fgSizer87->SetFlexibleDirection( wxBOTH );
    fgSizer87->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT16 = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Offset current"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT16->Wrap( -1 );
    fgSizer87->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbIOFFSET_CP_CGEN = new NumericSlider( sbSizer69->GetStaticBox(), ID_IOFFSET_CP_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
    cmbIOFFSET_CP_CGEN->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer87->Add( cmbIOFFSET_CP_CGEN, 1, wxEXPAND, 5 );
    
    ID_STATICTEXT17 = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Pulse current"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT17->Wrap( -1 );
    fgSizer87->Add( ID_STATICTEXT17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbIPULSE_CP_CGEN = new NumericSlider( sbSizer69->GetStaticBox(), ID_IPULSE_CP_CGEN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
    cmbIPULSE_CP_CGEN->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer87->Add( cmbIPULSE_CP_CGEN, 1, wxEXPAND, 5 );
    
    
    sbSizer69->Add( fgSizer87, 1, wxEXPAND, 0 );
    
    
    fgSizer245->Add( sbSizer69, 1, wxEXPAND, 5 );
    
    sizerR3 = new wxFlexGridSizer( 0, 2, 0, 0 );
    sizerR3->SetFlexibleDirection( wxBOTH );
    sizerR3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    
    fgSizer245->Add( sizerR3, 1, wxEXPAND, 5 );
    
    
    fgSizer244->Add( fgSizer245, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( fgSizer244 );
    this->Layout();
    fgSizer244->Fit( this );
    
    // Connect Events
    chkPD_CP_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkPD_FDIV_FB_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkPD_FDIV_O_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkPD_SDM_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkPD_VCO_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkPD_VCO_COMP_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkEN_G_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_VCO_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkRESET_N_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkEN_COARSE_CKLGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkEN_INTONLY_SDM_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkEN_SDM_CLK_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkREV_SDMCLK_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkSX_DITHER_EN_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkREV_CLKDAC_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkREV_CLKADC_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkREVPH_PFD_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    chkCOARSE_START_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    rgrSEL_SDMCLK_CGEN->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbTST_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbCP2_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbCP3_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbCZ_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbEN_ADCCLKH_CLKGN->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbCLKH_OV_CLKL_CGEN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    rxPhase->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
    txPhase->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
    chkAutoPhase->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::OnAutoPhase ), NULL, this );
    btnCalculate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::onbtnCalculateClick ), NULL, this );
    btnTune->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::onbtnTuneClick ), NULL, this );
    btnUpdateValues1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::OnbtnReadComparators ), NULL, this );
    btnUpdateCoarse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::OnbtnUpdateCoarse ), NULL, this );
    cmbCSW_VCO_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbICT_VCO_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    btnShowVCO->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCLKGEN_view::OnShowVCOclicked ), NULL, this );
    cmbIOFFSET_CP_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );
    cmbIPULSE_CP_CGEN->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCLKGEN_view::ParameterChangeHandler ), NULL, this );

    sizerR3->Add(new wxStaticText(this, wxID_ANY, _("CMPLO_CTRL:")), 1, wxALIGN_CENTER_VERTICAL, 0);
    cmbCMPLO_CTRL = new wxComboBox(this, wxID_ANY);
    cmbCMPLO_CTRL->Append(_("Low threshold is set to 0.18V"));
    cmbCMPLO_CTRL->Append(_("Low threshold is set to 0.1V"));
    cmbCMPLO_CTRL->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(lms7002_pnlCLKGEN_view::ParameterChangeHandler), NULL, this);
    sizerR3->Add(cmbCMPLO_CTRL, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
    wndId2Enum[cmbCMPLO_CTRL] = LMS7_CMPLO_CTRL_CGEN;

    wndId2Enum[cmbCLKH_OV_CLKL_CGEN] = LMS7param(CLKH_OV_CLKL_CGEN);
    wndId2Enum[chkCOARSE_START_CGEN] = LMS7param(COARSE_START_CGEN);
    wndId2Enum[cmbCP2_CGEN] = LMS7param(CP2_CGEN);
    wndId2Enum[cmbCP3_CGEN] = LMS7param(CP3_CGEN);
    wndId2Enum[cmbCSW_VCO_CGEN] = LMS7param(CSW_VCO_CGEN);
    wndId2Enum[cmbCZ_CGEN] = LMS7param(CZ_CGEN);
    wndId2Enum[chkEN_COARSE_CKLGEN] = LMS7param(EN_COARSE_CKLGEN);
    wndId2Enum[cmbEN_ADCCLKH_CLKGN] = LMS7param(EN_ADCCLKH_CLKGN);
    wndId2Enum[chkEN_G_CGEN] = LMS7param(EN_G_CGEN);
    wndId2Enum[chkEN_INTONLY_SDM_CGEN] = LMS7param(EN_INTONLY_SDM_CGEN);
    wndId2Enum[chkEN_SDM_CLK_CGEN] = LMS7param(EN_SDM_CLK_CGEN);
    wndId2Enum[cmbICT_VCO_CGEN] = LMS7param(ICT_VCO_CGEN);
    wndId2Enum[lblINT_SDM_CGEN] = LMS7param(INT_SDM_CGEN);
    wndId2Enum[cmbIOFFSET_CP_CGEN] = LMS7param(IOFFSET_CP_CGEN);
    wndId2Enum[cmbIPULSE_CP_CGEN] = LMS7param(IPULSE_CP_CGEN);
    wndId2Enum[chkPD_CP_CGEN] = LMS7param(PD_CP_CGEN);
    wndId2Enum[chkPD_FDIV_FB_CGEN] = LMS7param(PD_FDIV_FB_CGEN);
    wndId2Enum[chkPD_FDIV_O_CGEN] = LMS7param(PD_FDIV_O_CGEN);
    wndId2Enum[chkPD_SDM_CGEN] = LMS7param(PD_SDM_CGEN);
    wndId2Enum[chkPD_VCO_CGEN] = LMS7param(PD_VCO_CGEN);
    wndId2Enum[chkPD_VCO_COMP_CGEN] = LMS7param(PD_VCO_COMP_CGEN);
    wndId2Enum[chkRESET_N_CGEN] = LMS7param(RESET_N_CGEN);
    wndId2Enum[chkREVPH_PFD_CGEN] = LMS7param(REVPH_PFD_CGEN);
    wndId2Enum[chkREV_CLKADC_CGEN] = LMS7param(REV_CLKADC_CGEN);
    wndId2Enum[chkREV_CLKDAC_CGEN] = LMS7param(REV_CLKDAC_CGEN);
    wndId2Enum[chkREV_SDMCLK_CGEN] = LMS7param(REV_SDMCLK_CGEN);
    wndId2Enum[rgrSEL_SDMCLK_CGEN] = LMS7param(SEL_SDMCLK_CGEN);
    wndId2Enum[chkSPDUP_VCO_CGEN] = LMS7param(SPDUP_VCO_CGEN);
    wndId2Enum[chkSX_DITHER_EN_CGEN] = LMS7param(SX_DITHER_EN_CGEN);
    wndId2Enum[cmbTST_CGEN] = LMS7param(TST_CGEN);

    wxArrayString temp;
    temp.clear();
    temp.push_back(_("1"));
    temp.push_back(_("2"));
    temp.push_back(_("4"));
    temp.push_back(_("8"));
    cmbCLKH_OV_CLKL_CGEN->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        temp.push_back(wxString::Format(_("%.3f pF"), (i * 6 * 63.2) / 1000.0));
    }
    cmbCP2_CGEN->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        temp.push_back(wxString::Format(_("%.3f pF"), (i*248.0) / 1000.0));
    }
    cmbCP3_CGEN->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
    {
        temp.push_back(wxString::Format(_("%.0f pF"), (i * 8 * 365.0) / 1000.0));
    }
    cmbCZ_CGEN->Set(temp);

    temp.clear();
    temp.push_back(_("TST disabled"));
    temp.push_back(_("TST[0]=ADC clk; TST[1]=DAC clk; TSTA=Hi Z"));
    temp.push_back(_("TST[0]=SDM clk; TST[1]=FBD output; TSTA=Vtune@60kOhm"));
    temp.push_back(_("TST[0]=Ref clk; TST[1]=FBD output; TSTA=Vtune@10kOhm"));
    temp.push_back(_("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Hi Z"));
    temp.push_back(_("TST[0]=CP Down offset; TST[1]=CP Up offset; TSTA=Hi Z"));
    temp.push_back(_("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Vtune@60kOhm"));
    temp.push_back(_("TST[0]=Hi Z; TST[1]=Hi Z; TSTA=Vtune@10kOhm"));
    cmbTST_CGEN->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlCLKGEN_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    double freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_CGEN,&freq);
    txtFrequency->SetValue(wxString::Format(_("%.3f"), freq));
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"), freq));
}

void lms7002_pnlCLKGEN_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlCLKGEN_view::ParameterChangeHandler(wxCommandEvent& event)
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

    LMS_WriteParam(lmsControl,parameter,event.GetInt());

    if(event.GetEventObject() == cmbEN_ADCCLKH_CLKGN)
        UpdateInterfaceFrequencies();
    else if(event.GetEventObject() == cmbCSW_VCO_CGEN)
    {
        OnbtnReadComparators(event);
    }
    else if(event.GetEventObject() == cmbCLKH_OV_CLKL_CGEN)
    {
        double cgenFreq;
        txtFrequency->GetValue().ToDouble(&cgenFreq);
        txtFrequencyCLKL->SetValue(wxString::Format(_("%.3f"), cgenFreq / pow(2.0, cmbCLKH_OV_CLKL_CGEN->GetSelection())));
        UpdateInterfaceFrequencies();
        UpdateCLKL();
    }
}

 void lms7002_pnlCLKGEN_view::OnAutoPhase(wxCommandEvent& event)
 {
     bool disabled = this->chkAutoPhase->GetValue();
     this->txPhase->Enable(!disabled);
     this->rxPhase->Enable(!disabled);
 }

void lms7002_pnlCLKGEN_view::onbtnCalculateClick(wxSpinEvent& event)
{
    double freqMHz;
    auto device = ((LMS7_Device*)lmsControl);
    txtFrequency->GetValue().ToDouble(&freqMHz);
    LMS7002M* lms = device->GetLMS();
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
    int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    if (lms->SetInterfaceFrequency(freqMHz*1e6, interp, decim))
    {
        wxMessageBox(_("CLKGEN: failed to set interface frequency"));
        return;
    }
    device->SetFPGAInterfaceFreq(interp, decim, txPhase->GetValue(), rxPhase->GetValue());

    auto freq = lms->GetFrequencyCGEN();
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
    UpdateGUI();
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    wxCommandEvent cmd;
    cmd.SetString(_("CGEN frequency set to ") + lblRealOutFrequency->GetLabel() + _(" MHz"));
    cmd.SetEventType(LOG_MESSAGE);
    cmd.SetInt(lime::LOG_LEVEL_INFO);
    wxPostEvent(this, cmd);
}

void lms7002_pnlCLKGEN_view::onbtnCalculateClick( wxCommandEvent& event )
{
    double freqMHz;
    auto device = ((LMS7_Device*)lmsControl);
    txtFrequency->GetValue().ToDouble(&freqMHz);
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
    int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
    if (lms->SetInterfaceFrequency(freqMHz*1e6, interp, decim))
    {
        wxMessageBox(_("CLKGEN: failed to set interface frequency"));
        auto freq = lms->GetFrequencyCGEN();
        lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
        UpdateGUI();
        return ;
    }

    int status;
    if (this->chkAutoPhase->GetValue())
        status = device->SetFPGAInterfaceFreq(interp, decim);
    else
        status = device->SetFPGAInterfaceFreq(interp, decim, txPhase->GetValue(), rxPhase->GetValue());
    if (status != 0)
        wxMessageBox(_("CLKGEN: failed to set interface frequency"));

    auto freq = lms->GetFrequencyCGEN();
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
    UpdateGUI();
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    wxCommandEvent cmd;
    cmd.SetString(_("CGEN frequency set to ") + lblRealOutFrequency->GetLabel() + _(" MHz"));
    cmd.SetEventType(LOG_MESSAGE);
    cmd.SetInt(lime::LOG_LEVEL_INFO);
    wxPostEvent(this, cmd);
}

void lms7002_pnlCLKGEN_view::onbtnTuneClick( wxCommandEvent& event )
{
    auto device = ((LMS7_Device*)lmsControl);
    LMS7002M* lms = device->GetLMS();
    lms->Modify_SPI_Reg_bits(LMS7param(MAC),1,true);
    if (lms->TuneVCO(lime::LMS7002M::VCO_CGEN)!=0)
    {
        wxMessageBox(wxString(_("CLKGEN VCO Tune failed")));
        return ;
    }

    int interp = lms->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
    int decim = lms->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));

    int status;
    if (this->chkAutoPhase->GetValue())
        status = device->SetFPGAInterfaceFreq(interp, decim);
    else
        status = device->SetFPGAInterfaceFreq(interp, decim, txPhase->GetValue(), rxPhase->GetValue());
    if (status != 0)
        wxMessageBox(_("CLKGEN VCO Tune: failed to set interface frequency"));

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(CSW_VCO_CGEN),&value);
    cmbCSW_VCO_CGEN->SetValue(value);
    OnbtnReadComparators(event);
}

void lms7002_pnlCLKGEN_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    wxCommandEvent evt;
    OnbtnReadComparators(evt);
    UpdateInterfaceFrequencies();
    UpdateCLKL();
    double freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_CGEN,&freq);
    lblRealOutFrequency->SetLabel(wxString::Format(_("%f"), freq / 1e6));
    txtFrequency->SetValue(wxString::Format(_("%.3f"), freq / 1e6));
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblRefClk_MHz->SetLabel(wxString::Format(_("%.3f"),freq / 1e6 ));
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(FRAC_SDM_CGEN_MSB),&value);
    int fracValue = value << 16;
    LMS_ReadParam(lmsControl,LMS7param(FRAC_SDM_CGEN_LSB),&value);
    fracValue |= value;
    lblFRAC_SDM_CGEN->SetLabel(wxString::Format("%i", fracValue));
    LMS_ReadParam(lmsControl,LMS7param(DIV_OUTCH_CGEN),&value);
    lblDivider->SetLabel(wxString::Format("2*%i", value+1));
}

void lms7002_pnlCLKGEN_view::UpdateInterfaceFrequencies()
{
    double freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_RXTSP,&freq);
    lblRxTSPfreq->SetLabel(wxString::Format(_("%.3f"), freq / 1e6));
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_TXTSP,&freq);
    lblTxTSPfreq->SetLabel(wxString::Format(_("%.3f"), freq / 1e6));
}

void lms7002_pnlCLKGEN_view::OnbtnReadComparators(wxCommandEvent& event)
{

    uint16_t param;
    LMS_ReadParam(lmsControl,LMS7param(VCO_CMPHO_CGEN),&param);

    lblVCO_CMPHO_CGEN->SetLabel(wxString::Format(_("%i"), param));
    if (param == 1)
        lblVCO_CMPHO_CGEN->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPHO_CGEN->SetBackgroundColour(*wxRED);

    LMS_ReadParam(lmsControl,LMS7param(VCO_CMPLO_CGEN),&param);

    lblVCO_CMPLO_CGEN->SetLabel(wxString::Format(_("%i"), param));
    if (param == 0)
        lblVCO_CMPLO_CGEN->SetBackgroundColour(*wxGREEN);
    else
        lblVCO_CMPLO_CGEN->SetBackgroundColour(*wxRED);
}

void lms7002_pnlCLKGEN_view::OnbtnUpdateCoarse(wxCommandEvent& event)
{
    uint16_t param;
    LMS_ReadParam(lmsControl,LMS7param(COARSE_STEPDONE),&param);
    lblCOARSE_STEPDONE_CGEN->SetLabel(wxString::Format(_("%i"), param));
    LMS_ReadParam(lmsControl,LMS7param(COARSEPLL_COMPO_CGEN),&param);
    lblCOARSEPLL_COMPO_CGEN->SetLabel(wxString::Format(_("%i"), param));
}

void lms7002_pnlCLKGEN_view::UpdateCLKL()
{
    int dMul;
    dMul = cmbCLKH_OV_CLKL_CGEN->GetSelection();
    double cgenFreq;
    txtFrequency->GetValue().ToDouble(&cgenFreq);
    double clklfreq = cgenFreq / pow(2.0, dMul);
    txtFrequencyCLKL->SetLabel(wxString::Format("%.3f", clklfreq));
}

void lms7002_pnlCLKGEN_view::OnShowVCOclicked(wxCommandEvent& event)
{
    lms7002_dlgVCOfrequencies* dlg = new lms7002_dlgVCOfrequencies(this, lmsControl);
    dlg->ShowModal();
    dlg->Destroy();
}
