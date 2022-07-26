#include "lms7002_pnlBIAS_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
#include "lms7_device.h"
using namespace lime;

lms7002_pnlBIAS_view::lms7002_pnlBIAS_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer65;
    fgSizer65 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer65->SetFlexibleDirection( wxBOTH );
    fgSizer65->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerPowerDowns;
    sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer66;
    fgSizer66 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer66->SetFlexibleDirection( wxBOTH );
    fgSizer66->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkPD_FRP_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_FRP_BIAS, wxT("Fix/RP block"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_FRP_BIAS->SetToolTip( wxT("Power down signal for Fix/RP block") );
    
    fgSizer66->Add( chkPD_FRP_BIAS, 0, flags, 0 );
    
    chkPD_F_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_F_BIAS, wxT("Fix"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_F_BIAS->SetToolTip( wxT("Power down signal for Fix") );
    
    fgSizer66->Add( chkPD_F_BIAS, 0, flags, 0 );
    
    chkPD_PTRP_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PTRP_BIAS, wxT("PTAT/RP block"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_PTRP_BIAS->SetToolTip( wxT("Power down signal for PTAT/RP block") );
    
    fgSizer66->Add( chkPD_PTRP_BIAS, 0, flags, 0 );
    
    chkPD_PT_BIAS = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_PT_BIAS, wxT("PTAT block"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_PT_BIAS->SetToolTip( wxT("Power down signal for PTAT block") );
    
    fgSizer66->Add( chkPD_PT_BIAS, 0, flags, 0 );
    
    chkPD_BIAS_MASTER = new wxCheckBox( sbSizerPowerDowns->GetStaticBox(), ID_PD_BIAS_MASTER, wxT("Power down all block"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_BIAS_MASTER->SetToolTip( wxT("Enable signal for central bias block") );
    
    fgSizer66->Add( chkPD_BIAS_MASTER, 0, flags, 0 );
    
    
    sbSizerPowerDowns->Add( fgSizer66, 0, flags, 0 );
    
    
    fgSizer65->Add( sbSizerPowerDowns, 0, flags, 0 );
    
    wxFlexGridSizer* fgSizer67;
    fgSizer67 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer67->SetFlexibleDirection( wxBOTH );
    fgSizer67->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT1 = new wxStaticText( this, wxID_ANY, wxT("BIAS_TOP test mode"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer67->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbMUX_BIAS_OUT = new wxComboBox( this, ID_MUX_BIAS_OUT, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
    cmbMUX_BIAS_OUT->SetToolTip( wxT("Test mode of the BIAS_TOP") );
    
    fgSizer67->Add( cmbMUX_BIAS_OUT, 0, flags, 0 );
    
    ID_STATICTEXT2 = new wxStaticText( this, wxID_ANY, wxT("RP_CALIB_BIAS"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer67->Add( ID_STATICTEXT2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbRP_CALIB_BIAS = new wxComboBox( this, ID_RP_CALIB_BIAS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRP_CALIB_BIAS->SetToolTip( wxT("Calibration code for rppolywo. This code is set by the calibration algorithm: BIAS_RPPOLY_calibration") );
    
    fgSizer67->Add( cmbRP_CALIB_BIAS, 0, flags, 5 );
    
    btnCalibrateRP_BIAS = new wxButton( this, wxID_ANY, wxT("Calibrate RP_BIAS"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer67->Add( btnCalibrateRP_BIAS, 0, 0, 5 );
    
    
    fgSizer65->Add( fgSizer67, 0, flags, 0 );
    
    
    this->SetSizer( fgSizer65 );
    this->Layout();
    fgSizer65->Fit( this );
    
    // Connect Events
    chkPD_FRP_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIAS_view::ParameterChangeHandler ), NULL, this );
    chkPD_F_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIAS_view::ParameterChangeHandler ), NULL, this );
    chkPD_PTRP_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIAS_view::ParameterChangeHandler ), NULL, this );
    chkPD_PT_BIAS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIAS_view::ParameterChangeHandler ), NULL, this );
    chkPD_BIAS_MASTER->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIAS_view::ParameterChangeHandler ), NULL, this );
    cmbMUX_BIAS_OUT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlBIAS_view::ParameterChangeHandler ), NULL, this );
    cmbRP_CALIB_BIAS->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlBIAS_view::ParameterChangeHandler ), NULL, this );
    btnCalibrateRP_BIAS->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlBIAS_view::OnCalibrateRP_BIAS ), NULL, this );

    wndId2Enum[chkPD_BIAS_MASTER] = LMS7param(PD_BIAS_MASTER);
    wndId2Enum[cmbMUX_BIAS_OUT] = LMS7param(MUX_BIAS_OUT);
    wndId2Enum[chkPD_FRP_BIAS] = LMS7param(PD_FRP_BIAS);
    wndId2Enum[chkPD_F_BIAS] = LMS7param(PD_F_BIAS);
    wndId2Enum[chkPD_PTRP_BIAS] = LMS7param(PD_PTRP_BIAS);
    wndId2Enum[chkPD_PT_BIAS] = LMS7param(PD_PT_BIAS);
    wndId2Enum[cmbRP_CALIB_BIAS] = LMS7param(RP_CALIB_BIAS);

    wxArrayString temp;
    temp.clear();
    temp.push_back(_("NO test mode"));
    temp.push_back(_("vr_ext_bak and vr_cal_ref=600mV passed to ADC input MUX"));
    temp.push_back(_("BIAS_TOP test outputs will be connected to ADC channel 1 input"));
    temp.push_back(_("RSSI 1 output will be connected to ADC 1 input"));
    cmbMUX_BIAS_OUT->Set(temp);

    temp.clear();
    for (int i = 0; i<32; ++i)
    {
        temp.push_back(wxString::Format(_("%i"), i));
    }
    cmbRP_CALIB_BIAS->Set(temp);
    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlBIAS_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlBIAS_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlBIAS_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}

void lms7002_pnlBIAS_view::OnCalibrateRP_BIAS( wxCommandEvent& event )
{
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
    lms->CalibrateRP_BIAS();
    UpdateGUI();
}
