#include "lms7002_pnlRBB_view.h"
#include <map>
#include <wx/msgdlg.h>
#include "numericSlider.h"
#include "lms7002_gui_utilities.h"
#include "lms7suiteEvents.h"
#include "lms7suiteAppFrame.h"
using namespace lime;

lms7002_pnlRBB_view::lms7002_pnlRBB_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer52;
    fgSizer52 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer52->SetFlexibleDirection( wxBOTH );
    fgSizer52->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerPowerDowns;
    sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );

    chkPD_LPFH_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFH_RBB, wxT("LPFH block"));
    chkPD_LPFH_RBB->SetToolTip( wxT("Power down of the LPFH block") );
    
    sbSizerPowerDowns->Add( chkPD_LPFH_RBB, 0, flags, 0 );
    
    chkPD_LPFL_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_LPFL_RBB, wxT("LPFL block"));
    chkPD_LPFL_RBB->SetToolTip( wxT("Power down of the LPFL block") );
    
    sbSizerPowerDowns->Add( chkPD_LPFL_RBB, 0, flags, 0 );
    
    chkPD_PGA_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PGA_RBB, wxT("PGA block"));
    chkPD_PGA_RBB->SetToolTip( wxT("Power down of the PGA block") );
    
    sbSizerPowerDowns->Add( chkPD_PGA_RBB, 0, flags, 0 );
    
    chkEN_G_RBB = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_RBB, wxT("Enable RBB module"));
    chkEN_G_RBB->SetToolTip( wxT("Enable control for all the RBB_1 power downs") );
    
    sbSizerPowerDowns->Add( chkEN_G_RBB, 0, flags, 0 );
    
    wxStaticBoxSizer* sbSizer35;
    sbSizer35 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxVERTICAL );
    
    chkEN_DIR_RBB = new wxCheckBox( sbSizer35->GetStaticBox(), ID_EN_DIR_RBB, wxT("Direct control of PDs and ENs"));
    chkEN_DIR_RBB->SetToolTip( wxT("Enables direct control of PDs and ENs for RBB module") );
    
    sbSizer35->Add( chkEN_DIR_RBB, 0, wxALIGN_LEFT, 5 );
    
    
    sbSizerPowerDowns->Add( sbSizer35, 0, wxALL|wxALIGN_LEFT, 5 );
    
    
    fgSizer52->Add( sbSizerPowerDowns, 0, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer259;
    fgSizer259 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer259->AddGrowableCol( 0 );
    fgSizer259->SetFlexibleDirection( wxBOTH );
    fgSizer259->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer53;
    fgSizer53 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer53->AddGrowableCol( 1 );
    fgSizer53->SetFlexibleDirection( wxBOTH );
    fgSizer53->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT14 = new wxStaticText( this, wxID_ANY, wxT("BB loopback to RXLPF"));
    ID_STATICTEXT14->Wrap( -1 );
    fgSizer53->Add( ID_STATICTEXT14, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbBBLoopback = new wxComboBox( this, ID_BBLoopback, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbBBLoopback->Append( wxT("LPFH & RBB") );
    cmbBBLoopback->Append( wxT("LPFL & RBB") );
    cmbBBLoopback->Append( wxT("Disabled") );
    cmbBBLoopback->SetSelection( 0 );
    cmbBBLoopback->SetToolTip( wxT("This is the loopback enable signal that is enabled when high band LPFH_RBB or low band LPFL_RBB  is selected for the loopback path. \n0x0115 [15] EN_LB_LPFH_RBB\n0x0115 [14] EN_LB_LPFL_RBB") );
    
    fgSizer53->Add( cmbBBLoopback, 1, wxEXPAND, 5 );
    
    ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("PGA input connected to"));
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer53->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbINPUT_CTL_PGA_RBB = new wxComboBox( this, ID_INPUT_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
    cmbINPUT_CTL_PGA_RBB->SetToolTip( wxT("There are a total of four different differential inputs to the PGA. Only one of them is active at a time") );
    
    fgSizer53->Add( cmbINPUT_CTL_PGA_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("PGA gain"));
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer53->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbG_PGA_RBB = new wxComboBox( this, ID_G_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbG_PGA_RBB->SetToolTip( wxT("This is the gain of the PGA") );
    
    fgSizer53->Add( cmbG_PGA_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("PGA Feedback capacitor"));
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer53->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbC_CTL_PGA_RBB = new NumericSlider( this, ID_C_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    fgSizer53->Add( cmbC_CTL_PGA_RBB, 0, wxEXPAND, 5 );
    
    
    fgSizer259->Add( fgSizer53, 0, wxEXPAND, 0 );
    
    wxString rgrOSW_PGA_RBBChoices[] = { wxT("ADC"), wxT("Output pads") };
    int rgrOSW_PGA_RBBNChoices = sizeof( rgrOSW_PGA_RBBChoices ) / sizeof( wxString );
    rgrOSW_PGA_RBB = new wxRadioBox( this, ID_OSW_PGA_RBB, wxT("PGA output connected to"), wxDefaultPosition, wxDefaultSize, rgrOSW_PGA_RBBNChoices, rgrOSW_PGA_RBBChoices, 2, wxRA_SPECIFY_COLS );
    rgrOSW_PGA_RBB->SetSelection( 1 );
    rgrOSW_PGA_RBB->SetToolTip( wxT("There are two instances of the PGA circuit in the design. The output of the RBB_LPF blocks are connected the input of these PGA blocks (common). The output of one of them is connected to two pads pgaoutn and pgaoutp and the output of the other PGA is connected directly to the ADC input") );
    
    fgSizer259->Add( rgrOSW_PGA_RBB, 0, wxALIGN_LEFT|wxALIGN_TOP, 0 );
    
    
    fgSizer52->Add( fgSizer259, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerRCtimeConstants;
    sbSizerRCtimeConstants = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("RXLPF RC time constant") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer54;
    fgSizer54 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer54->SetFlexibleDirection( wxBOTH );
    fgSizer54->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT4 = new wxStaticText( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("Resistance"));
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer54->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbR_CTL_LPF_RBB = new wxComboBox( sbSizerRCtimeConstants->GetStaticBox(), ID_R_CTL_LPF_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbR_CTL_LPF_RBB->SetToolTip( wxT("Controls the absolute value of the resistance of the RC time constant of the RBB_LPF blocks (both Low and High)") );
    
    fgSizer54->Add( cmbR_CTL_LPF_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT5 = new wxStaticText( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("LPFH capacitance value"));
    ID_STATICTEXT5->Wrap( -1 );
    fgSizer54->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbC_CTL_LPFH_RBB = new NumericSlider( sbSizerRCtimeConstants->GetStaticBox(), ID_C_CTL_LPFH_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbC_CTL_LPFH_RBB->SetMinSize( wxSize( 100,-1 ) );
    
    fgSizer54->Add( cmbC_CTL_LPFH_RBB, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT6 = new wxStaticText( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("LPFL capacitance value"));
    ID_STATICTEXT6->Wrap( -1 );
    fgSizer54->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbC_CTL_LPFL_RBB = new NumericSlider( sbSizerRCtimeConstants->GetStaticBox(), ID_C_CTL_LPFL_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 2047, 0 );
    fgSizer54->Add( cmbC_CTL_LPFL_RBB, 0, wxEXPAND, 5 );
    
    
    sbSizerRCtimeConstants->Add( fgSizer54, 0, flags, 0 );
    
    wxStaticBoxSizer* sbSizerRxFilters;
    sbSizerRxFilters = new wxStaticBoxSizer( new wxStaticBox( sbSizerRCtimeConstants->GetStaticBox(), wxID_ANY, wxT("Rx Filters") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer199;
    fgSizer199 = new wxFlexGridSizer( 0, 3, 5, 5 );
    fgSizer199->SetFlexibleDirection( wxBOTH );
    fgSizer199->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText309 = new wxStaticText( sbSizerRxFilters->GetStaticBox(), wxID_ANY, wxT("RF bandwidth (MHz)"));
    m_staticText309->Wrap( -1 );
    fgSizer199->Add( m_staticText309, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    txtLowBW_MHz = new wxTextCtrl( sbSizerRxFilters->GetStaticBox(), ID_TXT_LOWBW, wxT("10"));
    #ifdef __WXGTK__
    if ( !txtLowBW_MHz->HasFlag( wxTE_MULTILINE ) )
    {
    txtLowBW_MHz->SetMaxLength( 10 );
    }
    #else
    txtLowBW_MHz->SetMaxLength( 10 );
    #endif
    fgSizer199->Add( txtLowBW_MHz, 0, 0, 5 );
    
    btnTuneFilter = new wxButton( sbSizerRxFilters->GetStaticBox(), ID_BTN_TUNE_FILTER, wxT("TUNE"));
    fgSizer199->Add( btnTuneFilter, 0, wxEXPAND, 5 );
    
    
    sbSizerRxFilters->Add( fgSizer199, 1, wxEXPAND, 5 );
    
    
    sbSizerRCtimeConstants->Add( sbSizerRxFilters, 1, wxEXPAND, 5 );
    
    
    fgSizer52->Add( sbSizerRCtimeConstants, 0, wxALIGN_LEFT|wxALIGN_TOP, 0 );
    
    wxStaticBoxSizer* sbSizerOpAmp;
    sbSizerOpAmp = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Operational amplifier") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer56;
    fgSizer56 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer56->AddGrowableCol( 1 );
    fgSizer56->SetFlexibleDirection( wxBOTH );
    fgSizer56->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT7 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("LPFH stability passive compensation"));
    ID_STATICTEXT7->Wrap( -1 );
    fgSizer56->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRCC_CTL_LPFH_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_RCC_CTL_LPFH_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRCC_CTL_LPFH_RBB->SetToolTip( wxT("Controls the stability passive compensation of the LPFH_RBB operational amplifier") );
    
    fgSizer56->Add( cmbRCC_CTL_LPFH_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT8 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("LPFL stability passive compensation"));
    ID_STATICTEXT8->Wrap( -1 );
    fgSizer56->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRCC_CTL_LPFL_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_RCC_CTL_LPFL_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRCC_CTL_LPFL_RBB->SetToolTip( wxT("Controls the stability passive compensation of the LPFL_RBB operational amplifier") );
    
    fgSizer56->Add( cmbRCC_CTL_LPFL_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT9 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Input stage reference bias current (RBB_LPF)"));
    ID_STATICTEXT9->Wrap( -1 );
    fgSizer56->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPF_IN_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_LPF_IN_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPF_IN_RBB->SetToolTip( wxT("Controls the reference bias current of the input stage of the operational amplifier used in RBB_LPF blocks (Low or High). ") );
    
    fgSizer56->Add( cmbICT_LPF_IN_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT10 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Output stage reference bias current (RBB_LPF)"));
    ID_STATICTEXT10->Wrap( -1 );
    fgSizer56->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_LPF_OUT_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_LPF_OUT_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_LPF_OUT_RBB->SetToolTip( wxT("The reference bias current of the output stage of the operational amplifier used in RBB_LPF blocks (low or High)") );
    
    fgSizer56->Add( cmbICT_LPF_OUT_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT11 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Output stage reference bias current (PGA)"));
    ID_STATICTEXT11->Wrap( -1 );
    fgSizer56->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_PGA_OUT_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_PGA_OUT_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_PGA_OUT_RBB->SetToolTip( wxT("Controls the output stage reference bias current of the operational amplifier used in the PGA circuit") );
    
    fgSizer56->Add( cmbICT_PGA_OUT_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT12 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("Input stage reference bias current (PGA)"));
    ID_STATICTEXT12->Wrap( -1 );
    fgSizer56->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbICT_PGA_IN_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_ICT_PGA_IN_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbICT_PGA_IN_RBB->SetToolTip( wxT("Controls the input stage reference bias current of the operational amplifier used in the PGA circuit") );
    
    fgSizer56->Add( cmbICT_PGA_IN_RBB, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT13 = new wxStaticText( sbSizerOpAmp->GetStaticBox(), wxID_ANY, wxT("PGA stability passive compensation"));
    ID_STATICTEXT13->Wrap( -1 );
    fgSizer56->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRCC_CTL_PGA_RBB = new wxComboBox( sbSizerOpAmp->GetStaticBox(), ID_RCC_CTL_PGA_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRCC_CTL_PGA_RBB->SetToolTip( wxT("Controls the stability passive compensation of the PGA_RBB operational amplifier") );
    
    fgSizer56->Add( cmbRCC_CTL_PGA_RBB, 0, wxEXPAND, 0 );
    
    
    sbSizerOpAmp->Add( fgSizer56, 0, wxEXPAND, 0 );
    
    
    fgSizer52->Add( sbSizerOpAmp, 0, wxEXPAND, 0 );
    
    
    this->SetSizer( fgSizer52 );
    this->Layout();
    fgSizer52->Fit( this );
    
    // Connect Events
    chkPD_LPFH_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    chkPD_LPFL_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    chkPD_PGA_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    chkEN_G_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    chkEN_DIR_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbBBLoopback->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::OncmbBBLoopbackSelected ), NULL, this );
    cmbINPUT_CTL_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbG_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    rgrOSW_PGA_RBB->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbR_CTL_LPF_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbC_CTL_LPFH_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbC_CTL_LPFL_RBB->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    btnTuneFilter->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlRBB_view::OnbtnTuneFilter ), NULL, this );
    cmbRCC_CTL_LPFH_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbRCC_CTL_LPFL_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPF_IN_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_LPF_OUT_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_PGA_OUT_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbICT_PGA_IN_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );
    cmbRCC_CTL_PGA_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlRBB_view::ParameterChangeHandler ), NULL, this );

    wndId2Enum[cmbC_CTL_LPFH_RBB] = LMS7param(C_CTL_LPFH_RBB);
    wndId2Enum[cmbC_CTL_LPFL_RBB] = LMS7param(C_CTL_LPFL_RBB);
    wndId2Enum[cmbC_CTL_PGA_RBB] = LMS7param(C_CTL_PGA_RBB);
    wndId2Enum[chkEN_G_RBB] = LMS7param(EN_G_RBB);
    wndId2Enum[cmbG_PGA_RBB] = LMS7param(G_PGA_RBB);
    wndId2Enum[cmbICT_LPF_IN_RBB] = LMS7param(ICT_LPF_IN_RBB);
    wndId2Enum[cmbICT_LPF_OUT_RBB] = LMS7param(ICT_LPF_OUT_RBB);
    wndId2Enum[cmbICT_PGA_IN_RBB] = LMS7param(ICT_PGA_IN_RBB);
    wndId2Enum[cmbICT_PGA_OUT_RBB] = LMS7param(ICT_PGA_OUT_RBB);
    wndId2Enum[cmbINPUT_CTL_PGA_RBB] = LMS7param(INPUT_CTL_PGA_RBB);
    wndId2Enum[rgrOSW_PGA_RBB] = LMS7param(OSW_PGA_RBB);
    wndId2Enum[chkPD_LPFH_RBB] = LMS7param(PD_LPFH_RBB);
    wndId2Enum[chkPD_LPFL_RBB] = LMS7param(PD_LPFL_RBB);
    wndId2Enum[chkPD_PGA_RBB] = LMS7param(PD_PGA_RBB);
    wndId2Enum[cmbRCC_CTL_LPFH_RBB] = LMS7param(RCC_CTL_LPFH_RBB);
    wndId2Enum[cmbRCC_CTL_LPFL_RBB] = LMS7param(RCC_CTL_LPFL_RBB);
    wndId2Enum[cmbRCC_CTL_PGA_RBB] = LMS7param(RCC_CTL_PGA_RBB);
    wndId2Enum[cmbR_CTL_LPF_RBB] = LMS7param(R_CTL_LPF_RBB);
    wndId2Enum[chkEN_DIR_RBB] = LMS7param(EN_DIR_RBB);

    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<32; ++i)
    {
        temp.push_back(wxString::Format(_("%i dB"), -12 + i));
    }
    cmbG_PGA_RBB->Set(temp);

    temp.clear();
    for (int i = 0; i<32; ++i)
    {
        temp.push_back(wxString::Format(_("%i"), i));
    }
    cmbICT_LPF_IN_RBB->Set(temp);
    cmbICT_LPF_OUT_RBB->Set(temp);
    cmbICT_PGA_IN_RBB->Set(temp);
    cmbICT_PGA_OUT_RBB->Set(temp);
    cmbRCC_CTL_PGA_RBB->Append(temp);
    cmbR_CTL_LPF_RBB->Append(temp);

    temp.clear();
    temp.push_back("LPFL_RBB");
    temp.push_back("LPFH_RBB");
    temp.push_back("LPF bypass");
    temp.push_back("Loopback from TX");
    temp.push_back("Loopback path from peak detector");
    cmbINPUT_CTL_PGA_RBB->Set(temp);

    temp.clear();
    for (int i = 0; i<8; ++i)
    {
        temp.push_back(wxString::Format(_("%i"), i));
    }
    cmbRCC_CTL_LPFH_RBB->Set(temp);

    temp.clear();
    temp.push_back("when rxMode is 1.4 MHz");
    temp.push_back("when 3 MHz");
    temp.push_back("when 5 MHz");
    temp.push_back("when 10 MHz");
    temp.push_back("when 15 MHz");
    temp.push_back("when 20 MHz");
    cmbRCC_CTL_LPFL_RBB->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlRBB_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlRBB_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlRBB_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlRBB_view::OncmbBBLoopbackSelected( wxCommandEvent& event )
{
    switch (cmbBBLoopback->GetSelection())
    {
    case 0:
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFH_RBB),true);
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFL_RBB),false);
        break;
    case 1:
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFH_RBB),false);
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFL_RBB),true);
        break;
    case 2:
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFH_RBB),false);
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFL_RBB),false);
        break;
    default:
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFH_RBB),false);
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFL_RBB),false);
    }
}

