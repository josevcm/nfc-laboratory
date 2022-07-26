#include "lms7002_pnlLimeLightPAD_view.h"
#include <map>
#include "lms7002_gui_utilities.h"
using namespace lime;

lms7002_pnlLimeLightPAD_view::lms7002_pnlLimeLightPAD_view( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    const int flags = wxALL;
    wxFlexGridSizer* fgSizer105;
    fgSizer105 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer105->SetFlexibleDirection( wxBOTH );
    fgSizer105->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_NOTEBOOK1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    ID_PANEL1 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxFlexGridSizer* fgMainSizer;
    fgMainSizer = new wxFlexGridSizer( 0, 3, 5, 5 );
    fgMainSizer->SetFlexibleDirection( wxBOTH );
    fgMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer195;
    fgSizer195 = new wxFlexGridSizer( 0, 1, 5, 0 );
    fgSizer195->SetFlexibleDirection( wxBOTH );
    fgSizer195->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerEngagePullUp;
    sbSizerEngagePullUp = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Engage pull up control") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer107;
    fgSizer107 = new wxFlexGridSizer( 0, 3, 0, 0 );
    fgSizer107->SetFlexibleDirection( wxBOTH );
    fgSizer107->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkSDA_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SDA_PE, wxT("SDA pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSDA_PE->SetToolTip( wxT("Pull up control of SDA pad") );
    
    fgSizer107->Add( chkSDA_PE, 0, flags, 0 );
    
    chkSCL_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SCL_PE, wxT("SCL pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSCL_PE->SetToolTip( wxT("Pull up control of SCL pad") );
    
    fgSizer107->Add( chkSCL_PE, 0, flags, 0 );
    
    chkRX_CLK_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_RX_CLK_PE, wxT("RX_CLK pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkRX_CLK_PE->SetToolTip( wxT("Pull up control of RX_CLK pad") );
    
    fgSizer107->Add( chkRX_CLK_PE, 0, flags, 0 );
    
    chkSDIO_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SDIO_PE, wxT("SDIO pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSDIO_PE->SetToolTip( wxT("Pull up control of SDIO pad") );
    
    fgSizer107->Add( chkSDIO_PE, 0, flags, 0 );
    
    chkSDO_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SDO_PE, wxT("SDO pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSDO_PE->SetToolTip( wxT("Pull up control of SDO pad") );
    
    fgSizer107->Add( chkSDO_PE, 0, flags, 0 );
    
    chkTX_CLK_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_TX_CLK_PE, wxT("TX_CLK pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTX_CLK_PE->SetToolTip( wxT("Pull up control of TX_CLK pad") );
    
    fgSizer107->Add( chkTX_CLK_PE, 0, flags, 0 );
    
    chkSEN_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SEN_PE, wxT("SEN pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSEN_PE->SetToolTip( wxT("Pull up control of SEN pad") );
    
    fgSizer107->Add( chkSEN_PE, 0, flags, 0 );
    
    chkDIQ1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_DIQ1_PE, wxT("DIQ1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkDIQ1_PE->SetToolTip( wxT("Pull up control of DIQ1 pad") );
    
    fgSizer107->Add( chkDIQ1_PE, 0, flags, 0 );
    
    chkTXNRX1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_TXNRX1_PE, wxT("TXNRX1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTXNRX1_PE->SetToolTip( wxT("Pull up control of TXNRX1 pad") );
    
    fgSizer107->Add( chkTXNRX1_PE, 0, flags, 0 );
    
    chkSCLK_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_SCLK_PE, wxT("CLK pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSCLK_PE->SetToolTip( wxT("Pull up control of SCLK pad") );
    
    fgSizer107->Add( chkSCLK_PE, 0, flags, 0 );
    
    chkDIQ2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_DIQ2_PE, wxT("DIQ2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkDIQ2_PE->SetToolTip( wxT("Pull up control of DIQ2 pad") );
    
    fgSizer107->Add( chkDIQ2_PE, 0, flags, 0 );
    
    chkTXNRX2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_TXNRX2_PE, wxT("TXNRX2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTXNRX2_PE->SetValue(true); 
    chkTXNRX2_PE->SetToolTip( wxT("Pull up control of TXNRX2 pad") );
    
    fgSizer107->Add( chkTXNRX2_PE, 0, flags, 0 );
    
    chkIQ_SEL_EN_1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_IQ_SEL_EN_1_PE, wxT("IQ_SEL_EN_1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkIQ_SEL_EN_1_PE->SetToolTip( wxT("Pull up control of IQ_SEL_EN_1 pad") );
    
    fgSizer107->Add( chkIQ_SEL_EN_1_PE, 0, flags, 0 );
    
    chkFCLK1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_FCLK1_PE, wxT("FCLK1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkFCLK1_PE->SetToolTip( wxT("Pull up control of FCLK1 pad") );
    
    fgSizer107->Add( chkFCLK1_PE, 0, flags, 0 );
    
    chkMCLK1_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_MCLK1_PE, wxT("MCLK1 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMCLK1_PE->SetToolTip( wxT("Pull up control of MCLK1 pad") );
    
    fgSizer107->Add( chkMCLK1_PE, 0, flags, 0 );
    
    chkIQ_SEL_EN_2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_IQ_SEL_EN_2_PE, wxT("IQ_SEL_EN_2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkIQ_SEL_EN_2_PE->SetToolTip( wxT("Pull up control of IQ_SEL_EN_2 pad") );
    
    fgSizer107->Add( chkIQ_SEL_EN_2_PE, 0, flags, 0 );
    
    chkFCLK2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_FCLK2_PE, wxT("FCLK2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkFCLK2_PE->SetToolTip( wxT("Pull up control of FCLK2 pad") );
    
    fgSizer107->Add( chkFCLK2_PE, 0, flags, 0 );
    
    chkMCLK2_PE = new wxCheckBox( sbSizerEngagePullUp->GetStaticBox(), ID_MCLK2_PE, wxT("MCLK2 pad"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMCLK2_PE->SetToolTip( wxT("Pull up control of MCLK2 pad") );
    
    fgSizer107->Add( chkMCLK2_PE, 0, flags, 0 );
    
    
    sbSizerEngagePullUp->Add( fgSizer107, 0, wxEXPAND, 0 );
    
    
    fgSizer195->Add( sbSizerEngagePullUp, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerResetSignals;
    sbSizerResetSignals = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Reset signals") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer196;
    fgSizer196 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer196->AddGrowableCol( 0 );
    fgSizer196->SetFlexibleDirection( wxBOTH );
    fgSizer196->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer197;
    fgSizer197 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer197->AddGrowableCol( 0 );
    fgSizer197->AddGrowableCol( 1 );
    fgSizer197->SetFlexibleDirection( wxBOTH );
    fgSizer197->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkSRST_RXFIFO = new wxCheckBox( sbSizerResetSignals->GetStaticBox(), ID_SRST_RXFIFO, wxT("Rx FIFO soft reset"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSRST_RXFIFO->SetToolTip( wxT("RX FIFO soft reset (LimeLight Interface)") );
    
    fgSizer197->Add( chkSRST_RXFIFO, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    chkSRST_TXFIFO = new wxCheckBox( sbSizerResetSignals->GetStaticBox(), ID_SRST_TXFIFO, wxT("Tx FIFO soft reset"), wxDefaultPosition, wxDefaultSize, 0 );
    chkSRST_TXFIFO->SetToolTip( wxT("TX FIFO soft reset (LimeLight Interface)") );
    
    fgSizer197->Add( chkSRST_TXFIFO, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    
    fgSizer196->Add( fgSizer197, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer198;
    fgSizer198 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer198->AddGrowableCol( 0 );
    fgSizer198->AddGrowableCol( 1 );
    fgSizer198->SetFlexibleDirection( wxBOTH );
    fgSizer198->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerLogicRegistersReset;
    sbSizerLogicRegistersReset = new wxStaticBoxSizer( new wxStaticBox( sbSizerResetSignals->GetStaticBox(), wxID_ANY, wxT("Logic registers") ), wxVERTICAL );
    
    chkLRST_TX_A = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_TX_A, wxT("Tx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLRST_TX_A->SetToolTip( wxT("Resets all the logic registers to the default state for Tx MIMO channel A") );
    
    sbSizerLogicRegistersReset->Add( chkLRST_TX_A, 0, flags, 0 );
    
    chkLRST_TX_B = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_TX_B, wxT("Tx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLRST_TX_B->SetToolTip( wxT("Resets all the logic registers to the default state for Tx MIMO channel B") );
    
    sbSizerLogicRegistersReset->Add( chkLRST_TX_B, 0, flags, 0 );
    
    chkLRST_RX_A = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_RX_A, wxT("Rx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLRST_RX_A->SetToolTip( wxT("Resets all the logic registers to the default state for Rx MIMO channel A") );
    
    sbSizerLogicRegistersReset->Add( chkLRST_RX_A, 0, flags, 0 );
    
    chkLRST_RX_B = new wxCheckBox( sbSizerLogicRegistersReset->GetStaticBox(), ID_LRST_RX_B, wxT("Rx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLRST_RX_B->SetToolTip( wxT("Resets all the logic registers to the default state for Rx MIMO channel B") );
    
    sbSizerLogicRegistersReset->Add( chkLRST_RX_B, 0, flags, 0 );
    
    
    fgSizer198->Add( sbSizerLogicRegistersReset, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerConfigurationMemoryReset;
    sbSizerConfigurationMemoryReset = new wxStaticBoxSizer( new wxStaticBox( sbSizerResetSignals->GetStaticBox(), wxID_ANY, wxT("Configuration memory") ), wxVERTICAL );
    
    chkMRST_TX_A = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_TX_A, wxT("Tx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMRST_TX_A->SetToolTip( wxT("Resets all the configuration memory to the default state for Tx MIMO channel A") );
    
    sbSizerConfigurationMemoryReset->Add( chkMRST_TX_A, 0, flags, 0 );
    
    chkMRST_TX_B = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_TX_B, wxT("Tx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMRST_TX_B->SetToolTip( wxT("Resets all the configuration memory to the default state for Tx MIMO channel B") );
    
    sbSizerConfigurationMemoryReset->Add( chkMRST_TX_B, 0, flags, 0 );
    
    chkMRST_RX_A = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_RX_A, wxT("Rx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMRST_RX_A->SetToolTip( wxT("Resets all the configuration memory to the default state for Rx MIMO channel A") );
    
    sbSizerConfigurationMemoryReset->Add( chkMRST_RX_A, 0, flags, 0 );
    
    chkMRST_RX_B = new wxCheckBox( sbSizerConfigurationMemoryReset->GetStaticBox(), ID_MRST_RX_B, wxT("Rx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMRST_RX_B->SetToolTip( wxT("Resets all the configuration memory to the default state for Rx MIMO channel B") );
    
    sbSizerConfigurationMemoryReset->Add( chkMRST_RX_B, 0, flags, 0 );
    
    
    fgSizer198->Add( sbSizerConfigurationMemoryReset, 1, wxEXPAND, 5 );
    
    
    fgSizer196->Add( fgSizer198, 1, wxEXPAND, 5 );
    
    
    sbSizerResetSignals->Add( fgSizer196, 1, wxEXPAND, 5 );
    
    
    fgSizer195->Add( sbSizerResetSignals, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerPowerControls;
    sbSizerPowerControls = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Power Control") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer113;
    fgSizer113 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer113->SetFlexibleDirection( wxBOTH );
    fgSizer113->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkRXEN_A = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_RXEN_A, wxT("Enable Rx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
    chkRXEN_A->SetToolTip( wxT("Power control for Rx MIMO channel A") );
    
    fgSizer113->Add( chkRXEN_A, 0, flags, 0 );
    
    chkTXEN_A = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_TXEN_A, wxT("Enable Tx MIMO ch. A"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTXEN_A->SetToolTip( wxT("Power control for Tx MIMO channel A") );
    
    fgSizer113->Add( chkTXEN_A, 0, flags, 0 );
    
    chkRXEN_B = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_RXEN_B, wxT("Enable Rx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
    chkRXEN_B->SetToolTip( wxT("Power control for Rx MIMO channel B") );
    
    fgSizer113->Add( chkRXEN_B, 0, flags, 0 );
    
    chkTXEN_B = new wxCheckBox( sbSizerPowerControls->GetStaticBox(), ID_TXEN_B, wxT("Enable Tx MIMO ch. B"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTXEN_B->SetToolTip( wxT("Power control for Tx MIMO channel B") );
    
    fgSizer113->Add( chkTXEN_B, 0, flags, 0 );
    
    
    sbSizerPowerControls->Add( fgSizer113, 0, wxEXPAND, 0 );
    
    
    fgSizer195->Add( sbSizerPowerControls, 1, wxEXPAND, 5 );
    
    chkMIMO_SISO = new wxCheckBox( ID_PANEL1, ID_MIMO_SISO, wxT("Disable MIMO channel B"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMIMO_SISO->SetValue(true); 
    fgSizer195->Add( chkMIMO_SISO, 0, wxLEFT, 5 );
    
    
    fgMainSizer->Add( fgSizer195, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer199;
    fgSizer199 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer199->AddGrowableCol( 0 );
    fgSizer199->SetFlexibleDirection( wxBOTH );
    fgSizer199->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerLimeLightModes;
    sbSizerLimeLightModes = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("LimeLight modes") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer108;
    fgSizer108 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer108->AddGrowableCol( 0 );
    fgSizer108->AddGrowableCol( 1 );
    fgSizer108->SetFlexibleDirection( wxBOTH );
    fgSizer108->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    chkMOD_EN = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_MOD_EN, wxT("Enable LimeLight interface"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMOD_EN->SetToolTip( wxT("LimeLight interface enable") );
    
    fgSizer108->Add( chkMOD_EN, 0, wxALIGN_LEFT|wxLEFT, 5 );
    
    
    fgSizer108->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxString rgrLML_FIDM1Choices[] = { wxT("0"), wxT("1") };
    int rgrLML_FIDM1NChoices = sizeof( rgrLML_FIDM1Choices ) / sizeof( wxString );
    rgrLML_FIDM1 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_FIDM1, wxT("Frame start for Port 1"), wxDefaultPosition, wxDefaultSize, rgrLML_FIDM1NChoices, rgrLML_FIDM1Choices, 2, wxRA_SPECIFY_COLS );
    rgrLML_FIDM1->SetSelection( 0 );
    rgrLML_FIDM1->SetToolTip( wxT("Frame start ID selection for Port 1 when LML_MODE1 = 0") );
    
    fgSizer108->Add( rgrLML_FIDM1, 0, wxEXPAND, 0 );
    
    wxString rgrLML_FIDM2Choices[] = { wxT("0"), wxT("1") };
    int rgrLML_FIDM2NChoices = sizeof( rgrLML_FIDM2Choices ) / sizeof( wxString );
    rgrLML_FIDM2 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_FIDM2, wxT("Frame start for Port 2"), wxDefaultPosition, wxDefaultSize, rgrLML_FIDM2NChoices, rgrLML_FIDM2Choices, 2, wxRA_SPECIFY_COLS );
    rgrLML_FIDM2->SetSelection( 1 );
    rgrLML_FIDM2->SetToolTip( wxT("Frame start ID selection for Port 2 when LML_MODE2 = 0") );
    
    fgSizer108->Add( rgrLML_FIDM2, 0, wxEXPAND, 0 );
    
    wxString rgrLML_MODE1Choices[] = { wxT("TRXIQ"), wxT("JESD207") };
    int rgrLML_MODE1NChoices = sizeof( rgrLML_MODE1Choices ) / sizeof( wxString );
    rgrLML_MODE1 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_MODE1, wxT("LimeLight port 1 mode"), wxDefaultPosition, wxDefaultSize, rgrLML_MODE1NChoices, rgrLML_MODE1Choices, 2, wxRA_SPECIFY_COLS );
    rgrLML_MODE1->SetSelection( 0 );
    rgrLML_MODE1->SetToolTip( wxT("Mode of LimeLight Port 1") );
    
    fgSizer108->Add( rgrLML_MODE1, 0, wxEXPAND, 0 );
    
    wxString rgrLML_MODE2Choices[] = { wxT("TRXIQ"), wxT("JESD207") };
    int rgrLML_MODE2NChoices = sizeof( rgrLML_MODE2Choices ) / sizeof( wxString );
    rgrLML_MODE2 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_MODE2, wxT("LimeLight port 2 mode"), wxDefaultPosition, wxDefaultSize, rgrLML_MODE2NChoices, rgrLML_MODE2Choices, 2, wxRA_SPECIFY_COLS );
    rgrLML_MODE2->SetSelection( 0 );
    rgrLML_MODE2->SetToolTip( wxT("Mode of LimeLight Port 2") );
    
    fgSizer108->Add( rgrLML_MODE2, 0, wxEXPAND, 0 );
    
    wxString rgrLML_TXNRXIQ1Choices[] = { wxT("RXIQ"), wxT("TXIQ") };
    int rgrLML_TXNRXIQ1NChoices = sizeof( rgrLML_TXNRXIQ1Choices ) / sizeof( wxString );
    rgrLML_TXNRXIQ1 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_TXNRXIQ1, wxT("Port 1 mode selection"), wxDefaultPosition, wxDefaultSize, rgrLML_TXNRXIQ1NChoices, rgrLML_TXNRXIQ1Choices, 2, wxRA_SPECIFY_COLS );
    rgrLML_TXNRXIQ1->SetSelection( 0 );
    rgrLML_TXNRXIQ1->SetToolTip( wxT("TXIQ/RXIQ mode selection for Port 1 when LML_MODE1 = 0") );
    
    fgSizer108->Add( rgrLML_TXNRXIQ1, 0, wxEXPAND, 0 );
    
    wxString rgrLML_TXNRXIQ2Choices[] = { wxT("RXIQ"), wxT("TXIQ") };
    int rgrLML_TXNRXIQ2NChoices = sizeof( rgrLML_TXNRXIQ2Choices ) / sizeof( wxString );
    rgrLML_TXNRXIQ2 = new wxRadioBox( sbSizerLimeLightModes->GetStaticBox(), ID_LML_TXNRXIQ2, wxT("Port 2 mode selection"), wxDefaultPosition, wxDefaultSize, rgrLML_TXNRXIQ2NChoices, rgrLML_TXNRXIQ2Choices, 2, wxRA_SPECIFY_COLS );
    rgrLML_TXNRXIQ2->SetSelection( 0 );
    rgrLML_TXNRXIQ2->SetToolTip( wxT("TXIQ/RXIQ mode selection for Port 2 when LML_MODE2 = 0") );
    
    fgSizer108->Add( rgrLML_TXNRXIQ2, 0, wxEXPAND, 0 );
    
    chkLML1_TRXIQPULSE = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML1 TRXIQPULSE mode"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLML1_TRXIQPULSE->SetToolTip( wxT("Power control for Tx MIMO channel B") );
    
    fgSizer108->Add( chkLML1_TRXIQPULSE, 0, wxALIGN_LEFT, 0 );
    
    chkLML2_TRXIQPULSE = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML2 TRXIQPULSE mode"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLML2_TRXIQPULSE->SetToolTip( wxT("Power control for Tx MIMO channel B") );
    
    fgSizer108->Add( chkLML2_TRXIQPULSE, 0, wxALIGN_LEFT, 0 );
    
    chkLML1_SISODDR = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML1 SISODDR mode"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLML1_SISODDR->SetToolTip( wxT("Power control for Tx MIMO channel B") );
    
    fgSizer108->Add( chkLML1_SISODDR, 0, wxALIGN_LEFT, 0 );
    
    chkLML2_SISODDR = new wxCheckBox( sbSizerLimeLightModes->GetStaticBox(), ID_TXEN_B, wxT("LML2 SISODDR mode"), wxDefaultPosition, wxDefaultSize, 0 );
    chkLML2_SISODDR->SetToolTip( wxT("Power control for Tx MIMO channel B") );
    
    fgSizer108->Add( chkLML2_SISODDR, 0, wxALIGN_LEFT, 0 );
    
    
    sbSizerLimeLightModes->Add( fgSizer108, 0, wxEXPAND|wxALL, 5);
    
    
    fgSizer199->Add( sbSizerLimeLightModes, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerDriverStrength;
    sbSizerDriverStrength = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL1, wxID_ANY, wxT("Driver strength") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer111;
    fgSizer111 = new wxFlexGridSizer(0, 5, 5, 5);
    fgSizer111->SetFlexibleDirection( wxBOTH );
    fgSizer111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString rgrSDA_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
    int rgrSDA_DSNChoices = sizeof( rgrSDA_DSChoices ) / sizeof( wxString );
    rgrSDA_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_SDA_DS, wxT("SDA pad"), wxDefaultPosition, wxDefaultSize, rgrSDA_DSNChoices, rgrSDA_DSChoices, 1, wxRA_SPECIFY_COLS );
    rgrSDA_DS->SetSelection( 0 );
    rgrSDA_DS->SetToolTip( wxT("Driver strength of SDA pad") );
    
    fgSizer111->Add( rgrSDA_DS, 0, wxEXPAND, 0 );
    
    wxString rgrSCL_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
    int rgrSCL_DSNChoices = sizeof( rgrSCL_DSChoices ) / sizeof( wxString );
    rgrSCL_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_SCL_DS, wxT("SCL pad"), wxDefaultPosition, wxDefaultSize, rgrSCL_DSNChoices, rgrSCL_DSChoices, 1, wxRA_SPECIFY_COLS );
    rgrSCL_DS->SetSelection( 0 );
    rgrSCL_DS->SetToolTip( wxT("Driver strength of SCL pad") );
    
    fgSizer111->Add( rgrSCL_DS, 0, wxEXPAND, 0 );
    
    wxString rgrSDIO_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
    int rgrSDIO_DSNChoices = sizeof( rgrSDIO_DSChoices ) / sizeof( wxString );
    rgrSDIO_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_SDIO_DS, wxT("SDIO pad"), wxDefaultPosition, wxDefaultSize, rgrSDIO_DSNChoices, rgrSDIO_DSChoices, 1, wxRA_SPECIFY_COLS );
    rgrSDIO_DS->SetSelection( 0 );
    rgrSDIO_DS->SetToolTip( wxT("Driver strength of SDIO pad") );
    
    fgSizer111->Add( rgrSDIO_DS, 0, wxEXPAND, 0 );
    
    wxString rgrDIQ2_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
    int rgrDIQ2_DSNChoices = sizeof( rgrDIQ2_DSChoices ) / sizeof( wxString );
    rgrDIQ2_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_DIQ2_DS, wxT("DIQ2 pad"), wxDefaultPosition, wxDefaultSize, rgrDIQ2_DSNChoices, rgrDIQ2_DSChoices, 1, wxRA_SPECIFY_COLS );
    rgrDIQ2_DS->SetSelection( 0 );
    rgrDIQ2_DS->SetToolTip( wxT("Driver strength of DIQ2 pad") );
    
    fgSizer111->Add( rgrDIQ2_DS, 0, wxEXPAND, 0 );
    
    wxString rgrDIQ1_DSChoices[] = { wxT("4 mA"), wxT("8 mA") };
    int rgrDIQ1_DSNChoices = sizeof( rgrDIQ1_DSChoices ) / sizeof( wxString );
    rgrDIQ1_DS = new wxRadioBox( sbSizerDriverStrength->GetStaticBox(), ID_DIQ1_DS, wxT("DIQ1 pad"), wxDefaultPosition, wxDefaultSize, rgrDIQ1_DSNChoices, rgrDIQ1_DSChoices, 1, wxRA_SPECIFY_COLS );
    rgrDIQ1_DS->SetSelection( 0 );
    rgrDIQ1_DS->SetToolTip( wxT("Pull up control of MCLK2 pad") );
    
    fgSizer111->Add( rgrDIQ1_DS, 0, wxEXPAND, 0 );
    
    
    sbSizerDriverStrength->Add( fgSizer111, 0, wxEXPAND|wxALL, 5 );
    
    
    fgSizer199->Add( sbSizerDriverStrength, 1, wxEXPAND, 5 );
    
    
    fgMainSizer->Add( fgSizer199, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer222;
    fgSizer222 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer222->SetFlexibleDirection( wxBOTH );
    fgSizer222->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer202;
    fgSizer202 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer202->AddGrowableCol( 0 );
    fgSizer202->SetFlexibleDirection( wxBOTH );
    fgSizer202->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString rgrRX_MUXChoices[] = { wxT("RxTSP"), wxT("TxFIFO"), wxT("LFSR") };
    int rgrRX_MUXNChoices = sizeof( rgrRX_MUXChoices ) / sizeof( wxString );
    rgrRX_MUX = new wxRadioBox( ID_PANEL1, ID_RX_MUX, wxT("RxFIFO source"), wxDefaultPosition, wxDefaultSize, rgrRX_MUXNChoices, rgrRX_MUXChoices, 1, wxRA_SPECIFY_COLS );
    rgrRX_MUX->SetSelection( 0 );
    fgSizer202->Add( rgrRX_MUX, 0, wxEXPAND, 5 );
    
    wxString rgrTX_MUXChoices[] = { wxT("Port1"), wxT("Port2"), wxT("RxTSP") };
    int rgrTX_MUXNChoices = sizeof( rgrTX_MUXChoices ) / sizeof( wxString );
    rgrTX_MUX = new wxRadioBox( ID_PANEL1, ID_TX_MUX, wxT("Data transmit port"), wxDefaultPosition, wxDefaultSize, rgrTX_MUXNChoices, rgrTX_MUXChoices, 1, wxRA_SPECIFY_COLS );
    rgrTX_MUX->SetSelection( 0 );
    fgSizer202->Add( rgrTX_MUX, 0, wxEXPAND, 5 );
    
    wxString rgrSPIMODEChoices[] = { wxT("3 wire mode"), wxT("4 wire mode") };
    int rgrSPIMODENChoices = sizeof( rgrSPIMODEChoices ) / sizeof( wxString );
    rgrSPIMODE = new wxRadioBox( ID_PANEL1, ID_SPIMODE, wxT("SPI mode"), wxDefaultPosition, wxDefaultSize, rgrSPIMODENChoices, rgrSPIMODEChoices, 1, wxRA_SPECIFY_COLS );
    rgrSPIMODE->SetSelection( 0 );
    rgrSPIMODE->SetToolTip( wxT("SPI communication mode") );
    
    fgSizer202->Add( rgrSPIMODE, 0, wxEXPAND, 0 );
    
    
    fgSizer222->Add( fgSizer202, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer114;
    fgSizer114 = new wxFlexGridSizer( 0, 6, 5, 5 );
    fgSizer114->SetFlexibleDirection( wxBOTH );
    fgSizer114->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT36 = new wxStaticText( ID_PANEL1, wxID_ANY, wxT("Ver:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT36->Wrap( -1 );
    fgSizer114->Add( ID_STATICTEXT36, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    lblVER = new wxStaticText( ID_PANEL1, ID_VER, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblVER->Wrap( -1 );
    fgSizer114->Add( lblVER, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT38 = new wxStaticText( ID_PANEL1, wxID_ANY, wxT("Rev:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT38->Wrap( -1 );
    fgSizer114->Add( ID_STATICTEXT38, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    lblREV = new wxStaticText( ID_PANEL1, ID_REV, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblREV->Wrap( -1 );
    fgSizer114->Add( lblREV, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    ID_STATICTEXT40 = new wxStaticText( ID_PANEL1, wxID_ANY, wxT("Mask:"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT40->Wrap( -1 );
    fgSizer114->Add( ID_STATICTEXT40, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    lblMASK = new wxStaticText( ID_PANEL1, ID_MASK, wxT("???"), wxDefaultPosition, wxDefaultSize, 0 );
    lblMASK->Wrap( -1 );
    fgSizer114->Add( lblMASK, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
    
    
    fgSizer222->Add( fgSizer114, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    btnReadVerRevMask = new wxButton( ID_PANEL1, wxID_ANY, wxT("Read"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer222->Add( btnReadVerRevMask, 0, 0, 5 );
    
    
    fgMainSizer->Add( fgSizer222, 1, wxEXPAND, 5 );
    
    
    ID_PANEL1->SetSizer( fgMainSizer );
    ID_PANEL1->Layout();
    fgMainSizer->Fit( ID_PANEL1 );
    ID_NOTEBOOK1->AddPage( ID_PANEL1, wxT("Modes"), true );
    ID_PANEL2 = new wxPanel( ID_NOTEBOOK1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxFlexGridSizer* fgSizer115;
    fgSizer115 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer115->SetFlexibleDirection( wxBOTH );
    fgSizer115->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer224;
    fgSizer224 = new wxFlexGridSizer( 0, 3, 5, 5 );
    fgSizer224->SetFlexibleDirection( wxBOTH );
    fgSizer224->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer117;
    fgSizer117 = new wxFlexGridSizer( 0, 2, 5, 5 );
    fgSizer117->SetFlexibleDirection( wxBOTH );
    fgSizer117->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerSampleSource;
    sbSizerSampleSource = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Sample source") ), wxVERTICAL );
    
    wxStaticBoxSizer* sbSizer87;
    sbSizer87 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSampleSource->GetStaticBox(), wxID_ANY, wxT("When Port 1 is RF2BB") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer118;
    fgSizer118 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer118->AddGrowableCol( 1 );
    fgSizer118->SetFlexibleDirection( wxBOTH );
    fgSizer118->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT1 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 3"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT1->Wrap( -1 );
    fgSizer118->Add( ID_STATICTEXT1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_S3S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S3S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML1_S3S->Append( wxT("AI") );
    cmbLML1_S3S->Append( wxT("AQ") );
    cmbLML1_S3S->Append( wxT("BI") );
    cmbLML1_S3S->Append( wxT("BQ") );
    cmbLML1_S3S->SetSelection( 0 );
    cmbLML1_S3S->SetToolTip( wxT("Sample source in position 3, when Port 1 is TX") );
    
    fgSizer118->Add( cmbLML1_S3S, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT2 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 2"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT2->Wrap( -1 );
    fgSizer118->Add( ID_STATICTEXT2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_S2S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S2S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML1_S2S->Append( wxT("AI") );
    cmbLML1_S2S->Append( wxT("AQ") );
    cmbLML1_S2S->Append( wxT("BI") );
    cmbLML1_S2S->Append( wxT("BQ") );
    cmbLML1_S2S->SetSelection( 0 );
    cmbLML1_S2S->SetToolTip( wxT("Sample source in position 2, when Port 1 is TX") );
    
    fgSizer118->Add( cmbLML1_S2S, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT3 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 1"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT3->Wrap( -1 );
    fgSizer118->Add( ID_STATICTEXT3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_S1S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S1S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML1_S1S->Append( wxT("AI") );
    cmbLML1_S1S->Append( wxT("AQ") );
    cmbLML1_S1S->Append( wxT("BI") );
    cmbLML1_S1S->Append( wxT("BQ") );
    cmbLML1_S1S->SetSelection( 0 );
    cmbLML1_S1S->SetToolTip( wxT("Sample source in position 1, when Port 1 is TX") );
    
    fgSizer118->Add( cmbLML1_S1S, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT4 = new wxStaticText( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Position 0"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT4->Wrap( -1 );
    fgSizer118->Add( ID_STATICTEXT4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_S0S = new wxComboBox( sbSizer87->GetStaticBox(), ID_LML1_S0S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML1_S0S->Append( wxT("AI") );
    cmbLML1_S0S->Append( wxT("AQ") );
    cmbLML1_S0S->Append( wxT("BI") );
    cmbLML1_S0S->Append( wxT("BQ") );
    cmbLML1_S0S->SetSelection( 0 );
    cmbLML1_S0S->SetToolTip( wxT("Sample source in position 0, when Port 1 is TX") );
    
    fgSizer118->Add( cmbLML1_S0S, 0, wxEXPAND, 0 );
    
    
    sbSizer87->Add( fgSizer118, 0, wxEXPAND, 0 );
    
    
    sbSizerSampleSource->Add( sbSizer87, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer89;
    sbSizer89 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSampleSource->GetStaticBox(), wxID_ANY, wxT("When Port 2 is RF2BB") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer120;
    fgSizer120 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer120->AddGrowableCol( 1 );
    fgSizer120->SetFlexibleDirection( wxBOTH );
    fgSizer120->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT9 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 3"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT9->Wrap( -1 );
    fgSizer120->Add( ID_STATICTEXT9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_S3S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S3S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML2_S3S->Append( wxT("AI") );
    cmbLML2_S3S->Append( wxT("AQ") );
    cmbLML2_S3S->Append( wxT("BI") );
    cmbLML2_S3S->Append( wxT("BQ") );
    cmbLML2_S3S->SetSelection( 0 );
    cmbLML2_S3S->SetToolTip( wxT("Sample source in position 3, when Port 2 is TX") );
    
    fgSizer120->Add( cmbLML2_S3S, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT10 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 2"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT10->Wrap( -1 );
    fgSizer120->Add( ID_STATICTEXT10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_S2S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S2S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML2_S2S->Append( wxT("AI") );
    cmbLML2_S2S->Append( wxT("AQ") );
    cmbLML2_S2S->Append( wxT("BI") );
    cmbLML2_S2S->Append( wxT("BQ") );
    cmbLML2_S2S->SetSelection( 0 );
    cmbLML2_S2S->SetToolTip( wxT("Sample source in position 2, when Port 2 is TX") );
    
    fgSizer120->Add( cmbLML2_S2S, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT11 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 1"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT11->Wrap( -1 );
    fgSizer120->Add( ID_STATICTEXT11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_S1S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S1S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML2_S1S->Append( wxT("AI") );
    cmbLML2_S1S->Append( wxT("AQ") );
    cmbLML2_S1S->Append( wxT("BI") );
    cmbLML2_S1S->Append( wxT("BQ") );
    cmbLML2_S1S->SetSelection( 0 );
    cmbLML2_S1S->SetToolTip( wxT("Sample source in position 1, when Port 2 is TX") );
    
    fgSizer120->Add( cmbLML2_S1S, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT12 = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Position 0"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT12->Wrap( -1 );
    fgSizer120->Add( ID_STATICTEXT12, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_S0S = new wxComboBox( sbSizer89->GetStaticBox(), ID_LML2_S0S, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML2_S0S->Append( wxT("AI") );
    cmbLML2_S0S->Append( wxT("AQ") );
    cmbLML2_S0S->Append( wxT("BI") );
    cmbLML2_S0S->Append( wxT("BQ") );
    cmbLML2_S0S->SetSelection( 0 );
    cmbLML2_S0S->SetToolTip( wxT("Sample source in position 0, when Port 2 is TX") );
    
    fgSizer120->Add( cmbLML2_S0S, 0, wxEXPAND, 0 );
    
    
    sbSizer89->Add( fgSizer120, 0, wxEXPAND, 0 );
    
    
    sbSizerSampleSource->Add( sbSizer89, 0, wxEXPAND, 5 );
    
    
    fgSizer117->Add( sbSizerSampleSource, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerSamplePositions;
    sbSizerSamplePositions = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Sample positions") ), wxVERTICAL );
    
    wxStaticBoxSizer* sbSizer88;
    sbSizer88 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSamplePositions->GetStaticBox(), wxID_ANY, wxT("When Port 1 is BB2RF") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer119;
    fgSizer119 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer119->SetFlexibleDirection( wxBOTH );
    fgSizer119->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT5 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("BQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT5->Wrap( -1 );
    fgSizer119->Add( ID_STATICTEXT5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_BQP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_BQP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML1_BQP->Append( wxT("position 0") );
    cmbLML1_BQP->Append( wxT("position 1") );
    cmbLML1_BQP->Append( wxT("position 2") );
    cmbLML1_BQP->Append( wxT("position 3") );
    cmbLML1_BQP->SetToolTip( wxT("BQ sample position in frame, when Port 1 is RX") );
    
    fgSizer119->Add( cmbLML1_BQP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT6 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("BI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT6->Wrap( -1 );
    fgSizer119->Add( ID_STATICTEXT6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_BIP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_BIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
    cmbLML1_BIP->Append( wxT("position 0") );
    cmbLML1_BIP->Append( wxT("position 1") );
    cmbLML1_BIP->Append( wxT("position 2") );
    cmbLML1_BIP->Append( wxT("position 3") );
    cmbLML1_BIP->SetToolTip( wxT("BI sample position in frame, when Port 1 is RX") );
    
    fgSizer119->Add( cmbLML1_BIP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT7 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("AQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT7->Wrap( -1 );
    fgSizer119->Add( ID_STATICTEXT7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_AQP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_AQP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
    cmbLML1_AQP->Append( wxT("position 0") );
    cmbLML1_AQP->Append( wxT("position 1") );
    cmbLML1_AQP->Append( wxT("position 2") );
    cmbLML1_AQP->Append( wxT("position 3") );
    cmbLML1_AQP->SetToolTip( wxT("AQ sample position in frame, when Port 1 is RX") );
    
    fgSizer119->Add( cmbLML1_AQP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT8 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("AI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT8->Wrap( -1 );
    fgSizer119->Add( ID_STATICTEXT8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_AIP = new wxComboBox( sbSizer88->GetStaticBox(), ID_LML1_AIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
    cmbLML1_AIP->Append( wxT("position 0") );
    cmbLML1_AIP->Append( wxT("position 1") );
    cmbLML1_AIP->Append( wxT("position 2") );
    cmbLML1_AIP->Append( wxT("position 3") );
    cmbLML1_AIP->SetToolTip( wxT("AI sample position in frame, when Port 1 is RX") );
    
    fgSizer119->Add( cmbLML1_AIP, 0, wxEXPAND, 0 );
    
    
    sbSizer88->Add( fgSizer119, 0, wxEXPAND, 0 );
    
    
    sbSizerSamplePositions->Add( sbSizer88, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer90;
    sbSizer90 = new wxStaticBoxSizer( new wxStaticBox( sbSizerSamplePositions->GetStaticBox(), wxID_ANY, wxT("When Port 2 is BB2RF") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer121;
    fgSizer121 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer121->SetFlexibleDirection( wxBOTH );
    fgSizer121->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT13 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("BQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT13->Wrap( -1 );
    fgSizer121->Add( ID_STATICTEXT13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_BQP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_BQP, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLML2_BQP->Append( wxT("position 0") );
    cmbLML2_BQP->Append( wxT("position 1") );
    cmbLML2_BQP->Append( wxT("position 2") );
    cmbLML2_BQP->Append( wxT("position 3") );
    cmbLML2_BQP->SetToolTip( wxT("BQ sample position in frame, when Port 2 is RX") );
    
    fgSizer121->Add( cmbLML2_BQP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT14 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("BI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT14->Wrap( -1 );
    fgSizer121->Add( ID_STATICTEXT14, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_BIP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_BIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
    cmbLML2_BIP->Append( wxT("position 0") );
    cmbLML2_BIP->Append( wxT("position 1") );
    cmbLML2_BIP->Append( wxT("position 2") );
    cmbLML2_BIP->Append( wxT("position 3") );
    cmbLML2_BIP->SetToolTip( wxT("BI sample position in frame, when Port 2is RX") );
    
    fgSizer121->Add( cmbLML2_BIP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT15 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("AQ sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT15->Wrap( -1 );
    fgSizer121->Add( ID_STATICTEXT15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_AQP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_AQP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
    cmbLML2_AQP->Append( wxT("position 0") );
    cmbLML2_AQP->Append( wxT("position 1") );
    cmbLML2_AQP->Append( wxT("position 2") );
    cmbLML2_AQP->Append( wxT("position 3") );
    cmbLML2_AQP->SetToolTip( wxT("AQ sample position in frame, when Port 2 is RX") );
    
    fgSizer121->Add( cmbLML2_AQP, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT16 = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("AI sample position"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT16->Wrap( -1 );
    fgSizer121->Add( ID_STATICTEXT16, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_AIP = new wxComboBox( sbSizer90->GetStaticBox(), ID_LML2_AIP, wxEmptyString, wxDefaultPosition, wxSize( 85,-1 ), 0, NULL, 0 );
    cmbLML2_AIP->Append( wxT("position 0") );
    cmbLML2_AIP->Append( wxT("position 1") );
    cmbLML2_AIP->Append( wxT("position 2") );
    cmbLML2_AIP->Append( wxT("position 3") );
    cmbLML2_AIP->SetToolTip( wxT("AI sample position in frame, when Port 2 is RX") );
    
    fgSizer121->Add( cmbLML2_AIP, 0, wxEXPAND, 0 );
    
    
    sbSizer90->Add( fgSizer121, 0, wxEXPAND, 0 );
    
    
    sbSizerSamplePositions->Add( sbSizer90, 1, wxEXPAND, 5 );
    
    
    fgSizer117->Add( sbSizerSamplePositions, 1, wxEXPAND, 5 );
    
    
    fgSizer224->Add( fgSizer117, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxStaticBoxSizer* sbSizerClockControls;
    sbSizerClockControls = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Clock controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer122;
    fgSizer122 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer122->SetFlexibleDirection( wxBOTH );
    fgSizer122->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT25 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("TX FIFO read clock source"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT25->Wrap( -1 );
    fgSizer122->Add( ID_STATICTEXT25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbTXRDCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_TXRDCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbTXRDCLK_MUX->Append( wxT("FCLK1") );
    cmbTXRDCLK_MUX->Append( wxT("FCLK2") );
    cmbTXRDCLK_MUX->Append( wxT("TxTSPCLK") );
    cmbTXRDCLK_MUX->SetToolTip( wxT("TX FIFO read clock selection") );
    
    fgSizer122->Add( cmbTXRDCLK_MUX, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT26 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("Tx FIFO write clock source"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT26->Wrap( -1 );
    fgSizer122->Add( ID_STATICTEXT26, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbTXWRCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_TXWRCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbTXWRCLK_MUX->Append( wxT("FCLK1") );
    cmbTXWRCLK_MUX->Append( wxT("FCLK2") );
    cmbTXWRCLK_MUX->Append( wxT("RxTSPCLK") );
    cmbTXWRCLK_MUX->SetToolTip( wxT("TX FIFO write clock selection") );
    
    fgSizer122->Add( cmbTXWRCLK_MUX, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT27 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("Rx FIFO read clock source"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT27->Wrap( -1 );
    fgSizer122->Add( ID_STATICTEXT27, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRXRDCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_RXRDCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbRXRDCLK_MUX->Append( wxT("MCLK1") );
    cmbRXRDCLK_MUX->Append( wxT("MCLK2") );
    cmbRXRDCLK_MUX->Append( wxT("FCLK1") );
    cmbRXRDCLK_MUX->Append( wxT("FCLK2") );
    cmbRXRDCLK_MUX->SetToolTip( wxT("RX FIFO read clock selection") );
    
    fgSizer122->Add( cmbRXRDCLK_MUX, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT28 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("Rx FIFO write clock source"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT28->Wrap( -1 );
    fgSizer122->Add( ID_STATICTEXT28, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRXWRCLK_MUX = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_RXWRCLK_MUX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbRXWRCLK_MUX->Append( wxT("FCLK1") );
    cmbRXWRCLK_MUX->Append( wxT("FCLK2") );
    cmbRXWRCLK_MUX->Append( wxT("RxTSPCLK") );
    cmbRXWRCLK_MUX->SetToolTip( wxT("RX FIFO write clock selection") );
    
    fgSizer122->Add( cmbRXWRCLK_MUX, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT29 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK2 clock source"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT29->Wrap( -1 );
    fgSizer122->Add( ID_STATICTEXT29, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbMCLK2SRC = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK2SRC, wxEmptyString, wxDefaultPosition, wxSize( 154,-1 ), 0, NULL, 0 );
    cmbMCLK2SRC->Append( wxT("TxTSPCLKA after divider") );
    cmbMCLK2SRC->Append( wxT("RxTSPCLKA after divider") );
    cmbMCLK2SRC->Append( wxT("TxTSPCLKA") );
    cmbMCLK2SRC->Append( wxT("RxTSPCLKA") );
    cmbMCLK2SRC->SetToolTip( wxT("MCLK2 clock source") );
    
    fgSizer122->Add( cmbMCLK2SRC, 0, wxEXPAND, 0 );
    
    ID_STATICTEXT30 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK1 clock source"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT30->Wrap( -1 );
    fgSizer122->Add( ID_STATICTEXT30, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbMCLK1SRC = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK1SRC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbMCLK1SRC->Append( wxT("TxTSPCLKA after divider") );
    cmbMCLK1SRC->Append( wxT("RxTSPCLKA after divider") );
    cmbMCLK1SRC->Append( wxT("TxTSPCLKA") );
    cmbMCLK1SRC->Append( wxT("RxTSPCLKA") );
    cmbMCLK1SRC->SetToolTip( wxT("MCLK1 clock source") );
    
    fgSizer122->Add( cmbMCLK1SRC, 0, wxEXPAND, 0 );
    
    chkTXDIVEN = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_TXDIVEN, wxT("Enable Tx clock divider"), wxDefaultPosition, wxDefaultSize, 0 );
    chkTXDIVEN->SetToolTip( wxT("TX clock divider enable") );
    
    fgSizer122->Add( chkTXDIVEN, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbTXTSPCLKA_DIV = new wxSpinCtrl( sbSizerClockControls->GetStaticBox(), ID_TXTSPCLKA_DIV, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    fgSizer122->Add( cmbTXTSPCLKA_DIV, 0, wxEXPAND, 5 );
    
    chkRXDIVEN = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_RXDIVEN, wxT("Enable Rx clock divider"), wxDefaultPosition, wxDefaultSize, 0 );
    chkRXDIVEN->SetToolTip( wxT("RX clock divider enable") );
    
    fgSizer122->Add( chkRXDIVEN, 0, wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbRXTSPCLKA_DIV = new wxSpinCtrl( sbSizerClockControls->GetStaticBox(), ID_RXTSPCLKA_DIV, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    fgSizer122->Add( cmbRXTSPCLKA_DIV, 0, wxEXPAND, 5 );
    
    chkFCLK1_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK1_INV, wxT("FCLK1 invert"), wxDefaultPosition, wxDefaultSize, 0 );
    chkFCLK1_INV->SetToolTip( wxT("FCLK1 clock inversion") );
    
    fgSizer122->Add( chkFCLK1_INV, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    chkFCLK2_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2_INV, wxT("FCLK2 invert"), wxDefaultPosition, wxDefaultSize, 0 );
    chkFCLK2_INV->SetToolTip( wxT("FCLK2 clock inversion") );
    
    fgSizer122->Add( chkFCLK2_INV, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    chkMCLK1_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2_INV, wxT("MCLK1 invert"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMCLK1_INV->SetToolTip( wxT("FCLK2 clock inversion") );
    
    fgSizer122->Add( chkMCLK1_INV, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
    
    chkMCLK2_INV = new wxCheckBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2_INV, wxT("MCLK2 invert"), wxDefaultPosition, wxDefaultSize, 0 );
    chkMCLK2_INV->SetToolTip( wxT("FCLK2 clock inversion") );
    
    fgSizer122->Add( chkMCLK2_INV, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
    
    
    sbSizerClockControls->Add( fgSizer122, 0, wxEXPAND, 0 );
    
    wxFlexGridSizer* fgSizer251;
    fgSizer251 = new wxFlexGridSizer( 0, 4, 0, 5 );
    fgSizer251->SetFlexibleDirection( wxBOTH );
    fgSizer251->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT301 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("FCLK1 delay"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT301->Wrap( -1 );
    fgSizer251->Add( ID_STATICTEXT301, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
    
    cmbFCLK1_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_FCLK1DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbFCLK1_DLY->Append( wxT("No delay") );
    cmbFCLK1_DLY->Append( wxT("1x delay") );
    cmbFCLK1_DLY->Append( wxT("2x delay") );
    cmbFCLK1_DLY->Append( wxT("3x delay") );
    cmbFCLK1_DLY->SetToolTip( wxT("FCLK1 clock source") );
    
    fgSizer251->Add( cmbFCLK1_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
    
    ID_STATICTEXT302 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("FCLK2 delay"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT302->Wrap( -1 );
    fgSizer251->Add( ID_STATICTEXT302, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
    
    cmbFCLK2_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_FCLK2DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbFCLK2_DLY->Append( wxT("No delay") );
    cmbFCLK2_DLY->Append( wxT("1x delay") );
    cmbFCLK2_DLY->Append( wxT("2x delay") );
    cmbFCLK2_DLY->Append( wxT("3x delay") );
    cmbFCLK2_DLY->SetToolTip( wxT("FCLK2 clock source") );
    
    fgSizer251->Add( cmbFCLK2_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
    
    ID_STATICTEXT303 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK1 delay"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT303->Wrap( -1 );
    fgSizer251->Add( ID_STATICTEXT303, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
    
    cmbMCLK1_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK1DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbMCLK1_DLY->Append( wxT("No delay") );
    cmbMCLK1_DLY->Append( wxT("1x delay") );
    cmbMCLK1_DLY->Append( wxT("2x delay") );
    cmbMCLK1_DLY->Append( wxT("3x delay") );
    cmbMCLK1_DLY->SetToolTip( wxT("MCLK1 clock source") );
    
    fgSizer251->Add( cmbMCLK1_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
    
    ID_STATICTEXT304 = new wxStaticText( sbSizerClockControls->GetStaticBox(), wxID_ANY, wxT("MCLK2 delay"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT304->Wrap( -1 );
    fgSizer251->Add( ID_STATICTEXT304, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 0 );
    
    cmbMCLK2_DLY = new wxComboBox( sbSizerClockControls->GetStaticBox(), ID_MCLK2DLY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbMCLK2_DLY->Append( wxT("No delay") );
    cmbMCLK2_DLY->Append( wxT("1x delay") );
    cmbMCLK2_DLY->Append( wxT("2x delay") );
    cmbMCLK2_DLY->Append( wxT("3x delay") );
    cmbMCLK2_DLY->SetToolTip( wxT("MCLK2 clock source") );
    
    fgSizer251->Add( cmbMCLK2_DLY, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
    
    
    sbSizerClockControls->Add( fgSizer251, 1, wxEXPAND, 0 );
    
    
    fgSizer224->Add( sbSizerClockControls, 1, wxALIGN_LEFT|wxALIGN_TOP, 0 );
    
    wxFlexGridSizer* fgSizer124;
    fgSizer124 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer124->AddGrowableRow( 0 );
    fgSizer124->AddGrowableRow( 1 );
    fgSizer124->SetFlexibleDirection( wxBOTH );
    fgSizer124->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerClockCyclesToWaitLML1;
    sbSizerClockCyclesToWaitLML1 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("LML1 Clock cycles to wait before") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer125;
    fgSizer125 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer125->AddGrowableRow( 0 );
    fgSizer125->AddGrowableRow( 1 );
    fgSizer125->AddGrowableRow( 2 );
    fgSizer125->AddGrowableRow( 3 );
    fgSizer125->SetFlexibleDirection( wxBOTH );
    fgSizer125->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT17 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data drive stop"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT17->Wrap( -1 );
    fgSizer125->Add( ID_STATICTEXT17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_TX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_TX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML1_TX_PST->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer125->Add( cmbLML1_TX_PST, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT18 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data drive start"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT18->Wrap( -1 );
    fgSizer125->Add( ID_STATICTEXT18, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_TX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_TX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML1_TX_PRE->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer125->Add( cmbLML1_TX_PRE, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT19 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data capture stop"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT19->Wrap( -1 );
    fgSizer125->Add( ID_STATICTEXT19, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_RX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_RX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML1_RX_PST->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer125->Add( cmbLML1_RX_PST, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT20 = new wxStaticText( sbSizerClockCyclesToWaitLML1->GetStaticBox(), wxID_ANY, wxT("Data capture start"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT20->Wrap( -1 );
    fgSizer125->Add( ID_STATICTEXT20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML1_RX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML1->GetStaticBox(), ID_LML1_RX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML1_RX_PRE->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer125->Add( cmbLML1_RX_PRE, 0, wxEXPAND, 5 );
    
    
    sbSizerClockCyclesToWaitLML1->Add( fgSizer125, 1, wxEXPAND, 0 );
    
    
    fgSizer124->Add( sbSizerClockCyclesToWaitLML1, 1, wxALIGN_TOP, 5 );
    
    wxStaticBoxSizer* sbSizerClockCyclesToWaitLML2;
    sbSizerClockCyclesToWaitLML2 = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("LML2 Clock cycles to wait before") ), wxHORIZONTAL );
    
    wxFlexGridSizer* fgSizer126;
    fgSizer126 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer126->AddGrowableRow( 0 );
    fgSizer126->AddGrowableRow( 1 );
    fgSizer126->AddGrowableRow( 2 );
    fgSizer126->AddGrowableRow( 3 );
    fgSizer126->SetFlexibleDirection( wxBOTH );
    fgSizer126->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_STATICTEXT21 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data drive stop"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT21->Wrap( -1 );
    fgSizer126->Add( ID_STATICTEXT21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_TX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_TX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML2_TX_PST->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer126->Add( cmbLML2_TX_PST, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT22 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data drive start"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT22->Wrap( -1 );
    fgSizer126->Add( ID_STATICTEXT22, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_TX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_TX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML2_TX_PRE->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer126->Add( cmbLML2_TX_PRE, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT23 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data capture stop"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT23->Wrap( -1 );
    fgSizer126->Add( ID_STATICTEXT23, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_RX_PST = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_RX_PST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML2_RX_PST->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer126->Add( cmbLML2_RX_PST, 0, wxEXPAND, 5 );
    
    ID_STATICTEXT24 = new wxStaticText( sbSizerClockCyclesToWaitLML2->GetStaticBox(), wxID_ANY, wxT("Data capture start"), wxDefaultPosition, wxDefaultSize, 0 );
    ID_STATICTEXT24->Wrap( -1 );
    fgSizer126->Add( ID_STATICTEXT24, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbLML2_RX_PRE = new wxSpinCtrl( sbSizerClockCyclesToWaitLML2->GetStaticBox(), ID_LML2_RX_PRE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
    //cmbLML2_RX_PRE->SetMinSize( wxSize( 48,-1 ) );
    
    fgSizer126->Add( cmbLML2_RX_PRE, 0, wxEXPAND, 5 );
    
    
    sbSizerClockCyclesToWaitLML2->Add( fgSizer126, 0, wxEXPAND, 0 );
    
    
    fgSizer124->Add( sbSizerClockCyclesToWaitLML2, 1, wxEXPAND, 5 );
    
    
    fgSizer224->Add( fgSizer124, 1, wxALIGN_LEFT|wxALIGN_TOP, 15 );
    
    
    fgSizer115->Add( fgSizer224, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerDirectionControls;
    sbSizerDirectionControls = new wxStaticBoxSizer( new wxStaticBox( ID_PANEL2, wxID_ANY, wxT("Direction controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer123;
    fgSizer123 = new wxFlexGridSizer( 0, 8, 0, 5 );
    fgSizer123->SetFlexibleDirection( wxBOTH );
    fgSizer123->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxString rgrDIQDIRCTR1Choices[] = { wxT("Automatic"), wxT("Manual") };
    int rgrDIQDIRCTR1NChoices = sizeof( rgrDIQDIRCTR1Choices ) / sizeof( wxString );
    rgrDIQDIRCTR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIRCTR1, wxT("DIQ1 mode"), wxDefaultPosition, wxDefaultSize, rgrDIQDIRCTR1NChoices, rgrDIQDIRCTR1Choices, 1, wxRA_SPECIFY_COLS );
    rgrDIQDIRCTR1->SetSelection( 0 );
    rgrDIQDIRCTR1->SetToolTip( wxT("DIQ1 direction control mode") );
    
    fgSizer123->Add( rgrDIQDIRCTR1, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxString rgrDIQDIR1Choices[] = { wxT("Output"), wxT("Input") };
    int rgrDIQDIR1NChoices = sizeof( rgrDIQDIR1Choices ) / sizeof( wxString );
    rgrDIQDIR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIR1, wxT("DIQ1 direction"), wxDefaultPosition, wxDefaultSize, rgrDIQDIR1NChoices, rgrDIQDIR1Choices, 1, wxRA_SPECIFY_COLS );
    rgrDIQDIR1->SetSelection( 0 );
    rgrDIQDIR1->SetToolTip( wxT("DIQ1 direction") );
    
    fgSizer123->Add( rgrDIQDIR1, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxString rgrENABLEDIRCTR1Choices[] = { wxT("Automatic"), wxT("Manual") };
    int rgrENABLEDIRCTR1NChoices = sizeof( rgrENABLEDIRCTR1Choices ) / sizeof( wxString );
    rgrENABLEDIRCTR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIRCTR1, wxT("ENABLE1 mode"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIRCTR1NChoices, rgrENABLEDIRCTR1Choices, 1, wxRA_SPECIFY_COLS );
    rgrENABLEDIRCTR1->SetSelection( 0 );
    rgrENABLEDIRCTR1->SetToolTip( wxT("ENABLE1 direction control mode") );
    
    fgSizer123->Add( rgrENABLEDIRCTR1, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxString rgrENABLEDIR1Choices[] = { wxT("Output"), wxT("Input") };
    int rgrENABLEDIR1NChoices = sizeof( rgrENABLEDIR1Choices ) / sizeof( wxString );
    rgrENABLEDIR1 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIR1, wxT("ENABLE1 direction"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIR1NChoices, rgrENABLEDIR1Choices, 1, wxRA_SPECIFY_COLS );
    rgrENABLEDIR1->SetSelection( 0 );
    rgrENABLEDIR1->SetToolTip( wxT("ENABLE1 direction.") );
    
    fgSizer123->Add( rgrENABLEDIR1, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxString rgrDIQDIRCTR2Choices[] = { wxT("Automatic"), wxT("Manual") };
    int rgrDIQDIRCTR2NChoices = sizeof( rgrDIQDIRCTR2Choices ) / sizeof( wxString );
    rgrDIQDIRCTR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIRCTR2, wxT("DIQ2 mode"), wxDefaultPosition, wxDefaultSize, rgrDIQDIRCTR2NChoices, rgrDIQDIRCTR2Choices, 1, wxRA_SPECIFY_COLS );
    rgrDIQDIRCTR2->SetSelection( 0 );
    rgrDIQDIRCTR2->SetToolTip( wxT("DIQ2 direction control mode") );
    
    fgSizer123->Add( rgrDIQDIRCTR2, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxString rgrDIQDIR2Choices[] = { wxT("Output"), wxT("Input") };
    int rgrDIQDIR2NChoices = sizeof( rgrDIQDIR2Choices ) / sizeof( wxString );
    rgrDIQDIR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_DIQDIR2, wxT("DIQ2 direction"), wxDefaultPosition, wxDefaultSize, rgrDIQDIR2NChoices, rgrDIQDIR2Choices, 1, wxRA_SPECIFY_COLS );
    rgrDIQDIR2->SetSelection( 0 );
    rgrDIQDIR2->SetToolTip( wxT("DIQ2 direction") );
    
    fgSizer123->Add( rgrDIQDIR2, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxString rgrENABLEDIRCTR2Choices[] = { wxT("Automatic"), wxT("Manual") };
    int rgrENABLEDIRCTR2NChoices = sizeof( rgrENABLEDIRCTR2Choices ) / sizeof( wxString );
    rgrENABLEDIRCTR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIRCTR2, wxT("ENABLE2 mode"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIRCTR2NChoices, rgrENABLEDIRCTR2Choices, 1, wxRA_SPECIFY_COLS );
    rgrENABLEDIRCTR2->SetSelection( 0 );
    rgrENABLEDIRCTR2->SetToolTip( wxT("ENABLE2 direction control mode") );
    
    fgSizer123->Add( rgrENABLEDIRCTR2, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    wxString rgrENABLEDIR2Choices[] = { wxT("Output"), wxT("Input") };
    int rgrENABLEDIR2NChoices = sizeof( rgrENABLEDIR2Choices ) / sizeof( wxString );
    rgrENABLEDIR2 = new wxRadioBox( sbSizerDirectionControls->GetStaticBox(), ID_ENABLEDIR2, wxT("ENABLE2 direction"), wxDefaultPosition, wxDefaultSize, rgrENABLEDIR2NChoices, rgrENABLEDIR2Choices, 1, wxRA_SPECIFY_COLS );
    rgrENABLEDIR2->SetSelection( 1 );
    rgrENABLEDIR2->SetToolTip( wxT("ENABLE2 direction") );
    
    fgSizer123->Add( rgrENABLEDIR2, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    sbSizerDirectionControls->Add( fgSizer123, 1, wxEXPAND, 5 );
    
    
    fgSizer115->Add( sbSizerDirectionControls, 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    ID_PANEL2->SetSizer( fgSizer115 );
    ID_PANEL2->Layout();
    fgSizer115->Fit( ID_PANEL2 );
    ID_NOTEBOOK1->AddPage( ID_PANEL2, wxT("Sample position && Clock"), false );
    
    fgSizer105->Add( ID_NOTEBOOK1, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( fgSizer105 );
    this->Layout();
    fgSizer105->Fit( this );
    
    // Connect Events
    chkSDA_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkSCL_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkRX_CLK_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkSDIO_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkSDO_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkTX_CLK_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkSEN_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkDIQ1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkTXNRX1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkSCLK_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkDIQ2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkTXNRX2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkIQ_SEL_EN_1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkFCLK1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMCLK1_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkIQ_SEL_EN_2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkFCLK2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMCLK2_PE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkSRST_RXFIFO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkSRST_TXFIFO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLRST_TX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLRST_TX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLRST_RX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLRST_RX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMRST_TX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMRST_TX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMRST_RX_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMRST_RX_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkRXEN_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkTXEN_A->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkRXEN_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkTXEN_B->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMIMO_SISO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMOD_EN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrLML_FIDM1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrLML_FIDM2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrLML_MODE1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrLML_MODE2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrLML_TXNRXIQ1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrLML_TXNRXIQ2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLML1_TRXIQPULSE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLML2_TRXIQPULSE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLML1_SISODDR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkLML2_SISODDR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrSDA_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrSCL_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrSDIO_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrDIQ2_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrDIQ1_DS->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrRX_MUX->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrTX_MUX->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrSPIMODE->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    btnReadVerRevMask->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::onbtnReadVerRevMask ), NULL, this );
    cmbLML1_S3S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_S2S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_S1S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_S0S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_S3S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_S2S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_S1S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_S0S->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_BQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_BIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_AQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_AIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_BQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_BIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_AQP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_AIP->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbTXRDCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbTXWRCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbRXRDCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbRXWRCLK_MUX->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbMCLK2SRC->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbMCLK1SRC->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkTXDIVEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbTXTSPCLKA_DIV->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkRXDIVEN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbRXTSPCLKA_DIV->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkFCLK1_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkFCLK2_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMCLK1_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    chkMCLK2_INV->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbFCLK1_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbFCLK2_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbMCLK1_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbMCLK2_DLY->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_TX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_TX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_RX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML1_RX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_TX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_TX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_RX_PST->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    cmbLML2_RX_PRE->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrDIQDIRCTR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrDIQDIR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrENABLEDIRCTR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrENABLEDIR1->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrDIQDIRCTR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrDIQDIR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrENABLEDIRCTR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );
    rgrENABLEDIR2->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( lms7002_pnlLimeLightPAD_view::ParameterChangeHandler ), NULL, this );

    //ids for updating from chip
    wndId2Enum[rgrDIQ1_DS] = LMS7param(DIQ1_DS);
    wndId2Enum[chkDIQ1_PE] = LMS7param(DIQ1_PE);
    wndId2Enum[rgrDIQ2_DS] = LMS7param(DIQ2_DS);
    wndId2Enum[chkDIQ2_PE] = LMS7param(DIQ2_PE);
    wndId2Enum[rgrRX_MUX] = LMS7param(RX_MUX);
    wndId2Enum[chkFCLK1_PE] = LMS7param(FCLK1_PE);
    wndId2Enum[chkFCLK2_PE] = LMS7param(FCLK2_PE);
    wndId2Enum[chkIQ_SEL_EN_1_PE] = LMS7param(IQ_SEL_EN_1_PE);
    wndId2Enum[chkIQ_SEL_EN_2_PE] = LMS7param(IQ_SEL_EN_2_PE);
    wndId2Enum[cmbLML1_AIP] = LMS7param(LML1_AIP);
    wndId2Enum[cmbLML1_AQP] = LMS7param(LML1_AQP);
    wndId2Enum[cmbLML1_BIP] = LMS7param(LML1_BIP);
    wndId2Enum[cmbLML1_BQP] = LMS7param(LML1_BQP);
    wndId2Enum[cmbLML1_RX_PRE] = LMS7param(LML1_RF2BB_PRE);
    wndId2Enum[cmbLML1_RX_PST] = LMS7param(LML1_RF2BB_PST);
    wndId2Enum[cmbLML1_S0S] = LMS7param(LML1_S0S);
    wndId2Enum[cmbLML1_S1S] = LMS7param(LML1_S1S);
    wndId2Enum[cmbLML1_S2S] = LMS7param(LML1_S2S);
    wndId2Enum[cmbLML1_S3S] = LMS7param(LML1_S3S);
    wndId2Enum[cmbLML1_TX_PRE] = LMS7param(LML1_BB2RF_PRE);
    wndId2Enum[cmbLML1_TX_PST] = LMS7param(LML1_BB2RF_PST);
    wndId2Enum[cmbLML2_AIP] = LMS7param(LML2_AIP);
    wndId2Enum[cmbLML2_AQP] = LMS7param(LML2_AQP);
    wndId2Enum[cmbLML2_BIP] = LMS7param(LML2_BIP);
    wndId2Enum[cmbLML2_BQP] = LMS7param(LML2_BQP);
    wndId2Enum[cmbLML2_RX_PRE] = LMS7param(LML2_RF2BB_PRE);
    wndId2Enum[cmbLML2_RX_PST] = LMS7param(LML2_RF2BB_PST);
    wndId2Enum[cmbLML2_S0S] = LMS7param(LML2_S0S);
    wndId2Enum[cmbLML2_S1S] = LMS7param(LML2_S1S);
    wndId2Enum[cmbLML2_S2S] = LMS7param(LML2_S2S);
    wndId2Enum[cmbLML2_S3S] = LMS7param(LML2_S3S);
    wndId2Enum[cmbLML2_TX_PRE] = LMS7param(LML2_BB2RF_PRE);
    wndId2Enum[cmbLML2_TX_PST] = LMS7param(LML2_BB2RF_PST);
    wndId2Enum[rgrLML_MODE1] = LMS7param(LML1_MODE);
    wndId2Enum[rgrLML_MODE2] = LMS7param(LML2_MODE);
    wndId2Enum[rgrLML_TXNRXIQ1] = LMS7param(LML1_TXNRXIQ);
    wndId2Enum[rgrLML_TXNRXIQ2] = LMS7param(LML2_TXNRXIQ);
    wndId2Enum[chkLRST_RX_A] = LMS7param(LRST_RX_A);
    wndId2Enum[chkLRST_RX_B] = LMS7param(LRST_RX_B);
    wndId2Enum[chkLRST_TX_A] = LMS7param(LRST_TX_A);
    wndId2Enum[chkLRST_TX_B] = LMS7param(LRST_TX_B);
    wndId2Enum[cmbMCLK1SRC] = LMS7param(MCLK1SRC);
    wndId2Enum[cmbRXTSPCLKA_DIV] = LMS7param(RXTSPCLKA_DIV);
    wndId2Enum[chkMCLK1_PE] = LMS7param(MCLK1_PE);
    wndId2Enum[cmbMCLK2SRC] = LMS7param(MCLK2SRC);
    wndId2Enum[cmbTXTSPCLKA_DIV] = LMS7param(TXTSPCLKA_DIV);
    wndId2Enum[chkMCLK2_PE] = LMS7param(MCLK2_PE);
    wndId2Enum[chkMIMO_SISO] = LMS7param(MIMO_SISO);
    wndId2Enum[chkMOD_EN] = LMS7param(MOD_EN);
    wndId2Enum[chkMRST_RX_A] = LMS7param(MRST_RX_A);
    wndId2Enum[chkMRST_RX_B] = LMS7param(MRST_RX_B);
    wndId2Enum[chkMRST_TX_A] = LMS7param(MRST_TX_A);
    wndId2Enum[chkMRST_TX_B] = LMS7param(MRST_TX_B);
    wndId2Enum[chkRXDIVEN] = LMS7param(RXDIVEN);
    wndId2Enum[chkRXEN_A] = LMS7param(RXEN_A);
    wndId2Enum[chkRXEN_B] = LMS7param(RXEN_B);
    wndId2Enum[cmbRXRDCLK_MUX] = LMS7param(RXRDCLK_MUX);
    wndId2Enum[cmbRXWRCLK_MUX] = LMS7param(RXWRCLK_MUX);
    wndId2Enum[chkRX_CLK_PE] = LMS7param(RX_CLK_PE);
    wndId2Enum[chkSCLK_PE] = LMS7param(SCLK_PE);
    wndId2Enum[rgrSCL_DS] = LMS7param(SCL_DS);
    wndId2Enum[chkSCL_PE] = LMS7param(SCL_PE);
    wndId2Enum[rgrSDA_DS] = LMS7param(SDA_DS);
    wndId2Enum[chkSDA_PE] = LMS7param(SDA_PE);
    wndId2Enum[rgrSDIO_DS] = LMS7param(SDIO_DS);
    wndId2Enum[chkSDIO_PE] = LMS7param(SDIO_PE);
    wndId2Enum[chkSDO_PE] = LMS7param(SDO_PE);
    wndId2Enum[chkSEN_PE] = LMS7param(SEN_PE);
    wndId2Enum[rgrSPIMODE] = LMS7param(SPIMODE);
    wndId2Enum[chkSRST_RXFIFO] = LMS7param(SRST_RXFIFO);
    wndId2Enum[chkSRST_TXFIFO] = LMS7param(SRST_TXFIFO);
    wndId2Enum[chkTXDIVEN] = LMS7param(TXDIVEN);
    wndId2Enum[chkTXEN_A] = LMS7param(TXEN_A);
    wndId2Enum[chkTXEN_B] = LMS7param(TXEN_B);
    wndId2Enum[rgrTX_MUX] = LMS7param(TX_MUX);
    wndId2Enum[chkTXNRX1_PE] = LMS7param(TXNRX1_PE);
    wndId2Enum[chkTXNRX2_PE] = LMS7param(TXNRX2_PE);
    wndId2Enum[cmbTXRDCLK_MUX] = LMS7param(TXRDCLK_MUX);
    wndId2Enum[cmbTXWRCLK_MUX] = LMS7param(TXWRCLK_MUX);
    wndId2Enum[chkTX_CLK_PE] = LMS7param(TX_CLK_PE);
    wndId2Enum[rgrLML_FIDM1] = LMS7param(LML1_FIDM);
    wndId2Enum[rgrLML_FIDM2] = LMS7param(LML2_FIDM);
    wndId2Enum[chkFCLK1_INV] = LMS7param(FCLK1_INV);
    wndId2Enum[chkFCLK2_INV] = LMS7param(FCLK2_INV);
    wndId2Enum[rgrDIQDIRCTR2] = LMS7param(DIQDIRCTR2);
    wndId2Enum[rgrDIQDIR2] = LMS7param(DIQDIR2);
    wndId2Enum[rgrDIQDIRCTR1] = LMS7param(DIQDIRCTR1);
    wndId2Enum[rgrDIQDIR1] = LMS7param(DIQDIR1);
    wndId2Enum[rgrENABLEDIRCTR2] = LMS7param(ENABLEDIRCTR2);
    wndId2Enum[rgrENABLEDIR2] = LMS7param(ENABLEDIR2);
    wndId2Enum[rgrENABLEDIRCTR1] = LMS7param(ENABLEDIRCTR1);
    wndId2Enum[rgrENABLEDIR1] = LMS7param(ENABLEDIR1);
    wndId2Enum[chkMCLK1_INV] = LMS7param(MCLK1_INV);
    wndId2Enum[chkMCLK2_INV] = LMS7param(MCLK2_INV);
    wndId2Enum[cmbFCLK1_DLY] = LMS7param(FCLK1_DLY);
    wndId2Enum[cmbFCLK2_DLY] = LMS7param(FCLK2_DLY);
    wndId2Enum[cmbMCLK1_DLY] = LMS7param(MCLK1_DLY);
    wndId2Enum[cmbMCLK2_DLY] = LMS7param(MCLK2_DLY);
    wndId2Enum[chkLML1_TRXIQPULSE] = LMS7param(LML1_TRXIQPULSE);
    wndId2Enum[chkLML2_TRXIQPULSE] = LMS7param(LML2_TRXIQPULSE);
    wndId2Enum[chkLML1_SISODDR] = LMS7param(LML1_SISODDR);
    wndId2Enum[chkLML2_SISODDR] = LMS7param(LML2_SISODDR);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlLimeLightPAD_view::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    assert(lmsControl != nullptr);
    uint16_t value;
    if (LMS_ReadParam(lmsControl,LMS7param(MASK),&value)!=0  || value != 0)
         value = 1;
    chkMCLK1_INV->Enable(value);
    chkMCLK2_INV->Enable(value);
    cmbFCLK1_DLY->Enable(value);
    cmbFCLK2_DLY->Enable(value);
    cmbMCLK1_DLY->Enable(value);
    cmbMCLK2_DLY->Enable(value);
    chkLML1_TRXIQPULSE->Enable(value);
    chkLML2_TRXIQPULSE->Enable(value);
    chkLML1_SISODDR->Enable(value);
    chkLML2_SISODDR->Enable(value);

}

void lms7002_pnlLimeLightPAD_view::ParameterChangeHandler(wxSpinEvent& event)
{
    wxCommandEvent evt;
    evt.SetInt(event.GetInt());
    evt.SetId(event.GetId());
    evt.SetEventObject(event.GetEventObject());
    ParameterChangeHandler(evt);
}

void lms7002_pnlLimeLightPAD_view::ParameterChangeHandler(wxCommandEvent& event)
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

void lms7002_pnlLimeLightPAD_view::onbtnReadVerRevMask( wxCommandEvent& event )
{
    uint16_t value = 0;

    LMS_ReadParam(lmsControl,LMS7param(VER),&value);
    lblVER->SetLabel(wxString::Format(_("%i"), value));

    LMS_ReadParam(lmsControl,LMS7param(REV),&value);
    lblREV->SetLabel(wxString::Format(_("%i"), value));

    LMS_ReadParam(lmsControl,LMS7param(MASK),&value);
    lblMASK->SetLabel(wxString::Format(_("%i"), value));
}

void lms7002_pnlLimeLightPAD_view::UpdateGUI()
{
    wxArrayString padStrenghts;
    uint16_t value = 0;
    LMS_ReadParam(lmsControl, LMS7_MASK, &value);
    if(value == 0)
    {
        padStrenghts.push_back("4 mA");
        padStrenghts.push_back("8 mA");
    }
    else
    {
        padStrenghts.push_back("8 mA");
        padStrenghts.push_back("12 mA");
    }
    wxRadioBox* padStrenghtCtrl[] = {rgrSDA_DS, rgrSCL_DS, rgrSDIO_DS, rgrDIQ2_DS, rgrDIQ1_DS};
    for(auto i : padStrenghtCtrl)
    {
        i->SetString(0, padStrenghts[0]);
        i->SetString(1, padStrenghts[1]);
    }
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);
}
