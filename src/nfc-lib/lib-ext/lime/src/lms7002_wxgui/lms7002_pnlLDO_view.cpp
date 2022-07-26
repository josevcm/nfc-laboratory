#include "lms7002_pnlLDO_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlLDO_view::lms7002_pnlLDO_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = 0;
    wxFlexGridSizer* fgSizer68;
    fgSizer68 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer68->SetFlexibleDirection( wxBOTH );
    fgSizer68->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_NOTEBOOK1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    ID_PANEL3 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxFlexGridSizer* fgSizer69;
    fgSizer69 = new wxFlexGridSizer( 3, 1, 0, 0 );
    fgSizer69->SetFlexibleDirection( wxBOTH );
    fgSizer69->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer46;
    sbSizer46 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL3, wxID_ANY, wxT("Power controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer70;
    fgSizer70 = new wxFlexGridSizer( 0, 7, 2, 5 );
    fgSizer70->SetFlexibleDirection( wxBOTH );
    fgSizer70->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkEN_G_LDOP = new wxCheckBox( sbSizer46->GetStaticBox(), ID_EN_G_LDOP, wxT("Enable LDO digital module"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_G_LDOP->SetToolTip( wxT("Enable control for all the LDO power downs") );
    
    fgSizer70->Add( chkEN_G_LDOP, 0, flags, 0 );
    
    chkEN_G_LDO = new wxCheckBox( sbSizer46->GetStaticBox(), ID_EN_G_LDO, wxT("Enable LDO analog module"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_G_LDO->SetToolTip( wxT("Enable control for all the LDO power downs") );
    
    fgSizer70->Add( chkEN_G_LDO, 0, flags, 0 );
    
    
    fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    fgSizer70->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer47;
    sbSizer47 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("CLKGEN") ), wxVERTICAL );
    
    chkEN_LDO_DIVGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_DIVGN, wxT("EN_LDO_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_DIVGN->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer47->Add( chkEN_LDO_DIVGN, 0, flags, 0 );
    
    chkEN_LDO_DIGGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_DIGGN, wxT("EN_LDO_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_DIGGN->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer47->Add( chkEN_LDO_DIGGN, 0, flags, 0 );
    
    chkEN_LDO_CPGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_CPGN, wxT("EN_LDO_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_CPGN->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer47->Add( chkEN_LDO_CPGN, 0, flags, 0 );
    
    chkEN_LDO_VCOGN = new wxCheckBox( sbSizer47->GetStaticBox(), ID_EN_LDO_VCOGN, wxT("EN_LDO_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_VCOGN->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer47->Add( chkEN_LDO_VCOGN, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer47, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer48;
    sbSizer48 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("RX RF") ), wxVERTICAL );
    
    chkEN_LDO_MXRFE = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_MXRFE, wxT("EN_LDO_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_MXRFE->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer48->Add( chkEN_LDO_MXRFE, 0, flags, 0 );
    
    chkEN_LDO_LNA14 = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_LNA14, wxT("EN_LDO_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_LNA14->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer48->Add( chkEN_LDO_LNA14, 0, flags, 0 );
    
    chkEN_LDO_LNA12 = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_LNA12, wxT("EN_LDO_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_LNA12->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer48->Add( chkEN_LDO_LNA12, 0, flags, 0 );
    
    chkEN_LDO_TIA14 = new wxCheckBox( sbSizer48->GetStaticBox(), ID_EN_LDO_TIA14, wxT("EN_LDO_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_TIA14->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer48->Add( chkEN_LDO_TIA14, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer48, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer49;
    sbSizer49 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("SX TX") ), wxVERTICAL );
    
    chkEN_LDO_DIVSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_DIVSXT, wxT("EN_LDO_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_DIVSXT->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer49->Add( chkEN_LDO_DIVSXT, 0, flags, 0 );
    
    chkEN_LDO_DIGSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_DIGSXT, wxT("EN_LDO_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_DIGSXT->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer49->Add( chkEN_LDO_DIGSXT, 0, flags, 0 );
    
    chkEN_LDO_CPSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_CPSXT, wxT("EN_LDO_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_CPSXT->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer49->Add( chkEN_LDO_CPSXT, 0, flags, 0 );
    
    chkEN_LDO_VCOSXT = new wxCheckBox( sbSizer49->GetStaticBox(), ID_EN_LDO_VCOSXT, wxT("EN_LDO_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_VCOSXT->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer49->Add( chkEN_LDO_VCOSXT, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer49, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer50;
    sbSizer50 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("SX RX") ), wxVERTICAL );
    
    chkEN_LDO_DIVSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_DIVSXR, wxT("EN_LDO_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_DIVSXR->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer50->Add( chkEN_LDO_DIVSXR, 0, flags, 0 );
    
    chkEN_LDO_DIGSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_DIGSXR, wxT("EN_LDO_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_DIGSXR->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer50->Add( chkEN_LDO_DIGSXR, 0, flags, 0 );
    
    chkEN_LDO_CPSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_CPSXR, wxT("EN_LDO_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_CPSXR->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer50->Add( chkEN_LDO_CPSXR, 0, flags, 0 );
    
    chkEN_LDO_VCOSXR = new wxCheckBox( sbSizer50->GetStaticBox(), ID_EN_LDO_VCOSXR, wxT("EN_LDO_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_VCOSXR->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer50->Add( chkEN_LDO_VCOSXR, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer50, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer51;
    sbSizer51 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Digital Blocks") ), wxVERTICAL );
    
    chkPD_LDO_DIGIp1 = new wxCheckBox( sbSizer51->GetStaticBox(), ID_PD_LDO_DIGIp1, wxT("PD_LDO_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_LDO_DIGIp1->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer51->Add( chkPD_LDO_DIGIp1, 0, flags, 0 );
    
    chkPD_LDO_DIGIp2 = new wxCheckBox( sbSizer51->GetStaticBox(), ID_PD_LDO_DIGIp2, wxT("PD_LDO_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_LDO_DIGIp2->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer51->Add( chkPD_LDO_DIGIp2, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer51, 1, wxALIGN_TOP, 5 );
    
    wxStaticBoxSizer* sbSizer52;
    sbSizer52 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("XBUF") ), wxVERTICAL );
    
    chkEN_LDO_TXBUF = new wxCheckBox( sbSizer52->GetStaticBox(), ID_EN_LDO_TXBUF, wxT("EN_LDO_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_TXBUF->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer52->Add( chkEN_LDO_TXBUF, 0, flags, 0 );
    
    chkEN_LDO_RXBUF = new wxCheckBox( sbSizer52->GetStaticBox(), ID_EN_LDO_RXBUF, wxT("EN_LDO_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_RXBUF->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer52->Add( chkEN_LDO_RXBUF, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer52, 1, wxALIGN_TOP, 5 );
    
    wxStaticBoxSizer* sbSizer58;
    sbSizer58 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Tx RF") ), wxVERTICAL );
    
    chkEN_LDO_TPAD = new wxCheckBox( sbSizer58->GetStaticBox(), ID_EN_LDO_TPAD, wxT("EN_LDO_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_TPAD->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer58->Add( chkEN_LDO_TPAD, 0, flags, 0 );
    
    chkEN_LDO_TLOB = new wxCheckBox( sbSizer58->GetStaticBox(), ID_EN_LDO_TLOB, wxT("EN_LDO_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_TLOB->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer58->Add( chkEN_LDO_TLOB, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer58, 1, wxALIGN_TOP, 5 );
    
    wxStaticBoxSizer* sbSizer54;
    sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("SPI Buffer to analog blocks") ), wxVERTICAL );
    
    chkPD_LDO_SPIBUF = new wxCheckBox( sbSizer54->GetStaticBox(), ID_PD_LDO_SPIBUF, wxT("PD_LDO_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkPD_LDO_SPIBUF->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer54->Add( chkPD_LDO_SPIBUF, 0, wxEXPAND, 0 );
    
    wxFlexGridSizer* fgSizer251;
    fgSizer251 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer251->SetFlexibleDirection( wxBOTH );
    fgSizer251->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText349 = new wxStaticText( sbSizer54->GetStaticBox(), wxID_ANY, wxT("ISINK_SPI_BUFF"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText349->Wrap( -1 );
    fgSizer251->Add( m_staticText349, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbISINK_SPIBUFF = new wxComboBox( sbSizer54->GetStaticBox(), ID_RDIV_TXBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbISINK_SPIBUFF->Append( wxT("Off") );
    cmbISINK_SPIBUFF->Append( wxT("10 kOhm") );
    cmbISINK_SPIBUFF->Append( wxT("2.5 kOhm") );
    cmbISINK_SPIBUFF->Append( wxT("2 kOhm") );
    cmbISINK_SPIBUFF->Append( wxT("625 Ohm") );
    cmbISINK_SPIBUFF->Append( wxT("588 Ohm") );
    cmbISINK_SPIBUFF->Append( wxT("500 Ohm") );
    cmbISINK_SPIBUFF->Append( wxT("476 Ohm") );
    cmbISINK_SPIBUFF->Append( wxT("10 kOhm") );
    fgSizer251->Add( cmbISINK_SPIBUFF, 0, wxEXPAND, 0 );
    
    
    sbSizer54->Add( fgSizer251, 1, wxEXPAND, 5 );
    
    
    fgSizer70->Add( sbSizer54, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    wxStaticBoxSizer* sbSizer53;
    sbSizer53 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("AFE") ), wxVERTICAL );
    
    chkEN_LDO_AFE = new wxCheckBox( sbSizer53->GetStaticBox(), ID_EN_LDO_AFE, wxT("EN_LDO_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_AFE->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer53->Add( chkEN_LDO_AFE, 0, flags, 0 );
    
    
    fgSizer70->Add( sbSizer53, 1, wxEXPAND, 0 );
    
    wxStaticBoxSizer* sbSizer55;
    sbSizer55 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Tx BB") ), wxVERTICAL );
    
    chkEN_LDO_TBB = new wxCheckBox( sbSizer55->GetStaticBox(), ID_EN_LDO_TBB, wxT("EN_LDO_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_TBB->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer55->Add( chkEN_LDO_TBB, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 0 );
    
    
    fgSizer70->Add( sbSizer55, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer56;
    sbSizer56 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Rx RBB") ), wxVERTICAL );
    
    chkEN_LDO_RBB = new wxCheckBox( sbSizer56->GetStaticBox(), ID_EN_LDO_RBB, wxT("EN_LDO_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_RBB->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer56->Add( chkEN_LDO_RBB, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 0 );
    
    
    fgSizer70->Add( sbSizer56, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer57;
    sbSizer57 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Rx RF+ Rx RBB") ), wxVERTICAL );
    
    chkEN_LDO_TIA12 = new wxCheckBox( sbSizer57->GetStaticBox(), ID_EN_LDO_TIA12, wxT("EN_LDO_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_TIA12->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer57->Add( chkEN_LDO_TIA12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 0 );
    
    
    fgSizer70->Add( sbSizer57, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer59;
    sbSizer59 = new wxStaticBoxSizer( new wxStaticBox( sbSizer46->GetStaticBox(), wxID_ANY, wxT("Misc") ), wxVERTICAL );
    
    chkEN_LDO_DIG = new wxCheckBox( sbSizer59->GetStaticBox(), ID_EN_LDO_DIG, wxT("EN_LDO_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LDO_DIG->SetToolTip( wxT("Enables the LDO") );
    
    sbSizer59->Add( chkEN_LDO_DIG, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP, 0 );
    
    
    fgSizer70->Add( sbSizer59, 1, wxEXPAND, 5 );
    
    
    sbSizer46->Add( fgSizer70, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0 );
    
    
    fgSizer69->Add( sbSizer46, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxFlexGridSizer* fgSizer255;
    fgSizer255 = new wxFlexGridSizer( 1, 3, 0, 0 );
    fgSizer255->SetFlexibleDirection( wxBOTH );
    fgSizer255->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer61;
    sbSizer61 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL3, wxID_ANY, wxT("Short noise filter resistor") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer72;
    fgSizer72 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer72->SetFlexibleDirection( wxBOTH );
    fgSizer72->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkSPDUP_LDO_TBB = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TBB, wxT("SPDUP_LDO_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_TBB->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_TBB, 0, flags, 0 );
    
    chkSPDUP_LDO_TIA12 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TIA12, wxT("SPDUP_LDO_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_TIA12->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_TIA12, 0, flags, 0 );
    
    chkSPDUP_LDO_TIA14 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TIA14, wxT("SPDUP_LDO_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_TIA14->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_TIA14, 0, flags, 0 );
    
    chkSPDUP_LDO_TLOB = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TLOB, wxT("SPDUP_LDO_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_TLOB->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_TLOB, 0, flags, 0 );
    
    chkSPDUP_LDO_TPAD = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TPAD, wxT("SPDUP_LDO_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_TPAD->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_TPAD, 0, flags, 0 );
    
    chkSPDUP_LDO_TXBUF = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_TXBUF, wxT("SPDUP_LDO_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_TXBUF->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_TXBUF, 0, flags, 0 );
    
    chkSPDUP_LDO_VCOGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_VCOGN, wxT("SPDUP_LDO_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_VCOGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_VCOGN, 0, flags, 0 );
    
    chkSPDUP_LDO_DIVSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIVSXR, wxT("SPDUP_LDO_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIVSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIVSXR, 0, flags, 0 );
    
    chkSPDUP_LDO_DIVSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIVSXT, wxT("SPDUP_LDO_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIVSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIVSXT, 0, flags, 0 );
    
    chkSPDUP_LDO_AFE = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_AFE, wxT("SPDUP_LDO_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_AFE->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_AFE, 0, flags, 0 );
    
    chkSPDUP_LDO_CPGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_CPGN, wxT("SPDUP_LDO_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_CPGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_CPGN, 0, flags, 0 );
    
    chkSPDUP_LDO_VCOSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_VCOSXR, wxT("SPDUP_LDO_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_VCOSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_VCOSXR, 0, flags, 0 );
    
    chkSPDUP_LDO_VCOSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_VCOSXT, wxT("SPDUP_LDO_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_VCOSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_VCOSXT, 0, flags, 0 );
    
    chkSPDUP_LDO_DIG = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIG, wxT("SPDUP_LDO_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIG->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIG, 0, flags, 0 );
    
    chkSPDUP_LDO_DIGGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGGN, wxT("SPDUP_LDO_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIGGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIGGN, 0, flags, 0 );
    
    chkSPDUP_LDO_DIGSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGSXR, wxT("SPDUP_LDO_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIGSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIGSXR, 0, flags, 0 );
    
    chkSPDUP_LDO_DIGSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGSXT, wxT("SPDUP_LDO_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIGSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIGSXT, 0, flags, 0 );
    
    chkSPDUP_LDO_DIVGN = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIVGN, wxT("SPDUP_LDO_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIVGN->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIVGN, 0, flags, 0 );
    
    chkSPDUP_LDO_CPSXR = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_CPSXR, wxT("SPDUP_LDO_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_CPSXR->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_CPSXR, 0, flags, 0 );
    
    chkSPDUP_LDO_CPSXT = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_CPSXT, wxT("SPDUP_LDO_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_CPSXT->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_CPSXT, 0, flags, 0 );
    
    chkSPDUP_LDO_LNA12 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_LNA12, wxT("SPDUP_LDO_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_LNA12->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_LNA12, 0, flags, 0 );
    
    chkSPDUP_LDO_LNA14 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_LNA14, wxT("SPDUP_LDO_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_LNA14->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_LNA14, 0, flags, 0 );
    
    chkSPDUP_LDO_MXRFE = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_MXRFE, wxT("SPDUP_LDO_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_MXRFE->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_MXRFE, 0, flags, 0 );
    
    chkSPDUP_LDO_RBB = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_RBB, wxT("SPDUP_LDO_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_RBB->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_RBB, 0, flags, 0 );
    
    chkSPDUP_LDO_RXBUF = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_RXBUF, wxT("SPDUP_LDO_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_RXBUF->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_RXBUF, 0, flags, 0 );
    
    chkSPDUP_LDO_SPIBUF = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_SPIBUF, wxT("SPDUP_LDO_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_SPIBUF->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_SPIBUF, 0, flags, 0 );
    
    chkSPDUP_LDO_DIGIp2 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGIp2, wxT("SPDUP_LDO_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIGIp2->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIGIp2, 0, flags, 0 );
    
    chkSPDUP_LDO_DIGIp1 = new wxCheckBox( sbSizer61->GetStaticBox(), ID_SPDUP_LDO_DIGIp1, wxT("SPDUP_LDO_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSPDUP_LDO_DIGIp1->SetToolTip( wxT("Short the noise filter resistor to speed up the settling time") );
    
    fgSizer72->Add( chkSPDUP_LDO_DIGIp1, 0, flags, 0 );
    
    
    sbSizer61->Add( fgSizer72, 0, flags, 0 );
    
    
    fgSizer255->Add( sbSizer61, 1, wxLEFT|wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer62;
    sbSizer62 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL3, wxID_ANY, wxT("Bias") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer73;
    fgSizer73 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer73->SetFlexibleDirection( wxBOTH );
    fgSizer73->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkEN_LOADIMP_LDO_TBB = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TBB, wxT("EN_LOADIMP_LDO_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_TBB->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_TBB, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_TIA12 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TIA12, wxT("EN_LOADIMP_LDO_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_TIA12->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_TIA12, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_TIA14 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TIA14, wxT("EN_LOADIMP_LDO_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_TIA14->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_TIA14, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_TLOB = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TLOB, wxT("EN_LOADIMP_LDO_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_TLOB->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_TLOB, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_TPAD = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TPAD, wxT("EN_LOADIMP_LDO_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_TPAD->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_TPAD, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_TXBUF = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_TXBUF, wxT("EN_LOADIMP_LDO_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_TXBUF->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_TXBUF, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_VCOGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_VCOGN, wxT("EN_LOADIMP_LDO_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_VCOGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_VCOGN, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_VCOSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_VCOSXR, wxT("EN_LOADIMP_LDO_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_VCOSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_VCOSXR, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_VCOSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_VCOSXT, wxT("EN_LOADIMP_LDO_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_VCOSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_VCOSXT, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_AFE = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_AFE, wxT("EN_LOADIMP_LDO_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_AFE->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_AFE, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_CPGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_CPGN, wxT("EN_LOADIMP_LDO_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_CPGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_CPGN, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIVSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIVSXR, wxT("EN_LOADIMP_LDO_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIVSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIVSXR, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIVSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIVSXT, wxT("EN_LOADIMP_LDO_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIVSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIVSXT, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIG = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIG, wxT("EN_LOADIMP_LDO_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIG->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIG, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIGGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGGN, wxT("EN_LOADIMP_LDO_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIGGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIGGN, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIGSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGSXR, wxT("EN_LOADIMP_LDO_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIGSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIGSXR, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIGSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGSXT, wxT("EN_LOADIMP_LDO_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIGSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIGSXT, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIVGN = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIVGN, wxT("EN_LOADIMP_LDO_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIVGN->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIVGN, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_CPSXR = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_CPSXR, wxT("EN_LOADIMP_LDO_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_CPSXR->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_CPSXR, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_CPSXT = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_CPSXT, wxT("EN_LOADIMP_LDO_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_CPSXT->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_CPSXT, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_LNA12 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_LNA12, wxT("EN_LOADIMP_LDO_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_LNA12->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_LNA12, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_LNA14 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_LNA14, wxT("EN_LOADIMP_LDO_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_LNA14->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_LNA14, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_MXRFE = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_MXRFE, wxT("EN_LOADIMP_LDO_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_MXRFE->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_MXRFE, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_RBB = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_RBB, wxT("EN_LOADIMP_LDO_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_RBB->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_RBB, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_RXBUF = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_RXBUF, wxT("EN_LOADIMP_LDO_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_RXBUF->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_RXBUF, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_SPIBUF = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_SPIBUF, wxT("EN_LOADIMP_LDO_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_SPIBUF->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_SPIBUF, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIGIp2 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGIp2, wxT("EN_LOADIMP_LDO_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIGIp2->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIGIp2, 0, flags, 0 );
    
    chkEN_LOADIMP_LDO_DIGIp1 = new wxCheckBox( sbSizer62->GetStaticBox(), ID_EN_LOADIMP_LDO_DIGIp1, wxT("EN_LOADIMP_LDO_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_LOADIMP_LDO_DIGIp1->SetToolTip( wxT("Enables the load dependent bias to optimize the load regulation") );
    
    fgSizer73->Add( chkEN_LOADIMP_LDO_DIGIp1, 0, flags, 0 );
    
    
    sbSizer62->Add( fgSizer73, 0, wxEXPAND, 0 );
    
    
    fgSizer255->Add( sbSizer62, 1, wxLEFT|wxEXPAND, 5 );
    
    
    fgSizer69->Add( fgSizer255, 1, wxEXPAND, 5 );
    
    
    ID_PANEL3->SetSizer( fgSizer69 );
    ID_PANEL3->Layout();
    fgSizer69->Fit( ID_PANEL3 );
    ID_NOTEBOOK1->AddPage( ID_PANEL3, wxT("Bias && Noise filter"), true );
    ID_PANEL2 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxFlexGridSizer* fgSizer75;
    fgSizer75 = new wxFlexGridSizer( 0, 4, 0, 5 );
    fgSizer75->SetFlexibleDirection( wxBOTH );
    fgSizer75->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT1 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_VCOSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_VCOSXR = new wxComboBox( ID_PANEL2, ID_RDIV_VCOSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_VCOSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_VCOSXR, 0, flags, 0 );
    
    ID_STATICTEXT2 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_VCOSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_VCOSXT = new wxComboBox( ID_PANEL2, ID_RDIV_VCOSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_VCOSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_VCOSXT, 0, flags, 0 );
    
    ID_STATICTEXT3 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_TXBUF = new wxComboBox( ID_PANEL2, ID_RDIV_TXBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_TXBUF->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_TXBUF, 0, flags, 0 );
    
    ID_STATICTEXT4 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_VCOGN"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT4->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_VCOGN = new wxComboBox( ID_PANEL2, ID_RDIV_VCOGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_VCOGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_VCOGN, 0, flags, 0 );
    
    ID_STATICTEXT5 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TLOB"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT5->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_TLOB = new wxComboBox( ID_PANEL2, ID_RDIV_TLOB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_TLOB->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_TLOB, 0, flags, 0 );
    
    ID_STATICTEXT6 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TPAD"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT6->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_TPAD = new wxComboBox( ID_PANEL2, ID_RDIV_TPAD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_TPAD->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_TPAD, 0, flags, 0 );
    
    ID_STATICTEXT7 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TIA12"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT7->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_TIA12 = new wxComboBox( ID_PANEL2, ID_RDIV_TIA12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_TIA12->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_TIA12, 0, flags, 0 );
    
    ID_STATICTEXT8 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TIA14"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT8->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_TIA14 = new wxComboBox( ID_PANEL2, ID_RDIV_TIA14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_TIA14->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_TIA14, 0, flags, 0 );
    
    ID_STATICTEXT9 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_RXBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT9->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_RXBUF = new wxComboBox( ID_PANEL2, ID_RDIV_RXBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_RXBUF->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_RXBUF, 0, flags, 0 );
    
    ID_STATICTEXT10 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_TBB"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT10->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_TBB = new wxComboBox( ID_PANEL2, ID_RDIV_TBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_TBB->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_TBB, 0, flags, 0 );
    
    ID_STATICTEXT11 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_MXRFE"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT11->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_MXRFE = new wxComboBox( ID_PANEL2, ID_RDIV_MXRFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_MXRFE->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_MXRFE, 0, flags, 0 );
    
    ID_STATICTEXT12 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_RBB"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT12->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_RBB = new wxComboBox( ID_PANEL2, ID_RDIV_RBB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_RBB->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_RBB, 0, flags, 0 );
    
    ID_STATICTEXT13 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_LNA12"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT13->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_LNA12 = new wxComboBox( ID_PANEL2, ID_RDIV_LNA12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_LNA12->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_LNA12, 0, flags, 0 );
    
    ID_STATICTEXT14 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_LNA14"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT14->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_LNA14 = new wxComboBox( ID_PANEL2, ID_RDIV_LNA14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_LNA14->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_LNA14, 0, flags, 0 );
    
    ID_STATICTEXT15 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIVSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT15->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIVSXR = new wxComboBox( ID_PANEL2, ID_RDIV_DIVSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIVSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIVSXR, 0, flags, 0 );
    
    ID_STATICTEXT16 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIVSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT16->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIVSXT = new wxComboBox( ID_PANEL2, ID_RDIV_DIVSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIVSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIVSXT, 0, flags, 0 );
    
    ID_STATICTEXT17 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT17->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIGSXT = new wxComboBox( ID_PANEL2, ID_RDIV_DIGSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIGSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIGSXT, 0, flags, 0 );
    
    ID_STATICTEXT18 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIVGN"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT18->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIVGN = new wxComboBox( ID_PANEL2, ID_RDIV_DIVGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIVGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIVGN, 0, flags, 0 );
    
    ID_STATICTEXT19 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGGN"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT19->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT19, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIGGN = new wxComboBox( ID_PANEL2, ID_RDIV_DIGGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIGGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIGGN, 0, flags, 0 );
    
    ID_STATICTEXT20 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT20->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIGSXR = new wxComboBox( ID_PANEL2, ID_RDIV_DIGSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIGSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIGSXR, 0, flags, 0 );
    
    ID_STATICTEXT21 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_CPSXT"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT21->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_CPSXT = new wxComboBox( ID_PANEL2, ID_RDIV_CPSXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_CPSXT->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_CPSXT, 0, flags, 0 );
    
    ID_STATICTEXT22 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIG"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT22->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT22, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIG = new wxComboBox( ID_PANEL2, ID_RDIV_DIG, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIG->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIG, 0, flags, 0 );
    
    ID_STATICTEXT23 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_CPGN"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT23->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT23, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_CPGN = new wxComboBox( ID_PANEL2, ID_RDIV_CPGN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_CPGN->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_CPGN, 0, flags, 0 );
    
    ID_STATICTEXT24 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_CPSXR"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT24->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT24, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_CPSXR = new wxComboBox( ID_PANEL2, ID_RDIV_CPSXR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_CPSXR->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_CPSXR, 0, flags, 0 );
    
    ID_STATICTEXT25 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_SPIBUF"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT25->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_SPIBUF = new wxComboBox( ID_PANEL2, ID_RDIV_SPIBUF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_SPIBUF->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_SPIBUF, 0, flags, 0 );
    
    ID_STATICTEXT26 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_AFE"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT26->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_AFE = new wxComboBox( ID_PANEL2, ID_RDIV_AFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    fgSizer75->Add( cmbRDIV_AFE, 0, 0, 5 );
    
    ID_STATICTEXT27 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGIp2"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT27->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIGIp2 = new wxComboBox( ID_PANEL2, ID_RDIV_DIGIp2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIGIp2->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIGIp2, 0, flags, 0 );
    
    ID_STATICTEXT28 = new wxStaticText( ID_PANEL2, wxID_ANY, wxT("RDIV_DIGIp1"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT28->Wrap( 0 );
    fgSizer75->Add( ID_STATICTEXT28, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRDIV_DIGIp1 = new wxComboBox( ID_PANEL2, ID_RDIV_DIGIp1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    cmbRDIV_DIGIp1->SetToolTip( wxT("Controls the output voltage of the LDO by setting the resistive voltage divider ratio") );
    
    fgSizer75->Add( cmbRDIV_DIGIp1, 0, flags, 0 );
    
    
    ID_PANEL2->SetSizer( fgSizer75 );
    ID_PANEL2->Layout();
    fgSizer75->Fit( ID_PANEL2 );
    ID_NOTEBOOK1->AddPage( ID_PANEL2, wxT("Output Voltage"), false );
    
    fgSizer68->Add( ID_NOTEBOOK1, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( fgSizer68 );
    this->Layout();
    
    // Connect Events
    chkEN_G_LDOP->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_G_LDO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_DIVGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_DIGGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_CPGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_VCOGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_MXRFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_LNA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_LNA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_TIA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_DIVSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_DIGSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_CPSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_VCOSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_DIVSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_DIGSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_CPSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_VCOSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkPD_LDO_DIGIp1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkPD_LDO_DIGIp2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_TXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_RXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_TPAD->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_TLOB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkPD_LDO_SPIBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbISINK_SPIBUFF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_TIA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LDO_DIG->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_TIA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_TIA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_TLOB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_TPAD->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_TXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_VCOGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIVSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIVSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_CPGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_VCOSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_VCOSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIG->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIGGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIGSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIGSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIVGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_CPSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_CPSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_LNA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_LNA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_MXRFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_RXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_SPIBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIGIp2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkSPDUP_LDO_DIGIp1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_TBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_TIA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_TIA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_TLOB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_TPAD->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_TXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_VCOGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_VCOSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_VCOSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_AFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_CPGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIVSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIVSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIG->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIGGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIGSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIGSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIVGN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_CPSXR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_CPSXT->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_LNA12->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_LNA14->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_MXRFE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_RBB->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_RXBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_SPIBUF->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIGIp2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    chkEN_LOADIMP_LDO_DIGIp1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_VCOSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_VCOSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_TXBUF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_VCOGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_TLOB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_TPAD->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_TIA12->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_TIA14->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_RXBUF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_TBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_MXRFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_RBB->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_LNA12->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_LNA14->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIVSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIVSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIGSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIVGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIGGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIGSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_CPSXT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIG->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_CPGN->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_CPSXR->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_SPIBUF->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_AFE->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIGIp2->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );
    cmbRDIV_DIGIp1->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLDO_view::ParameterChangeHandler ), NULL, this );

    /*wndId2Enum[chkBYP_LDO_AFE] = BYP_LDO_AFE;
    wndId2Enum[chkBYP_LDO_CPGN] = BYP_LDO_CPGN;
    wndId2Enum[chkBYP_LDO_CPSXR] = BYP_LDO_CPSXR;
    wndId2Enum[chkBYP_LDO_CPSXT] = BYP_LDO_CPSXT;
    wndId2Enum[chkBYP_LDO_DIG] = BYP_LDO_DIG;
    wndId2Enum[chkBYP_LDO_DIGGN] = BYP_LDO_DIGGN;
    wndId2Enum[chkBYP_LDO_DIGIp1] = BYP_LDO_DIGIp1;
    wndId2Enum[chkBYP_LDO_DIGIp2] = BYP_LDO_DIGIp2;
    wndId2Enum[chkBYP_LDO_DIGSXR] = BYP_LDO_DIGSXR;
    wndId2Enum[chkBYP_LDO_DIGSXT] = BYP_LDO_DIGSXT;
    wndId2Enum[chkBYP_LDO_DIVGN] = BYP_LDO_DIVGN;
    wndId2Enum[chkBYP_LDO_DIVSXR] = BYP_LDO_DIVSXR;
    wndId2Enum[chkBYP_LDO_DIVSXT] = BYP_LDO_DIVSXT;
    wndId2Enum[chkBYP_LDO_LNA12] = BYP_LDO_LNA12;
    wndId2Enum[chkBYP_LDO_LNA14] = BYP_LDO_LNA14;
    wndId2Enum[chkBYP_LDO_MXRFE] = BYP_LDO_MXRFE;
    wndId2Enum[chkBYP_LDO_RBB] = BYP_LDO_RBB;
    wndId2Enum[chkBYP_LDO_RXBUF] = BYP_LDO_RXBUF;
    wndId2Enum[chkBYP_LDO_SPIBUF] = BYP_LDO_SPIBUF;
    wndId2Enum[chkBYP_LDO_TBB] = BYP_LDO_TBB;
    wndId2Enum[chkBYP_LDO_TIA12] = BYP_LDO_TIA12;
    wndId2Enum[chkBYP_LDO_TIA14] = BYP_LDO_TIA14;
    wndId2Enum[chkBYP_LDO_TLOB] = BYP_LDO_TLOB;
    wndId2Enum[chkBYP_LDO_TPAD] = BYP_LDO_TPAD;
    wndId2Enum[chkBYP_LDO_TXBUF] = BYP_LDO_TXBUF;
    wndId2Enum[chkBYP_LDO_VCOGN] = BYP_LDO_VCOGN;
    wndId2Enum[chkBYP_LDO_VCOSXR] = BYP_LDO_VCOSXR;
    wndId2Enum[chkBYP_LDO_VCOSXT] = BYP_LDO_VCOSXT;*/
    wndId2Enum[chkEN_G_LDO] = LMS7param(EN_G_LDO);
    wndId2Enum[chkEN_G_LDOP] = LMS7param(EN_G_LDOP);
    wndId2Enum[chkEN_LDO_AFE] = LMS7param(EN_LDO_AFE);
    wndId2Enum[chkEN_LDO_CPGN] = LMS7param(EN_LDO_CPGN);
    wndId2Enum[chkEN_LDO_CPSXR] = LMS7param(EN_LDO_CPSXR);
    wndId2Enum[chkEN_LDO_CPSXT] = LMS7param(EN_LDO_CPSXT);
    wndId2Enum[chkEN_LDO_DIG] = LMS7param(EN_LDO_DIG);
    wndId2Enum[chkEN_LDO_DIGGN] = LMS7param(EN_LDO_DIGGN);
    wndId2Enum[chkPD_LDO_DIGIp1] = LMS7param(PD_LDO_DIGIp1);
    wndId2Enum[chkPD_LDO_DIGIp2] = LMS7param(PD_LDO_DIGIp2);
    wndId2Enum[chkEN_LDO_DIGSXR] = LMS7param(EN_LDO_DIGSXR);
    wndId2Enum[chkEN_LDO_DIGSXT] = LMS7param(EN_LDO_DIGSXT);
    wndId2Enum[chkEN_LDO_DIVGN] = LMS7param(EN_LDO_DIVGN);
    wndId2Enum[chkEN_LDO_DIVSXR] = LMS7param(EN_LDO_DIVSXR);
    wndId2Enum[chkEN_LDO_DIVSXT] = LMS7param(EN_LDO_DIVSXT);
    wndId2Enum[chkEN_LDO_LNA12] = LMS7param(EN_LDO_LNA12);
    wndId2Enum[chkEN_LDO_LNA14] = LMS7param(EN_LDO_LNA14);
    wndId2Enum[chkEN_LDO_MXRFE] = LMS7param(EN_LDO_MXRFE);
    wndId2Enum[chkEN_LDO_RBB] = LMS7param(EN_LDO_RBB);
    wndId2Enum[chkEN_LDO_RXBUF] = LMS7param(EN_LDO_RXBUF);
    wndId2Enum[chkPD_LDO_SPIBUF] = LMS7param(PD_LDO_SPIBUF);
    wndId2Enum[chkEN_LDO_TBB] = LMS7param(EN_LDO_TBB);
    wndId2Enum[chkEN_LDO_TIA12] = LMS7param(EN_LDO_TIA12);
    wndId2Enum[chkEN_LDO_TIA14] = LMS7param(EN_LDO_TIA14);
    wndId2Enum[chkEN_LDO_TLOB] = LMS7param(EN_LDO_TLOB);
    wndId2Enum[chkEN_LDO_TPAD] = LMS7param(EN_LDO_TPAD);
    wndId2Enum[chkEN_LDO_TXBUF] = LMS7param(EN_LDO_TXBUF);
    wndId2Enum[chkEN_LDO_VCOGN] = LMS7param(EN_LDO_VCOGN);
    wndId2Enum[chkEN_LDO_VCOSXR] = LMS7param(EN_LDO_VCOSXR);
    wndId2Enum[chkEN_LDO_VCOSXT] = LMS7param(EN_LDO_VCOSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_AFE] = LMS7param(EN_LOADIMP_LDO_AFE);
    wndId2Enum[chkEN_LOADIMP_LDO_CPGN] = LMS7param(EN_LOADIMP_LDO_CPGN);
    wndId2Enum[chkEN_LOADIMP_LDO_CPSXR] = LMS7param(EN_LOADIMP_LDO_CPSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_CPSXT] = LMS7param(EN_LOADIMP_LDO_CPSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_DIG] = LMS7param(EN_LOADIMP_LDO_DIG);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGGN] = LMS7param(EN_LOADIMP_LDO_DIGGN);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGIp1] = LMS7param(EN_LOADIMP_LDO_DIGIp1);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGIp2] = LMS7param(EN_LOADIMP_LDO_DIGIp2);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGSXR] = LMS7param(EN_LOADIMP_LDO_DIGSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_DIGSXT] = LMS7param(EN_LOADIMP_LDO_DIGSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_DIVGN] = LMS7param(EN_LOADIMP_LDO_DIVGN);
    wndId2Enum[chkEN_LOADIMP_LDO_DIVSXR] = LMS7param(EN_LOADIMP_LDO_DIVSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_DIVSXT] = LMS7param(EN_LOADIMP_LDO_DIVSXT);
    wndId2Enum[chkEN_LOADIMP_LDO_LNA12] = LMS7param(EN_LOADIMP_LDO_LNA12);
    wndId2Enum[chkEN_LOADIMP_LDO_LNA14] = LMS7param(EN_LOADIMP_LDO_LNA14);
    wndId2Enum[chkEN_LOADIMP_LDO_MXRFE] = LMS7param(EN_LOADIMP_LDO_MXRFE);
    wndId2Enum[chkEN_LOADIMP_LDO_RBB] = LMS7param(EN_LOADIMP_LDO_RBB);
    wndId2Enum[chkEN_LOADIMP_LDO_RXBUF] = LMS7param(EN_LOADIMP_LDO_RXBUF);
    wndId2Enum[chkEN_LOADIMP_LDO_SPIBUF] = LMS7param(EN_LOADIMP_LDO_SPIBUF);
    wndId2Enum[chkEN_LOADIMP_LDO_TBB] = LMS7param(EN_LOADIMP_LDO_TBB);
    wndId2Enum[chkEN_LOADIMP_LDO_TIA12] = LMS7param(EN_LOADIMP_LDO_TIA12);
    wndId2Enum[chkEN_LOADIMP_LDO_TIA14] = LMS7param(EN_LOADIMP_LDO_TIA14);
    wndId2Enum[chkEN_LOADIMP_LDO_TLOB] = LMS7param(EN_LOADIMP_LDO_TLOB);
    wndId2Enum[chkEN_LOADIMP_LDO_TPAD] = LMS7param(EN_LOADIMP_LDO_TPAD);
    wndId2Enum[chkEN_LOADIMP_LDO_TXBUF] = LMS7param(EN_LOADIMP_LDO_TXBUF);
    wndId2Enum[chkEN_LOADIMP_LDO_VCOGN] = LMS7param(EN_LOADIMP_LDO_VCOGN);
    wndId2Enum[chkEN_LOADIMP_LDO_VCOSXR] = LMS7param(EN_LOADIMP_LDO_VCOSXR);
    wndId2Enum[chkEN_LOADIMP_LDO_VCOSXT] = LMS7param(EN_LOADIMP_LDO_VCOSXT);
    wndId2Enum[cmbRDIV_AFE] = LMS7param(RDIV_AFE);
    wndId2Enum[cmbRDIV_CPGN] = LMS7param(RDIV_CPGN);
    wndId2Enum[cmbRDIV_CPSXR] = LMS7param(RDIV_CPSXR);
    wndId2Enum[cmbRDIV_CPSXT] = LMS7param(RDIV_CPSXT);
    wndId2Enum[cmbRDIV_DIG] = LMS7param(RDIV_DIG);
    wndId2Enum[cmbRDIV_DIGGN] = LMS7param(RDIV_DIGGN);
    wndId2Enum[cmbRDIV_DIGIp1] = LMS7param(RDIV_DIGIp1);
    wndId2Enum[cmbRDIV_DIGIp2] = LMS7param(RDIV_DIGIp2);
    wndId2Enum[cmbRDIV_DIGSXR] = LMS7param(RDIV_DIGSXR);
    wndId2Enum[cmbRDIV_DIGSXT] = LMS7param(RDIV_DIGSXT);
    wndId2Enum[cmbRDIV_DIVGN] = LMS7param(RDIV_DIVGN);
    wndId2Enum[cmbRDIV_DIVSXR] = LMS7param(RDIV_DIVSXR);
    wndId2Enum[cmbRDIV_DIVSXT] = LMS7param(RDIV_DIVSXT);
    wndId2Enum[cmbRDIV_LNA12] = LMS7param(RDIV_LNA12);
    wndId2Enum[cmbRDIV_LNA14] = LMS7param(RDIV_LNA14);
    wndId2Enum[cmbRDIV_MXRFE] = LMS7param(RDIV_MXRFE);
    wndId2Enum[cmbRDIV_RBB] = LMS7param(RDIV_RBB);
    wndId2Enum[cmbRDIV_RXBUF] = LMS7param(RDIV_RXBUF);
    wndId2Enum[cmbRDIV_SPIBUF] = LMS7param(RDIV_SPIBUF);
    wndId2Enum[cmbRDIV_TBB] = LMS7param(RDIV_TBB);
    wndId2Enum[cmbRDIV_TIA12] = LMS7param(RDIV_TIA12);
    wndId2Enum[cmbRDIV_TIA14] = LMS7param(RDIV_TIA14);
    wndId2Enum[cmbRDIV_TLOB] = LMS7param(RDIV_TLOB);
    wndId2Enum[cmbRDIV_TPAD] = LMS7param(RDIV_TPAD);
    wndId2Enum[cmbRDIV_TXBUF] = LMS7param(RDIV_TXBUF);
    wndId2Enum[cmbRDIV_VCOGN] = LMS7param(RDIV_VCOGN);
    wndId2Enum[cmbRDIV_VCOSXR] = LMS7param(RDIV_VCOSXR);
    wndId2Enum[cmbRDIV_VCOSXT] = LMS7param(RDIV_VCOSXT);
    wndId2Enum[chkSPDUP_LDO_AFE] = LMS7param(SPDUP_LDO_AFE);
    wndId2Enum[chkSPDUP_LDO_CPGN] = LMS7param(SPDUP_LDO_CPGN);
    wndId2Enum[chkSPDUP_LDO_CPSXR] = LMS7param(SPDUP_LDO_CPSXR);
    wndId2Enum[chkSPDUP_LDO_CPSXT] = LMS7param(SPDUP_LDO_CPSXT);
    wndId2Enum[chkSPDUP_LDO_DIG] = LMS7param(SPDUP_LDO_DIG);
    wndId2Enum[chkSPDUP_LDO_DIGGN] = LMS7param(SPDUP_LDO_DIGGN);
    wndId2Enum[chkSPDUP_LDO_DIGIp1] = LMS7param(SPDUP_LDO_DIGIp1);
    wndId2Enum[chkSPDUP_LDO_DIGIp2] = LMS7param(SPDUP_LDO_DIGIp2);
    wndId2Enum[chkSPDUP_LDO_DIGSXR] = LMS7param(SPDUP_LDO_DIGSXR);
    wndId2Enum[chkSPDUP_LDO_DIGSXT] = LMS7param(SPDUP_LDO_DIGSXT);
    wndId2Enum[chkSPDUP_LDO_DIVGN] = LMS7param(SPDUP_LDO_DIVGN);
    wndId2Enum[chkSPDUP_LDO_DIVSXR] = LMS7param(SPDUP_LDO_DIVSXR);
    wndId2Enum[chkSPDUP_LDO_DIVSXT] = LMS7param(SPDUP_LDO_DIVSXT);
    wndId2Enum[chkSPDUP_LDO_LNA12] = LMS7param(SPDUP_LDO_LNA12);
    wndId2Enum[chkSPDUP_LDO_LNA14] = LMS7param(SPDUP_LDO_LNA14);
    wndId2Enum[chkSPDUP_LDO_MXRFE] = LMS7param(SPDUP_LDO_MXRFE);
    wndId2Enum[chkSPDUP_LDO_RBB] = LMS7param(SPDUP_LDO_RBB);
    wndId2Enum[chkSPDUP_LDO_RXBUF] = LMS7param(SPDUP_LDO_RXBUF);
    wndId2Enum[chkSPDUP_LDO_SPIBUF] = LMS7param(SPDUP_LDO_SPIBUF);
    wndId2Enum[chkSPDUP_LDO_TBB] = LMS7param(SPDUP_LDO_TBB);
    wndId2Enum[chkSPDUP_LDO_TIA12] = LMS7param(SPDUP_LDO_TIA12);
    wndId2Enum[chkSPDUP_LDO_TIA14] = LMS7param(SPDUP_LDO_TIA14);
    wndId2Enum[chkSPDUP_LDO_TLOB] = LMS7param(SPDUP_LDO_TLOB);
    wndId2Enum[chkSPDUP_LDO_TPAD] = LMS7param(SPDUP_LDO_TPAD);
    wndId2Enum[chkSPDUP_LDO_TXBUF] = LMS7param(SPDUP_LDO_TXBUF);
    wndId2Enum[chkSPDUP_LDO_VCOGN] = LMS7param(SPDUP_LDO_VCOGN);
    wndId2Enum[chkSPDUP_LDO_VCOSXR] = LMS7param(SPDUP_LDO_VCOSXR);
    wndId2Enum[chkSPDUP_LDO_VCOSXT] = LMS7param(SPDUP_LDO_VCOSXT);
    wndId2Enum[cmbISINK_SPIBUFF] = LMS7param(ISINK_SPIBUFF);
    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<256; ++i)
    {
        temp.push_back(wxString::Format(_("%.3f V"), (860.0 + 3.92*i) / 1000.0));
    }
    cmbRDIV_AFE->Append(temp);
    cmbRDIV_CPGN->Append(temp);
    cmbRDIV_CPSXR->Append(temp);
    cmbRDIV_CPSXT->Append(temp);
    cmbRDIV_DIG->Append(temp);
    cmbRDIV_DIGGN->Append(temp);
    cmbRDIV_DIGIp1->Append(temp);
    cmbRDIV_DIGIp2->Append(temp);
    cmbRDIV_DIGSXR->Append(temp);
    cmbRDIV_DIGSXT->Append(temp);
    cmbRDIV_DIVGN->Append(temp);
    cmbRDIV_DIVSXR->Append(temp);
    cmbRDIV_DIVSXT->Append(temp);
    cmbRDIV_LNA12->Append(temp);
    cmbRDIV_LNA14->Append(temp);
    cmbRDIV_MXRFE->Append(temp);
    cmbRDIV_RBB->Append(temp);
    cmbRDIV_RXBUF->Append(temp);
    cmbRDIV_SPIBUF->Append(temp);
    cmbRDIV_TBB->Append(temp);
    cmbRDIV_TIA12->Append(temp);
    cmbRDIV_TIA14->Append(temp);
    cmbRDIV_TLOB->Append(temp);
    cmbRDIV_TPAD->Append(temp);
    cmbRDIV_TXBUF->Append(temp);
    cmbRDIV_VCOGN->Append(temp);
    cmbRDIV_VCOSXR->Append(temp);
    cmbRDIV_VCOSXT->Append(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlLDO_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
         value = 1;
    cmbISINK_SPIBUFF->Enable(value);
}

void lms7002_pnlLDO_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlLDO_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
