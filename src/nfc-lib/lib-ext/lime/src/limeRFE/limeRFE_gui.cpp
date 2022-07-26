///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "limeRFE_gui.h"

///////////////////////////////////////////////////////////////////////////

limeRFE_view::limeRFE_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

	mbar = new wxMenuBar( 0 );
	mFile = new wxMenu();
	wxMenuItem* miQuit;
	miQuit = new wxMenuItem( mFile, wxID_ANY, wxString( wxT("&Quit") ) + wxT('\t') + wxT("Alt+F4"), wxEmptyString, wxITEM_NORMAL );
	mFile->Append( miQuit );

	mbar->Append( mFile, wxT("&File") );

	m_menu7 = new wxMenu();
	mbar->Append( m_menu7, wxT("MyMenu") );

	this->SetMenuBar( mbar );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	pnlMain = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlMain->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );
	pnlMain->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	m_panel15 = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer* sbSizer31;
	sbSizer31 = new wxStaticBoxSizer( new wxStaticBox( m_panel15, wxID_ANY, wxT("Communication") ), wxHORIZONTAL );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );

	rbI2C = new wxRadioButton( sbSizer31->GetStaticBox(), wxID_ANY, wxT("SDR"), wxDefaultPosition, wxSize( -1,-1 ), wxRB_GROUP );
	rbI2C->SetValue( true );
	bSizer19->Add( rbI2C, 0, wxALL, 5 );


	bSizer21->Add( bSizer19, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	rbUSB = new wxRadioButton( sbSizer31->GetStaticBox(), wxID_ANY, wxT("Direct (USB)"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer20->Add( rbUSB, 0, wxALL, 5 );


	bSizer21->Add( bSizer20, 1, wxEXPAND, 5 );


	sbSizer31->Add( bSizer21, 1, wxEXPAND, 5 );


	bSizer15->Add( sbSizer31, 1, wxEXPAND, 5 );


	m_panel15->SetSizer( bSizer15 );
	m_panel15->Layout();
	bSizer15->Fit( m_panel15 );
	fgSizer2->Add( m_panel15, 1, wxEXPAND | wxALL, 2 );

	m_panel161 = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( m_panel161, wxID_ANY, wxT("USB") ), wxHORIZONTAL );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );

	cmbbPorts = new wxComboBox( sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 );
	cmbbPorts->SetMaxSize( wxSize( 160,-1 ) );

	bSizer13->Add( cmbbPorts, 0, wxALL|wxEXPAND, 5 );


	bSizer12->Add( bSizer13, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );

	btnRefreshPorts = new wxButton( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer14->Add( btnRefreshPorts, 0, wxALL, 5 );


	bSizer12->Add( bSizer14, 1, wxEXPAND, 5 );


	sbSizer3->Add( bSizer12, 1, wxEXPAND, 5 );


	bSizer9->Add( sbSizer3, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer18;
	sbSizer18 = new wxStaticBoxSizer( new wxStaticBox( m_panel161, wxID_ANY, wxT("Port") ), wxHORIZONTAL );

	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxVERTICAL );

	btnOpenPort = new wxButton( sbSizer18->GetStaticBox(), wxID_ANY, wxT("Open"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer211->Add( btnOpenPort, 0, wxALL, 5 );


	sbSizer18->Add( bSizer211, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxVERTICAL );

	btnClosePort = new wxButton( sbSizer18->GetStaticBox(), wxID_ANY, wxT("Close"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer22->Add( btnClosePort, 0, wxALL, 5 );


	sbSizer18->Add( bSizer22, 1, wxEXPAND, 5 );


	bSizer9->Add( sbSizer18, 1, wxEXPAND, 5 );


	m_panel161->SetSizer( bSizer9 );
	m_panel161->Layout();
	bSizer9->Fit( m_panel161 );
	fgSizer2->Add( m_panel161, 1, wxEXPAND | wxALL, 2 );

	m_panel172 = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer* sbSizer21;
	sbSizer21 = new wxStaticBoxSizer( new wxStaticBox( m_panel172, wxID_ANY, wxT("Configuration") ), wxHORIZONTAL );

	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxVERTICAL );

	btnOpen = new wxButton( sbSizer21->GetStaticBox(), wxID_ANY, wxT("Open"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer23->Add( btnOpen, 0, wxALL, 5 );


	sbSizer21->Add( bSizer23, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );

	btnSave = new wxButton( sbSizer21->GetStaticBox(), wxID_ANY, wxT("Save"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer25->Add( btnSave, 0, wxALL, 5 );


	sbSizer21->Add( bSizer25, 1, wxEXPAND, 5 );


	bSizer7->Add( sbSizer21, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( m_panel172, wxID_ANY, wxT("Synchronize") ), wxHORIZONTAL );

	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxVERTICAL );

	btnReset = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Reset"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer26->Add( btnReset, 0, wxALL, 5 );


	sbSizer2->Add( bSizer26, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxVERTICAL );

	btnBoard2GUI = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Board->GUI"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer27->Add( btnBoard2GUI, 0, wxALL, 5 );


	sbSizer2->Add( bSizer27, 1, wxEXPAND, 5 );


	bSizer7->Add( sbSizer2, 1, wxEXPAND, 5 );


	m_panel172->SetSizer( bSizer7 );
	m_panel172->Layout();
	bSizer7->Fit( m_panel172 );
	fgSizer2->Add( m_panel172, 1, wxALL|wxEXPAND, 2 );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 0, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	pnlConfiguration = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxFlexGridSizer* fgSizer18;
	fgSizer18 = new wxFlexGridSizer( 0, 0, 0, 0 );
	fgSizer18->SetFlexibleDirection( wxBOTH );
	fgSizer18->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );

	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );

	m_panel17 = new wxPanel( pnlConfiguration, rx_channel_pnl, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( m_panel17, wxID_ANY, wxT("RX Channel") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer26;
	fgSizer26 = new wxFlexGridSizer( 3, 2, 0, 5 );
	fgSizer26->SetFlexibleDirection( wxBOTH );
	fgSizer26->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxString cTypeRXChoices[] = { wxT("Wideband"), wxT("HAM"), wxT("Cellular") };
	int cTypeRXNChoices = sizeof( cTypeRXChoices ) / sizeof( wxString );
	cTypeRX = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), cTypeRXNChoices, cTypeRXChoices, 0 );
	cTypeRX->SetSelection( 0 );
	fgSizer26->Add( cTypeRX, 1, wxALL|wxEXPAND, 5 );

	wxString cChannelRXChoices[] = { wxT("1 - 1000 MHz") };
	int cChannelRXNChoices = sizeof( cChannelRXChoices ) / sizeof( wxString );
	cChannelRX = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), cChannelRXNChoices, cChannelRXChoices, 0 );
	cChannelRX->SetSelection( 0 );
	cChannelRX->SetMinSize( wxSize( 190,-1 ) );

	fgSizer26->Add( cChannelRX, 1, wxALL|wxEXPAND, 5 );

	m_staticText261 = new wxStaticText( sbSizer9->GetStaticBox(), wxID_ANY, wxT("RX Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText261->Wrap( -1 );
	fgSizer26->Add( m_staticText261, 0, wxALL, 9 );

	wxString cPortRXChoices[] = { wxT("asdf"), wxT("asdf") };
	int cPortRXNChoices = sizeof( cPortRXChoices ) / sizeof( wxString );
	cPortRX = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), cPortRXNChoices, cPortRXChoices, 0 );
	cPortRX->SetSelection( 1 );
	fgSizer26->Add( cPortRX, 0, wxALL|wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer29;
	fgSizer29 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer29->AddGrowableCol( 1 );
	fgSizer29->SetFlexibleDirection( wxBOTH );
	fgSizer29->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText9 = new wxStaticText( sbSizer9->GetStaticBox(), wxID_ANY, wxT("Attenuation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	fgSizer29->Add( m_staticText9, 1, wxALL, 9 );

	wxString cAttenuationChoices[] = { wxT("asdf") };
	int cAttenuationNChoices = sizeof( cAttenuationChoices ) / sizeof( wxString );
	cAttenuation = new wxChoice( sbSizer9->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cAttenuationNChoices, cAttenuationChoices, 0 );
	cAttenuation->SetSelection( 0 );
	fgSizer29->Add( cAttenuation, 1, wxALIGN_RIGHT|wxALL, 5 );


	fgSizer26->Add( fgSizer29, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	cbNotch = new wxCheckBox( sbSizer9->GetStaticBox(), wxID_ANY, wxT("AM/FM Notch Filter"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( cbNotch, 0, wxTOP, 5 );


	fgSizer26->Add( bSizer6, 1, wxALL, 5 );


	sbSizer9->Add( fgSizer26, 1, wxEXPAND, 5 );


	m_panel17->SetSizer( sbSizer9 );
	m_panel17->Layout();
	sbSizer9->Fit( m_panel17 );
	bSizer51->Add( m_panel17, 0, wxEXPAND, 5 );

	m_panel171 = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxStaticBoxSizer* sbSizer91;
	sbSizer91 = new wxStaticBoxSizer( new wxStaticBox( m_panel171, wxID_ANY, wxT("TX Channel") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer261;
	fgSizer261 = new wxFlexGridSizer( 3, 2, 0, 5 );
	fgSizer261->AddGrowableCol( 0 );
	fgSizer261->AddGrowableRow( 0 );
	fgSizer261->SetFlexibleDirection( wxBOTH );
	fgSizer261->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	cbTXasRX = new wxCheckBox( sbSizer91->GetStaticBox(), wxID_ANY, wxT("Same as RX"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer261->Add( cbTXasRX, 0, wxALL, 5 );


	fgSizer261->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString cTypeTXChoices[] = { wxT("Wideband"), wxT("HAM"), wxT("Cellular") };
	int cTypeTXNChoices = sizeof( cTypeTXChoices ) / sizeof( wxString );
	cTypeTX = new wxChoice( sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), cTypeTXNChoices, cTypeTXChoices, 0 );
	cTypeTX->SetSelection( 0 );
	fgSizer261->Add( cTypeTX, 0, wxALL|wxEXPAND, 5 );

	wxString cChannelTXChoices[] = { wxT("1 - 1000 MHz") };
	int cChannelTXNChoices = sizeof( cChannelTXChoices ) / sizeof( wxString );
	cChannelTX = new wxChoice( sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), cChannelTXNChoices, cChannelTXChoices, 0 );
	cChannelTX->SetSelection( 0 );
	cChannelTX->SetMinSize( wxSize( 190,-1 ) );

	fgSizer261->Add( cChannelTX, 0, wxALL|wxEXPAND, 5 );

	m_staticText26 = new wxStaticText( sbSizer91->GetStaticBox(), wxID_ANY, wxT("TX Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	fgSizer261->Add( m_staticText26, 0, wxALL|wxEXPAND, 9 );

	wxString cPortTXChoices[] = { wxT("asdf"), wxT("asdf") };
	int cPortTXNChoices = sizeof( cPortTXChoices ) / sizeof( wxString );
	cPortTX = new wxChoice( sbSizer91->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), cPortTXNChoices, cPortTXChoices, 0 );
	cPortTX->SetSelection( 0 );
	fgSizer261->Add( cPortTX, 0, wxALL|wxEXPAND, 5 );


	sbSizer91->Add( fgSizer261, 1, wxEXPAND, 5 );


	m_panel171->SetSizer( sbSizer91 );
	m_panel171->Layout();
	sbSizer91->Fit( m_panel171 );
	bSizer51->Add( m_panel171, 1, wxEXPAND, 5 );

	pnlSWR = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( pnlSWR, wxID_ANY, wxT("SWR") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer22;
	fgSizer22 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer22->SetFlexibleDirection( wxBOTH );
	fgSizer22->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxBoxSizer* bSizer221;
	bSizer221 = new wxBoxSizer( wxVERTICAL );

	cbEnableSWR = new wxCheckBox( sbSizer12->GetStaticBox(), wxID_ANY, wxT("Enable SWR"), wxDefaultPosition, wxDefaultSize, 0 );
	cbEnableSWR->SetMinSize( wxSize( 140,-1 ) );

	bSizer221->Add( cbEnableSWR, 0, wxALL, 5 );


	fgSizer22->Add( bSizer221, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer231;
	bSizer231 = new wxBoxSizer( wxHORIZONTAL );

	rbSWRext = new wxRadioButton( sbSizer12->GetStaticBox(), wxID_ANY, wxT("External"), wxDefaultPosition, wxDefaultSize, 0 );
	rbSWRext->SetValue( true );
	bSizer231->Add( rbSWRext, 0, wxALL, 5 );

	rbSWRcell = new wxRadioButton( sbSizer12->GetStaticBox(), wxID_ANY, wxT("Cell"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer231->Add( rbSWRcell, 0, wxALL, 5 );


	fgSizer22->Add( bSizer231, 1, wxEXPAND, 5 );


	sbSizer12->Add( fgSizer22, 1, wxEXPAND, 5 );


	pnlSWR->SetSizer( sbSizer12 );
	pnlSWR->Layout();
	sbSizer12->Fit( pnlSWR );
	bSizer51->Add( pnlSWR, 0, wxEXPAND, 5 );


	fgSizer12->Add( bSizer51, 1, wxEXPAND, 5 );

	pnlTXRX = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( pnlTXRX, wxID_ANY, wxT("Mode:") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer262;
	fgSizer262 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer262->AddGrowableRow( 0 );
	fgSizer262->SetFlexibleDirection( wxBOTH );
	fgSizer262->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	tbtnTXRX = new wxToggleButton( sbSizer11->GetStaticBox(), wxID_ANY, wxT("TX/RX"), wxDefaultPosition, wxSize( 140,80 ), 0 );
	tbtnTXRX->SetFont( wxFont( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	fgSizer262->Add( tbtnTXRX, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	sbSizer11->Add( fgSizer262, 1, wxEXPAND, 5 );


	pnlTXRX->SetSizer( sbSizer11 );
	pnlTXRX->Layout();
	sbSizer11->Fit( pnlTXRX );
	fgSizer12->Add( pnlTXRX, 1, wxEXPAND, 5 );

	pnlTXRXEN = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxStaticBoxSizer* sbSizer111;
	sbSizer111 = new wxStaticBoxSizer( new wxStaticBox( pnlTXRXEN, wxID_ANY, wxT("Mode:") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer2621;
	fgSizer2621 = new wxFlexGridSizer( 3, 0, 0, 0 );
	fgSizer2621->AddGrowableRow( 0 );
	fgSizer2621->AddGrowableRow( 2 );
	fgSizer2621->SetFlexibleDirection( wxBOTH );
	fgSizer2621->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	fgSizer2621->Add( 0, 0, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer291;
	fgSizer291 = new wxFlexGridSizer( 2, 0, 0, 0 );
	fgSizer291->SetFlexibleDirection( wxBOTH );
	fgSizer291->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	tbtnRX = new wxToggleButton( sbSizer111->GetStaticBox(), wxID_ANY, wxT("RX"), wxDefaultPosition, wxSize( 140,35 ), 0 );
	tbtnRX->SetValue( true );
	tbtnRX->SetFont( wxFont( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	fgSizer291->Add( tbtnRX, 0, wxALL, 5 );

	tbtnTX = new wxToggleButton( sbSizer111->GetStaticBox(), wxID_ANY, wxT("TX"), wxDefaultPosition, wxSize( 140,35 ), 0 );
	tbtnTX->SetValue( true );
	tbtnTX->SetFont( wxFont( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	fgSizer291->Add( tbtnTX, 0, wxALL, 5 );


	fgSizer2621->Add( fgSizer291, 1, wxEXPAND, 5 );


	fgSizer2621->Add( 0, 0, 1, wxEXPAND, 5 );


	sbSizer111->Add( fgSizer2621, 1, wxEXPAND, 5 );


	pnlTXRXEN->SetSizer( sbSizer111 );
	pnlTXRXEN->Layout();
	sbSizer111->Fit( pnlTXRXEN );
	fgSizer12->Add( pnlTXRXEN, 1, wxEXPAND, 5 );

	btnConfigure = new wxButton( pnlConfiguration, wxID_ANY, wxT("Configure"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	btnConfigure->SetMinSize( wxSize( -1,50 ) );

	fgSizer12->Add( btnConfigure, 0, wxALL|wxEXPAND, 2 );

	pnlTXRXMode = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbsMode;
	sbsMode = new wxStaticBoxSizer( new wxStaticBox( pnlTXRXMode, wxID_ANY, wxT("Mode:") ), wxVERTICAL );

	txtTXRX = new wxStaticText( sbsMode->GetStaticBox(), wxID_ANY, wxT("RX"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTXRX->Wrap( -1 );
	txtTXRX->SetFont( wxFont( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	sbsMode->Add( txtTXRX, 0, wxLEFT, 5 );


	pnlTXRXMode->SetSizer( sbsMode );
	pnlTXRXMode->Layout();
	sbsMode->Fit( pnlTXRXMode );
	fgSizer12->Add( pnlTXRXMode, 1, wxEXPAND, 5 );

	pnlTXRXModeEN = new wxPanel( pnlConfiguration, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbsMode1;
	sbsMode1 = new wxStaticBoxSizer( new wxStaticBox( pnlTXRXModeEN, wxID_ANY, wxT("Mode:") ), wxVERTICAL );

	txtTXRXEN = new wxStaticText( sbsMode1->GetStaticBox(), wxID_ANY, wxT("RX"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTXRXEN->Wrap( -1 );
	txtTXRXEN->SetFont( wxFont( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	sbsMode1->Add( txtTXRXEN, 0, wxLEFT, 5 );


	pnlTXRXModeEN->SetSizer( sbsMode1 );
	pnlTXRXModeEN->Layout();
	sbsMode1->Fit( pnlTXRXModeEN );
	fgSizer12->Add( pnlTXRXModeEN, 1, wxEXPAND, 5 );


	fgSizer18->Add( fgSizer12, 0, wxEXPAND, 5 );


	pnlConfiguration->SetSizer( fgSizer18 );
	pnlConfiguration->Layout();
	fgSizer18->Fit( pnlConfiguration );
	fgSizer1->Add( pnlConfiguration, 1, wxEXPAND, 2 );

	pnlPowerMeter = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxSize( 200,-1 ), wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( pnlPowerMeter, wxID_ANY, wxT("Power Meter") ), wxVERTICAL );

	sbSizer8->SetMinSize( wxSize( 180,-1 ) );
	wxBoxSizer* bSizer232;
	bSizer232 = new wxBoxSizer( wxVERTICAL );

	m_panel16 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxFlexGridSizer* fgSizer303;
	fgSizer303 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer303->AddGrowableCol( 0 );
	fgSizer303->SetFlexibleDirection( wxBOTH );
	fgSizer303->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	btnReadADC = new wxButton( m_panel16, wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer303->Add( btnReadADC, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

	btnCalibrate = new wxButton( m_panel16, wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer303->Add( btnCalibrate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );


	m_panel16->SetSizer( fgSizer303 );
	m_panel16->Layout();
	fgSizer303->Fit( m_panel16 );
	bSizer232->Add( m_panel16, 0, wxEXPAND, 5 );

	pnlADC1 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBORDER_RAISED );
	pnlADC1->SetMinSize( wxSize( 30,-1 ) );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer33;
	fgSizer33 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer33->SetFlexibleDirection( wxBOTH );
	fgSizer33->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText13 = new wxStaticText( pnlADC1, wxID_ANY, wxT("ADC:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	m_staticText13->SetMinSize( wxSize( 80,-1 ) );

	fgSizer33->Add( m_staticText13, 0, wxALL, 2 );

	txtADC1 = new wxStaticText( pnlADC1, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtADC1->Wrap( -1 );
	txtADC1->SetMinSize( wxSize( 50,-1 ) );

	fgSizer33->Add( txtADC1, 1, wxALL, 2 );


	fgSizer33->Add( 0, 0, 1, wxEXPAND, 2 );


	fgSizer33->Add( 0, 0, 1, wxEXPAND, 5 );

	txtADC1_V = new wxStaticText( pnlADC1, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtADC1_V->Wrap( -1 );
	txtADC1_V->SetMinSize( wxSize( 50,-1 ) );

	fgSizer33->Add( txtADC1_V, 1, wxALL, 2 );

	m_staticText27 = new wxStaticText( pnlADC1, wxID_ANY, wxT("V"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_staticText27->Wrap( -1 );
	fgSizer33->Add( m_staticText27, 0, wxALL, 2 );

	m_staticText10 = new wxStaticText( pnlADC1, wxID_ANY, wxT("Power:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer33->Add( m_staticText10, 0, wxALL, 2 );

	txtPFWD_dBm = new wxStaticText( pnlADC1, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtPFWD_dBm->Wrap( -1 );
	txtPFWD_dBm->SetMinSize( wxSize( 50,-1 ) );

	fgSizer33->Add( txtPFWD_dBm, 1, wxALL, 2 );

	m_staticText12 = new wxStaticText( pnlADC1, wxID_ANY, wxT("dBm"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer33->Add( m_staticText12, 0, wxALL, 2 );


	fgSizer33->Add( 0, 0, 1, wxEXPAND, 2 );

	txtPFWD_W = new wxStaticText( pnlADC1, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtPFWD_W->Wrap( -1 );
	txtPFWD_W->SetMinSize( wxSize( 50,-1 ) );

	fgSizer33->Add( txtPFWD_W, 1, wxALL, 2 );

	m_staticText16 = new wxStaticText( pnlADC1, wxID_ANY, wxT("W"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer33->Add( m_staticText16, 0, wxALL, 2 );

	m_staticText102 = new wxStaticText( pnlADC1, wxID_ANY, wxT("Calib. Power:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText102->Wrap( -1 );
	fgSizer33->Add( m_staticText102, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	tcPowerCalibrate = new wxTextCtrl( pnlADC1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tcPowerCalibrate->SetMinSize( wxSize( 50,-1 ) );

	fgSizer33->Add( tcPowerCalibrate, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	m_staticText122 = new wxStaticText( pnlADC1, wxID_ANY, wxT("dBm"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText122->Wrap( -1 );
	fgSizer33->Add( m_staticText122, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );


	bSizer3->Add( fgSizer33, 1, wxEXPAND, 5 );


	pnlADC1->SetSizer( bSizer3 );
	pnlADC1->Layout();
	bSizer3->Fit( pnlADC1 );
	bSizer232->Add( pnlADC1, 1, wxEXPAND, 5 );

	pnlADC2 = new wxPanel( sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer331;
	fgSizer331 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer331->SetFlexibleDirection( wxBOTH );
	fgSizer331->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText131 = new wxStaticText( pnlADC2, wxID_ANY, wxT("ADC:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText131->Wrap( -1 );
	m_staticText131->SetMinSize( wxSize( 80,-1 ) );

	fgSizer331->Add( m_staticText131, 0, wxALL, 2 );

	txtADC2 = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtADC2->Wrap( -1 );
	txtADC2->SetMinSize( wxSize( 50,-1 ) );

	fgSizer331->Add( txtADC2, 0, wxALL, 2 );


	fgSizer331->Add( 0, 0, 1, wxEXPAND, 2 );


	fgSizer331->Add( 0, 0, 1, wxEXPAND, 5 );

	txtADC2_V = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtADC2_V->Wrap( -1 );
	txtADC2_V->SetMinSize( wxSize( 50,-1 ) );

	fgSizer331->Add( txtADC2_V, 0, wxALL, 2 );

	m_staticText29 = new wxStaticText( pnlADC2, wxID_ANY, wxT("V"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_staticText29->Wrap( -1 );
	m_staticText29->SetMinSize( wxSize( 30,-1 ) );

	fgSizer331->Add( m_staticText29, 0, wxALL, 2 );

	m_staticText101 = new wxStaticText( pnlADC2, wxID_ANY, wxT("RL:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	fgSizer331->Add( m_staticText101, 0, wxALL, 2 );

	txtRL_dB = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtRL_dB->Wrap( -1 );
	txtRL_dB->SetMinSize( wxSize( 50,-1 ) );

	fgSizer331->Add( txtRL_dB, 0, wxALL, 2 );

	m_staticText121 = new wxStaticText( pnlADC2, wxID_ANY, wxT("dB"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText121->Wrap( -1 );
	fgSizer331->Add( m_staticText121, 0, wxALL, 2 );

	m_staticText31 = new wxStaticText( pnlADC2, wxID_ANY, wxT("SWR:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText31->Wrap( -1 );
	fgSizer331->Add( m_staticText31, 0, wxALL, 2 );

	txtSWR = new wxStaticText( pnlADC2, wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	txtSWR->Wrap( -1 );
	txtSWR->SetMinSize( wxSize( 50,-1 ) );

	fgSizer331->Add( txtSWR, 0, wxALL, 2 );


	fgSizer331->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText1021 = new wxStaticText( pnlADC2, wxID_ANY, wxT("Calibrate RL:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1021->Wrap( -1 );
	fgSizer331->Add( m_staticText1021, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	tcRLCalibrate = new wxTextCtrl( pnlADC2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tcRLCalibrate->SetMinSize( wxSize( 50,-1 ) );

	fgSizer331->Add( tcRLCalibrate, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALIGN_RIGHT, 2 );

	m_staticText1221 = new wxStaticText( pnlADC2, wxID_ANY, wxT("dB"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1221->Wrap( -1 );
	fgSizer331->Add( m_staticText1221, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );


	bSizer5->Add( fgSizer331, 1, wxEXPAND, 5 );


	pnlADC2->SetSizer( bSizer5 );
	pnlADC2->Layout();
	bSizer5->Fit( pnlADC2 );
	bSizer232->Add( pnlADC2, 1, wxEXPAND, 5 );


	sbSizer8->Add( bSizer232, 1, wxEXPAND, 5 );


	pnlPowerMeter->SetSizer( sbSizer8 );
	pnlPowerMeter->Layout();
	fgSizer1->Add( pnlPowerMeter, 0, wxEXPAND, 2 );


	fgSizer2->Add( fgSizer1, 0, wxEXPAND, 2 );

	m_panel173 = new wxPanel( pnlMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBORDER_RAISED );
	wxFlexGridSizer* fgSizer17;
	fgSizer17 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer17->AddGrowableCol( 0 );
	fgSizer17->SetFlexibleDirection( wxBOTH );
	fgSizer17->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	txtMessageField = new wxTextCtrl( m_panel173, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	txtMessageField->SetMinSize( wxSize( -1,100 ) );

	fgSizer17->Add( txtMessageField, 0, wxEXPAND, 5 );

	m_panel18 = new wxPanel( m_panel173, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );

	btnClearMessages = new wxButton( m_panel18, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer24->Add( btnClearMessages, 0, 0, 5 );


	m_panel18->SetSizer( bSizer24 );
	m_panel18->Layout();
	bSizer24->Fit( m_panel18 );
	fgSizer17->Add( m_panel18, 0, wxEXPAND, 5 );


	m_panel173->SetSizer( fgSizer17 );
	m_panel173->Layout();
	fgSizer17->Fit( m_panel173 );
	fgSizer2->Add( m_panel173, 1, wxEXPAND, 5 );


	pnlMain->SetSizer( fgSizer2 );
	pnlMain->Layout();
	fgSizer2->Fit( pnlMain );
	bSizer2->Add( pnlMain, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer2 );
	this->Layout();
	bSizer2->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	mFile->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( limeRFE_view::OnQuit ), this, miQuit->GetId());
	rbI2C->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	rbUSB->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	btnRefreshPorts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnRefreshPorts ), NULL, this );
	btnOpenPort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpenPort ), NULL, this );
	btnClosePort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClosePort ), NULL, this );
	btnOpen->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpen ), NULL, this );
	btnSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnSave ), NULL, this );
	btnReset->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReset ), NULL, this );
	btnBoard2GUI->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnBoard2GUI ), NULL, this );
	pnlConfiguration->Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( limeRFE_view::OnEraseBackground_pnlConfiguration ), NULL, this );
	cTypeRX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeRX ), NULL, this );
	cChannelRX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelRX ), NULL, this );
	cPortRX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortRX ), NULL, this );
	cAttenuation->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncAttenuation ), NULL, this );
	cbNotch->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbNotch ), NULL, this );
	cbTXasRX->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbTXasRX ), NULL, this );
	cTypeTX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeTX ), NULL, this );
	cChannelTX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelTX ), NULL, this );
	cPortTX->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortTX ), NULL, this );
	cbEnableSWR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbEnableSWR ), NULL, this );
	rbSWRext->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRext ), NULL, this );
	rbSWRcell->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRcell ), NULL, this );
	tbtnTXRX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRX ), NULL, this );
	tbtnRX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnRXEN ), NULL, this );
	tbtnTX->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXEN ), NULL, this );
	btnConfigure->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnConfigure ), NULL, this );
	btnReadADC->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReadADC ), NULL, this );
	btnCalibrate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnCalibrate ), NULL, this );
	btnClearMessages->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClearMessages ), NULL, this );
}

limeRFE_view::~limeRFE_view()
{
	// Disconnect Events
	rbI2C->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	rbUSB->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbI2CrbUSB ), NULL, this );
	btnRefreshPorts->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnRefreshPorts ), NULL, this );
	btnOpenPort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpenPort ), NULL, this );
	btnClosePort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClosePort ), NULL, this );
	btnOpen->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnOpen ), NULL, this );
	btnSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnSave ), NULL, this );
	btnReset->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReset ), NULL, this );
	btnBoard2GUI->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnBoard2GUI ), NULL, this );
	pnlConfiguration->Disconnect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( limeRFE_view::OnEraseBackground_pnlConfiguration ), NULL, this );
	cTypeRX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeRX ), NULL, this );
	cChannelRX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelRX ), NULL, this );
	cPortRX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortRX ), NULL, this );
	cAttenuation->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncAttenuation ), NULL, this );
	cbNotch->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbNotch ), NULL, this );
	cbTXasRX->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbTXasRX ), NULL, this );
	cTypeTX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncTypeTX ), NULL, this );
	cChannelTX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncChannelTX ), NULL, this );
	cPortTX->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( limeRFE_view::OncPortTX ), NULL, this );
	cbEnableSWR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( limeRFE_view::OncbEnableSWR ), NULL, this );
	rbSWRext->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRext ), NULL, this );
	rbSWRcell->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( limeRFE_view::OnrbSWRcell ), NULL, this );
	tbtnTXRX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXRX ), NULL, this );
	tbtnRX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnRXEN ), NULL, this );
	tbtnTX->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OntbtnTXEN ), NULL, this );
	btnConfigure->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnConfigure ), NULL, this );
	btnReadADC->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnReadADC ), NULL, this );
	btnCalibrate->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnCalibrate ), NULL, this );
	btnClearMessages->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( limeRFE_view::OnbtnClearMessages ), NULL, this );

}
