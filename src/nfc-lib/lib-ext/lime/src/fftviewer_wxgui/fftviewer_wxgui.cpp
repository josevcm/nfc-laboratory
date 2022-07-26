///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "OpenGLGraph.h"

#include "fftviewer_wxgui.h"

///////////////////////////////////////////////////////////////////////////

frFFTviewer::frFFTviewer( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer7->AddGrowableCol( 0 );
	fgSizer7->AddGrowableRow( 0 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	mPlotsSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	mPlotsSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( frFFTviewer::mPlotsSplitterOnIdle ), NULL, this );
	
	mTimeConstellationPanel = new wxPanel( mPlotsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer9->AddGrowableCol( 0 );
	fgSizer9->AddGrowableRow( 0 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	mTimeDomainPanel = new OpenGLGraph( mTimeConstellationPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL );
	bSizer2->Add( mTimeDomainPanel, 1, wxALL|wxEXPAND, 5 );
	
	mConstelationPanel = new OpenGLGraph( mTimeConstellationPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL );
	bSizer2->Add( mConstelationPanel, 0, wxALL|wxEXPAND|wxSHAPED, 5 );
	
	
	fgSizer9->Add( bSizer2, 1, wxEXPAND, 5 );
	
	
	mTimeConstellationPanel->SetSizer( fgSizer9 );
	mTimeConstellationPanel->Layout();
	fgSizer9->Fit( mTimeConstellationPanel );
	mFFTpanel = new OpenGLGraph( mPlotsSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	mPlotsSplitter->SplitHorizontally( mTimeConstellationPanel, mFFTpanel, 0 );
	fgSizer7->Add( mPlotsSplitter, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("FFT parameters") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer11->AddGrowableCol( 1 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText6 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Nyquist freq (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer11->Add( m_staticText6, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtNyquistFreqMHz = new wxTextCtrl( sbSizer1->GetStaticBox(), wxID_ANY, wxT("15.36"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtNyquistFreqMHz->HasFlag( wxTE_MULTILINE ) )
	{
	txtNyquistFreqMHz->SetMaxLength( 8 );
	}
	#else
	txtNyquistFreqMHz->SetMaxLength( 8 );
	#endif
	
	fgSizer11->Add( txtNyquistFreqMHz, 0, wxEXPAND, 5 );
	
	m_staticText7 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Samples count"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	fgSizer11->Add( m_staticText7, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	spinFFTsize = new wxSpinCtrl( sbSizer1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 64, 65536, 16384 );
	
	fgSizer11->Add( spinFFTsize, 0, wxEXPAND, 5 );
	
	
	fgSizer11->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	sbSizer1->Add( fgSizer11, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( sbSizer1, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Data reading") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxString cmbStreamTypeChoices[] = { wxT("LMS#1 SISO"), wxT("LMS#1 MIMO"), wxT("LMS#2 SISO"), wxT("LMS#2 MIMO"), wxT("Ext. ADC/DAC") };
	int cmbStreamTypeNChoices = sizeof( cmbStreamTypeChoices ) / sizeof( wxString );
	cmbStreamType = new wxChoice( sbSizer2->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cmbStreamTypeNChoices, cmbStreamTypeChoices, 0 );
	cmbStreamType->SetSelection( 0 );
	fgSizer12->Add( cmbStreamType, 0, wxEXPAND, 5 );
	
	wxString cmbFmtChoices[] = { wxT("12 bit format"), wxT("16 bit format") };
	int cmbFmtNChoices = sizeof( cmbFmtChoices ) / sizeof( wxString );
	cmbFmt = new wxChoice( sbSizer2->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cmbFmtNChoices, cmbFmtChoices, 0 );
	cmbFmt->SetSelection( 0 );
	fgSizer12->Add( cmbFmt, 0, wxEXPAND, 5 );
	
	chkEnTx = new wxCheckBox( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Loopback RX to TX"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEnTx->SetToolTip( wxT("Freezes FFT plot") );
	
	fgSizer12->Add( chkEnTx, 0, wxALL, 2 );
	
	chkEnSync = new wxCheckBox( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Sync timestamp"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEnSync->Enable( false );
	chkEnSync->SetToolTip( wxT("Freezes FFT plot") );
	
	fgSizer12->Add( chkEnSync, 0, wxALL, 2 );
	
	btnStartStop = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("START"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer12->Add( btnStartStop, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer13;
	fgSizer13 = new wxFlexGridSizer( 0, 2, 0, 5 );
	fgSizer13->SetFlexibleDirection( wxBOTH );
	fgSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText8 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Rx rate:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	fgSizer13->Add( m_staticText8, 0, 0, 5 );
	
	lblRxDataRate = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("0 MB/s"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRxDataRate->Wrap( -1 );
	fgSizer13->Add( lblRxDataRate, 0, 0, 5 );
	
	m_staticText18 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Tx rate:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	fgSizer13->Add( m_staticText18, 0, 0, 5 );
	
	lblTxDataRate = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("0 MB/s"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTxDataRate->Wrap( -1 );
	fgSizer13->Add( lblTxDataRate, 0, 0, 5 );
	
	
	fgSizer12->Add( fgSizer13, 1, wxEXPAND, 5 );
	
	
	sbSizer2->Add( fgSizer12, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Window function:") ), wxVERTICAL );
	
	wxString cmbWindowFuncChoices[] = { wxT("Rectangular"), wxT("Blackman-harris"), wxT("Hamming"), wxT("Hanning") };
	int cmbWindowFuncNChoices = sizeof( cmbWindowFuncChoices ) / sizeof( wxString );
	cmbWindowFunc = new wxChoice( sbSizer6->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cmbWindowFuncNChoices, cmbWindowFuncChoices, 0 );
	cmbWindowFunc->SetSelection( 0 );
	sbSizer6->Add( cmbWindowFunc, 0, wxEXPAND, 5 );
	
	
	sbSizer2->Add( sbSizer6, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer51;
	sbSizer51 = new wxStaticBoxSizer( new wxStaticBox( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Capture to file") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer121;
	fgSizer121 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer121->SetFlexibleDirection( wxBOTH );
	fgSizer121->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkCaptureToFile = new wxCheckBox( sbSizer51->GetStaticBox(), wxID_ANY, wxT("Capture enable"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer121->Add( chkCaptureToFile, 0, 0, 5 );
	
	
	fgSizer121->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText12 = new wxStaticText( sbSizer51->GetStaticBox(), wxID_ANY, wxT("Samples to capture:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer121->Add( m_staticText12, 0, wxALL, 5 );
	
	spinCaptureCount = new wxSpinCtrl( sbSizer51->GetStaticBox(), wxID_ANY, wxT("16384"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 30000000, 16384 );
	fgSizer121->Add( spinCaptureCount, 0, 0, 5 );
	
	
	sbSizer51->Add( fgSizer121, 1, wxEXPAND, 5 );
	
	
	sbSizer2->Add( sbSizer51, 0, wxTOP, 5 );
	
	
	fgSizer10->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Graphs") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer14->SetFlexibleDirection( wxBOTH );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkFreezeTimeDomain = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Freeze time"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFreezeTimeDomain->SetToolTip( wxT("Freezes time domain plot") );
	
	fgSizer14->Add( chkFreezeTimeDomain, 0, 0, 5 );
	
	chkFreezeConstellation = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Freeze constellation"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFreezeConstellation->SetToolTip( wxT("Freezes constellation plot") );
	
	fgSizer14->Add( chkFreezeConstellation, 0, 0, 5 );
	
	chkFreezeFFT = new wxCheckBox( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Freeze FFT"), wxDefaultPosition, wxDefaultSize, 0 );
	chkFreezeFFT->SetToolTip( wxT("Freezes FFT plot") );
	
	fgSizer14->Add( chkFreezeFFT, 0, 0, 5 );
	
	wxFlexGridSizer* fgSizer101;
	fgSizer101 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer101->SetFlexibleDirection( wxBOTH );
	fgSizer101->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText11 = new wxStaticText( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Display channel:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	fgSizer101->Add( m_staticText11, 0, wxALL, 5 );
	
	wxString cmbChannelVisibilityChoices[] = { wxT("A"), wxT("B"), wxT("A&B") };
	int cmbChannelVisibilityNChoices = sizeof( cmbChannelVisibilityChoices ) / sizeof( wxString );
	cmbChannelVisibility = new wxChoice( sbSizer3->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cmbChannelVisibilityNChoices, cmbChannelVisibilityChoices, 0 );
	cmbChannelVisibility->SetSelection( 0 );
	fgSizer101->Add( cmbChannelVisibility, 0, 0, 5 );
	
	m_staticText23 = new wxStaticText( sbSizer3->GetStaticBox(), wxID_ANY, wxT("FFT averaging:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	m_staticText23->SetToolTip( wxT("Number of FFTs to average") );
	
	fgSizer101->Add( m_staticText23, 0, wxALL, 5 );
	
	spinAvgCount = new wxSpinCtrl( sbSizer3->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 512, 1 );
	spinAvgCount->SetToolTip( wxT("Number of FFTs to average") );
	
	fgSizer101->Add( spinAvgCount, 0, 0, 5 );
	
	
	fgSizer14->Add( fgSizer101, 1, wxEXPAND, 5 );
	
	
	sbSizer3->Add( fgSizer14, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( sbSizer3, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Buffers status") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer19;
	fgSizer19 = new wxFlexGridSizer( 0, 2, 5, 5 );
	fgSizer19->AddGrowableCol( 1 );
	fgSizer19->SetFlexibleDirection( wxBOTH );
	fgSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText15 = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, wxT("Rx:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	fgSizer19->Add( m_staticText15, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	gaugeRxBuffer = new wxGauge( sbSizer5->GetStaticBox(), wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	gaugeRxBuffer->SetValue( 0 ); 
	fgSizer19->Add( gaugeRxBuffer, 1, wxEXPAND, 5 );
	
	m_staticText16 = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, wxT("Tx:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer19->Add( m_staticText16, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	gaugeTxBuffer = new wxGauge( sbSizer5->GetStaticBox(), wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	gaugeTxBuffer->SetValue( 0 ); 
	fgSizer19->Add( gaugeTxBuffer, 1, wxEXPAND, 5 );
	
	
	sbSizer5->Add( fgSizer19, 1, wxEXPAND, 5 );
	
	
	fgSizer10->Add( sbSizer5, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Measurement") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer122;
	fgSizer122 = new wxFlexGridSizer( 0, 3, 5, 5 );
	fgSizer122->AddGrowableCol( 1 );
	fgSizer122->AddGrowableCol( 2 );
	fgSizer122->SetFlexibleDirection( wxBOTH );
	fgSizer122->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkEnPwr = new wxCheckBox( sbSizer7->GetStaticBox(), wxID_ANY, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEnPwr->SetToolTip( wxT("Freezes FFT plot") );
	
	fgSizer122->Add( chkEnPwr, 0, wxTOP, 5 );
	
	m_staticText13 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("Ch 1"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	fgSizer122->Add( m_staticText13, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_staticText14 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("Ch 2"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	fgSizer122->Add( m_staticText14, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_staticText151 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("Center offset (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText151->Wrap( -1 );
	fgSizer122->Add( m_staticText151, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtCenterOffset1 = new wxTextCtrl( sbSizer7->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtCenterOffset1->HasFlag( wxTE_MULTILINE ) )
	{
	txtCenterOffset1->SetMaxLength( 8 );
	}
	#else
	txtCenterOffset1->SetMaxLength( 8 );
	#endif
	txtCenterOffset1->Enable( false );
	txtCenterOffset1->SetMinSize( wxSize( 32,-1 ) );
	
	fgSizer122->Add( txtCenterOffset1, 0, wxEXPAND, 5 );
	
	txtCenterOffset2 = new wxTextCtrl( sbSizer7->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtCenterOffset2->HasFlag( wxTE_MULTILINE ) )
	{
	txtCenterOffset2->SetMaxLength( 8 );
	}
	#else
	txtCenterOffset2->SetMaxLength( 8 );
	#endif
	txtCenterOffset2->Enable( false );
	txtCenterOffset2->SetMinSize( wxSize( 32,-1 ) );
	
	fgSizer122->Add( txtCenterOffset2, 0, wxEXPAND, 5 );
	
	m_staticText161 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("Bandwidth (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText161->Wrap( -1 );
	fgSizer122->Add( m_staticText161, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	txtBW1 = new wxTextCtrl( sbSizer7->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtBW1->HasFlag( wxTE_MULTILINE ) )
	{
	txtBW1->SetMaxLength( 8 );
	}
	#else
	txtBW1->SetMaxLength( 8 );
	#endif
	txtBW1->Enable( false );
	txtBW1->SetMinSize( wxSize( 32,-1 ) );
	
	fgSizer122->Add( txtBW1, 0, wxEXPAND, 5 );
	
	txtBW2 = new wxTextCtrl( sbSizer7->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtBW2->HasFlag( wxTE_MULTILINE ) )
	{
	txtBW2->SetMaxLength( 8 );
	}
	#else
	txtBW2->SetMaxLength( 8 );
	#endif
	txtBW2->Enable( false );
	txtBW2->SetMinSize( wxSize( 32,-1 ) );
	
	fgSizer122->Add( txtBW2, 0, wxEXPAND, 5 );
	
	m_staticText17 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("Power (dBFS):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	fgSizer122->Add( m_staticText17, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	lblPower1 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPower1->Wrap( -1 );
	fgSizer122->Add( lblPower1, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	lblPower2 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPower2->Wrap( -1 );
	fgSizer122->Add( lblPower2, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText20 = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("dBc"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	fgSizer122->Add( m_staticText20, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	lbldBc = new wxStaticText( sbSizer7->GetStaticBox(), wxID_ANY, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
	lbldBc->Wrap( -1 );
	fgSizer122->Add( lbldBc, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer7->Add( fgSizer122, 1, wxEXPAND|wxLEFT, 5 );
	
	
	fgSizer10->Add( sbSizer7, 1, wxEXPAND, 5 );
	
	
	fgSizer7->Add( fgSizer10, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer7 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	spinFFTsize->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( frFFTviewer::OnFFTsamplesCountChanged ), NULL, this );
	cmbStreamType->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnStreamChange ), NULL, this );
	cmbFmt->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnFmtChange ), NULL, this );
	chkEnTx->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( frFFTviewer::OnEnTx ), NULL, this );
	btnStartStop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frFFTviewer::OnbtnStartStop ), NULL, this );
	cmbWindowFunc->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnWindowFunctionChange ), NULL, this );
	cmbChannelVisibility->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnChannelVisibilityChange ), NULL, this );
	spinAvgCount->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( frFFTviewer::OnAvgChange ), NULL, this );
	spinAvgCount->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( frFFTviewer::OnAvgChangeEnter ), NULL, this );
	chkEnPwr->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( frFFTviewer::OnEnPwr ), NULL, this );
}

frFFTviewer::~frFFTviewer()
{
	// Disconnect Events
	spinFFTsize->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( frFFTviewer::OnFFTsamplesCountChanged ), NULL, this );
	cmbStreamType->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnStreamChange ), NULL, this );
	cmbFmt->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnFmtChange ), NULL, this );
	chkEnTx->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( frFFTviewer::OnEnTx ), NULL, this );
	btnStartStop->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frFFTviewer::OnbtnStartStop ), NULL, this );
	cmbWindowFunc->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnWindowFunctionChange ), NULL, this );
	cmbChannelVisibility->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( frFFTviewer::OnChannelVisibilityChange ), NULL, this );
	spinAvgCount->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( frFFTviewer::OnAvgChange ), NULL, this );
	spinAvgCount->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( frFFTviewer::OnAvgChangeEnter ), NULL, this );
	chkEnPwr->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( frFFTviewer::OnEnPwr ), NULL, this );
	
}
