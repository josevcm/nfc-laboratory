#include <assert.h>
#include "lms7002_mainPanel.h"
#include "lms7002_pnlAFE_view.h"
#include "lms7002_pnlBIAS_view.h"
#include "lms7002_pnlBIST_view.h"
#include "lms7002_pnlCDS_view.h"
#include "lms7002_pnlCLKGEN_view.h"
#include "lms7002_pnlLDO_view.h"
#include "lms7002_pnlLimeLightPAD_view.h"
#include "lms7002_pnlTxTSP_view.h"
#include "lms7002_pnlRxTSP_view.h"
#include "lms7002_pnlRBB_view.h"
#include "lms7002_pnlRFE_view.h"
#include "lms7002_pnlSX_view.h"
#include "lms7002_pnlTBB_view.h"
#include "lms7002_pnlTRF_view.h"
#include "lms7002_pnlXBUF_view.h"
#include "lms7002_pnlGains_view.h"
#include "lms7002_pnlCalibrations_view.h"
#include <wx/time.h>
#include <wx/msgdlg.h>
#include <iostream>
#include <wx/filedlg.h>
#include "lms7suiteEvents.h"
#include "lms7002_pnlMCU_BD_view.h"
#include "lms7002_pnlR3.h"
#include "lime/LimeSuite.h"
#include "lms7_device.h"
using namespace std;
using namespace lime;

