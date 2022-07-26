#include "lms7002_pnlCalibrations_view.h"
#include "lms7002_gui_utilities.h"
#include <wx/msgdlg.h>
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include <wx/busyinfo.h>
#include "lms7suiteAppFrame.h"
using namespace lime;

lms7002_pnlCalibrations_view::lms7002_pnlCalibrations_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int textFlags = wxALIGN_LEFT|wxLEFT|wxALIGN_CENTER_VERTICAL;
    const int margins = 5;
    wxFlexGridSizer* mainSizer;
    mainSizer = new wxFlexGridSizer( 0, 3, margins, margins);
    mainSizer->AddGrowableCol( 0 );
    mainSizer->AddGrowableCol( 1 );
    mainSizer->SetFlexibleDirection( wxHORIZONTAL );
    mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbReceiverSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Receiver") ), wxVERTICAL );
    {
        wxStaticBoxSizer* sbSizer971 = new wxStaticBoxSizer( new wxStaticBox( sbReceiverSizer->GetStaticBox(), wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
        
        wxFlexGridSizer* fgSizer1311 = new wxFlexGridSizer( 0, 2, 0, margins);
        fgSizer1311->AddGrowableCol( 1 );
        fgSizer1311->SetFlexibleDirection( wxHORIZONTAL );
        fgSizer1311->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        
        fgSizer1311->Add( new wxStaticText( sbSizer971->GetStaticBox(), wxID_ANY, wxT("I:")), 0, textFlags, margins);
        cmbGCORRI_RXTSP = new NumericSlider( sbSizer971->GetStaticBox(), ID_GCORRI_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
        //cmbGCORRI_RXTSP->SetMinSize( wxSize( 48,-1 ) );
        fgSizer1311->Add( cmbGCORRI_RXTSP, 0, wxEXPAND|wxRIGHT, margins);
        
        fgSizer1311->Add( new wxStaticText( sbSizer971->GetStaticBox(), wxID_ANY, wxT("Q:")), 0, textFlags, margins);
        cmbGCORRQ_RXTSP = new NumericSlider( sbSizer971->GetStaticBox(), ID_GCORRQ_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
        //cmbGCORRQ_RXTSP->SetMinSize( wxSize( 48,-1 ) );
        
        fgSizer1311->Add( cmbGCORRQ_RXTSP, 0, wxEXPAND|wxRIGHT, margins);
        sbSizer971->Add( fgSizer1311, 0, wxEXPAND, 0 );
        sbReceiverSizer->Add( sbSizer971, 0, wxEXPAND|wxALL, margins);
    }
    {
        wxStaticBoxSizer* sbSizer961 = new wxStaticBoxSizer( new wxStaticBox( sbReceiverSizer->GetStaticBox(), wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
        cmbIQCORR_RXTSP = new NumericSlider( sbSizer961->GetStaticBox(), ID_IQCORR_RXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
        //cmbIQCORR_RXTSP->SetMinSize( wxSize( 200,-1 ) );    
        sbSizer961->Add( cmbIQCORR_RXTSP, 0, wxEXPAND|wxALL, margins);
        wxBoxSizer* fgSizer1301 = new wxBoxSizer(wxHORIZONTAL);
        fgSizer1301->Add( new wxStaticText( sbSizer961->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):")), 0, textFlags, margins );
        txtRxPhaseAlpha = new wxStaticText( sbSizer961->GetStaticBox(), wxID_ANY, wxT("0"));
        fgSizer1301->Add( txtRxPhaseAlpha, 0, textFlags, margins);
        sbSizer961->Add( fgSizer1301, 0, wxALL, margins);
        sbReceiverSizer->Add( sbSizer961, 0, wxEXPAND|wxALL, margins);
    }
     
    {
        wxStaticBoxSizer* sbSizerDC = new wxStaticBoxSizer( new wxStaticBox( sbReceiverSizer->GetStaticBox(), wxID_ANY, wxT("DC") ), wxVERTICAL );
        
        wxFlexGridSizer* fgSizer45 = new wxFlexGridSizer( 0, 2, 0, margins);
        fgSizer45->AddGrowableCol( 1 );
        fgSizer45->SetFlexibleDirection( wxBOTH );
        fgSizer45->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        
        fgSizer45->Add( new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset I:")), 0, textFlags, margins);
        cmbDCOFFI_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFI_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
        fgSizer45->Add( cmbDCOFFI_RFE, 0, wxEXPAND|wxRIGHT, margins);
        
        fgSizer45->Add( new wxStaticText( sbSizerDC->GetStaticBox(), wxID_ANY, wxT("Offset Q:")), 0, textFlags, margins);
        cmbDCOFFQ_RFE = new NumericSlider( sbSizerDC->GetStaticBox(), ID_DCOFFQ_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
        fgSizer45->Add( cmbDCOFFQ_RFE, 0, wxEXPAND|wxRIGHT, margins);
        
        sbSizerDC->Add( fgSizer45, 0, wxEXPAND, 0);
        
        chkEN_DCOFF_RXFE_RFE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_EN_DCOFF_RXFE_RFE, wxT("Enable DC offset"));
        chkEN_DCOFF_RXFE_RFE->SetToolTip( wxT("Enables the DCOFFSET block for the RXFE") );
        
        sbSizerDC->Add(chkEN_DCOFF_RXFE_RFE);
        chkDCMODE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_DCMODE, wxT("Automatic DC calibration mode"));
        sbSizerDC->Add(chkDCMODE);
        sbReceiverSizer->Add( sbSizerDC, 0, wxEXPAND|wxALL, margins);
    }
    
    btnCalibrateRx = new wxButton( sbReceiverSizer->GetStaticBox(), wxID_ANY, wxT("Calibrate RX"));
    sbReceiverSizer->Add( btnCalibrateRx, 0, wxALL|wxALIGN_LEFT, margins);
    
    
    mainSizer->Add( sbReceiverSizer, 1, wxLEFT|wxEXPAND, margins);
    
    wxStaticBoxSizer* sbTransmitterSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Transmitter") ), wxVERTICAL );
    {
        wxStaticBoxSizer* sbSizer97 = new wxStaticBoxSizer( new wxStaticBox( sbTransmitterSizer->GetStaticBox(), wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
        
        wxFlexGridSizer* fgSizer131;
        fgSizer131 = new wxFlexGridSizer( 0, 2, 0, 0);
        fgSizer131->AddGrowableCol( 1 );
        fgSizer131->SetFlexibleDirection( wxBOTH );
        fgSizer131->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        
        fgSizer131->Add( new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("I:")), 0, textFlags, margins);
        
        cmbGCORRI_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
        //cmbGCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );        
        fgSizer131->Add( cmbGCORRI_TXTSP, 0, wxEXPAND|wxRIGHT, margins);

        fgSizer131->Add( new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Q:")), 0, textFlags, margins);
        cmbGCORRQ_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
        //cmbGCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
        fgSizer131->Add( cmbGCORRQ_TXTSP, 0, wxEXPAND|wxRIGHT, margins);
        
        sbSizer97->Add( fgSizer131, 0, wxEXPAND, 0);
        sbTransmitterSizer->Add( sbSizer97, 0, wxEXPAND|wxALL, margins);
    }
    
    {
        wxStaticBoxSizer* sbSizer96 = new wxStaticBoxSizer( new wxStaticBox( sbTransmitterSizer->GetStaticBox(), wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
        
        cmbIQCORR_TXTSP = new NumericSlider( sbSizer96->GetStaticBox(), ID_IQCORR_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
        //cmbIQCORR_TXTSP->SetMinSize( wxSize( 200,-1 ) );
        
        sbSizer96->Add( cmbIQCORR_TXTSP, 0, wxEXPAND|wxALL, margins);
        wxFlexGridSizer* fgSizer130 = new wxFlexGridSizer( 0, 3, 0, 0 );
        fgSizer130->SetFlexibleDirection( wxBOTH );
        fgSizer130->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        fgSizer130->Add( new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):")), 0, textFlags, margins );
        txtPhaseAlpha = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("0"));
        fgSizer130->Add( txtPhaseAlpha, 0, textFlags, margins );
        sbSizer96->Add( fgSizer130, 0, wxALL, margins );
        sbTransmitterSizer->Add( sbSizer96, 0, wxEXPAND|wxALL, margins);
    }
    
    {
        wxStaticBoxSizer* sbSizer95 = new wxStaticBoxSizer( new wxStaticBox( sbTransmitterSizer->GetStaticBox(), wxID_ANY, wxT("DC Corrector") ), wxVERTICAL );
        
        wxFlexGridSizer* fgSizer129;
        fgSizer129 = new wxFlexGridSizer( 0, 2, 0, margins);
        fgSizer129->AddGrowableCol( 1 );
        fgSizer129->SetFlexibleDirection( wxHORIZONTAL );
        fgSizer129->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        
        fgSizer129->Add( new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("I:")), 0, textFlags, margins);
        
        cmbDCCORRI_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
        //cmbDCCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );        
        fgSizer129->Add( cmbDCCORRI_TXTSP, 0, wxEXPAND|wxRIGHT, margins);

        fgSizer129->Add( new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Q:")), 0, textFlags, margins);
        cmbDCCORRQ_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
        //cmbDCCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
        fgSizer129->Add( cmbDCCORRQ_TXTSP, 0, wxEXPAND|wxRIGHT, margins);
        
        sbSizer95->Add( fgSizer129, 0, wxEXPAND, margins);
        sbTransmitterSizer->Add( sbSizer95, 0, wxEXPAND|wxALL, margins);
    }
    

    btnCalibrateTx = new wxButton( sbTransmitterSizer->GetStaticBox(), wxID_ANY, wxT("Calibrate TX"));
    sbTransmitterSizer->Add( btnCalibrateTx, 0, wxALL|wxALIGN_LEFT, margins);
    
    
    mainSizer->Add( sbTransmitterSizer, 1, wxEXPAND, margins);
    
    wxStaticBoxSizer* fullCalibrationSizer;
    fullCalibrationSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Full calibration") ), wxVERTICAL );
    
    btnCalibrateAll = new wxButton( fullCalibrationSizer->GetStaticBox(), wxID_ANY, wxT("Calibrate All"));
    fullCalibrationSizer->Add( btnCalibrateAll, 0, wxALL, margins);
    
    wxFlexGridSizer* fgSizer328;
    fgSizer328 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer328->SetFlexibleDirection( wxBOTH );
    fgSizer328->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    fgSizer328->Add( new wxStaticText( fullCalibrationSizer->GetStaticBox(), wxID_ANY, wxT("CGEN Ref. Clk (MHz):")), 0, textFlags, margins);
    
    lblCGENrefClk = new wxStaticText( fullCalibrationSizer->GetStaticBox(), wxID_ANY, wxT("???"));
    fgSizer328->Add( lblCGENrefClk, 0, textFlags, margins);
    
    fgSizer328->Add( new wxStaticText( fullCalibrationSizer->GetStaticBox(), wxID_ANY, wxT("Calibration bandwidth (MHz):")), 0, textFlags, margins);
    txtCalibrationBW = new wxTextCtrl( fullCalibrationSizer->GetStaticBox(), wxID_ANY, wxT("5"));
    
    txtCalibrationBW->SetMinSize( wxSize( 50,-1 ) );
    
    fgSizer328->Add( txtCalibrationBW, 0, wxEXPAND, margins);
    
    fullCalibrationSizer->Add( fgSizer328, 0, wxRIGHT, margins);
    
    wxFlexGridSizer* fgSizer246;
    fgSizer246 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer246->SetFlexibleDirection( wxVERTICAL );
    fgSizer246->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString rgrCalibrationMethodChoices[] = { wxT("Chip internal"), wxT("On board external") };
    int rgrCalibrationMethodNChoices = sizeof( rgrCalibrationMethodChoices ) / sizeof( wxString );
    rgrCalibrationMethod = new wxRadioBox( fullCalibrationSizer->GetStaticBox(), wxID_ANY, wxT("Calibration loopback:"), wxDefaultPosition, wxDefaultSize, rgrCalibrationMethodNChoices, rgrCalibrationMethodChoices, 1, wxRA_SPECIFY_COLS );
    rgrCalibrationMethod->SetSelection( 0 );
    fgSizer246->Add( rgrCalibrationMethod, 0, wxALL, margins);
    
    lblCalibrationNote = new wxStaticText( fullCalibrationSizer->GetStaticBox(), wxID_ANY, wxEmptyString);
    lblCalibrationNote->Wrap( -1 );
    fgSizer246->Add( lblCalibrationNote, 0, wxALL, margins);
    
    
    fullCalibrationSizer->Add( fgSizer246, 0, 0, margins);
    
    
    mainSizer->Add( fullCalibrationSizer, 1, wxRIGHT, margins);
    
    
    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );
    
    // Connect Events
    cmbGCORRI_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbGCORRQ_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbIQCORR_RXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCOFFI_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCOFFQ_RFE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    chkEN_DCOFF_RXFE_RFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    chkDCMODE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    btnCalibrateRx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnbtnCalibrateRx ), NULL, this );
    cmbGCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbGCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbIQCORR_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    cmbDCCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlCalibrations_view::ParameterChangeHandler ), NULL, this );
    btnCalibrateTx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnbtnCalibrateTx ), NULL, this );
    btnCalibrateAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnbtnCalibrateAll ), NULL, this );
    rgrCalibrationMethod->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCalibrations_view::OnCalibrationMethodChange ), NULL, this );

    wndId2Enum[cmbIQCORR_TXTSP] = LMS7param(IQCORR_TXTSP);
    wndId2Enum[cmbDCCORRI_TXTSP] = LMS7param(DCCORRI_TXTSP);
    wndId2Enum[cmbDCCORRQ_TXTSP] = LMS7param(DCCORRQ_TXTSP);
    wndId2Enum[cmbGCORRI_TXTSP] = LMS7param(GCORRI_TXTSP);
    wndId2Enum[cmbGCORRQ_TXTSP] = LMS7param(GCORRQ_TXTSP);
    wndId2Enum[cmbGCORRI_RXTSP] = LMS7param(GCORRI_RXTSP);
    wndId2Enum[cmbGCORRQ_RXTSP] = LMS7param(GCORRQ_RXTSP);
    wndId2Enum[cmbIQCORR_RXTSP] = LMS7param(IQCORR_RXTSP);
    wndId2Enum[chkEN_DCOFF_RXFE_RFE] = LMS7param(EN_DCOFF_RXFE_RFE);
    wndId2Enum[cmbDCOFFI_RFE] = LMS7param(DCOFFI_RFE);
    wndId2Enum[cmbDCOFFQ_RFE] = LMS7param(DCOFFQ_RFE);
    wndId2Enum[chkDCMODE] = LMS7param(DCMODE);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
    rgrCalibrationMethod->Hide();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateRx(wxCommandEvent& event)
{
    int flags = 0;
    if(rgrCalibrationMethod->GetSelection() == 0)
        flags = 0;
    else
    {
        flags = 1;
    }
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    int status;
    {
#ifdef NDEBUG
        wxBusyInfo wait("Please wait, calibrating receiver...");
#endif
        uint16_t ch;
        LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
        ch = (ch == 2) ? 1 : 0;
        ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
        status = LMS_Calibrate(lmsControl, LMS_CH_RX, ch, bandwidth_MHz * 1e6, flags);
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Rx calibration failed: %s"), LMS_GetLastErrorMessage()));
    else
    {
        wxMessageBox(_("Rx Calibration Finished"), _("Info"), wxOK, this);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetInt(lime::LOG_LEVEL_INFO);
        evt.SetString(_("Rx Calibrated"));
        wxPostEvent(this, evt);
    }
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateTx( wxCommandEvent& event )
{
    bool useExtLoopback = false;
    if(rgrCalibrationMethod->GetSelection() == 0)
        useExtLoopback = false;
    else
    {
        useExtLoopback = true;
    }
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    int status = 0;
    {
#ifdef NDEBUG
        wxBusyInfo wait("Please wait, calibrating transmitter...");
#endif
        uint16_t ch;
        LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
        status = LMS_Calibrate(lmsControl,LMS_CH_TX,ch-1,bandwidth_MHz * 1e6,useExtLoopback);
    }
    if (status != 0)
        wxMessageBox(wxString::Format(_("Tx calibration failed: %s"), LMS_GetLastErrorMessage()));
    else
    {
        wxMessageBox(_("Tx Calibration Finished"), _("Info"), wxOK, this);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetInt(lime::LOG_LEVEL_INFO);
        evt.SetString(_("Tx Calibrated"));
        wxPostEvent(this, evt);
    }
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::OnbtnCalibrateAll( wxCommandEvent& event )
{
    bool useExtLoopback = false;
    if(rgrCalibrationMethod->GetSelection() == 0)
        useExtLoopback = false;
    else
        useExtLoopback = true;
    double bandwidth_MHz = 0;
    txtCalibrationBW->GetValue().ToDouble(&bandwidth_MHz);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    int status = LMS_Calibrate(lmsControl,LMS_CH_TX,ch-1,bandwidth_MHz * 1e6,useExtLoopback);

    if (status != 0)
    {
        wxMessageBox(wxString::Format(_("Tx Calibration Failed: %s"), LMS_GetLastErrorMessage()), _("Info"), wxOK, this);
        UpdateGUI();
        return;
    }

    status |= LMS_Calibrate(lmsControl,LMS_CH_RX,ch-1,bandwidth_MHz * 1e6,useExtLoopback);
    if (status != 0)
        wxMessageBox(wxString::Format(_("Rx Calibration Failed: %s"), LMS_GetLastErrorMessage()), _("Info"), wxOK, this);
    else
        wxMessageBox(_("Calibration Finished"), _("Info"), wxOK, this);
    UpdateGUI();
}

void lms7002_pnlCalibrations_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
        value = 1;
    chkDCMODE->Enable(value);
}

void lms7002_pnlCalibrations_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlCalibrations_view::ParameterChangeHandler(wxCommandEvent& event)
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
    if(event.GetEventObject() == cmbDCOFFI_RFE || event.GetEventObject() == cmbDCOFFQ_RFE)
    {
        int16_t value = (event.GetInt() < 0) << 6;
        value |= abs(event.GetInt()) & 0x3F;
        LMS_WriteParam(lmsControl,parameter,value);
    }
    else if(event.GetEventObject() == cmbIQCORR_TXTSP)
    {
        float angle = atan(event.GetInt() / 2048.0) * 180 / 3.141596;
        txtPhaseAlpha->SetLabel(wxString::Format("%.3f", angle));
    }
    else if(event.GetEventObject() == cmbIQCORR_RXTSP)
    {
        float angle = atan(event.GetInt() / 2048.0) * 180 / 3.141596;
        txtRxPhaseAlpha->SetLabel(wxString::Format("%.3f", angle));
    }
    else
        LMS_WriteParam(lmsControl,parameter,event.GetInt());
}

void lms7002_pnlCalibrations_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    int16_t value;
    LMS_ReadParam(lmsControl,LMS7param(IQCORR_RXTSP),(uint16_t*)&value);
    int bitsToShift = (15 - LMS7param(IQCORR_RXTSP).msb - LMS7param(IQCORR_RXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_RXTSP->SetValue(value);
    float angle = atan(value / 2048.0) * 180 / 3.141596;
    txtRxPhaseAlpha->SetLabel(wxString::Format("%.3f", angle));

    LMS_ReadParam(lmsControl,LMS7param(IQCORR_TXTSP),(uint16_t*)&value);
    bitsToShift = (15 - LMS7param(IQCORR_TXTSP).msb - LMS7param(IQCORR_TXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_TXTSP->SetValue(value);
    angle = atan(value / 2048.0) * 180 / 3.141596;
    txtPhaseAlpha->SetLabel(wxString::Format("%.3f", angle));

    LMS_ReadParam(lmsControl,LMS7param(DCOFFI_RFE),(uint16_t*)&value);
    int16_t dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFI_RFE->SetValue(dcvalue);
    LMS_ReadParam(lmsControl,LMS7param(DCOFFQ_RFE),(uint16_t*)&value);
    dcvalue = value & 0x3F;
    if ((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFQ_RFE->SetValue(dcvalue);

    int8_t dccorr;
    LMS_ReadParam(lmsControl,LMS7param(DCCORRI_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRI_TXTSP->SetValue(dccorr);
    LMS_ReadParam(lmsControl,LMS7param(DCCORRQ_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRQ_TXTSP->SetValue(dccorr);
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_REF,&freq);
    lblCGENrefClk->SetLabel(wxString::Format(_("%f"), freq/1e6));
}

void lms7002_pnlCalibrations_view::OnCalibrationMethodChange( wxCommandEvent& event )
{
    if(rgrCalibrationMethod->GetSelection() == 0)
        lblCalibrationNote->SetLabel("");
    else
        lblCalibrationNote->SetLabel("Some boards might not have onboard loopback for selected Rx/Tx paths");
}