void lms7002_pnlRBB_view::UpdateGUI()
{
    assert(lmsControl != nullptr);

    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);

    long BBloopbackValue = 0;
    uint16_t EN_LB_LPFH_RBBvalue;
    LMS_ReadParam(lmsControl,LMS7param(EN_LB_LPFH_RBB),&EN_LB_LPFH_RBBvalue);

    uint16_t EN_LB_LPFL_RBBvalue;
    LMS_ReadParam(lmsControl,LMS7param(EN_LB_LPFL_RBB),&EN_LB_LPFL_RBBvalue);
    if (!EN_LB_LPFH_RBBvalue && !EN_LB_LPFL_RBBvalue)
        BBloopbackValue = 2;
    else if (EN_LB_LPFH_RBBvalue && !EN_LB_LPFL_RBBvalue)
        BBloopbackValue = 0;
    else if (!EN_LB_LPFH_RBBvalue && EN_LB_LPFL_RBBvalue)
        BBloopbackValue = 1;
    else //invalid combination
    {
        BBloopbackValue = 2;
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFH_RBB),false);
        LMS_WriteParam(lmsControl,LMS7param(EN_LB_LPFL_RBB),false);
    }
    cmbBBLoopback->SetSelection(BBloopbackValue);

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
    cmbG_PGA_RBB->Enable(!value);
    cmbC_CTL_PGA_RBB->Enable(!value);
}

void lms7002_pnlRBB_view::OnbtnTuneFilter(wxCommandEvent& event)
{
    double input1;
    txtLowBW_MHz->GetValue().ToDouble(&input1);

    int status;
    uint16_t ch;

    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    status = LMS_SetLPFBW(lmsControl,LMS_CH_RX,ch,input1*1e6);
    if (status != 0){
        wxMessageBox(wxString(_("Rx Filter tune failed")), _("Error"));
        return;
    }
    LMS_Synchronize(lmsControl,false);
    UpdateGUI();
}