lms7002_mainPanel::lms7002_mainPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    :
    wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    wxFlexGridSizer* fgSizer298;
    fgSizer298 = new wxFlexGridSizer( 3, 1, 0, 0 );
    fgSizer298->AddGrowableCol( 0 );
    fgSizer298->AddGrowableRow( 1 );
    fgSizer298->SetFlexibleDirection( wxBOTH );
    fgSizer298->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer299;
    fgSizer299 = new wxFlexGridSizer( 0, 7, 0, 0 );
    fgSizer299->AddGrowableCol( 3 );
    fgSizer299->SetFlexibleDirection( wxBOTH );
    fgSizer299->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    ID_BUTTON2 = new wxButton( this, wxID_ANY, wxT("Open"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    ID_BUTTON2->SetDefault(); 
    fgSizer299->Add( ID_BUTTON2, 1, 0, 5 );
    
    ID_BUTTON3 = new wxButton( this, wxID_ANY, wxT("Save"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    ID_BUTTON3->SetDefault(); 
    fgSizer299->Add( ID_BUTTON3, 1, 0, 5 );
    
    wxFlexGridSizer* fgSizer300;
    fgSizer300 = new wxFlexGridSizer( 0, 5, 0, 0 );
    fgSizer300->AddGrowableRow( 0 );
    fgSizer300->SetFlexibleDirection( wxBOTH );
    fgSizer300->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer248;
    fgSizer248 = new wxFlexGridSizer( 0, 1, 0, 0 );
    fgSizer248->SetFlexibleDirection( wxBOTH );
    fgSizer248->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer249;
    fgSizer249 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer249->SetFlexibleDirection( wxHORIZONTAL );
    fgSizer249->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    cmbLmsDevice = new wxComboBox( this, ID_G_LNA_RFE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    cmbLmsDevice->Append( wxT("LMS 1") );
    cmbLmsDevice->Append( wxT("LMS 2") );
    cmbLmsDevice->SetSelection( 0 );
    cmbLmsDevice->SetToolTip( wxT("Controls the gain of the LNA") );
    
    fgSizer249->Add( cmbLmsDevice, 0, wxALL, 0 );
    
    rbChannelA = new wxRadioButton( this, ID_BTN_CH_A, wxT("A CHANNEL"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( rbChannelA, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    rbChannelB = new wxRadioButton( this, ID_BTN_CH_B, wxT("B CHANNEL"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( rbChannelB, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    chkEnableMIMO = new wxCheckBox( this, wxID_ANY, wxT("Enable MIMO"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEnableMIMO->SetToolTip( wxT("Enables required registers for MIMO mode") );
    
    fgSizer249->Add( chkEnableMIMO, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    
    fgSizer248->Add( fgSizer249, 0, wxEXPAND, 5 );
    
    
    fgSizer300->Add( fgSizer248, 0, wxEXPAND, 5 );
    
    btnDownloadAll = new wxButton( this, ID_BTN_CHIP_TO_GUI, wxT("Chip-->GUI"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer300->Add( btnDownloadAll, 0, 0, 5 );
    
    btnUploadAll = new wxButton( this, wxID_ANY, wxT("GUI-->Chip"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer300->Add( btnUploadAll, 0, 0, 5 );
    
    btnResetChip = new wxButton( this, ID_BTN_RESET_CHIP, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer300->Add( btnResetChip, 0, 0, 5 );
    
    btnLoadDefault = new wxButton( this, ID_BTN_RESET_CHIP, wxT("Default"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer300->Add( btnLoadDefault, 0, 0, 5 );
    
    wxFlexGridSizer* fgSizer247;
    fgSizer247 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer247->SetFlexibleDirection( wxBOTH );
    fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    
    fgSizer300->Add( fgSizer247, 1, wxEXPAND, 5 );
    
    
    fgSizer299->Add( fgSizer300, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, 10 );
    
    txtTemperature = new wxStaticText( this, wxID_ANY, wxT("Temperature: ?????"), wxDefaultPosition, wxDefaultSize, 0 );
    txtTemperature->Wrap( -1 );
    fgSizer299->Add( txtTemperature, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    btnReadTemperature = new wxButton( this, wxID_ANY, wxT("Read Temp"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer299->Add( btnReadTemperature, 0, wxALIGN_CENTER_VERTICAL, 5 );
    
    
    fgSizer298->Add( fgSizer299, 0, wxALIGN_LEFT|wxALIGN_TOP|wxBOTTOM, 10 );
    
    tabsNotebook = new wxNotebook( this, ID_TABS_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
    mTabCalibrations = new lms7002_pnlCalibrations_view( tabsNotebook, ID_TAB_CALIBRATIONS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabCalibrations, wxT("Calibrations"), true );
    mTabRFE = new lms7002_pnlRFE_view( tabsNotebook, ID_TAB_RFE, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabRFE, wxT("RFE"), false );
    mTabRBB = new lms7002_pnlRBB_view( tabsNotebook, ID_TAB_RBB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabRBB, wxT("RBB"), false );
    mTabTRF = new lms7002_pnlTRF_view( tabsNotebook, ID_TAB_TRF, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabTRF, wxT("TRF"), false );
    mTabTBB = new lms7002_pnlTBB_view( tabsNotebook, ID_TAB_TBB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabTBB, wxT("TBB"), false );
    mTabAFE = new lms7002_pnlAFE_view( tabsNotebook, ID_TAB_AFE, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabAFE, wxT("AFE"), false );
    mTabBIAS = new lms7002_pnlBIAS_view( tabsNotebook, ID_TAB_BIAS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabBIAS, wxT("BIAS"), false );
    mTabLDO = new lms7002_pnlLDO_view( tabsNotebook, ID_TAB_LDO, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabLDO, wxT("LDO"), false );
    mTabXBUF = new lms7002_pnlXBUF_view( tabsNotebook, ID_TAB_XBUF, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabXBUF, wxT("XBUF"), false );
    mTabCGEN = new lms7002_pnlCLKGEN_view( tabsNotebook, ID_TAB_CGEN, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabCGEN, wxT("CLKGEN"), false );
    mTabSXR = new lms7002_pnlSX_view( tabsNotebook, ID_TAB_SXR, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabSXR, wxT("SXR"), false );
    mTabSXT = new lms7002_pnlSX_view( tabsNotebook, ID_TAB_SXT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    mTabSXT->mIsSXT = true;
    tabsNotebook->AddPage( mTabSXT, wxT("SXT"), false );
    mTabLimeLight = new lms7002_pnlLimeLightPAD_view( tabsNotebook, ID_TAB_LIMELIGHT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabLimeLight, wxT("LimeLight && PAD"), false );
    mTabTxTSP = new lms7002_pnlTxTSP_view( tabsNotebook, ID_TAB_TXTSP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabTxTSP, wxT("TxTSP"), false );
    mTabRxTSP = new lms7002_pnlRxTSP_view( tabsNotebook, ID_TAB_RXTSP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabRxTSP, wxT("RxTSP"), false );
    mTabCDS = new lms7002_pnlCDS_view( tabsNotebook, ID_TAB_CDS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabCDS, wxT("CDS"), false );
    mTabBIST = new lms7002_pnlBIST_view( tabsNotebook, ID_TAB_BIST, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabBIST, wxT("BIST"), false );
    mTabTrxGain = new lms7002_pnlGains_view( tabsNotebook, ID_TAB_GAINS, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    tabsNotebook->AddPage( mTabTrxGain, wxT("TRX Gain"), false );
    
    
    
    // Connect Events
    ID_BUTTON2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnOpenProject ), NULL, this );
    ID_BUTTON3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnSaveProject ), NULL, this );
    cmbLmsDevice->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( lms7002_mainPanel::OnLmsDeviceSelect ), NULL, this );
    rbChannelA->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_mainPanel::OnSwitchToChannelA ), NULL, this );
    rbChannelB->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( lms7002_mainPanel::OnSwitchToChannelB ), NULL, this );
    chkEnableMIMO->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnEnableMIMOchecked ), NULL, this );
    btnDownloadAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnDownloadAll ), NULL, this );
    btnUploadAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnUploadAll ), NULL, this );
    btnResetChip->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnResetChip ), NULL, this );
    btnLoadDefault->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnLoadDefault ), NULL, this );
    btnReadTemperature->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_mainPanel::OnReadTemperature ), NULL, this );
    tabsNotebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( lms7002_mainPanel::Onnotebook_modulesPageChanged ), NULL, this );

    mTabMCU = new lms7002_pnlMCU_BD_view(tabsNotebook);
    tabsNotebook->AddPage(mTabMCU, _("MCU"));

    mTabR3 = new lms7002_pnlR3_view(tabsNotebook, wxNewId());
    tabsNotebook->AddPage(mTabR3, _("R3 Controls"));
    fgSizer298->Add( tabsNotebook, 0, wxEXPAND, 5 );
    this->SetSizer( fgSizer298 );
    this->Layout();
}

void lms7002_mainPanel::UpdateVisiblePanel()
{
    wxWindow* currentPage = tabsNotebook->GetCurrentPage();
    uint16_t spisw_ctrl = 0;
    if (((LMS7_Device*)lmsControl)->GetConnection())
    {
        if (currentPage == mTabSXR) //change active channel to A
            LMS_WriteParam(lmsControl,LMS7param(MAC),1);
        else if (currentPage == mTabSXT) //change active channel to B
            LMS_WriteParam(lmsControl,LMS7param(MAC),2);
        else
            LMS_WriteParam(lmsControl,LMS7param(MAC),rbChannelA->GetValue() == 1 ? 1: 2);
        LMS_ReadLMSReg(lmsControl, 0x0006, &spisw_ctrl);
    }

    if(spisw_ctrl & 1) // transceiver controlled by MCU
    {
        if(currentPage != mTabMCU && currentPage != mTabTrxGain)
            currentPage->Disable();
        return;
    }
    else
        currentPage->Enable();

    wxLongLong t1, t2;
    t1 = wxGetUTCTimeMillis();
    long visibleTabId = tabsNotebook->GetCurrentPage()->GetId();
    switch (visibleTabId)
    {
    case ID_TAB_AFE:
        mTabAFE->UpdateGUI();
        break;
    case ID_TAB_BIAS:
        mTabBIAS->UpdateGUI();
        break;
    case ID_TAB_BIST:
        mTabBIST->UpdateGUI();
        break;
    case ID_TAB_CDS:
        mTabCDS->UpdateGUI();
        break;
    case ID_TAB_CGEN:
        mTabCGEN->UpdateGUI();
        break;
    case ID_TAB_LDO:
        mTabLDO->UpdateGUI();
        break;
    case ID_TAB_LIMELIGHT:
        mTabLimeLight->UpdateGUI();
        break;
    case ID_TAB_TXTSP:
        mTabTxTSP->UpdateGUI();
        break;
    case ID_TAB_RXTSP:
        mTabRxTSP->UpdateGUI();
        break;
    case ID_TAB_RBB:
        mTabRBB->UpdateGUI();
        break;
    case ID_TAB_RFE:
        mTabRFE->UpdateGUI();
        break;
    case ID_TAB_SXR:
        mTabSXR->UpdateGUI();
        break;
    case ID_TAB_SXT:
        mTabSXT->UpdateGUI();
        break;
    case ID_TAB_TBB:
        mTabTBB->UpdateGUI();
        break;
    case ID_TAB_TRF:
        mTabTRF->UpdateGUI();
        break;
    case ID_TAB_XBUF:
        mTabXBUF->UpdateGUI();
        break;
    case ID_TAB_CALIBRATIONS:
        mTabCalibrations->UpdateGUI();
        break;
    case ID_TAB_GAINS:
        mTabTrxGain->UpdateGUI();
        break;
    }
    if(visibleTabId == mTabR3->GetId())
        mTabR3->UpdateGUI();
    t2 = wxGetUTCTimeMillis();
#ifndef NDEBUG
    //cout << "Visible GUI update time: " << (t2 - t1).ToString() << endl;
#endif
}

void lms7002_mainPanel::Initialize(lms_device_t* pControl)
{
    assert(pControl != nullptr);
    lmsControl = pControl;
    mTabRFE->Initialize(lmsControl);
    mTabRBB->Initialize(lmsControl);
    mTabTRF->Initialize(lmsControl);
    mTabTBB->Initialize(lmsControl);
    mTabAFE->Initialize(lmsControl);
    mTabBIAS->Initialize(lmsControl);
    mTabLDO->Initialize(lmsControl);
    mTabXBUF->Initialize(lmsControl);
    mTabCGEN->Initialize(lmsControl);
    mTabSXR->Initialize(lmsControl);
    mTabSXT->Initialize(lmsControl);
    mTabTrxGain->Initialize(lmsControl);
    mTabLimeLight->Initialize(lmsControl);
    mTabTxTSP->Initialize(lmsControl);
    mTabRxTSP->Initialize(lmsControl);
    mTabCDS->Initialize(lmsControl);
    mTabBIST->Initialize(lmsControl);
    mTabCalibrations->Initialize(lmsControl);
    mTabMCU->Initialize(lmsControl);
    mTabR3->Initialize(lmsControl);
    ((LMS7_Device*)lmsControl)->SetActiveChip(0);
    cmbLmsDevice->SetSelection(0);

    if (((LMS7_Device*)lmsControl)->GetNumChannels() > 2)
        cmbLmsDevice->Show();
    else
        cmbLmsDevice->Hide();
    rbChannelA->SetValue(true);
    rbChannelB->SetValue(false);
    UpdateGUI();
    Layout();
}

void lms7002_mainPanel::OnResetChip(wxCommandEvent &event)
{
    int status = LMS_Reset(lmsControl);
    if (status != 0)
        wxMessageBox(_("Chip reset failed"), _("Warning"));
    wxNotebookEvent evt;
    chkEnableMIMO->SetValue(false);
    Onnotebook_modulesPageChanged(evt); //after reset chip active channel might change, this refresh channel for active tab
}

void lms7002_mainPanel::OnLoadDefault(wxCommandEvent& event)
{
    int status = LMS_Init(lmsControl);
    if (status != 0)
        wxMessageBox(_("Load Default failed"), _("Warning"));
    LMS_EnableChannel(lmsControl, LMS_CH_TX, 0, true); //enable TX, LMS_Init() no longer does it
    wxNotebookEvent evt;
    chkEnableMIMO->SetValue(false);
    ((LMS7_Device*)lmsControl)->SetActiveChip(cmbLmsDevice->GetSelection());
    Onnotebook_modulesPageChanged(evt); //after reset chip active channel might change, this refresh channel for active tab
    wxCommandEvent evt2;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt2);
}

void lms7002_mainPanel::UpdateGUI()
{
    wxLongLong t1, t2;
    t1 = wxGetUTCTimeMillis();
    t2 = wxGetUTCTimeMillis();
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnNewProject( wxCommandEvent& event )
{
    LMS_Reset(lmsControl);
    LMS_WriteParam(lmsControl,LMS7param(MAC),rbChannelA->GetValue() == 1 ? 1: 2);
    LMS_WriteParam(lmsControl,LMS7param(MAC),1);
    UpdateGUI();
}

void lms7002_mainPanel::OnOpenProject( wxCommandEvent& event )
{
    wxFileDialog dlg(this, _("Open config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    int status = LMS_LoadConfig(lmsControl,dlg.GetPath().To8BitData());
    if (status != 0)
    {
        wxMessageBox(_("Failed to load file"), _("Warning"));
    }
    wxCommandEvent tevt;
    LMS_WriteParam(lmsControl,LMS7param(MAC),rbChannelA->GetValue() == 1 ? 1: 2);
    UpdateGUI();
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
}

void lms7002_mainPanel::OnSaveProject( wxCommandEvent& event )
{
    wxFileDialog dlg(this, _("Save config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    int status = LMS_SaveConfig(lmsControl,dlg.GetPath().To8BitData());
    if (status != 0)
        wxMessageBox(_("Failed to save file"), _("Warning"));
}

void lms7002_mainPanel::OnSwitchToChannelA(wxCommandEvent& event)
{
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnSwitchToChannelB(wxCommandEvent& event)
{
    UpdateVisiblePanel();
}

void lms7002_mainPanel::Onnotebook_modulesPageChanged( wxNotebookEvent& event )
{
    wxNotebookPage* page = tabsNotebook->GetCurrentPage();
    if (page == mTabAFE || page == mTabBIAS || page == mTabLDO || page == mTabXBUF || page == mTabCGEN || page == mTabCDS || page == mTabBIST)
    {
        rbChannelA->Disable();
        rbChannelB->Disable();
    }
    else if (page == mTabSXR) //change active channel to A
    {
        rbChannelA->Disable();
        rbChannelB->Disable();
    }
    else if (page == mTabSXT) //change active channel to B
    {
        rbChannelA->Disable();
        rbChannelB->Disable();
    }
    else
    {
        rbChannelA->Enable();
        rbChannelB->Enable();
    }

#ifdef __APPLE__
    //force show the page selected by the event (needed on apple)
    if (event.GetSelection() != -1)
    {
        dynamic_cast<wxNotebook*>(event.GetEventObject())->GetPage(event.GetSelection())->Show(true);
    }
#endif

    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnDownloadAll(wxCommandEvent& event)
{
    int status = LMS_Synchronize(lmsControl,false);
    if (status != 0)
        wxMessageBox(_("Download all registers failed"), _("Warning"));
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnUploadAll(wxCommandEvent& event)
{
    int status = LMS_Synchronize(lmsControl,true);
    if (status != 0)
        wxMessageBox(_("Upload all registers failed"), _("Warning"));
    wxCommandEvent evt;
    evt.SetEventType(CGEN_FREQUENCY_CHANGED);
    wxPostEvent(this, evt);
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnReadTemperature(wxCommandEvent& event)
{
    double t = 0.0;
    int status = LMS_GetChipTemperature(lmsControl,0,&t);
    if (status != 0)
        wxMessageBox(_("Failed to read chip temperature"), _("Warning"));
    txtTemperature->SetLabel(wxString::Format("Temperature: %.0f C", t));
}

void lms7002_mainPanel::OnSyncABchecked(wxCommandEvent& event)
{
    /*
    rbChannelA->Enable(!chkSyncAB->IsChecked());
    rbChannelB->Enable(!chkSyncAB->IsChecked());
    if(chkSyncAB->IsChecked())
    {
        int status = lmsControl->CopyChannelRegisters(LMS7002M::ChA, LMS7002M::ChB, false);
        if(status != 0)
            wxMessageBox(wxString::Format(_("Failed to copy A to B: %s"), wxString::From8BitData(GetLastErrorMessage())), _("Error"));
        wxNotebookPage* page = tabsNotebook->GetCurrentPage();
        if(page != mTabSXR && page != mTabSXT)
            lmsControl->SetActiveChannel(lime::LMS7002M::ChAB);
    }
    else
    {
        if(rbChannelA->GetValue() != 0)
            lmsControl->SetActiveChannel(lime::LMS7002M::ChA);
        else
            lmsControl->SetActiveChannel(lime::LMS7002M::ChB);
    }
    UpdateVisiblePanel();
    */
}

void lms7002_mainPanel::OnEnableMIMOchecked(wxCommandEvent& event)
{
    uint16_t chBck;
    LMS_ReadParam(lmsControl, LMS7param(MAC), &chBck);
    bool enable = chkEnableMIMO->IsChecked();
    for (int ch = enable ? 0 : 1 ; ch < LMS_GetNumChannels(lmsControl,false);ch++)
    {
        LMS_EnableChannel(lmsControl,LMS_CH_RX,ch,enable);
        LMS_EnableChannel(lmsControl,LMS_CH_TX,ch,enable);
    }
    LMS_WriteParam(lmsControl, LMS7param(MAC), chBck);
    UpdateVisiblePanel();
}

void lms7002_mainPanel::OnCalibrateInternalADC(wxCommandEvent& event)
{
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();
    int status = lms->CalibrateInternalADC();
    if (status != 0)
        wxMessageBox(_("Internal ADC calibration failed"), _("Warning"));
}

int lms7002_mainPanel::GetLmsSelection()
{
    return cmbLmsDevice->GetSelection();
}

void lms7002_mainPanel::OnLmsDeviceSelect( wxCommandEvent& event )
{
    int deviceSelection = cmbLmsDevice->GetSelection();
    ((LMS7_Device*)lmsControl)->SetActiveChip(deviceSelection);

    UpdateVisiblePanel();
    wxCommandEvent evt;
    evt.SetEventType(LMS_CHANGED);
    evt.SetInt(deviceSelection);
    wxPostEvent(this->GetParent(), evt);
}
