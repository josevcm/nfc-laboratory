#include "lms7002_pnlTxTSP_view.h"
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7002_dlgGFIR_Coefficients.h"
#include "lms7suiteAppFrame.h"

using namespace lime;
using namespace LMS7002_WXGUI;

indexValueMap hbi_ovr_txtsp_IndexValuePairs;
indexValueMap tsgfcw_txtsp_IndexValuePairs;
indexValueMap cmix_gain_txtsp_IndexValuePairs;

lms7002_pnlTxTSP_view::lms7002_pnlTxTSP_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer215;
    fgSizer215 = new wxFlexGridSizer( 0, 4, 5, 5 );
    fgSizer215->SetFlexibleDirection( wxBOTH );
    fgSizer215->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer216;
    fgSizer216 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer216->SetFlexibleDirection( wxBOTH );
    fgSizer216->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkEN_TXTSP = new wxCheckBox( this, ID_EN_TXTSP, wxT("Enable TxTSP"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_TXTSP->SetToolTip( wxT("TxTSP modules enable") );
    chkEN_TXTSP->SetMinSize( wxSize( 135,-1 ) );
    
    fgSizer216->Add( chkEN_TXTSP, 1, wxALIGN_LEFT|wxALL, 5 );
    
    wxStaticBoxSizer* sbSizer103;
    sbSizer103 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bypass") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer145;
    fgSizer145 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer145->SetFlexibleDirection( wxBOTH );
    fgSizer145->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkDC_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_DC_BYP_TXTSP, wxT("DC corrector"), wxDefaultPosition, wxDefaultSize, 0 );
    chkDC_BYP_TXTSP->SetToolTip( wxT("DC corrector bypass") );
    
    fgSizer145->Add( chkDC_BYP_TXTSP, 0, flags, 0 );
    
    chkGC_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GC_BYP_TXTSP, wxT("Gain corrector"), wxDefaultPosition, wxDefaultSize, 0 );
    chkGC_BYP_TXTSP->SetToolTip( wxT("Gain corrector bypass") );
    
    fgSizer145->Add( chkGC_BYP_TXTSP, 0, flags, 0 );
    
    chkPH_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_PH_BYP_TXTSP, wxT("Phase corrector"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPH_BYP_TXTSP->SetToolTip( wxT("Phase corrector bypass") );
    
    fgSizer145->Add( chkPH_BYP_TXTSP, 0, flags, 0 );
    
    chkCMIX_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_CMIX_BYP_TXTSP, wxT("CMIX"), wxDefaultPosition, wxDefaultSize, 0 );
    chkCMIX_BYP_TXTSP->SetValue(true); 
    chkCMIX_BYP_TXTSP->SetToolTip( wxT("CMIX bypass") );
    
    fgSizer145->Add( chkCMIX_BYP_TXTSP, 0, flags, 0 );
    
    chkISINC_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_ISINC_BYP_TXTSP, wxT("ISINC "), wxDefaultPosition, wxDefaultSize, 0 );
    chkISINC_BYP_TXTSP->SetValue(true); 
    chkISINC_BYP_TXTSP->SetToolTip( wxT("ISINC bypass") );
    
    fgSizer145->Add( chkISINC_BYP_TXTSP, 0, flags, 0 );
    
    chkGFIR1_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GFIR1_BYP_TXTSP, wxT("GFIR1"), wxDefaultPosition, wxDefaultSize, 0 );
    chkGFIR1_BYP_TXTSP->SetToolTip( wxT("GFIR1 bypass") );
    
    fgSizer145->Add( chkGFIR1_BYP_TXTSP, 0, flags, 0 );
    
    chkGFIR2_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GFIR2_BYP_TXTSP, wxT("GFIR2"), wxDefaultPosition, wxDefaultSize, 0 );
    chkGFIR2_BYP_TXTSP->SetValue(true); 
    chkGFIR2_BYP_TXTSP->SetToolTip( wxT("GFIR2 bypass") );
    
    fgSizer145->Add( chkGFIR2_BYP_TXTSP, 0, flags, 0 );
    
    chkGFIR3_BYP_TXTSP = new wxCheckBox( sbSizer103->GetStaticBox(), ID_GFIR3_BYP_TXTSP, wxT("GFIR3"), wxDefaultPosition, wxDefaultSize, 0 );
    chkGFIR3_BYP_TXTSP->SetValue(true); 
    chkGFIR3_BYP_TXTSP->SetToolTip( wxT("GFIR3 bypass") );
    
    fgSizer145->Add( chkGFIR3_BYP_TXTSP, 0, flags, 0 );
    
    
    sbSizer103->Add( fgSizer145, 0, 0, 0 );
    
    
    fgSizer216->Add( sbSizer103, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer104;
    sbSizer104 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("BIST") ), wxVERTICAL );
    
    chkBSTART_TXTSP = new wxCheckBox( sbSizer104->GetStaticBox(), ID_BSTART_TXTSP, wxT("Start BIST"), wxDefaultPosition, wxDefaultSize, 0 );
    chkBSTART_TXTSP->SetValue(true); 
    chkBSTART_TXTSP->SetToolTip( wxT("Starts delta sigma built in self test. Keep it at 1 one at least three clock cycles") );
    
    sbSizer104->Add( chkBSTART_TXTSP, 0, flags, 0 );
    
    wxFlexGridSizer* fgSizer146;
    fgSizer146 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer146->SetFlexibleDirection( wxBOTH );
    fgSizer146->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT2 = new wxStaticText( sbSizer104->GetStaticBox(), wxID_ANY, wxT("State"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer146->Add( ID_STATICTEXT2, 0, flags, 0 );
    
    lblBSTATE_TXTSP = new wxStaticText( sbSizer104->GetStaticBox(), ID_BSTATE_TXTSP, wxT("?"), wxDefaultPosition, wxDefaultSize, 0 );
    lblBSTATE_TXTSP->Wrap( 0 );
    fgSizer146->Add( lblBSTATE_TXTSP, 0, flags, 0 );
    
    ID_STATICTEXT1 = new wxStaticText( sbSizer104->GetStaticBox(), wxID_ANY, wxT("Signature ch. I"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer146->Add( ID_STATICTEXT1, 0, flags, 0 );
    
    lblBSIGI_TXTSP = new wxStaticText( sbSizer104->GetStaticBox(), ID_BSIGI_TXTSP, wxT("?????????"), wxDefaultPosition, wxDefaultSize, 0 );
    lblBSIGI_TXTSP->Wrap( 0 );
    fgSizer146->Add( lblBSIGI_TXTSP, 0, flags, 0 );
    
    ID_STATICTEXT3 = new wxStaticText( sbSizer104->GetStaticBox(), wxID_ANY, wxT("Signature ch. Q"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer146->Add( ID_STATICTEXT3, 0, flags, 0 );
    
    lblBSIGQ_TXTSP = new wxStaticText( sbSizer104->GetStaticBox(), ID_BSIGQ_TXTSP, wxT("?????????"), wxDefaultPosition, wxDefaultSize, 0 );
    lblBSIGQ_TXTSP->Wrap( 0 );
    fgSizer146->Add( lblBSIGQ_TXTSP, 0, flags, 0 );
    
    
    sbSizer104->Add( fgSizer146, 0, wxEXPAND, 0 );
    
    btnReadBIST = new wxButton( sbSizer104->GetStaticBox(), wxID_ANY, wxT("Read BIST"), wxDefaultPosition, wxDefaultSize, 0 );
    btnReadBIST->SetDefault(); 
    sbSizer104->Add( btnReadBIST, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    fgSizer216->Add( sbSizer104, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgSizer215->Add( fgSizer216, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer142;
    fgSizer142 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer142->SetFlexibleDirection( wxBOTH );
    fgSizer142->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer106;
    sbSizer106 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("NCO") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer152;
    fgSizer152 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer152->SetFlexibleDirection( wxBOTH );
    fgSizer152->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    sizerNCOgrid = new wxFlexGridSizer( 0, 1, 0, 5 );
    sizerNCOgrid->SetFlexibleDirection( wxBOTH );
    sizerNCOgrid->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer221;
    fgSizer221 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer221->SetFlexibleDirection( wxBOTH );
    fgSizer221->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    
    fgSizer221->Add( 0, 0, 1, wxEXPAND, 5 );
    
    tableTitleCol1 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("FCW (MHz)"), wxDefaultPosition, wxDefaultSize, 0 );
    tableTitleCol1->Wrap( -1 );
    fgSizer221->Add( tableTitleCol1, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    tableTitleCol2 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Angle (Deg)"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    tableTitleCol2->Wrap( -1 );
    fgSizer221->Add( tableTitleCol2, 1, wxLEFT|wxALIGN_RIGHT, 5 );
    
    int NCOlineflags = wxLEFT|wxALIGN_CENTER_VERTICAL;
    rgrSEL0 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer221->Add( rgrSEL0, 0, NCOlineflags, 5);
    
    txtFCWPHO0 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO0->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO0->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO0->SetMaxLength( 10 );
    #endif
    fgSizer221->Add( txtFCWPHO0, 0, 0, 5 );
    
    txtAnglePHO0 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.00000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO0->Wrap( -1 );
    fgSizer221->Add( txtAnglePHO0, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer221, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2211;
    fgSizer2211 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2211->SetFlexibleDirection( wxBOTH );
    fgSizer2211->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL01 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2211->Add( rgrSEL01, 0, NCOlineflags, 5);
    
    txtFCWPHO01 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO01->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO01->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO01->SetMaxLength( 10 );
    #endif
    fgSizer2211->Add( txtFCWPHO01, 0, 0, 5 );
    
    txtAnglePHO01 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO01->Wrap( -1 );
    fgSizer2211->Add( txtAnglePHO01, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2211, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2212;
    fgSizer2212 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2212->SetFlexibleDirection( wxBOTH );
    fgSizer2212->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL02 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2212->Add( rgrSEL02, 0, NCOlineflags, 5);
    
    txtFCWPHO02 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO02->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO02->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO02->SetMaxLength( 10 );
    #endif
    fgSizer2212->Add( txtFCWPHO02, 0, 0, 5 );
    
    txtAnglePHO02 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO02->Wrap( -1 );
    fgSizer2212->Add( txtAnglePHO02, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2212, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2213;
    fgSizer2213 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2213->SetFlexibleDirection( wxBOTH );
    fgSizer2213->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL03 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2213->Add( rgrSEL03, 0, NCOlineflags, 5);
    
    txtFCWPHO03 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO03->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO03->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO03->SetMaxLength( 10 );
    #endif
    fgSizer2213->Add( txtFCWPHO03, 0, 0, 5 );
    
    txtAnglePHO03 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO03->Wrap( -1 );
    fgSizer2213->Add( txtAnglePHO03, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2213, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2214;
    fgSizer2214 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2214->SetFlexibleDirection( wxBOTH );
    fgSizer2214->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL04 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2214->Add( rgrSEL04, 0, NCOlineflags, 5);
    
    txtFCWPHO04 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO04->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO04->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO04->SetMaxLength( 10 );
    #endif
    fgSizer2214->Add( txtFCWPHO04, 0, 0, 5 );
    
    txtAnglePHO04 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO04->Wrap( -1 );
    fgSizer2214->Add( txtAnglePHO04, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2214, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2215;
    fgSizer2215 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2215->SetFlexibleDirection( wxBOTH );
    fgSizer2215->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL05 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2215->Add( rgrSEL05, 0, NCOlineflags, 5);
    
    txtFCWPHO05 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO05->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO05->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO05->SetMaxLength( 10 );
    #endif
    fgSizer2215->Add( txtFCWPHO05, 0, 0, 5 );
    
    txtAnglePHO05 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO05->Wrap( -1 );
    fgSizer2215->Add( txtAnglePHO05, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2215, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2216;
    fgSizer2216 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2216->SetFlexibleDirection( wxBOTH );
    fgSizer2216->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL06 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2216->Add( rgrSEL06, 0, NCOlineflags, 5);
    
    txtFCWPHO06 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO06->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO06->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO06->SetMaxLength( 10 );
    #endif
    fgSizer2216->Add( txtFCWPHO06, 0, 0, 5 );
    
    txtAnglePHO06 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO06->Wrap( -1 );
    fgSizer2216->Add( txtAnglePHO06, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2216, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2217;
    fgSizer2217 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2217->SetFlexibleDirection( wxBOTH );
    fgSizer2217->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL07 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2217->Add( rgrSEL07, 0, NCOlineflags, 5);
    
    txtFCWPHO07 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO07->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO07->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO07->SetMaxLength( 10 );
    #endif
    fgSizer2217->Add( txtFCWPHO07, 0, 0, 5 );
    
    txtAnglePHO07 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO07->Wrap( -1 );
    fgSizer2217->Add( txtAnglePHO07, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2217, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2218;
    fgSizer2218 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2218->SetFlexibleDirection( wxBOTH );
    fgSizer2218->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL08 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2218->Add( rgrSEL08, 0, NCOlineflags, 5);
    
    txtFCWPHO08 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO08->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO08->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO08->SetMaxLength( 10 );
    #endif
    fgSizer2218->Add( txtFCWPHO08, 0, 0, 5 );
    
    txtAnglePHO08 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO08->Wrap( -1 );
    fgSizer2218->Add( txtAnglePHO08, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2218, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer2219;
    fgSizer2219 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer2219->SetFlexibleDirection( wxBOTH );
    fgSizer2219->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL09 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer2219->Add( rgrSEL09, 0, NCOlineflags, 5);
    
    txtFCWPHO09 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO09->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO09->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO09->SetMaxLength( 10 );
    #endif
    fgSizer2219->Add( txtFCWPHO09, 0, 0, 5 );
    
    txtAnglePHO09 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO09->Wrap( -1 );
    fgSizer2219->Add( txtAnglePHO09, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer2219, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer22110;
    fgSizer22110 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer22110->SetFlexibleDirection( wxBOTH );
    fgSizer22110->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL10 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer22110->Add( rgrSEL10, 0, NCOlineflags, 5);
    
    txtFCWPHO10 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO10->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO10->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO10->SetMaxLength( 10 );
    #endif
    fgSizer22110->Add( txtFCWPHO10, 0, 0, 5 );
    
    txtAnglePHO10 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO10->Wrap( -1 );
    fgSizer22110->Add( txtAnglePHO10, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer22110, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer22111;
    fgSizer22111 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer22111->SetFlexibleDirection( wxBOTH );
    fgSizer22111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL11 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer22111->Add( rgrSEL11, 0, NCOlineflags, 5);
    
    txtFCWPHO11 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO11->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO11->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO11->SetMaxLength( 10 );
    #endif
    fgSizer22111->Add( txtFCWPHO11, 0, 0, 5 );
    
    txtAnglePHO11 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO11->Wrap( -1 );
    fgSizer22111->Add( txtAnglePHO11, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer22111, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer22112;
    fgSizer22112 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer22112->SetFlexibleDirection( wxBOTH );
    fgSizer22112->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL12 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer22112->Add( rgrSEL12, 0, NCOlineflags, 5);
    
    txtFCWPHO12 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO12->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO12->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO12->SetMaxLength( 10 );
    #endif
    fgSizer22112->Add( txtFCWPHO12, 0, 0, 5 );
    
    txtAnglePHO12 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO12->Wrap( -1 );
    fgSizer22112->Add( txtAnglePHO12, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer22112, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer22113;
    fgSizer22113 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer22113->SetFlexibleDirection( wxBOTH );
    fgSizer22113->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL13 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer22113->Add( rgrSEL13, 0, NCOlineflags, 5);
    
    txtFCWPHO13 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO13->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO13->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO13->SetMaxLength( 10 );
    #endif
    fgSizer22113->Add( txtFCWPHO13, 0, 0, 5 );
    
    txtAnglePHO13 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO13->Wrap( -1 );
    fgSizer22113->Add( txtAnglePHO13, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer22113, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer22114;
    fgSizer22114 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer22114->SetFlexibleDirection( wxBOTH );
    fgSizer22114->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL14 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer22114->Add( rgrSEL14, 0, NCOlineflags, 5);
    
    txtFCWPHO14 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO14->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO14->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO14->SetMaxLength( 10 );
    #endif
    fgSizer22114->Add( txtFCWPHO14, 0, 0, 5 );
    
    txtAnglePHO14 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO14->Wrap( -1 );
    fgSizer22114->Add( txtAnglePHO14, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer22114, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer22115;
    fgSizer22115 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer22115->SetFlexibleDirection( wxBOTH );
    fgSizer22115->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    rgrSEL15 = new wxRadioButton( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer22115->Add( rgrSEL15, 0, NCOlineflags, 5);
    
    txtFCWPHO15 = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHO15->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHO15->SetMaxLength( 10 );
    }
    #else
    txtFCWPHO15->SetMaxLength( 10 );
    #endif
    fgSizer22115->Add( txtFCWPHO15, 0, 0, 5 );
    
    txtAnglePHO15 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("0.0000"), wxDefaultPosition, wxDefaultSize, 0 );
    txtAnglePHO15->Wrap( -1 );
    fgSizer22115->Add( txtAnglePHO15, 0, NCOlineflags, 5);
    
    
    sizerNCOgrid->Add( fgSizer22115, 1, wxEXPAND, 5 );
    
    
    fgSizer152->Add( sizerNCOgrid, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxFlexGridSizer* fgSizer228;
    fgSizer228 = new wxFlexGridSizer( 0, 1, 5, 0 );
    fgSizer228->SetFlexibleDirection( wxBOTH );
    fgSizer228->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer154;
    fgSizer154 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer154->SetFlexibleDirection( wxBOTH );
    fgSizer154->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT21 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("RefClk (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT21->Wrap( -1 );
    fgSizer154->Add( ID_STATICTEXT21, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    lblRefClk = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("30.72"), wxDefaultPosition, wxDefaultSize, 0 );
    lblRefClk->Wrap( -1 );
    fgSizer154->Add( lblRefClk, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgSizer228->Add( fgSizer154, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    btnUploadNCO = new wxButton( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Upload NCO"), wxDefaultPosition, wxDefaultSize, 0 );
    btnUploadNCO->SetDefault(); 
    fgSizer228->Add( btnUploadNCO, 1, wxEXPAND, 5 );
    
    wxString rgrMODE_TXChoices[] = { wxT("FCW"), wxT("PHO") };
    int rgrMODE_TXNChoices = sizeof( rgrMODE_TXChoices ) / sizeof( wxString );
    rgrMODE_TX = new wxRadioBox( sbSizer106->GetStaticBox(), ID_MODE_TX, wxT("Mode"), wxDefaultPosition, wxDefaultSize, rgrMODE_TXNChoices, rgrMODE_TXChoices, 2, wxRA_SPECIFY_COLS );
    rgrMODE_TX->SetSelection( 0 );
    rgrMODE_TX->SetToolTip( wxT("Memory table mode") );
    
    fgSizer228->Add( rgrMODE_TX, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer229;
    fgSizer229 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer229->AddGrowableCol( 1 );
    fgSizer229->SetFlexibleDirection( wxBOTH );
    fgSizer229->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    lblFCWPHOmodeName = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("FCW (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    lblFCWPHOmodeName->Wrap( -1 );
    fgSizer229->Add( lblFCWPHOmodeName, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    txtFCWPHOmodeAdditional = new wxTextCtrl( sbSizer106->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtFCWPHOmodeAdditional->HasFlag( wxTE_MULTILINE ) )
    {
    txtFCWPHOmodeAdditional->SetMaxLength( 10 );
    }
    #else
    txtFCWPHOmodeAdditional->SetMaxLength( 10 );
    #endif
    fgSizer229->Add( txtFCWPHOmodeAdditional, 1, wxEXPAND, 5 );
    
    
    fgSizer228->Add( fgSizer229, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer153;
    fgSizer153 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer153->AddGrowableCol( 1 );
    fgSizer153->SetFlexibleDirection( wxBOTH );
    fgSizer153->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT25 = new wxStaticText( sbSizer106->GetStaticBox(), wxID_ANY, wxT("Bits to dither:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT25->Wrap( -1 );
    fgSizer153->Add( ID_STATICTEXT25, 1, wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbDTHBIT_TX = new wxComboBox( sbSizer106->GetStaticBox(), ID_DTHBIT_TX, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 ); 
    cmbDTHBIT_TX->SetToolTip( wxT("NCO bits to dither") );
    
    fgSizer153->Add( cmbDTHBIT_TX, 1, wxEXPAND, 5 );
    
    
    fgSizer228->Add( fgSizer153, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer105;
    sbSizer105 = new wxStaticBoxSizer( new wxStaticBox( sbSizer106->GetStaticBox(), wxID_ANY, wxT("TSG") ), wxVERTICAL );
    
    chkTSGSWAPIQ_TXTSP = new wxCheckBox( sbSizer105->GetStaticBox(), ID_TSGSWAPIQ_TXTSP, wxT("Swap I and Q\n signal sources from TSG"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTSGSWAPIQ_TXTSP->SetValue(true); 
    chkTSGSWAPIQ_TXTSP->SetToolTip( wxT("Swap signals at test signal generator's output") );
    
    sbSizer105->Add( chkTSGSWAPIQ_TXTSP, 0, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxFlexGridSizer* fgSizer148;
    fgSizer148 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer148->SetFlexibleDirection( wxBOTH );
    fgSizer148->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString rgrTSGFCW_TXTSPChoices[] = { wxT("TSP clk/8"), wxT("TSP clk/4") };
    int rgrTSGFCW_TXTSPNChoices = sizeof( rgrTSGFCW_TXTSPChoices ) / sizeof( wxString );
    rgrTSGFCW_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGFCW_TXTSP, wxT("TSGFCW"), wxDefaultPosition, wxDefaultSize, rgrTSGFCW_TXTSPNChoices, rgrTSGFCW_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrTSGFCW_TXTSP->SetSelection( 0 );
    rgrTSGFCW_TXTSP->SetToolTip( wxT("Set frequency of TSG's NCO") );
    
    fgSizer148->Add( rgrTSGFCW_TXTSP, 1, wxEXPAND, 5 );
    
    wxString rgrTSGMODE_TXTSPChoices[] = { wxT("NCO"), wxT("DC source") };
    int rgrTSGMODE_TXTSPNChoices = sizeof( rgrTSGMODE_TXTSPChoices ) / sizeof( wxString );
    rgrTSGMODE_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGMODE_TXTSP, wxT("TSGMODE"), wxDefaultPosition, wxDefaultSize, rgrTSGMODE_TXTSPNChoices, rgrTSGMODE_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrTSGMODE_TXTSP->SetSelection( 0 );
    rgrTSGMODE_TXTSP->SetToolTip( wxT("Test signal generator mode") );
    
    fgSizer148->Add( rgrTSGMODE_TXTSP, 1, wxEXPAND, 5 );
    
    wxString rgrINSEL_TXTSPChoices[] = { wxT("LML output"), wxT("Test signal") };
    int rgrINSEL_TXTSPNChoices = sizeof( rgrINSEL_TXTSPChoices ) / sizeof( wxString );
    rgrINSEL_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_INSEL_TXTSP, wxT("Input source"), wxDefaultPosition, wxDefaultSize, rgrINSEL_TXTSPNChoices, rgrINSEL_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrINSEL_TXTSP->SetSelection( 0 );
    rgrINSEL_TXTSP->SetToolTip( wxT("Input source of TxTSP") );
    
    fgSizer148->Add( rgrINSEL_TXTSP, 1, wxEXPAND, 5 );
    
    wxString rgrTSGFC_TXTSPChoices[] = { wxT("-6 dB"), wxT("Full scale") };
    int rgrTSGFC_TXTSPNChoices = sizeof( rgrTSGFC_TXTSPChoices ) / sizeof( wxString );
    rgrTSGFC_TXTSP = new wxRadioBox( sbSizer105->GetStaticBox(), ID_TSGFC_TXTSP, wxT("TSGFC"), wxDefaultPosition, wxDefaultSize, rgrTSGFC_TXTSPNChoices, rgrTSGFC_TXTSPChoices, 1, wxRA_SPECIFY_COLS );
    rgrTSGFC_TXTSP->SetSelection( 0 );
    rgrTSGFC_TXTSP->SetToolTip( wxT("TSG full scale control") );
    
    fgSizer148->Add( rgrTSGFC_TXTSP, 1, wxEXPAND, 5 );
    
    sbSizer105->Add( fgSizer148, 0, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgSizer228->Add( sbSizer105, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgSizer152->Add( fgSizer228, 1, wxEXPAND, 5 );
    
    
    sbSizer106->Add( fgSizer152, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgSizer142->Add( sbSizer106, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgSizer215->Add( fgSizer142, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxFlexGridSizer* fgSizer218;
    fgSizer218 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer218->SetFlexibleDirection( wxBOTH );
    fgSizer218->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer220;
    fgSizer220 = new wxFlexGridSizer( 0, 3, 0, 5 );
    fgSizer220->AddGrowableCol( 1 );
    fgSizer220->AddGrowableCol( 2 );
    fgSizer220->SetFlexibleDirection( wxBOTH );
    fgSizer220->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer149;
    fgSizer149 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer149->SetFlexibleDirection( wxBOTH );
    fgSizer149->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer150;
    fgSizer150 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer150->SetFlexibleDirection( wxBOTH );
    fgSizer150->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT23 = new wxStaticText( this, wxID_ANY, wxT("DC_REG:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT23->Wrap( -1 );
    fgSizer150->Add( ID_STATICTEXT23, 1, wxALIGN_CENTER_VERTICAL, 5 );
    
    txtDC_REG_TXTSP = new wxTextCtrl( this, ID_DC_REG_TXTSP, wxT("ffff"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
    fgSizer150->Add( txtDC_REG_TXTSP, 1, wxEXPAND, 5 );
    
    
    fgSizer149->Add( fgSizer150, 1, flags, 5 );
    
    btnLoadDCI = new wxButton( this, wxID_ANY, wxT("Load to DC I"), wxDefaultPosition, wxDefaultSize, 0 );
    btnLoadDCI->SetDefault(); 
    fgSizer149->Add( btnLoadDCI, 1, wxEXPAND, 5 );
    
    btnLoadDCQ = new wxButton( this, wxID_ANY, wxT("Load to DC Q"), wxDefaultPosition, wxDefaultSize, 0 );
    btnLoadDCQ->SetDefault(); 
    fgSizer149->Add( btnLoadDCQ, 1, wxEXPAND, 5 );
    
    
    fgSizer220->Add( fgSizer149, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer98;
    sbSizer98 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("CMIX") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer132;
    fgSizer132 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer132->AddGrowableCol( 0 );
    fgSizer132->SetFlexibleDirection( wxBOTH );
    fgSizer132->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    cmbCMIX_SC_TXTSP = new wxComboBox( sbSizer98->GetStaticBox(), ID_CMIX_SC_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbCMIX_SC_TXTSP->Append( wxT("Upconvert") );
    cmbCMIX_SC_TXTSP->Append( wxT("Downconvert") );
    fgSizer132->Add( cmbCMIX_SC_TXTSP, 1, wxEXPAND|wxALL, 5 );
    
    wxFlexGridSizer* fgSizer133;
    fgSizer133 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer133->AddGrowableCol( 1 );
    fgSizer133->SetFlexibleDirection( wxBOTH );
    fgSizer133->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT24 = new wxStaticText( sbSizer98->GetStaticBox(), wxID_ANY, wxT("Gain:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT24->Wrap( -1 );
    fgSizer133->Add( ID_STATICTEXT24, 1, wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbCMIX_GAIN_TXTSP = new wxComboBox( sbSizer98->GetStaticBox(), ID_CMIX_GAIN_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbCMIX_GAIN_TXTSP->Append( wxT("-6 dB") );
    cmbCMIX_GAIN_TXTSP->Append( wxT("-3 dB") );
    cmbCMIX_GAIN_TXTSP->Append( wxT("0 dB") );
    cmbCMIX_GAIN_TXTSP->Append( wxT("+3 dB") );
    cmbCMIX_GAIN_TXTSP->Append( wxT("+6 dB") );
    cmbCMIX_GAIN_TXTSP->SetToolTip( wxT("Gain of CMIX output") );
    
    fgSizer133->Add( cmbCMIX_GAIN_TXTSP, 1, wxEXPAND|wxLEFT, 5 );
    
    
    fgSizer132->Add( fgSizer133, 1, wxALL|wxEXPAND, 5 );
    
    
    sbSizer98->Add( fgSizer132, 1, wxEXPAND, 5 );
    
    
    fgSizer220->Add( sbSizer98, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer99;
    sbSizer99 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Interpolation") ), wxHORIZONTAL );
    
    ID_STATICTEXT7 = new wxStaticText( sbSizer99->GetStaticBox(), wxID_ANY, wxT("HBI ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT7->Wrap( -1 );
    sbSizer99->Add( ID_STATICTEXT7, 0, wxALL, 5 );
    
    cmbHBI_OVR_TXTSP = new wxComboBox( sbSizer99->GetStaticBox(), ID_HBI_OVR_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbHBI_OVR_TXTSP->SetToolTip( wxT("HBI interpolation ratio") );
    
    sbSizer99->Add( cmbHBI_OVR_TXTSP, 0, wxLEFT|wxEXPAND, 5 );
    
    fgSizer220->Add( sbSizer99, 1, 0, 5 );
    
    
    fgSizer218->Add( fgSizer220, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer138;
    sbSizer138 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("GFIR") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer259;
    fgSizer259 = new wxFlexGridSizer( 2, 0, 0, 0 );
    fgSizer259->SetFlexibleDirection( wxBOTH );
    fgSizer259->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer135;
    fgSizer135 = new wxFlexGridSizer( 0, 3, 0, 5 );
    fgSizer135->SetFlexibleDirection( wxBOTH );
    fgSizer135->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer100;
    sbSizer100 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("GFIR1") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer137;
    fgSizer137 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer137->AddGrowableCol( 1 );
    fgSizer137->SetFlexibleDirection( wxBOTH );
    fgSizer137->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT10 = new wxStaticText( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT10->Wrap( -1 );
    fgSizer137->Add( ID_STATICTEXT10, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGFIR1_L_TXTSP = new wxComboBox( sbSizer100->GetStaticBox(), ID_GFIR1_L_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbGFIR1_L_TXTSP->SetToolTip( wxT("Parameter l of GFIR1 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
    
    fgSizer137->Add( cmbGFIR1_L_TXTSP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT11 = new wxStaticText( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT11->Wrap( -1 );
    fgSizer137->Add( ID_STATICTEXT11, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGFIR1_N_TXTSP = new wxSpinCtrl( sbSizer100->GetStaticBox(), ID_GFIR1_N_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255 );
    //cmbGFIR1_N_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer137->Add( cmbGFIR1_N_TXTSP, 0, wxEXPAND, 5 );
    
    
    sbSizer100->Add( fgSizer137, 0, wxEXPAND, 0 );
    
    btnGFIR1Coef = new wxButton( sbSizer100->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
    btnGFIR1Coef->SetDefault(); 
    sbSizer100->Add( btnGFIR1Coef, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    fgSizer135->Add( sbSizer100, 1, wxALIGN_CENTER_HORIZONTAL|flags, 5 );
    
    wxStaticBoxSizer* sbSizer101;
    sbSizer101 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("GFIR2") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer139;
    fgSizer139 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer139->AddGrowableCol( 1 );
    fgSizer139->SetFlexibleDirection( wxBOTH );
    fgSizer139->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT12 = new wxStaticText( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT12->Wrap( -1 );
    fgSizer139->Add( ID_STATICTEXT12, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGFIR2_L_TXTSP = new wxComboBox( sbSizer101->GetStaticBox(), ID_GFIR2_L_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbGFIR2_L_TXTSP->SetToolTip( wxT("Parameter l of GFIR2 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
    
    fgSizer139->Add( cmbGFIR2_L_TXTSP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT13 = new wxStaticText( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT13->Wrap( -1 );
    fgSizer139->Add( ID_STATICTEXT13, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGFIR2_N_TXTSP = new wxSpinCtrl( sbSizer101->GetStaticBox(), ID_GFIR2_N_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255 );
    //cmbGFIR2_N_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer139->Add( cmbGFIR2_N_TXTSP, 0, wxEXPAND, 5 );
    
    
    sbSizer101->Add( fgSizer139, 0, wxEXPAND, 0 );
    
    btnGFIR2Coef = new wxButton( sbSizer101->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
    btnGFIR2Coef->SetDefault(); 
    sbSizer101->Add( btnGFIR2Coef, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    fgSizer135->Add( sbSizer101, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer102;
    sbSizer102 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("GFIR3") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer141;
    fgSizer141 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer141->AddGrowableCol( 1 );
    fgSizer141->SetFlexibleDirection( wxBOTH );
    fgSizer141->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT14 = new wxStaticText( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT14->Wrap( -1 );
    fgSizer141->Add( ID_STATICTEXT14, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGFIR3_L_TXTSP = new wxComboBox( sbSizer102->GetStaticBox(), ID_GFIR3_L_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbGFIR3_L_TXTSP->SetToolTip( wxT("Parameter l of GFIR3 (l = roundUp(CoeffN/5)-1). Unsigned integer") );
    
    fgSizer141->Add( cmbGFIR3_L_TXTSP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT15 = new wxStaticText( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Clk ratio:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT15->Wrap( -1 );
    fgSizer141->Add( ID_STATICTEXT15, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGFIR3_N_TXTSP = new wxSpinCtrl( sbSizer102->GetStaticBox(), ID_GFIR3_N_TXTSP, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255 );
    //cmbGFIR3_N_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer141->Add( cmbGFIR3_N_TXTSP, 0, wxEXPAND, 5 );
    
    
    sbSizer102->Add( fgSizer141, 0, wxEXPAND, 0 );
    
    btnGFIR3Coef = new wxButton( sbSizer102->GetStaticBox(), wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
    btnGFIR3Coef->SetDefault(); 
    sbSizer102->Add( btnGFIR3Coef, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    fgSizer135->Add( sbSizer102, 1, wxEXPAND, 5 );
    
    
    fgSizer259->Add( fgSizer135, 1, wxALL|wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer145;
    sbSizer145 = new wxStaticBoxSizer( new wxStaticBox( sbSizer138->GetStaticBox(), wxID_ANY, wxT("Configure GFIRs as LPF") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer1351;
    fgSizer1351 = new wxFlexGridSizer( 0, 3, 0, 5 );
    fgSizer1351->SetFlexibleDirection( wxBOTH );
    fgSizer1351->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    txtBW = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Bandwidth (MHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    txtBW->Wrap( -1 );
    fgSizer1351->Add( txtBW, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
    
    txtLPFBW = new wxTextCtrl( sbSizer145->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtLPFBW->HasFlag( wxTE_MULTILINE ) )
    {
    txtLPFBW->SetMaxLength( 8 );
    }
    #else
    txtLPFBW->SetMaxLength( 8 );
    #endif
    fgSizer1351->Add( txtLPFBW, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0 );
    
    btnSetLPF = new wxButton( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
    btnSetLPF->SetDefault(); 
    fgSizer1351->Add( btnSetLPF, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0 );
    
    txtRATE = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("Sample rate:"), wxDefaultPosition, wxDefaultSize, 0 );
    txtRATE->Wrap( -1 );
    fgSizer1351->Add( txtRATE, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 0 );
    
    txtRATEVAL = new wxStaticText( sbSizer145->GetStaticBox(), wxID_ANY, wxT("0 MHz"), wxDefaultPosition, wxDefaultSize, 0 );
    txtRATEVAL->Wrap( -1 );
    fgSizer1351->Add( txtRATEVAL, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 5 );
    
    
    sbSizer145->Add( fgSizer1351, 1, wxEXPAND, 5 );
    
    
    fgSizer259->Add( sbSizer145, 1, wxEXPAND, 5 );
    
    
    sbSizer138->Add( fgSizer259, 1, wxEXPAND, 5 );
    
    
    fgSizer218->Add( sbSizer138, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer219;
    fgSizer219 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer219->AddGrowableCol( 0 );
    fgSizer219->SetFlexibleDirection( wxBOTH );
    fgSizer219->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer96;
    sbSizer96 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Phase Corr") ), wxVERTICAL );
    
    cmbIQCORR_TXTSP = new NumericSlider( sbSizer96->GetStaticBox(), ID_IQCORR_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -2048, 2047, 0 );
    //cmbIQCORR_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    sbSizer96->Add( cmbIQCORR_TXTSP, 0, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer130;
    fgSizer130 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer130->SetFlexibleDirection( wxBOTH );
    fgSizer130->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT16 = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Alpha (Deg):"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT16->Wrap( -1 );
    fgSizer130->Add( ID_STATICTEXT16, 1, wxALIGN_CENTER_VERTICAL, 5 );
    
    txtPhaseAlpha = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
    txtPhaseAlpha->Wrap( -1 );
    fgSizer130->Add( txtPhaseAlpha, 1, wxALIGN_CENTER_VERTICAL, 5 );
    
    
    sbSizer96->Add( fgSizer130, 1, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5 );
    
    
    fgSizer219->Add( sbSizer96, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer97;
    sbSizer97 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Gain Corrector") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer131;
    fgSizer131 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer131->AddGrowableCol( 1 );
    fgSizer131->SetFlexibleDirection( wxBOTH );
    fgSizer131->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT5 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT5->Wrap( -1 );
    fgSizer131->Add( ID_STATICTEXT5, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGCORRI_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
    //cmbGCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer131->Add( cmbGCORRI_TXTSP, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT4 = new wxStaticText( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer131->Add( ID_STATICTEXT4, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbGCORRQ_TXTSP = new NumericSlider( sbSizer97->GetStaticBox(), ID_GCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 2047, 0 );
    //cmbGCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer131->Add( cmbGCORRQ_TXTSP, 0, wxEXPAND, 5 );
    
    ID_BUTTON10 = new wxButton( sbSizer97->GetStaticBox(), wxID_ANY, wxT("Calibrate"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_BUTTON10->SetDefault(); 
    ID_BUTTON10->Hide();
    
    fgSizer131->Add( ID_BUTTON10, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    sbSizer97->Add( fgSizer131, 0, wxEXPAND|wxALL, 5 );
    
    
    fgSizer219->Add( sbSizer97, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer95;
    sbSizer95 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("DC Corrector") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer129;
    fgSizer129 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer129->AddGrowableCol( 1 );
    fgSizer129->SetFlexibleDirection( wxBOTH );
    fgSizer129->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT8 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("I:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT8->Wrap( -1 );
    fgSizer129->Add( ID_STATICTEXT8, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbDCCORRI_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRI_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
    //cmbDCCORRI_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer129->Add( cmbDCCORRI_TXTSP, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT9 = new wxStaticText( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Q:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT9->Wrap( -1 );
    fgSizer129->Add( ID_STATICTEXT9, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbDCCORRQ_TXTSP = new NumericSlider( sbSizer95->GetStaticBox(), ID_DCCORRQ_TXTSP, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, -128, 127, 0 );
    //cmbDCCORRQ_TXTSP->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer129->Add( cmbDCCORRQ_TXTSP, 0, wxEXPAND, 5 );
    
    
    sbSizer95->Add( fgSizer129, 1, wxEXPAND|wxALL, 5 );
    
    
    fgSizer219->Add( sbSizer95, 1, wxEXPAND, 5 );
    
    
    fgSizer218->Add( fgSizer219, 1, wxEXPAND, 5 );
    
    
    fgSizer215->Add( fgSizer218, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( fgSizer215 );
    this->Layout();
    fgSizer215->Fit( this );
    
    // Connect Events
    chkEN_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkDC_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkGC_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkPH_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkCMIX_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkISINC_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkGFIR1_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkGFIR2_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkGFIR3_BYP_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkBSTART_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    btnReadBIST->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::onbtnReadBISTSignature ), NULL, this );
    rgrSEL0->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO0->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL01->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO01->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL02->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO02->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL03->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO03->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL04->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO04->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL05->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO05->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL06->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO06->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL07->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO07->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL08->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO08->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL09->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO09->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL10->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO10->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL11->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO11->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL12->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO12->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL13->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO13->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL14->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO14->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    rgrSEL15->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnNCOSelectionChange ), NULL, this );
    txtFCWPHO15->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    btnUploadNCO->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnbtnUploadNCOClick ), NULL, this );
    rgrMODE_TX->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    txtFCWPHOmodeAdditional->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( lms7002_pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
    txtFCWPHOmodeAdditional->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( lms7002_pnlTxTSP_view::PHOinputChanged ), NULL, this );
    cmbDTHBIT_TX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    chkTSGSWAPIQ_TXTSP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    rgrTSGFCW_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    rgrTSGMODE_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    rgrINSEL_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    rgrTSGFC_TXTSP->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    btnLoadDCI->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnbtnLoadDCIClick ), NULL, this );
    btnLoadDCQ->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnbtnLoadDCQClick ), NULL, this );
    cmbCMIX_SC_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbCMIX_GAIN_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbHBI_OVR_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbGFIR1_L_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbGFIR1_N_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    btnGFIR1Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::onbtnGFIR1Coef ), NULL, this );
    cmbGFIR2_L_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbGFIR2_N_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    btnGFIR2Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::onbtnGFIR2Coef ), NULL, this );
    cmbGFIR3_L_TXTSP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbGFIR3_N_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    btnGFIR3Coef->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::onbtnGFIR3Coef ), NULL, this );
    txtLPFBW->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( lms7002_pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel ), NULL, this );
    btnSetLPF->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlTxTSP_view::OnbtnSetLPFClick ), NULL, this );
    cmbIQCORR_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbGCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbGCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbDCCORRI_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );
    cmbDCCORRQ_TXTSP->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlTxTSP_view::ParameterChangeHandler ), NULL, this );

    wndId2Enum[rgrMODE_TX] = LMS7param(MODE_TX);
    wndId2Enum[chkBSTART_TXTSP] = LMS7param(BSTART_TXTSP);
    wndId2Enum[chkCMIX_BYP_TXTSP] = LMS7param(CMIX_BYP_TXTSP);
    wndId2Enum[cmbCMIX_GAIN_TXTSP] = LMS7param(CMIX_GAIN_TXTSP);
    wndId2Enum[cmbDCCORRI_TXTSP] = LMS7param(DCCORRI_TXTSP);
    wndId2Enum[cmbDCCORRQ_TXTSP] = LMS7param(DCCORRQ_TXTSP);
    wndId2Enum[chkDC_BYP_TXTSP] = LMS7param(DC_BYP_TXTSP);
    wndId2Enum[chkEN_TXTSP] = LMS7param(EN_TXTSP);
    wndId2Enum[cmbGCORRI_TXTSP] = LMS7param(GCORRI_TXTSP);
    wndId2Enum[cmbGCORRQ_TXTSP] = LMS7param(GCORRQ_TXTSP);
    wndId2Enum[chkGC_BYP_TXTSP] = LMS7param(GC_BYP_TXTSP);
    wndId2Enum[chkGFIR1_BYP_TXTSP] = LMS7param(GFIR1_BYP_TXTSP);
    wndId2Enum[cmbGFIR1_L_TXTSP] = LMS7param(GFIR1_L_TXTSP);
    wndId2Enum[cmbGFIR1_N_TXTSP] = LMS7param(GFIR1_N_TXTSP);
    wndId2Enum[chkGFIR2_BYP_TXTSP] = LMS7param(GFIR2_BYP_TXTSP);
    wndId2Enum[cmbGFIR2_L_TXTSP] = LMS7param(GFIR2_L_TXTSP);
    wndId2Enum[cmbGFIR2_N_TXTSP] = LMS7param(GFIR2_N_TXTSP);
    wndId2Enum[chkGFIR3_BYP_TXTSP] = LMS7param(GFIR3_BYP_TXTSP);
    wndId2Enum[cmbGFIR3_L_TXTSP] = LMS7param(GFIR3_L_TXTSP);
    wndId2Enum[cmbGFIR3_N_TXTSP] = LMS7param(GFIR3_N_TXTSP);
    wndId2Enum[cmbHBI_OVR_TXTSP] = LMS7param(HBI_OVR_TXTSP);
    wndId2Enum[cmbIQCORR_TXTSP] = LMS7param(IQCORR_TXTSP);
    wndId2Enum[chkISINC_BYP_TXTSP] = LMS7param(ISINC_BYP_TXTSP);
    wndId2Enum[chkPH_BYP_TXTSP] = LMS7param(PH_BYP_TXTSP);
    wndId2Enum[cmbCMIX_SC_TXTSP] = LMS7param(CMIX_SC_TXTSP);

    wndId2Enum[rgrTSGFCW_TXTSP] = LMS7param(TSGFCW_TXTSP);
    wndId2Enum[chkTSGSWAPIQ_TXTSP] = LMS7param(TSGSWAPIQ_TXTSP);
    wndId2Enum[rgrTSGMODE_TXTSP] = LMS7param(TSGMODE_TXTSP);
    wndId2Enum[rgrINSEL_TXTSP] = LMS7param(INSEL_TXTSP);
    wndId2Enum[rgrTSGFC_TXTSP] = LMS7param(TSGFC_TXTSP);
    wndId2Enum[cmbDTHBIT_TX] = LMS7param(DTHBIT_TX);

    wndId2Enum[rgrSEL0] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL01] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL02] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL03] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL04] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL05] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL06] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL07] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL08] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL09] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL10] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL11] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL12] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL13] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL14] = LMS7param(SEL_TX);
    wndId2Enum[rgrSEL15] = LMS7param(SEL_TX);

    lblNCOangles.push_back(txtAnglePHO0);
    lblNCOangles.push_back(txtAnglePHO01);
    lblNCOangles.push_back(txtAnglePHO02);
    lblNCOangles.push_back(txtAnglePHO03);
    lblNCOangles.push_back(txtAnglePHO04);
    lblNCOangles.push_back(txtAnglePHO05);
    lblNCOangles.push_back(txtAnglePHO06);
    lblNCOangles.push_back(txtAnglePHO07);
    lblNCOangles.push_back(txtAnglePHO08);
    lblNCOangles.push_back(txtAnglePHO09);
    lblNCOangles.push_back(txtAnglePHO10);
    lblNCOangles.push_back(txtAnglePHO11);
    lblNCOangles.push_back(txtAnglePHO12);
    lblNCOangles.push_back(txtAnglePHO13);
    lblNCOangles.push_back(txtAnglePHO14);
    lblNCOangles.push_back(txtAnglePHO15);

    rgrNCOselections.push_back(rgrSEL0);
    rgrNCOselections.push_back(rgrSEL01);
    rgrNCOselections.push_back(rgrSEL02);
    rgrNCOselections.push_back(rgrSEL03);
    rgrNCOselections.push_back(rgrSEL04);
    rgrNCOselections.push_back(rgrSEL05);
    rgrNCOselections.push_back(rgrSEL06);
    rgrNCOselections.push_back(rgrSEL07);
    rgrNCOselections.push_back(rgrSEL08);
    rgrNCOselections.push_back(rgrSEL09);
    rgrNCOselections.push_back(rgrSEL10);
    rgrNCOselections.push_back(rgrSEL11);
    rgrNCOselections.push_back(rgrSEL12);
    rgrNCOselections.push_back(rgrSEL13);
    rgrNCOselections.push_back(rgrSEL14);
    rgrNCOselections.push_back(rgrSEL15);

    txtNCOinputs.push_back(txtFCWPHO0);
    txtNCOinputs.push_back(txtFCWPHO01);
    txtNCOinputs.push_back(txtFCWPHO02);
    txtNCOinputs.push_back(txtFCWPHO03);
    txtNCOinputs.push_back(txtFCWPHO04);
    txtNCOinputs.push_back(txtFCWPHO05);
    txtNCOinputs.push_back(txtFCWPHO06);
    txtNCOinputs.push_back(txtFCWPHO07);
    txtNCOinputs.push_back(txtFCWPHO08);
    txtNCOinputs.push_back(txtFCWPHO09);
    txtNCOinputs.push_back(txtFCWPHO10);
    txtNCOinputs.push_back(txtFCWPHO11);
    txtNCOinputs.push_back(txtFCWPHO12);
    txtNCOinputs.push_back(txtFCWPHO13);
    txtNCOinputs.push_back(txtFCWPHO14);
    txtNCOinputs.push_back(txtFCWPHO15);

    wxArrayString temp;

    temp.clear();
    for(int i=0; i<8; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbGFIR1_L_TXTSP->Set(temp);
    cmbGFIR2_L_TXTSP->Set(temp);
    cmbGFIR3_L_TXTSP->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbDTHBIT_TX->Set(temp);

    temp.clear();
    temp.push_back("2^1");
    temp.push_back("2^2");
    temp.push_back("2^3");
    temp.push_back("2^4");
    temp.push_back("2^5");
    temp.push_back("Bypass");
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(0, 0));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(1, 1));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(2, 2));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(3, 3));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(4, 4));
    hbi_ovr_txtsp_IndexValuePairs.push_back(indexValuePair(5, 7));
    cmbHBI_OVR_TXTSP->Set(temp);

    tsgfcw_txtsp_IndexValuePairs.push_back(indexValuePair(0, 1));
    tsgfcw_txtsp_IndexValuePairs.push_back(indexValuePair(1, 2));

    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(0, 2));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(1, 2));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(2, 0));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(3, 0));
    cmix_gain_txtsp_IndexValuePairs.push_back(indexValuePair(4, 1));

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlTxTSP_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
         value = 1;
    wxArrayString temp;
    temp.clear();
    temp.push_back("-6 dB");
    temp.push_back(value ? "-3 dB":"-6 dB");
    temp.push_back("0 dB");
    temp.push_back(value ? "+3 dB":"+6 dB");
    temp.push_back("+6 dB");
    cmbCMIX_GAIN_TXTSP->Set(temp);
}

void lms7002_pnlTxTSP_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlTxTSP_view::ParameterChangeHandler( wxCommandEvent& event )
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
    long value = event.GetInt();
    if(event.GetEventObject() == cmbIQCORR_TXTSP)
    {
        float angle = atan(value / 2048.0) * 180 / 3.141596;
        txtPhaseAlpha->SetLabel(wxString::Format("%.3f", angle));
    }
    else if (event.GetEventObject() == rgrTSGFCW_TXTSP)
    {
        value = index2value(value, tsgfcw_txtsp_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbHBI_OVR_TXTSP)
    {
        value = index2value(value, hbi_ovr_txtsp_IndexValuePairs);
    }
    else if(event.GetEventObject() == cmbCMIX_GAIN_TXTSP)
    {
        LMS_WriteParam(lmsControl, LMS7_CMIX_GAIN_TXTSP_R3, value % 0x2);
        value = index2value(value, cmix_gain_txtsp_IndexValuePairs);
    }

    LMS_WriteParam(lmsControl,parameter,value);

    if(event.GetEventObject() == rgrMODE_TX)
        UpdateNCOinputs();
}

void lms7002_pnlTxTSP_view::OnNCOSelectionChange(wxCommandEvent& event)
{
    wxRadioButton* btn = reinterpret_cast<wxRadioButton*>(event.GetEventObject());
    int value = 0;
    for (size_t i = 0; i < rgrNCOselections.size(); ++i)
        if (btn == rgrNCOselections[i])
        {
            value = i;
            break;
        }
    LMS_WriteParam(lmsControl,LMS7param(SEL_TX),value);
}

void lms7002_pnlTxTSP_view::onbtnReadBISTSignature( wxCommandEvent& event )
{
    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(BSTATE_TXTSP),&value);
    lblBSTATE_TXTSP->SetLabel(wxString::Format("%i", value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGI_TXTSP),&value);
    lblBSIGI_TXTSP->SetLabel(wxString::Format("0x%0.6X", value));
    LMS_ReadParam(lmsControl,LMS7param(BSIGQ_TXTSP),&value);
    lblBSIGQ_TXTSP->SetLabel(wxString::Format("0x%0.6X", value));
}

void lms7002_pnlTxTSP_view::OnbtnLoadDCIClick( wxCommandEvent& event )
{
    long value = 0;
    txtDC_REG_TXTSP->GetValue().ToLong(&value, 16);
    LMS_WriteParam(lmsControl,LMS7param(DC_REG_TXTSP),value);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_TXTSP),0);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_TXTSP),1);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDI_TXTSP),0);
}

void lms7002_pnlTxTSP_view::OnbtnLoadDCQClick( wxCommandEvent& event )
{
    long value = 0;
    txtDC_REG_TXTSP->GetValue().ToLong(&value, 16);
    LMS_WriteParam(lmsControl,LMS7param(DC_REG_TXTSP),value);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_TXTSP),0);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_TXTSP),1);
    LMS_WriteParam(lmsControl,LMS7param(TSGDCLDQ_TXTSP),0);
}

void lms7002_pnlTxTSP_view::onbtnGFIR1Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this, wxID_ANY, wxT("GFIR1 Coefficients"));
    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR1, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Failed to read GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }

    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        status = LMS_SetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR1, &coefficients[0],coefficients.size());
        if (status != 0)
            wxMessageBox(_("Failed to set GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlTxTSP_view::onbtnGFIR2Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this, wxID_ANY, wxT("GFIR2 Coefficients"));
    std::vector<double> coefficients;
    const int maxCoefCount = 40;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR2, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Failed to read GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }
    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        status = LMS_SetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR2, &coefficients[0],coefficients.size());
        if (status != 0)
            wxMessageBox(_("Failed to set GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlTxTSP_view::onbtnGFIR3Coef( wxCommandEvent& event )
{
    lms7002_dlgGFIR_Coefficients *dlg = new lms7002_dlgGFIR_Coefficients(this, wxID_ANY, wxT("GFIR3 Coefficients"));
    std::vector<double> coefficients;
    const int maxCoefCount = 120;
    coefficients.resize(maxCoefCount, 0);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    int status =  LMS_GetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR3, &coefficients[0]);
    if (status < 0)
    {
        wxMessageBox(_("Failed to read GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
        dlg->Destroy();
        return;
    }
    dlg->SetCoefficients(coefficients);
    if (dlg->ShowModal() == wxID_OK)
    {
        coefficients = dlg->GetCoefficients();
        status = LMS_SetGFIRCoeff(lmsControl, LMS_CH_TX, ch, LMS_GFIR3, &coefficients[0],coefficients.size());
        if (status != 0)
            wxMessageBox(_("Failed to set GFIR coefficients"), _("ERROR"), wxICON_ERROR | wxOK);
    }
    dlg->Destroy();
    UpdateGUI();
}

void lms7002_pnlTxTSP_view::OnbtnUploadNCOClick( wxCommandEvent& event )
{
    LMS_WriteParam(lmsControl,LMS7param(MODE_TX),rgrMODE_TX->GetSelection());
    assert(txtNCOinputs.size() == 16);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    if (rgrMODE_TX->GetSelection() == 0)
    {
        float_type nco_freq[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_freq[i]);
            nco_freq[i] *= 1e6;
        }
        long value;
        txtFCWPHOmodeAdditional->GetValue().ToLong(&value);
        LMS_SetNCOFrequency(lmsControl,LMS_CH_TX,ch,nco_freq,value);
    }
    else //PHO mode
    {
        float_type nco_phase[16];
        for (int i = 0; i < 16; ++i)
        {
            txtNCOinputs[i]->GetValue().ToDouble(&nco_phase[i]);
        }
        double freq_MHz;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq_MHz);
        LMS_SetNCOPhase(lmsControl, LMS_CH_TX, ch-1, nco_phase, freq_MHz);
    }
    UpdateGUI();// API changes nco selection
}

void lms7002_pnlTxTSP_view::OnbtnSetLPFClick( wxCommandEvent& event )
{
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    double bw;
    txtLPFBW->GetValue().ToDouble(&bw);
    if (LMS_SetGFIRLPF(lmsControl, LMS_CH_TX, ch, true, bw*1e6)!=0)
        wxMessageBox(_("GFIR configuration failed"), _("Error"));
    UpdateGUI();// API changes nco selection
}

void lms7002_pnlTxTSP_view::UpdateNCOinputs()
{
    assert(txtNCOinputs.size() == 16);
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    if (rgrMODE_TX->GetSelection() == 0) //FCW mode
    {
        float_type freq[16] = { 0 };
        float_type pho=0;
        LMS_GetNCOFrequency(lmsControl, LMS_CH_TX, ch, freq, &pho);
        for (size_t i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.6f"), freq[i]/1e6));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%3.3f"), pho));
        lblFCWPHOmodeName->SetLabel(_("PHO (deg)"));
        tableTitleCol1->SetLabel(_("FCW (MHz)"));
        tableTitleCol2->SetLabel(_("PHO (deg)"));
    }
    else //PHO mode
    {
        float_type phase[16] = { 0 };
        float_type fcw = 0;
        LMS_GetNCOPhase(lmsControl, LMS_CH_TX, ch, phase, &fcw);
        for (size_t i = 0; i < txtNCOinputs.size(); ++i)
        {
            txtNCOinputs[i]->SetValue(wxString::Format(_("%.3f"), phase[i]));
        }
        txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.6f"), fcw/1e6));
        lblFCWPHOmodeName->SetLabel(_("FCW (MHz)"));
        tableTitleCol2->SetLabel(_("FCW (MHz)"));
        tableTitleCol1->SetLabel(_("PHO (deg)"));
    }
}

void lms7002_pnlTxTSP_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
    float_type freq;
    LMS_GetClockFreq(lmsControl,LMS_CLOCK_TXTSP,&freq);
    lblRefClk->SetLabel(wxString::Format(_("%3.3f"), freq/1e6));

    uint16_t hbi;
    LMS_ReadParam(lmsControl,LMS7param(HBI_OVR_TXTSP),&hbi);
    cmbHBI_OVR_TXTSP->SetSelection(value2index(hbi, hbi_ovr_txtsp_IndexValuePairs));

    int16_t value;
    LMS_ReadParam(lmsControl,LMS7param(TSGFCW_TXTSP),(uint16_t*)&value);

    rgrTSGFCW_TXTSP->SetSelection(value2index(value, tsgfcw_txtsp_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(IQCORR_TXTSP),(uint16_t*)&value);
    int bitsToShift = (15 - LMS7param(IQCORR_TXTSP).msb - LMS7param(IQCORR_TXTSP).lsb);
    value = value << bitsToShift;
    value = value >> bitsToShift;
    cmbIQCORR_TXTSP->SetValue(value);

    LMS_ReadParam(lmsControl,LMS7param(SEL_TX),(uint16_t*)&value);
    assert(rgrNCOselections.size() == 16);
    rgrNCOselections[value & 0xF]->SetValue(true);
    UpdateNCOinputs();

    LMS_ReadParam(lmsControl,LMS7param(DCCORRI_TXTSP),(uint16_t*)&value);
    int8_t dccorr = value;
    cmbDCCORRI_TXTSP->SetValue(dccorr);
    LMS_ReadParam(lmsControl,LMS7param(DCCORRQ_TXTSP),(uint16_t*)&value);
    dccorr = value;
    cmbDCCORRQ_TXTSP->SetValue(dccorr);

    uint16_t g_cmix;
    LMS_ReadParam(lmsControl,LMS7param(CMIX_GAIN_TXTSP),&g_cmix);
    value = value2index(g_cmix, cmix_gain_txtsp_IndexValuePairs);
    LMS_ReadParam(lmsControl,LMS7param(CMIX_GAIN_TXTSP_R3),&g_cmix);
    if (g_cmix)
        value |= 1;
    else
        value &= ~1;
    cmbCMIX_GAIN_TXTSP->SetSelection(value);
    
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    
    double sr = 0;
    LMS_GetSampleRate(lmsControl, LMS_CH_TX, ch , &sr, nullptr);
    txtRATEVAL->SetLabel(wxString::Format("%3.3f MHz", sr/1e6));
    //check if B channel is enabled
    LMS_ReadParam(lmsControl,LMS7param(MAC),(uint16_t*)&value);
    if (value >= 2)
    {
        LMS_ReadParam(lmsControl,LMS7param(MIMO_SISO),(uint16_t*)&value);
        if (value != 0)
            wxMessageBox(_("MIMO channel B is disabled"), _("Warning"));
    }
}

void lms7002_pnlTxTSP_view::PHOinputChanged(wxCommandEvent& event)
{
    uint16_t ch;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&ch);
    ch = (ch == 2) ? 1 : 0;
    ch += 2*LMS7SuiteAppFrame::m_lmsSelection;
    // Write values for NCO phase or frequency each time they change - to ease the tuning of these values in measurements
    if (rgrMODE_TX->GetSelection() == 0)
    {
        double angle;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
        LMS_SetNCOFrequency(lmsControl,LMS_CH_TX,ch,nullptr,angle);
    }
    else //PHO mode
    {
        double freq;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq);
        LMS_SetNCOPhase(lmsControl, LMS_CH_TX, ch, nullptr, freq*1e6);
    }

    assert(lblNCOangles.size() == 16);
    if (rgrMODE_TX->GetSelection() == 1)
    {
        double freq;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&freq);
        for (int i = 0; i < 16; ++i){
            lblNCOangles[i]->SetLabel(wxString::Format("%3.3f", freq));
        }
    }
    else
    {
        double angle;
        txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
        for (int i = 0; i < 16; ++i){
            lblNCOangles[i]->SetLabel(wxString::Format("%3.3f", angle));
        }
    }
}

void lms7002_pnlTxTSP_view::txtFCWPHOmodeAdditional_OnMouseWheel(wxMouseEvent& event){
    double angle = 0;
    txtFCWPHOmodeAdditional->GetValue().ToDouble(&angle);
    int change = event.GetWheelRotation()/120;
    angle += change*0.1;
    txtFCWPHOmodeAdditional->SetValue(wxString::Format(_("%.1f"), angle > 0 ? angle : 0));
}
