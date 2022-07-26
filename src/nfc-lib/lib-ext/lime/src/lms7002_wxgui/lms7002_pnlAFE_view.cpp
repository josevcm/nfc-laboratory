#include "lms7002_pnlAFE_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlAFE_view::lms7002_pnlAFE_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    wxFlexGridSizer* fgSizer62;
    fgSizer62 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer62->SetFlexibleDirection( wxBOTH );
    fgSizer62->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerPowerDowns;
    sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer64;
    fgSizer64 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer64->SetFlexibleDirection( wxBOTH );
    fgSizer64->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkPD_AFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_AFE, wxT("AFE current mirror in BIAS_TOP"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_AFE->SetValue(true); 
    chkPD_AFE->SetToolTip( wxT("Power down control for the AFE current mirror in BIAS_TOP") );
    
    fgSizer64->Add( chkPD_AFE, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    chkPD_RX_AFE1 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_RX_AFE1, wxT("ADC ch. 1"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_RX_AFE1->SetToolTip( wxT("Power down control for the ADC of  channel 1") );
    
    fgSizer64->Add( chkPD_RX_AFE1, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    chkPD_RX_AFE2 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_RX_AFE2, wxT("ADC ch. 2"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_RX_AFE2->SetValue(true); 
    chkPD_RX_AFE2->SetToolTip( wxT("Power down control for the ADC of channel 2") );
    
    fgSizer64->Add( chkPD_RX_AFE2, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    chkPD_TX_AFE1 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TX_AFE1, wxT("DAC ch. 1"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_TX_AFE1->SetValue(true); 
    chkPD_TX_AFE1->SetToolTip( wxT("Power down control for the DAC of channel 1") );
    
    fgSizer64->Add( chkPD_TX_AFE1, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    chkPD_TX_AFE2 = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_TX_AFE2, wxT("DAC ch. 2"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_TX_AFE2->SetValue(true); 
    chkPD_TX_AFE2->SetToolTip( wxT("Power down control for the DAC of channel 2") );
    
    fgSizer64->Add( chkPD_TX_AFE2, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    chkEN_G_AFE = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_EN_G_AFE, wxT("Enable AFE module"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_G_AFE->SetToolTip( wxT("Enable control for all the AFE power downs") );
    
    fgSizer64->Add( chkEN_G_AFE, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    
    sbSizerPowerDowns->Add( fgSizer64, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    
    fgSizer62->Add( sbSizerPowerDowns, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    wxFlexGridSizer* fgSizer63;
    fgSizer63 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer63->SetFlexibleDirection( wxBOTH );
    fgSizer63->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("Peak current of DAC"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer63->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbISEL_DAC_AFE = new wxComboBox( this, ID_ISEL_DAC_AFE, wxEmptyString, wxDefaultPosition, wxSize( 162,-1 ), 0, NULL, 0 ); 
    cmbISEL_DAC_AFE->SetToolTip( wxT("Controls the peak current of the DAC output current") );
    
    fgSizer63->Add( cmbISEL_DAC_AFE, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("MUX input of ADC ch. 1"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer63->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbMUX_AFE_1 = new wxComboBox( this, ID_MUX_AFE_1, wxEmptyString, wxDefaultPosition, wxSize( 315,-1 ), 0, NULL, 0 ); 
    cmbMUX_AFE_1->SetToolTip( wxT("Controls the MUX at the input of the ADC channel 1") );
    
    fgSizer63->Add( cmbMUX_AFE_1, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT3 = new wxStaticText( this, wxID_ANY, wxT("MUX input of ADC ch. 2"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer63->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbMUX_AFE_2 = new wxComboBox( this, ID_MUX_AFE_2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbMUX_AFE_2->SetToolTip( wxT("Controls the MUX at the input of the ADC channel 2") );
    
    fgSizer63->Add( cmbMUX_AFE_2, 0, wxEXPAND, 0 );
    
    
    fgSizer63->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxString rgrMODE_INTERLEAVE_AFEChoices[] = { wxT("Two ADCs"), wxT("Interleaved") };
    int rgrMODE_INTERLEAVE_AFENChoices = sizeof( rgrMODE_INTERLEAVE_AFEChoices ) / sizeof( wxString );
    rgrMODE_INTERLEAVE_AFE = new wxRadioBox( this, ID_MODE_INTERLEAVE_AFE, wxT("Time interleave two ADCs into one ADC"), wxDefaultPosition, wxDefaultSize, rgrMODE_INTERLEAVE_AFENChoices, rgrMODE_INTERLEAVE_AFEChoices, 2, wxRA_SPECIFY_COLS );
    rgrMODE_INTERLEAVE_AFE->SetSelection( 1 );
    rgrMODE_INTERLEAVE_AFE->SetToolTip( wxT("time interleaves the two ADCs into one ADC") );
    
    fgSizer63->Add( rgrMODE_INTERLEAVE_AFE, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0 );
    
    
    fgSizer62->Add( fgSizer63, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0 );
    
    
    this->SetSizer( fgSizer62 );
    this->Layout();
    fgSizer62->Fit( this );
    
    // Connect Events
    chkPD_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    chkPD_RX_AFE1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    chkPD_RX_AFE2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    chkPD_TX_AFE1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    chkPD_TX_AFE2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    chkEN_G_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    cmbISEL_DAC_AFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    cmbMUX_AFE_1->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    cmbMUX_AFE_2->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );
    rgrMODE_INTERLEAVE_AFE->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlAFE_view::ParameterChangeHandler ), NULL, this );

    wndId2Enum[chkEN_G_AFE] = LMS7param(EN_G_AFE);
    wndId2Enum[cmbISEL_DAC_AFE] = LMS7param(ISEL_DAC_AFE);
    wndId2Enum[rgrMODE_INTERLEAVE_AFE] = LMS7param(MODE_INTERLEAVE_AFE);
    wndId2Enum[cmbMUX_AFE_1] = LMS7param(MUX_AFE_1);
    wndId2Enum[cmbMUX_AFE_2] = LMS7param(MUX_AFE_2);
    wndId2Enum[chkPD_AFE] = LMS7param(PD_AFE);
    wndId2Enum[chkPD_RX_AFE1] = LMS7param(PD_RX_AFE1);
    wndId2Enum[chkPD_RX_AFE2] = LMS7param(PD_RX_AFE2);
    wndId2Enum[chkPD_TX_AFE1] = LMS7param(PD_TX_AFE1);
    wndId2Enum[chkPD_TX_AFE2] = LMS7param(PD_TX_AFE2);

    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<8; ++i)
    {
        temp.push_back(wxString::Format(_("%i uA"), 325 + i * 75)); //nominal 625uA
    }
    cmbISEL_DAC_AFE->Set(temp);

    temp.clear();
    temp.push_back(_("PGA output is connected to ADC input"));
    temp.push_back(_("pdet_1 is connected to ADC ch. 1"));
    temp.push_back(_("BIAS_TOP test outputs will be connected to ADC ch.1 input"));
    temp.push_back(_("RSSI 1 output will be connected to ADC 1 input"));
    cmbMUX_AFE_1->Set(temp);

    temp.clear();
    temp.push_back(_("PGA output is connected to ADC input"));
    temp.push_back(_("pdet_2 is connected to ADC channel 2"));
    temp.push_back(_("RSSI 1 output will be connected to ADC 2 input"));
    temp.push_back(_("RSSI 2 output will be connected to ADC 2 input"));
    cmbMUX_AFE_2->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlAFE_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlAFE_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlAFE_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
