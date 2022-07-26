#include "lms7002_pnlBIST_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlBIST_view::lms7002_pnlBIST_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer193;
    fgSizer193 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer193->SetFlexibleDirection( wxBOTH );
    fgSizer193->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer123;
    sbSizer123 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("BIST") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer194;
    fgSizer194 = new wxFlexGridSizer( 0, 2, 2, 10 );
    fgSizer194->SetFlexibleDirection( wxBOTH );
    fgSizer194->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkBENC = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BENC, wxT("Enable CGEN BIST"), wxDefaultPosition, wxDefaultSize, 0 );
    chkBENC->SetToolTip( wxT("enables CGEN BIST") );
    
    fgSizer194->Add( chkBENC, 0, flags, 0 );
    
    chkSDM_TSTO_CGEN = new wxCheckBox( sbSizer123->GetStaticBox(), ID_SDM_TSTO_CGEN, wxT("Enable SDM_TSTO_CGEN outputs"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSDM_TSTO_CGEN->SetValue(true); 
    fgSizer194->Add( chkSDM_TSTO_CGEN, 0, 0, 5 );
    
    chkBENR = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BENR, wxT("Enable SXR BIST"), wxDefaultPosition, wxDefaultSize, 0 );
    chkBENR->SetToolTip( wxT("enables receiver BIST") );
    
    fgSizer194->Add( chkBENR, 0, flags, 0 );
    
    chkSDM_TSTO_SXR = new wxCheckBox( sbSizer123->GetStaticBox(), ID_SDM_TSTO_SXR, wxT("Enable SDM_TSTO_SXR outputs"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSDM_TSTO_SXR->SetValue(true); 
    fgSizer194->Add( chkSDM_TSTO_SXR, 0, 0, 5 );
    
    chkBENT = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BENT, wxT("Enable SXT BIST"), wxDefaultPosition, wxDefaultSize, 0 );
    chkBENT->SetToolTip( wxT("enables transmitter  BIST") );
    
    fgSizer194->Add( chkBENT, 0, flags, 0 );
    
    chkSDM_TSTO_SXT = new wxCheckBox( sbSizer123->GetStaticBox(), ID_SDM_TSTO_SXT, wxT("Enable SDM_TSTO_SXT outputs"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSDM_TSTO_SXT->SetValue(true); 
    fgSizer194->Add( chkSDM_TSTO_SXT, 0, 0, 5 );
    
    chkBSTART = new wxCheckBox( sbSizer123->GetStaticBox(), ID_BSTART, wxT("Start SDM BIST"), wxDefaultPosition, wxDefaultSize, 0 );
    chkBSTART->SetToolTip( wxT("Starts delta sigma built in self test. Keep it at 1 one at least three clock cycles") );
    
    fgSizer194->Add( chkBSTART, 0, flags, 0 );
    
    
    fgSizer194->Add( 0, 0, 1, wxEXPAND, 5 );
    
    ID_STATICTEXT2 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST state"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer194->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    lblBSTATE = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSTATE, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblBSTATE->Wrap( 0 );
    fgSizer194->Add( lblBSTATE, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    ID_STATICTEXT1 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST signature (Transmitter)"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer194->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    lblBSIGT = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSIGT, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblBSIGT->Wrap( 0 );
    fgSizer194->Add( lblBSIGT, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    ID_STATICTEXT3 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST signature (Receiver)"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer194->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    lblBSIGR = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSIGR, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblBSIGR->Wrap( 0 );
    fgSizer194->Add( lblBSIGR, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    ID_STATICTEXT4 = new wxStaticText( sbSizer123->GetStaticBox(), wxID_ANY, wxT("BIST signature (CGEN)"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer194->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    lblBSIGC = new wxStaticText( sbSizer123->GetStaticBox(), ID_BSIGC, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblBSIGC->Wrap( 0 );
    fgSizer194->Add( lblBSIGC, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    btnReadSignature = new wxButton( sbSizer123->GetStaticBox(), ID_BTN_READ_SIGNATURE, wxT("Read BIST"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer194->Add( btnReadSignature, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizer123->Add( fgSizer194, 0, flags, 0 );
    
    
    fgSizer193->Add( sbSizer123, 0, flags, 0 );
    
    
    this->SetSizer( fgSizer193 );
    this->Layout();
    fgSizer193->Fit( this );
    
    // Connect Events
    chkBENC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::ParameterChangeHandler ), NULL, this );
    chkSDM_TSTO_CGEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::ParameterChangeHandler ), NULL, this );
    chkBENR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::ParameterChangeHandler ), NULL, this );
    chkSDM_TSTO_SXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::ParameterChangeHandler ), NULL, this );
    chkBENT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::ParameterChangeHandler ), NULL, this );
    chkSDM_TSTO_SXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::ParameterChangeHandler ), NULL, this );
    chkBSTART->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::ParameterChangeHandler ), NULL, this );
    btnReadSignature->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlBIST_view::onbtnReadSignature ), NULL, this );

    wndId2Enum[chkBENC] = LMS7param(BENC);
    wndId2Enum[chkBENR] = LMS7param(BENR);
    wndId2Enum[chkBENT] = LMS7param(BENT);
    wndId2Enum[chkBSTART] = LMS7param(BSTART);
    wndId2Enum[chkSDM_TSTO_CGEN] = LMS7param(EN_SDM_TSTO_CGEN);
    wndId2Enum[chkSDM_TSTO_SXR] = LMS7param(EN_SDM_TSTO_SXR);
    wndId2Enum[chkSDM_TSTO_SXT] = LMS7param(EN_SDM_TSTO_SXT);
    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlBIST_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
}

void lms7002_pnlBIST_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlBIST_view::onbtnReadSignature( wxCommandEvent& event )
{

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(BSIGC),&value);
    lblBSIGC->SetLabel(wxString::Format(_("0x%0.6X"), value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGR),&value);
    lblBSIGR->SetLabel(wxString::Format(_("0x%0.6X"), value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGT),&value);
    lblBSIGT->SetLabel(wxString::Format(_("0x%0.6X"), value));
    LMS_ReadParam(lmsControl,LMS7param(BSTATE),&value);
    lblBSTATE->SetLabel(wxString::Format(_("0x%X"), value));
}

void lms7002_pnlBIST_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
