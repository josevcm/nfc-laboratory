#include "lms7002_pnlCDS_view.h"
#include <map>
#include "lms7002_gui_utilities.h"

using namespace lime;

lms7002_pnlCDS_view::lms7002_pnlCDS_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer190;
    fgSizer190 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer190->SetFlexibleDirection( wxBOTH );
    fgSizer190->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer121;
    sbSizer121 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Clock inversion") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer191;
    fgSizer191 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer191->SetFlexibleDirection( wxBOTH );
    fgSizer191->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkCDSN_TXBTSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXBTSP, wxT("TX TSPB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_TXBTSP->SetToolTip( wxT("TX TSPB clock inversion control") );
    
    fgSizer191->Add( chkCDSN_TXBTSP, 0, flags, 0 );
    
    chkCDSN_TXATSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXATSP, wxT("TX TSPA"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_TXATSP->SetToolTip( wxT("TX TSPA clock inversion control") );
    
    fgSizer191->Add( chkCDSN_TXATSP, 0, flags, 0 );
    
    chkCDSN_RXBTSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXBTSP, wxT("RX TSPB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_RXBTSP->SetToolTip( wxT("RX TSPB clock inversion control") );
    
    fgSizer191->Add( chkCDSN_RXBTSP, 0, flags, 0 );
    
    chkCDSN_RXATSP = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXATSP, wxT("RX TSPA"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_RXATSP->SetToolTip( wxT("RX TSPA clock inversion control") );
    
    fgSizer191->Add( chkCDSN_RXATSP, 0, flags, 0 );
    
    chkCDSN_TXBLML = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXBLML, wxT("TX LMLB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_TXBLML->SetToolTip( wxT("TX LMLB clock inversion control") );
    
    fgSizer191->Add( chkCDSN_TXBLML, 0, flags, 0 );
    
    chkCDSN_TXALML = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_TXALML, wxT("TX LMLA"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_TXALML->SetToolTip( wxT("TX LMLA clock inversion control") );
    
    fgSizer191->Add( chkCDSN_TXALML, 0, flags, 0 );
    
    chkCDSN_RXBLML = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXBLML, wxT("RX LMLB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_RXBLML->SetToolTip( wxT("RX LMLB clock inversion control") );
    
    fgSizer191->Add( chkCDSN_RXBLML, 0, flags, 0 );
    
    chkCDSN_RXALML  = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_RXALML , wxT("RX LMLA"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_RXALML ->SetToolTip( wxT("RX LMLA clock inversion control") );
    
    fgSizer191->Add( chkCDSN_RXALML , 0, flags, 0 );
    
    chkCDSN_MCLK2 = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_MCLK2, wxT("MCLK2"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_MCLK2->SetToolTip( wxT("MCLK2 clock inversion control") );
    
    fgSizer191->Add( chkCDSN_MCLK2, 0, flags, 0 );
    
    chkCDSN_MCLK1 = new wxCheckBox( sbSizer121->GetStaticBox(), ID_CDSN_MCLK1, wxT("MCLK1"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCDSN_MCLK1->SetToolTip( wxT("MCLK1 clock inversion control") );
    
    fgSizer191->Add( chkCDSN_MCLK1, 0, flags, 0 );
    
    
    sbSizer121->Add( fgSizer191, 0, wxEXPAND, 0 );
    
    
    fgSizer190->Add( sbSizer121, 0, wxEXPAND, 0 );
    
    wxStaticBoxSizer* sbSizer122;
    sbSizer122 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Clock delay") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer192;
    fgSizer192 = new wxFlexGridSizer( 0, 5, 5, 5 );
    fgSizer192->SetFlexibleDirection( wxBOTH );
    fgSizer192->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString rgrCDS_MCLK2Choices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
    int rgrCDS_MCLK2NChoices = sizeof( rgrCDS_MCLK2Choices ) / sizeof( wxString );
    rgrCDS_MCLK2 = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_MCLK2, wxT("MCLK2"), wxDefaultPosition, wxDefaultSize, rgrCDS_MCLK2NChoices, rgrCDS_MCLK2Choices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_MCLK2->SetSelection( 0 );
    rgrCDS_MCLK2->SetToolTip( wxT("MCLK2 clock delay") );
    
    fgSizer192->Add( rgrCDS_MCLK2, 0, flags, 0 );
    
    wxString rgrCDS_MCLK1Choices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
    int rgrCDS_MCLK1NChoices = sizeof( rgrCDS_MCLK1Choices ) / sizeof( wxString );
    rgrCDS_MCLK1 = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_MCLK1, wxT("MCLK1"), wxDefaultPosition, wxDefaultSize, rgrCDS_MCLK1NChoices, rgrCDS_MCLK1Choices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_MCLK1->SetSelection( 0 );
    rgrCDS_MCLK1->SetToolTip( wxT("MCLK1 clock delay") );
    
    fgSizer192->Add( rgrCDS_MCLK1, 0, flags, 0 );
    
    wxString rgrCDS_TXBTSPChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
    int rgrCDS_TXBTSPNChoices = sizeof( rgrCDS_TXBTSPChoices ) / sizeof( wxString );
    rgrCDS_TXBTSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXBTSP, wxT("TX TSP B"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXBTSPNChoices, rgrCDS_TXBTSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_TXBTSP->SetSelection( 0 );
    rgrCDS_TXBTSP->SetToolTip( wxT("TX TSP B clock delay") );
    
    fgSizer192->Add( rgrCDS_TXBTSP, 0, flags, 0 );
    
    wxString rgrCDS_TXATSPChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
    int rgrCDS_TXATSPNChoices = sizeof( rgrCDS_TXATSPChoices ) / sizeof( wxString );
    rgrCDS_TXATSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXATSP, wxT("TX TSP A"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXATSPNChoices, rgrCDS_TXATSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_TXATSP->SetSelection( 0 );
    rgrCDS_TXATSP->SetToolTip( wxT("TX TSP A clock delay") );
    
    fgSizer192->Add( rgrCDS_TXATSP, 0, flags, 0 );
    
    wxString rgrCDS_RXBTSPChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
    int rgrCDS_RXBTSPNChoices = sizeof( rgrCDS_RXBTSPChoices ) / sizeof( wxString );
    rgrCDS_RXBTSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXBTSP, wxT("RX TSP B"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXBTSPNChoices, rgrCDS_RXBTSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_RXBTSP->SetSelection( 0 );
    rgrCDS_RXBTSP->SetToolTip( wxT("RX TSP B clock delay") );
    
    fgSizer192->Add( rgrCDS_RXBTSP, 0, flags, 0 );
    
    wxString rgrCDS_RXATSPChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
    int rgrCDS_RXATSPNChoices = sizeof( rgrCDS_RXATSPChoices ) / sizeof( wxString );
    rgrCDS_RXATSP = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXATSP, wxT("RX TSP A"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXATSPNChoices, rgrCDS_RXATSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_RXATSP->SetSelection( 0 );
    rgrCDS_RXATSP->SetToolTip( wxT("RX TSP A clock delay") );
    
    fgSizer192->Add( rgrCDS_RXATSP, 0, flags, 0 );
    
    wxString rgrCDS_TXBLMLChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
    int rgrCDS_TXBLMLNChoices = sizeof( rgrCDS_TXBLMLChoices ) / sizeof( wxString );
    rgrCDS_TXBLML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXBLML, wxT("TX LML B"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXBLMLNChoices, rgrCDS_TXBLMLChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_TXBLML->SetSelection( 0 );
    rgrCDS_TXBLML->SetToolTip( wxT("TX LML B clock delay") );
    
    fgSizer192->Add( rgrCDS_TXBLML, 0, flags, 0 );
    
    wxString rgrCDS_TXALMLChoices[] = { wxT("400 ps"), wxT("500 ps"), wxT("600 ps"), wxT("700 ps") };
    int rgrCDS_TXALMLNChoices = sizeof( rgrCDS_TXALMLChoices ) / sizeof( wxString );
    rgrCDS_TXALML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_TXALML, wxT("TX LML A"), wxDefaultPosition, wxDefaultSize, rgrCDS_TXALMLNChoices, rgrCDS_TXALMLChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_TXALML->SetSelection( 0 );
    rgrCDS_TXALML->SetToolTip( wxT("TX LML A clock delay") );
    
    fgSizer192->Add( rgrCDS_TXALML, 0, flags, 0 );
    
    wxString rgrCDS_RXBLMLChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
    int rgrCDS_RXBLMLNChoices = sizeof( rgrCDS_RXBLMLChoices ) / sizeof( wxString );
    rgrCDS_RXBLML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXBLML, wxT("RX LML B"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXBLMLNChoices, rgrCDS_RXBLMLChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_RXBLML->SetSelection( 0 );
    rgrCDS_RXBLML->SetToolTip( wxT("RX LML B clock delay") );
    
    fgSizer192->Add( rgrCDS_RXBLML, 0, flags, 0 );
    
    wxString rgrCDS_RXALMLChoices[] = { wxT("200 ps"), wxT("500 ps"), wxT("800 ps"), wxT("1100 ps") };
    int rgrCDS_RXALMLNChoices = sizeof( rgrCDS_RXALMLChoices ) / sizeof( wxString );
    rgrCDS_RXALML = new wxRadioBox( sbSizer122->GetStaticBox(), ID_CDS_RXALML, wxT("RX LML A"), wxDefaultPosition, wxDefaultSize, rgrCDS_RXALMLNChoices, rgrCDS_RXALMLChoices, 1, wxRA_SPECIFY_COLS );
    rgrCDS_RXALML->SetSelection( 0 );
    rgrCDS_RXALML->SetToolTip( wxT("RX LML A clock delay") );
    
    fgSizer192->Add( rgrCDS_RXALML, 0, flags, 0 );
    
    
    sbSizer122->Add( fgSizer192, 0, flags, 0 );
    
    
    fgSizer190->Add( sbSizer122, 0, flags, 0 );
    
    
    this->SetSizer( fgSizer190 );
    this->Layout();
    fgSizer190->Fit( this );
    
    // Connect Events
    chkCDSN_TXBTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_TXATSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_RXBTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_RXATSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_TXBLML->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_TXALML->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_RXBLML->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_RXALML ->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_MCLK2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    chkCDSN_MCLK1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_MCLK2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_MCLK1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_TXBTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_TXATSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_RXBTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_RXATSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_TXBLML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_TXALML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_RXBLML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );
    rgrCDS_RXALML->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlCDS_view::ParameterChangeHandler ), NULL, this );

    wndId2Enum[chkCDSN_MCLK1] = LMS7param(CDSN_MCLK1);
    wndId2Enum[chkCDSN_MCLK2] = LMS7param(CDSN_MCLK2);
    wndId2Enum[chkCDSN_RXALML] = LMS7param(CDSN_RXALML);
    wndId2Enum[chkCDSN_RXATSP] = LMS7param(CDSN_RXATSP);
    wndId2Enum[chkCDSN_RXBLML] = LMS7param(CDSN_RXBLML);
    wndId2Enum[chkCDSN_RXBTSP] = LMS7param(CDSN_RXBTSP);
    wndId2Enum[chkCDSN_TXALML] = LMS7param(CDSN_TXALML);
    wndId2Enum[chkCDSN_TXATSP] = LMS7param(CDSN_TXATSP);
    wndId2Enum[chkCDSN_TXBLML] = LMS7param(CDSN_TXBLML);
    wndId2Enum[chkCDSN_TXBTSP] = LMS7param(CDSN_TXBTSP);
    wndId2Enum[rgrCDS_MCLK1] = LMS7param(CDS_MCLK1);
    wndId2Enum[rgrCDS_MCLK2] = LMS7param(CDS_MCLK2);
    wndId2Enum[rgrCDS_RXALML] = LMS7param(CDS_RXALML);
    wndId2Enum[rgrCDS_RXATSP] = LMS7param(CDS_RXATSP);
    wndId2Enum[rgrCDS_RXBLML] = LMS7param(CDS_RXBLML);
    wndId2Enum[rgrCDS_RXBTSP] = LMS7param(CDS_RXBTSP);
    wndId2Enum[rgrCDS_TXALML] = LMS7param(CDS_TXALML);
    wndId2Enum[rgrCDS_TXATSP] = LMS7param(CDS_TXATSP);
    wndId2Enum[rgrCDS_TXBLML] = LMS7param(CDS_TXBLML);
    wndId2Enum[rgrCDS_TXBTSP] = LMS7param(CDS_TXBTSP);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlCDS_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlCDS_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlCDS_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
