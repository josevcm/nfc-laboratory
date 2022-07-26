///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "utilities_gui.h"

///////////////////////////////////////////////////////////////////////////

dlgConnectionSettings_view::dlgConnectionSettings_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer211;
	fgSizer211 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer211->AddGrowableCol( 0 );
	fgSizer211->AddGrowableRow( 0 );
	fgSizer211->SetFlexibleDirection( wxBOTH );
	fgSizer211->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer212;
	fgSizer212 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer212->AddGrowableCol( 0 );
	fgSizer212->AddGrowableRow( 1 );
	fgSizer212->SetFlexibleDirection( wxBOTH );
	fgSizer212->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText472 = new wxStaticText( this, wxID_ANY, wxT("Board connections:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText472->Wrap( -1 );
	fgSizer212->Add( m_staticText472, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	mListLMS7ports = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
	mListLMS7ports->SetMinSize( wxSize( 400,150 ) );
	
	fgSizer212->Add( mListLMS7ports, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer211->Add( fgSizer212, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer213;
	fgSizer213 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer213->AddGrowableCol( 0 );
	fgSizer213->AddGrowableCol( 1 );
	fgSizer213->AddGrowableCol( 2 );
	fgSizer213->SetFlexibleDirection( wxBOTH );
	fgSizer213->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnConnect = new wxButton( this, wxID_ANY, wxT("Connect"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnConnect, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnCancel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	btnDisconnect = new wxButton( this, wxID_ANY, wxT("Disconnect"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer213->Add( btnDisconnect, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	fgSizer211->Add( fgSizer213, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	this->SetSizer( fgSizer211 );
	this->Layout();
	fgSizer211->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( dlgConnectionSettings_view::GetDeviceList ) );
	btnConnect->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnConnect ), NULL, this );
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnCancel ), NULL, this );
	btnDisconnect->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnDisconnect ), NULL, this );
}

dlgConnectionSettings_view::~dlgConnectionSettings_view()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( dlgConnectionSettings_view::GetDeviceList ) );
	btnConnect->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnConnect ), NULL, this );
	btnCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnCancel ), NULL, this );
	btnDisconnect->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgConnectionSettings_view::OnDisconnect ), NULL, this );
	
}

dlgFullMessageLog_view::dlgFullMessageLog_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 500,200 ), wxDefaultSize );
	
	wxFlexGridSizer* fgSizer19;
	fgSizer19 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer19->AddGrowableCol( 0 );
	fgSizer19->AddGrowableRow( 0 );
	fgSizer19->SetFlexibleDirection( wxBOTH );
	fgSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	txtMessageField = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2 );
	#ifdef __WXGTK__
	if ( !txtMessageField->HasFlag( wxTE_MULTILINE ) )
	{
	txtMessageField->SetMaxLength( 100 );
	}
	#else
	txtMessageField->SetMaxLength( 100 );
	#endif
	txtMessageField->SetMinSize( wxSize( 300,100 ) );
	
	fgSizer19->Add( txtMessageField, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer19 );
	this->Layout();
	fgSizer19->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgFullMessageLog_view::~dlgFullMessageLog_view()
{
}

