#ifndef __lms7002_mainPanel__
#define __lms7002_mainPanel__

#include "lime/LimeSuite.h"
#include "lms7002_wxgui.h"

namespace lime{
class MCU_BD;
}
class lms7002_pnlMCU_BD_view;
class lms7002_pnlR3_view;

class lms7002_mainPanel : public wxPanel
{
	protected:
        // Handlers for mainPanel events.
        void OnNewProject( wxCommandEvent& event );
        void OnOpenProject( wxCommandEvent& event );
        void OnSaveProject( wxCommandEvent& event );
        void OnUploadAll( wxCommandEvent& event );
        void OnDownloadAll( wxCommandEvent& event );
        void OnReset( wxCommandEvent& event );
        void OnRegistersTest( wxCommandEvent& event );
        void OnSwitchToChannelA(wxCommandEvent& event);
        void OnSwitchToChannelB(wxCommandEvent& event);
        void Onnotebook_modulesPageChanged( wxNotebookEvent& event );
        void OnResetChip(wxCommandEvent& event);
        void OnLoadDefault(wxCommandEvent& event);
        void OnReadTemperature(wxCommandEvent& event);
        void OnSyncABchecked(wxCommandEvent& event);
        void OnEnableMIMOchecked(wxCommandEvent& event);
        void OnCalibrateInternalADC(wxCommandEvent& event);
        int GetLmsSelection();
        void OnLmsDeviceSelect( wxCommandEvent& event );
	public:
		
		lms7002_mainPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
        void UpdateVisiblePanel();
        void UpdateGUI();
        void Initialize(lms_device_t* pControl);

        lms7002_pnlCalibrations_view* mTabCalibrations;
        lms7002_pnlRFE_view* mTabRFE;
        lms7002_pnlRBB_view* mTabRBB;
        lms7002_pnlTRF_view* mTabTRF;
        lms7002_pnlTBB_view* mTabTBB;
        lms7002_pnlAFE_view* mTabAFE;
        lms7002_pnlBIAS_view* mTabBIAS;
        lms7002_pnlLDO_view* mTabLDO;
        lms7002_pnlXBUF_view* mTabXBUF;
        lms7002_pnlCLKGEN_view* mTabCGEN;
        lms7002_pnlSX_view* mTabSXR;
        lms7002_pnlSX_view* mTabSXT;
        lms7002_pnlLimeLightPAD_view* mTabLimeLight;
        lms7002_pnlTxTSP_view* mTabTxTSP;
        lms7002_pnlRxTSP_view* mTabRxTSP;
        lms7002_pnlCDS_view* mTabCDS;
        lms7002_pnlBIST_view* mTabBIST;
        lms7002_pnlGains_view* mTabTrxGain;
protected:
        lms7002_pnlMCU_BD_view *mTabMCU;
        lms7002_pnlR3_view *mTabR3;
        lms_device_t* lmsControl;

        enum
        {
            ID_G_LNA_RFE = 2048,
            ID_BTN_CH_A,
            ID_BTN_CH_B,
            ID_BTN_CHIP_TO_GUI,
            ID_BTN_RESET_CHIP,
            ID_TABS_NOTEBOOK,
            ID_TAB_CALIBRATIONS,
            ID_TAB_RFE,
            ID_TAB_RBB,
            ID_TAB_TRF,
            ID_TAB_TBB,
            ID_TAB_AFE,
            ID_TAB_BIAS,
            ID_TAB_LDO,
            ID_TAB_XBUF,
            ID_TAB_CGEN,
            ID_TAB_SXR,
            ID_TAB_SXT,
            ID_TAB_LIMELIGHT,
            ID_TAB_TXTSP,
            ID_TAB_RXTSP,
            ID_TAB_CDS,
            ID_TAB_BIST,
            ID_TAB_GAINS
        };
        
        wxButton* ID_BUTTON2;
        wxButton* ID_BUTTON3;
        wxComboBox* cmbLmsDevice;
        wxRadioButton* rbChannelA;
        wxRadioButton* rbChannelB;
        wxCheckBox* chkEnableMIMO;
        wxButton* btnDownloadAll;
        wxButton* btnUploadAll;
        wxButton* btnResetChip;
        wxButton* btnLoadDefault;
        wxStaticText* txtTemperature;
        wxButton* btnReadTemperature;
        wxNotebook* tabsNotebook;
};

#endif // __lms7002_mainPanel__
