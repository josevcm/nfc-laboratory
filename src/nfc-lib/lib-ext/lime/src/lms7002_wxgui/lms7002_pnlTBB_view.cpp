#include "lms7002_pnlTBB_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include "lms7_device.h"
#include "lms7suiteAppFrame.h"

using namespace lime;

lms7002_pnlTBB_view::lms7002_pnlTBB_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = wxALL;
    wxFlexGridSizer* fgSizer57;
    fgSizer57 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer57->SetFlexibleDirection( wxBOTH );
    fgSizer57->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerPowerDowns;
    sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer59;
    fgSizer59 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer59->SetFlexibleDirection( wxBOTH );
    fgSizer59->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkPD_LPFH_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFH_TBB, wxT("LPFH_TBB biquad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_LPFH_TBB->SetToolTip( wxT("This selectively powers down the LPFH_TBB biquad") );
    
    fgSizer59->Add( chkPD_LPFH_TBB, 0, flags, 0 );
    
    chkPD_LPFIAMP_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFIAMP_TBB, wxT("LPFIAMP_TBB front-end current amp"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_LPFIAMP_TBB->SetToolTip( wxT("selectively powers down the LPFIAMP_TBB front-end current amp of the transmitter baseband") );
    
    fgSizer59->Add( chkPD_LPFIAMP_TBB, 0, flags, 0 );
    
    chkPD_LPFLAD_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFLAD_TBB, wxT("LPFLAD_TBB low pass ladder filter"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_LPFLAD_TBB->SetToolTip( wxT("This selectively powers down the LPFLAD_TBB low pass ladder filter of the transmitter baseband") );
    
    fgSizer59->Add( chkPD_LPFLAD_TBB, 0, flags, 0 );
    
    chkPD_LPFS5_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFS5_TBB, wxT("LPFS5_TBB low pass real-pole filter"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_LPFS5_TBB->SetToolTip( wxT("This selectively powers down the LPFS5_TBB low pass real-pole filter of the transmitter baseband") );
    
    fgSizer59->Add( chkPD_LPFS5_TBB, 0, flags, 0 );
    
    chkEN_G_TBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_TBB, wxT("Enable TBB module"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_G_TBB->SetToolTip( wxT("Enable control for all the TBB_TOP power downs") );
    
    fgSizer59->Add( chkEN_G_TBB, 0, flags, 0 );
    
    wxStaticBoxSizer* sbSizer39;
    sbSizer39 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
    
    chkEN_DIR_TBB = new wxCheckBox( sbSizer39->GetStaticBox(), ID_EN_DIR_TBB, wxT("Direct control of PDs and ENs"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_DIR_TBB->SetToolTip( wxT("Enables direct control of PDs and ENs for TBB module") );
    
    sbSizer39->Add( chkEN_DIR_TBB, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    fgSizer59->Add( sbSizer39, 1, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
    
    
    sbSizerPowerDowns->Add( fgSizer59, 0, wxEXPAND, 0 );
    
    
    fgSizer57->Add( sbSizerPowerDowns, 0, wxEXPAND, 0 );
    
    wxFlexGridSizer* fgSizer214;
    fgSizer214 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer214->AddGrowableCol( 0 );
    fgSizer214->SetFlexibleDirection( wxBOTH );
    fgSizer214->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer58;
    fgSizer58 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer58->AddGrowableCol( 1 );
    fgSizer58->SetFlexibleDirection( wxBOTH );
    fgSizer58->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkBYPLADDER_TBB = new wxCheckBox( this, ID_BYPLADDER_TBB, wxT("Bypass LPF ladder of TBB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkBYPLADDER_TBB->SetToolTip( wxT("This signal bypasses the LPF ladder of TBB and directly connects the output of current amplifier to the null port of the real pole stage of TBB low pass filter") );
    
    fgSizer58->Add( chkBYPLADDER_TBB, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    
    fgSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
    
    chkR5_LPF_BYP_TBB = new wxCheckBox( this, ID_BYPLADDER_TBB, wxT("Bypass LPFS5 filter capacitor banks"), wxDefaultPosition, wxDefaultSize, 0 );
    chkR5_LPF_BYP_TBB->SetToolTip( wxT("This signal bypasses the LPF ladder of TBB and directly connects the output of current amplifier to the null port of the real pole stage of TBB low pass filter") );
    
    fgSizer58->Add( chkR5_LPF_BYP_TBB, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
    
    
    fgSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
    
    ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("Tx BB loopback"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer58->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLOOPB_TBB = new wxComboBox( this, ID_LOOPB_TBB, wxEmptyString, wxDefaultPosition, wxSize( 197,-1 ), 0, NULL, 0 ); 
    cmbLOOPB_TBB->SetToolTip( wxT("This controls which signal is connected to the loopback output pins. Note: when both the lowpass ladder and real pole are powered down, the output of the active highband biquad is routed to the loopb output") );
    
    fgSizer58->Add( cmbLOOPB_TBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT5 = new wxStaticText( this, wxID_ANY, wxT("Enable Tx IQ analog input"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT5->Wrap( -1 );
    fgSizer58->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbTSTIN_TBB = new wxComboBox( this, ID_TSTIN_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbTSTIN_TBB->SetToolTip( wxT("This control selects where the input test signal (vinp/n_aux_bbq/i) is routed to as well as disabling the route.") );
    
    fgSizer58->Add( cmbTSTIN_TBB, 0, wxEXPAND, 0 );
    
    
    fgSizer214->Add( fgSizer58, 0, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer192;
    fgSizer192 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer192->AddGrowableCol( 1 );
    fgSizer192->SetFlexibleDirection( wxBOTH );
    fgSizer192->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("Frontend gain"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer192->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCG_IAMP_TBB = new NumericSlider( this, ID_CG_IAMP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 63, 0 );
    fgSizer192->Add( cmbCG_IAMP_TBB, 0, wxEXPAND, 5 );
    
    
    fgSizer214->Add( fgSizer192, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer141;
    sbSizer141 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Reference bias current") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer260;
    fgSizer260 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer260->AddGrowableCol( 1 );
    fgSizer260->SetFlexibleDirection( wxBOTH );
    fgSizer260->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT3 = new wxStaticText( sbSizer141->GetStaticBox(), wxID_ANY, wxT("IAMP main bias current sources"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer260->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_IAMP_FRP_TBB = new NumericSlider( sbSizer141->GetStaticBox(), ID_ICT_IAMP_FRP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
    fgSizer260->Add( cmbICT_IAMP_FRP_TBB, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT4 = new wxStaticText( sbSizer141->GetStaticBox(), wxID_ANY, wxT("IAMP cascade transistors gate voltage"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer260->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_IAMP_GG_FRP_TBB = new NumericSlider( sbSizer141->GetStaticBox(), ID_ICT_IAMP_GG_FRP_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
    fgSizer260->Add( cmbICT_IAMP_GG_FRP_TBB, 0, wxEXPAND, 5 );
    
    
    sbSizer141->Add( fgSizer260, 1, wxEXPAND, 5 );
    
    
    fgSizer214->Add( sbSizer141, 1, wxEXPAND, 5 );
    
    
    fgSizer57->Add( fgSizer214, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerOpamp;
    sbSizerOpamp = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Operational amplifier") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer60;
    fgSizer60 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer60->AddGrowableCol( 1 );
    fgSizer60->SetFlexibleDirection( wxBOTH );
    fgSizer60->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT6 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Output stage bias current low band real pole filter"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT6->Wrap( -1 );
    fgSizer60->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPFS5_F_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFS5_F_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPFS5_F_TBB->SetToolTip( wxT("This controls the operational amplifier's output stage bias current of the low band real pole filter of the transmitter's baseband") );
    
    fgSizer60->Add( cmbICT_LPFS5_F_TBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT7 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Input stage bias current of low band real pole filter"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT7->Wrap( -14 );
    fgSizer60->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPFS5_PT_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFS5_PT_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPFS5_PT_TBB->SetToolTip( wxT("This controls the operational amplifier's input stage bias current of the low band real pole filter of the transmitter's baseband") );
    
    fgSizer60->Add( cmbICT_LPFS5_PT_TBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT8 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Input stage bias reference current of high band low pass filter"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT8->Wrap( -1 );
    fgSizer60->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPF_H_PT_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPF_H_PT_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPF_H_PT_TBB->SetToolTip( wxT("This controls the operational amplifiers input stage bias reference current of the high band low pass filter of the transmitter's baseband ") );
    
    fgSizer60->Add( cmbICT_LPF_H_PT_TBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT9 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Output stage bias reference current of high band low pass filter"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT9->Wrap( -1 );
    fgSizer60->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPFH_F_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFH_F_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPFH_F_TBB->SetToolTip( wxT("controls the operational amplifiers output stage bias reference current of the high band low pass filter of the transmitter's baseband (LPFH_TBB)") );
    
    fgSizer60->Add( cmbICT_LPFH_F_TBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT10 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Output stage bias reference of low band ladder filter"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT10->Wrap( -1 );
    fgSizer60->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPFLAD_F_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFLAD_F_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPFLAD_F_TBB->SetToolTip( wxT("This controls the operational amplfiers' output stages bias referene current of the low band ladder filter of the transmisster's baseband") );
    
    fgSizer60->Add( cmbICT_LPFLAD_F_TBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT11 = new wxStaticText( sbSizerOpamp->GetStaticBox(), wxID_ANY, wxT("Input stage bias reference of low band ladder filter"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT11->Wrap( -1 );
    fgSizer60->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPFLAD_PT_TBB = new wxComboBox( sbSizerOpamp->GetStaticBox(), ID_ICT_LPFLAD_PT_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPFLAD_PT_TBB->SetToolTip( wxT("This controls the operational amplifers' input stages bias reference current of the low band ladder filter of the transmitter's baseband") );
    
    fgSizer60->Add( cmbICT_LPFLAD_PT_TBB, 0, wxEXPAND, 0 );
    
    
    sbSizerOpamp->Add( fgSizer60, 0, wxEXPAND, 0 );
    
    
    fgSizer57->Add( sbSizerOpamp, 0, wxEXPAND, 0 );
    
    wxStaticBoxSizer* sbSizerResistorBanks;
    sbSizerResistorBanks = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TxLPF resistor banks") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer61;
    fgSizer61 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer61->AddGrowableCol( 1 );
    fgSizer61->SetFlexibleDirection( wxBOTH );
    fgSizer61->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT12 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("LPFH equivalent resistance stage"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT12->Wrap( -1 );
    fgSizer61->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRCAL_LPFH_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_RCAL_LPFH_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    fgSizer61->Add( cmbRCAL_LPFH_TBB, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT13 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("LPFLAD equivalent resistance stage"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT13->Wrap( -1 );
    fgSizer61->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRCAL_LPFLAD_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_RCAL_LPFLAD_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    fgSizer61->Add( cmbRCAL_LPFLAD_TBB, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT15 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("LPFS5 equivalent resistance stage"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT15->Wrap( -1 );
    fgSizer61->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRCAL_LPFS5_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_RCAL_LPFS5_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    fgSizer61->Add( cmbRCAL_LPFS5_TBB, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT14 = new wxStaticText( sbSizerResistorBanks->GetStaticBox(), wxID_ANY, wxT("Common control signal for all TBB filters"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT14->Wrap( -1 );
    fgSizer61->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCCAL_LPFLAD_TBB = new NumericSlider( sbSizerResistorBanks->GetStaticBox(), ID_CCAL_LPFLAD_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
    fgSizer61->Add( cmbCCAL_LPFLAD_TBB, 0, wxEXPAND, 5 );
    
    
    sbSizerResistorBanks->Add( fgSizer61, 0, wxEXPAND, 0 );
    
    
    fgSizer57->Add( sbSizerResistorBanks, 0, flags, 0 );
    
    wxStaticBoxSizer* sbSizerRxFilters;
    sbSizerRxFilters = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Tx Filters") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer245;
    fgSizer245 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer245->SetFlexibleDirection( wxBOTH );
    fgSizer245->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    lblFilterInputName = new wxStaticText( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("RF bandwidth (MHz)"), wxDefaultPosition, wxDefaultSize, 0 );
    lblFilterInputName->Wrap( -1 );
    fgSizer245->Add( lblFilterInputName, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    txtFilterFrequency = new wxTextCtrl( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("56"), wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFilterFrequency->HasFlag( wxTE_MULTILINE ) )
    {
    txtFilterFrequency->SetMaxLength( 10 );
    }
    #else
    txtFilterFrequency->SetMaxLength( 10 );
    #endif
    fgSizer245->Add( txtFilterFrequency, 0, 0, 5 );
    
    btnTuneFilter = new wxButton( sbSizerRxFilters->GetStaticBox(), ID_BTN_TUNE_FILTER, wxT("TUNE"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer245->Add( btnTuneFilter, 0, wxEXPAND, 5 );
    
    btnTuneTxGain = new wxButton( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("Tune Gain"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer245->Add( btnTuneTxGain, 0, 0, 5 );
    
    
    sbSizerRxFilters->Add( fgSizer245, 1, wxEXPAND, 5 );
    
    
    fgSizer57->Add( sbSizerRxFilters, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( fgSizer57 );
    this->Layout();
    fgSizer57->Fit( this );
    
    // Connect Events
    chkPD_LPFH_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    chkPD_LPFIAMP_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    chkPD_LPFLAD_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    chkPD_LPFS5_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    chkEN_G_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    chkEN_DIR_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    chkBYPLADDER_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    chkR5_LPF_BYP_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbLOOPB_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbTSTIN_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbCG_IAMP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_IAMP_FRP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_IAMP_GG_FRP_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPFS5_F_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPFS5_PT_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPF_H_PT_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPFH_F_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPFLAD_F_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPFLAD_PT_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbRCAL_LPFH_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbRCAL_LPFLAD_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbRCAL_LPFS5_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    cmbCCAL_LPFLAD_TBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTBB_view::ParameterChangeHandler ), NULL, this );
    btnTuneFilter->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::OnbtnTuneFilter ), NULL, this );
    btnTuneTxGain->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTBB_view::OnbtnTuneTxGain ), NULL, this );

    wndId2Enum[chkBYPLADDER_TBB] = LMS7param(BYPLADDER_TBB);
    wndId2Enum[cmbCCAL_LPFLAD_TBB] = LMS7param(CCAL_LPFLAD_TBB);
    wndId2Enum[cmbCG_IAMP_TBB] = LMS7param(CG_IAMP_TBB);
    wndId2Enum[chkEN_G_TBB] = LMS7param(EN_G_TBB);
    wndId2Enum[cmbICT_IAMP_FRP_TBB] = LMS7param(ICT_IAMP_FRP_TBB);
    wndId2Enum[cmbICT_IAMP_GG_FRP_TBB] = LMS7param(ICT_IAMP_GG_FRP_TBB);
    wndId2Enum[cmbICT_LPFH_F_TBB] = LMS7param(ICT_LPFH_F_TBB);
    wndId2Enum[cmbICT_LPFLAD_F_TBB] = LMS7param(ICT_LPFLAD_F_TBB);
    wndId2Enum[cmbICT_LPFLAD_PT_TBB] = LMS7param(ICT_LPFLAD_PT_TBB);
    wndId2Enum[cmbICT_LPFS5_F_TBB] = LMS7param(ICT_LPFS5_F_TBB);
    wndId2Enum[cmbICT_LPFS5_PT_TBB] = LMS7param(ICT_LPFS5_PT_TBB);
    wndId2Enum[cmbICT_LPF_H_PT_TBB] = LMS7param(ICT_LPF_H_PT_TBB);
    wndId2Enum[cmbLOOPB_TBB] = LMS7param(LOOPB_TBB);
    wndId2Enum[chkPD_LPFH_TBB] = LMS7param(PD_LPFH_TBB);
    wndId2Enum[chkPD_LPFIAMP_TBB] = LMS7param(PD_LPFIAMP_TBB);
    wndId2Enum[chkPD_LPFLAD_TBB] = LMS7param(PD_LPFLAD_TBB);
    wndId2Enum[chkPD_LPFS5_TBB] = LMS7param(PD_LPFS5_TBB);
    wndId2Enum[cmbRCAL_LPFH_TBB] = LMS7param(RCAL_LPFH_TBB);
    wndId2Enum[cmbRCAL_LPFLAD_TBB] = LMS7param(RCAL_LPFLAD_TBB);
    wndId2Enum[cmbRCAL_LPFS5_TBB] = LMS7param(RCAL_LPFS5_TBB);
    wndId2Enum[cmbTSTIN_TBB] = LMS7param(TSTIN_TBB);
    wndId2Enum[chkEN_DIR_TBB] = LMS7param(EN_DIR_TBB);
    wndId2Enum[chkR5_LPF_BYP_TBB] = LMS7param(R5_LPF_BYP_TBB);

    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<32; ++i)
    {
        temp.push_back(wxString::Format(_("%i"), i));
    }
    cmbICT_LPFH_F_TBB->Set(temp);
    cmbICT_LPFLAD_F_TBB->Set(temp);
    cmbICT_LPFLAD_PT_TBB->Set(temp);
    cmbICT_LPFS5_F_TBB->Set(temp);
    cmbICT_LPFS5_PT_TBB->Set(temp);
    cmbICT_LPF_H_PT_TBB->Set(temp);

    temp.clear();
    temp.push_back(_("Disabled"));
    temp.push_back(_("DAC current output"));
    temp.push_back(_("LPFLAD ladder output"));
    temp.push_back(_("TBB output"));
    temp.push_back(_("DAC current output (IQ swap)"));
    temp.push_back(_("LPFLAD ladder output (IQ swap)"));
    temp.push_back(_("TBB output (IQ swap)"));
    cmbLOOPB_TBB->Set(temp);

    temp.clear();
    temp.push_back("Disabled");
    temp.push_back("to Highband filter");
    temp.push_back("to Lowband filter");
    temp.push_back("to current amplifier");
    cmbTSTIN_TBB->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlTBB_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
        value = 1;
    chkR5_LPF_BYP_TBB->Enable(value);
}

void lms7002_pnlTBB_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlTBB_view::ParameterChangeHandler(wxCommandEvent& event)
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
}

void lms7002_pnlTBB_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    //check if B channel is enabled
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&value);
    if (value >= 2)
    {
        LMS_ReadParam(lmsControl,LMS7param(MIMO_SISO),&value);
        if (value != 0)
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
    }
    LMS_ReadParam(lmsControl,LMS7param(TRX_GAIN_SRC),&value);
    cmbCG_IAMP_TBB->Enable(!value);
}

void lms7002_pnlTBB_view::OnbtnTuneFilter( wxCommandEvent& event )
{
    double input1;
    txtFilterFrequency->GetValue().ToDouble(&input1);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status;
    status = LMS_SetLPFBW(lmsControl,LMS_CH_TX,ch,input1*1e6);

    if (status != 0) {
        wxMessageBox(wxString(_("Tx calibration failed")));
        return;
    }
    LMS_Synchronize(lmsControl,false);
    UpdateGUI();
}

void lms7002_pnlTBB_view::OnbtnTuneTxGain( wxCommandEvent& event )
{
    double input1;
    txtFilterFrequency->GetValue().ToDouble(&input1);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
    int status = lms->CalibrateTxGain(0, nullptr);

    if (status != 0)
    {
        wxMessageBox(wxString(_("Tx gain calibration failed")));
        return;
    }
    LMS_Synchronize(lmsControl,false);
    UpdateGUI();
}