dlgDeviceInfo_view::dlgDeviceInfo_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnGetInfo = new wxButton( this, wxID_ANY, wxT("GET INFO"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer12->Add( btnGetInfo, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, wxT("Device:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer8->Add( m_staticText6, 0, wxALL, 5 );
	
	lblDeviceCtr = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblDeviceCtr->Wrap( -1 );
	fgSizer8->Add( lblDeviceCtr, 0, wxALL, 5 );
	
	m_staticText8 = new wxStaticText( this, wxID_ANY, wxT("Expansion:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	fgSizer8->Add( m_staticText8, 0, wxALL, 5 );
	
	lblExpansionCtr = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblExpansionCtr->Wrap( -1 );
	fgSizer8->Add( lblExpansionCtr, 0, wxALL, 5 );
	
	m_staticText10 = new wxStaticText( this, wxID_ANY, wxT("Firmware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer8->Add( m_staticText10, 0, wxALL, 5 );
	
	lblFirmwareCtr = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFirmwareCtr->Wrap( -1 );
	fgSizer8->Add( lblFirmwareCtr, 0, wxALL, 5 );
	
	m_staticText12 = new wxStaticText( this, wxID_ANY, wxT("Hardware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer8->Add( m_staticText12, 0, wxALL, 5 );
	
	lblHardwareCtr = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHardwareCtr->Wrap( -1 );
	fgSizer8->Add( lblHardwareCtr, 0, wxALL, 5 );
	
	m_staticText14 = new wxStaticText( this, wxID_ANY, wxT("Protocol:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	fgSizer8->Add( m_staticText14, 0, wxALL, 5 );
	
	lblProtocolCtr = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblProtocolCtr->Wrap( -1 );
	fgSizer8->Add( lblProtocolCtr, 0, wxALL, 5 );
	
	m_staticText41 = new wxStaticText( this, wxID_ANY, wxT("Gateware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	fgSizer8->Add( m_staticText41, 0, wxALL, 5 );
	
	lblGatewareCtr = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareCtr->Wrap( -1 );
	fgSizer8->Add( lblGatewareCtr, 0, wxALL, 5 );
	
	m_staticText45 = new wxStaticText( this, wxID_ANY, wxT("Gateware target:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText45->Wrap( -1 );
	fgSizer8->Add( m_staticText45, 0, wxALL, 5 );
	
	lblGatewareTargetCtr = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
	lblGatewareTargetCtr->Wrap( -1 );
	fgSizer8->Add( lblGatewareTargetCtr, 0, wxALL, 5 );
	
	
	fgSizer12->Add( fgSizer8, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer12 );
	this->Layout();
	fgSizer12->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	btnGetInfo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgDeviceInfo_view::OnGetInfo ), NULL, this );
}

dlgDeviceInfo_view::~dlgDeviceInfo_view()
{
	// Disconnect Events
	btnGetInfo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgDeviceInfo_view::OnGetInfo ), NULL, this );
	
}

SPI_view::SPI_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxFlexGridSizer* fgSizer305;
	fgSizer305 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer305->SetFlexibleDirection( wxBOTH );
	fgSizer305->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer160;
	sbSizer160 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LMS SPI ") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer306;
	fgSizer306 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer306->SetFlexibleDirection( wxBOTH );
	fgSizer306->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT1 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT1->Wrap( 0 );
	fgSizer306->Add( ID_STATICTEXT1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer306->Add( txtLMSwriteAddr, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT9 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT9->Wrap( 0 );
	fgSizer306->Add( ID_STATICTEXT9, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer306->Add( txtLMSwriteValue, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite->SetDefault(); 
	fgSizer306->Add( btnLMSwrite, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread->SetDefault(); 
	fgSizer306->Add( btnLMSread, 0, 0, 5 );
	
	ID_STATICTEXT5 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT5->Wrap( 0 );
	fgSizer306->Add( ID_STATICTEXT5, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus->Wrap( 0 );
	fgSizer306->Add( lblLMSwriteStatus, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer306, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3061;
	fgSizer3061 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3061->SetFlexibleDirection( wxBOTH );
	fgSizer3061->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT11 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT11->Wrap( 0 );
	fgSizer3061->Add( ID_STATICTEXT11, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr1 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr1->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3061->Add( txtLMSwriteAddr1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT91 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT91->Wrap( 0 );
	fgSizer3061->Add( ID_STATICTEXT91, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue1 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue1->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3061->Add( txtLMSwriteValue1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite1 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite1->SetDefault(); 
	fgSizer3061->Add( btnLMSwrite1, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread1 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread1->SetDefault(); 
	fgSizer3061->Add( btnLMSread1, 0, 0, 5 );
	
	ID_STATICTEXT51 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT51->Wrap( 0 );
	fgSizer3061->Add( ID_STATICTEXT51, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus1 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus1->Wrap( 0 );
	fgSizer3061->Add( lblLMSwriteStatus1, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer3061, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3062;
	fgSizer3062 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3062->SetFlexibleDirection( wxBOTH );
	fgSizer3062->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT12 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT12->Wrap( 0 );
	fgSizer3062->Add( ID_STATICTEXT12, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr2 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr2->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3062->Add( txtLMSwriteAddr2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT92 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT92->Wrap( 0 );
	fgSizer3062->Add( ID_STATICTEXT92, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue2 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue2->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3062->Add( txtLMSwriteValue2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite2 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite2->SetDefault(); 
	fgSizer3062->Add( btnLMSwrite2, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread2 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread2->SetDefault(); 
	fgSizer3062->Add( btnLMSread2, 0, 0, 5 );
	
	ID_STATICTEXT52 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT52->Wrap( 0 );
	fgSizer3062->Add( ID_STATICTEXT52, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus2 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus2->Wrap( 0 );
	fgSizer3062->Add( lblLMSwriteStatus2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer3062, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3063;
	fgSizer3063 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3063->SetFlexibleDirection( wxBOTH );
	fgSizer3063->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT13 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT13->Wrap( 0 );
	fgSizer3063->Add( ID_STATICTEXT13, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr3 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr3->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3063->Add( txtLMSwriteAddr3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT93 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT93->Wrap( 0 );
	fgSizer3063->Add( ID_STATICTEXT93, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue3 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue3->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3063->Add( txtLMSwriteValue3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite3 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite3->SetDefault(); 
	fgSizer3063->Add( btnLMSwrite3, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread3 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread3->SetDefault(); 
	fgSizer3063->Add( btnLMSread3, 0, 0, 5 );
	
	ID_STATICTEXT53 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT53->Wrap( 0 );
	fgSizer3063->Add( ID_STATICTEXT53, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus3 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus3->Wrap( 0 );
	fgSizer3063->Add( lblLMSwriteStatus3, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer3063, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3064;
	fgSizer3064 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3064->SetFlexibleDirection( wxBOTH );
	fgSizer3064->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT14 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT14->Wrap( 0 );
	fgSizer3064->Add( ID_STATICTEXT14, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr4 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr4->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3064->Add( txtLMSwriteAddr4, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT94 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT94->Wrap( 0 );
	fgSizer3064->Add( ID_STATICTEXT94, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue4 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue4->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3064->Add( txtLMSwriteValue4, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite4 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite4->SetDefault(); 
	fgSizer3064->Add( btnLMSwrite4, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread4 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread4->SetDefault(); 
	fgSizer3064->Add( btnLMSread4, 0, 0, 5 );
	
	ID_STATICTEXT54 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT54->Wrap( 0 );
	fgSizer3064->Add( ID_STATICTEXT54, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus4 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus4->Wrap( 0 );
	fgSizer3064->Add( lblLMSwriteStatus4, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer3064, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3065;
	fgSizer3065 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3065->SetFlexibleDirection( wxBOTH );
	fgSizer3065->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT18 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT18->Wrap( 0 );
	fgSizer3065->Add( ID_STATICTEXT18, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr5 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr5->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3065->Add( txtLMSwriteAddr5, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT95 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT95->Wrap( 0 );
	fgSizer3065->Add( ID_STATICTEXT95, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue5 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue5->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3065->Add( txtLMSwriteValue5, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite5 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite5->SetDefault(); 
	fgSizer3065->Add( btnLMSwrite5, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread5 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread5->SetDefault(); 
	fgSizer3065->Add( btnLMSread5, 0, 0, 5 );
	
	ID_STATICTEXT55 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT55->Wrap( 0 );
	fgSizer3065->Add( ID_STATICTEXT55, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus5 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus5->Wrap( 0 );
	fgSizer3065->Add( lblLMSwriteStatus5, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer3065, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3066;
	fgSizer3066 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3066->SetFlexibleDirection( wxBOTH );
	fgSizer3066->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT19 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT19->Wrap( 0 );
	fgSizer3066->Add( ID_STATICTEXT19, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr6 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr6->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3066->Add( txtLMSwriteAddr6, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT96 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT96->Wrap( 0 );
	fgSizer3066->Add( ID_STATICTEXT96, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue6 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue6->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3066->Add( txtLMSwriteValue6, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite6 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite6->SetDefault(); 
	fgSizer3066->Add( btnLMSwrite6, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread6 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread6->SetDefault(); 
	fgSizer3066->Add( btnLMSread6, 0, 0, 5 );
	
	ID_STATICTEXT56 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT56->Wrap( 0 );
	fgSizer3066->Add( ID_STATICTEXT56, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus6 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus6->Wrap( 0 );
	fgSizer3066->Add( lblLMSwriteStatus6, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer3066, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3067;
	fgSizer3067 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3067->SetFlexibleDirection( wxBOTH );
	fgSizer3067->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT110 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT110->Wrap( 0 );
	fgSizer3067->Add( ID_STATICTEXT110, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteAddr7 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtLMSwriteAddr7->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3067->Add( txtLMSwriteAddr7, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT97 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT97->Wrap( 0 );
	fgSizer3067->Add( ID_STATICTEXT97, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtLMSwriteValue7 = new wxTextCtrl( sbSizer160->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxDefaultSize, 0 );
	txtLMSwriteValue7->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3067->Add( txtLMSwriteValue7, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSwrite7 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSwrite7->SetDefault(); 
	fgSizer3067->Add( btnLMSwrite7, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	btnLMSread7 = new wxButton( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLMSread7->SetDefault(); 
	fgSizer3067->Add( btnLMSread7, 0, 0, 5 );
	
	ID_STATICTEXT57 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT57->Wrap( 0 );
	fgSizer3067->Add( ID_STATICTEXT57, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblLMSwriteStatus7 = new wxStaticText( sbSizer160->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 134,13 ), 0 );
	lblLMSwriteStatus7->Wrap( 0 );
	fgSizer3067->Add( lblLMSwriteStatus7, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer160->Add( fgSizer3067, 1, wxEXPAND, 5 );
	
	
	fgSizer305->Add( sbSizer160, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer168;
	sbSizer168 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Board SPI") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer320;
	fgSizer320 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer320->SetFlexibleDirection( wxBOTH );
	fgSizer320->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT15 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT15->Wrap( 0 );
	fgSizer320->Add( ID_STATICTEXT15, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer320->Add( txtBoardwriteAddr, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT16 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT16->Wrap( 0 );
	fgSizer320->Add( ID_STATICTEXT16, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteValue = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer320->Add( txtBoardwriteValue, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON24 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON24->SetDefault(); 
	fgSizer320->Add( ID_BUTTON24, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON25 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON25->SetDefault(); 
	fgSizer320->Add( ID_BUTTON25, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT17 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT17->Wrap( 0 );
	fgSizer320->Add( ID_STATICTEXT17, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus->Wrap( 0 );
	fgSizer320->Add( lblBoardwriteStatus, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer320, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3201;
	fgSizer3201 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3201->SetFlexibleDirection( wxBOTH );
	fgSizer3201->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT151 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT151->Wrap( 0 );
	fgSizer3201->Add( ID_STATICTEXT151, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr1 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr1->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3201->Add( txtBoardwriteAddr1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT161 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT161->Wrap( 0 );
	fgSizer3201->Add( ID_STATICTEXT161, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBoardwriteValue1 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue1->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3201->Add( txtBoardwriteValue1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON241 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON241->SetDefault(); 
	fgSizer3201->Add( ID_BUTTON241, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON251 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON251->SetDefault(); 
	fgSizer3201->Add( ID_BUTTON251, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT171 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT171->Wrap( 0 );
	fgSizer3201->Add( ID_STATICTEXT171, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus1 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus1->Wrap( 0 );
	fgSizer3201->Add( lblBoardwriteStatus1, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer3201, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3202;
	fgSizer3202 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3202->SetFlexibleDirection( wxBOTH );
	fgSizer3202->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT152 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT152->Wrap( 0 );
	fgSizer3202->Add( ID_STATICTEXT152, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr2 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr2->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3202->Add( txtBoardwriteAddr2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT162 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT162->Wrap( 0 );
	fgSizer3202->Add( ID_STATICTEXT162, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBoardwriteValue2 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue2->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3202->Add( txtBoardwriteValue2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON242 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON242->SetDefault(); 
	fgSizer3202->Add( ID_BUTTON242, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON252 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON252->SetDefault(); 
	fgSizer3202->Add( ID_BUTTON252, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT172 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT172->Wrap( 0 );
	fgSizer3202->Add( ID_STATICTEXT172, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus2 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus2->Wrap( 0 );
	fgSizer3202->Add( lblBoardwriteStatus2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer3202, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3203;
	fgSizer3203 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3203->SetFlexibleDirection( wxBOTH );
	fgSizer3203->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT153 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT153->Wrap( 0 );
	fgSizer3203->Add( ID_STATICTEXT153, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr3 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr3->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3203->Add( txtBoardwriteAddr3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT163 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT163->Wrap( 0 );
	fgSizer3203->Add( ID_STATICTEXT163, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBoardwriteValue3 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue3->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3203->Add( txtBoardwriteValue3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON243 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON243->SetDefault(); 
	fgSizer3203->Add( ID_BUTTON243, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON253 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON253->SetDefault(); 
	fgSizer3203->Add( ID_BUTTON253, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT173 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT173->Wrap( 0 );
	fgSizer3203->Add( ID_STATICTEXT173, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus3 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus3->Wrap( 0 );
	fgSizer3203->Add( lblBoardwriteStatus3, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer3203, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3204;
	fgSizer3204 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3204->SetFlexibleDirection( wxBOTH );
	fgSizer3204->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT154 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT154->Wrap( 0 );
	fgSizer3204->Add( ID_STATICTEXT154, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr4 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr4->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3204->Add( txtBoardwriteAddr4, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT164 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT164->Wrap( 0 );
	fgSizer3204->Add( ID_STATICTEXT164, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBoardwriteValue4 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue4->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3204->Add( txtBoardwriteValue4, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON244 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON244->SetDefault(); 
	fgSizer3204->Add( ID_BUTTON244, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON254 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON254->SetDefault(); 
	fgSizer3204->Add( ID_BUTTON254, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT174 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT174->Wrap( 0 );
	fgSizer3204->Add( ID_STATICTEXT174, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus4 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus4->Wrap( 0 );
	fgSizer3204->Add( lblBoardwriteStatus4, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer3204, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3205;
	fgSizer3205 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3205->SetFlexibleDirection( wxBOTH );
	fgSizer3205->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT155 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT155->Wrap( 0 );
	fgSizer3205->Add( ID_STATICTEXT155, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr5 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr5->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3205->Add( txtBoardwriteAddr5, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT165 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT165->Wrap( 0 );
	fgSizer3205->Add( ID_STATICTEXT165, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteValue5 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue5->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3205->Add( txtBoardwriteValue5, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON245 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON245->SetDefault(); 
	fgSizer3205->Add( ID_BUTTON245, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON255 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON255->SetDefault(); 
	fgSizer3205->Add( ID_BUTTON255, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT175 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT175->Wrap( 0 );
	fgSizer3205->Add( ID_STATICTEXT175, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus5 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus5->Wrap( 0 );
	fgSizer3205->Add( lblBoardwriteStatus5, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer3205, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3206;
	fgSizer3206 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3206->SetFlexibleDirection( wxBOTH );
	fgSizer3206->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT156 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT156->Wrap( 0 );
	fgSizer3206->Add( ID_STATICTEXT156, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr6 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr6->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3206->Add( txtBoardwriteAddr6, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT166 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT166->Wrap( 0 );
	fgSizer3206->Add( ID_STATICTEXT166, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteValue6 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue6->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3206->Add( txtBoardwriteValue6, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON246 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON246->SetDefault(); 
	fgSizer3206->Add( ID_BUTTON246, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON256 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON256->SetDefault(); 
	fgSizer3206->Add( ID_BUTTON256, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT176 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT176->Wrap( 0 );
	fgSizer3206->Add( ID_STATICTEXT176, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus6 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus6->Wrap( 0 );
	fgSizer3206->Add( lblBoardwriteStatus6, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer3206, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer3207;
	fgSizer3207 = new wxFlexGridSizer( 0, 8, 5, 5 );
	fgSizer3207->SetFlexibleDirection( wxBOTH );
	fgSizer3207->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	ID_STATICTEXT157 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Address(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT157->Wrap( 0 );
	fgSizer3207->Add( ID_STATICTEXT157, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteAddr7 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteAddr7->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3207->Add( txtBoardwriteAddr7, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT167 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Value(Hex):"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT167->Wrap( 0 );
	fgSizer3207->Add( ID_STATICTEXT167, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtBoardwriteValue7 = new wxTextCtrl( sbSizer168->GetStaticBox(), wxID_ANY, wxT("FFFF"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtBoardwriteValue7->SetMinSize( wxSize( 48,-1 ) );
	
	fgSizer3207->Add( txtBoardwriteValue7, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON247 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Write"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON247->SetDefault(); 
	fgSizer3207->Add( ID_BUTTON247, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_BUTTON257 = new wxButton( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_BUTTON257->SetDefault(); 
	fgSizer3207->Add( ID_BUTTON257, 1, wxALIGN_CENTER_VERTICAL, 5 );
	
	ID_STATICTEXT177 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	ID_STATICTEXT177->Wrap( 0 );
	fgSizer3207->Add( ID_STATICTEXT177, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	lblBoardwriteStatus7 = new wxStaticText( sbSizer168->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	lblBoardwriteStatus7->Wrap( 0 );
	fgSizer3207->Add( lblBoardwriteStatus7, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer168->Add( fgSizer3207, 1, wxEXPAND, 5 );
	
	
	fgSizer305->Add( sbSizer168, 1, wxALIGN_LEFT|wxALIGN_TOP|wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer305 );
	this->Layout();
	fgSizer305->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	btnLMSwrite->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite4->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread4->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite5->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread5->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite7->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread7->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	ID_BUTTON24->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON25->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON241->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON251->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON242->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON252->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON243->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON253->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON244->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON254->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON245->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON255->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON246->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON256->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON247->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON257->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
}

SPI_view::~SPI_view()
{
	// Disconnect Events
	btnLMSwrite->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite4->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread4->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite5->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread5->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	btnLMSwrite7->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSwrite ), NULL, this );
	btnLMSread7->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onLMSread ), NULL, this );
	ID_BUTTON24->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON25->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON241->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON251->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON242->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON252->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON243->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON253->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON244->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON254->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON245->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON255->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON246->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON256->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	ID_BUTTON247->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::onBoardWrite ), NULL, this );
	ID_BUTTON257->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SPI_view::OnBoardRead ), NULL, this );
	
}
