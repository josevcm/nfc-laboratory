#ifndef __lms7002_pnlSX_view__
#define __lms7002_pnlSX_view__

#include "lms7002_wxgui.h"

#include <map>
#include "lime/LimeSuite.h"
class lms7002_pnlSX_view : public wxPanel
{
	protected:
		// Handlers for pnlSX_view events.
		void ParameterChangeHandler( wxCommandEvent& event );
        void ParameterChangeHandler( wxSpinEvent& event);
        void OnbtnReadComparators(wxCommandEvent& event);
		void OnbtnChangeRefClkClick( wxCommandEvent& event );
		void OnbtnCalculateClick( wxCommandEvent& event );
		void OnbtnTuneClick( wxCommandEvent& event );
        void OnShowVCOclicked(wxCommandEvent& event);
        void OnEnableRefSpurCancelation(wxCommandEvent& event);

        wxComboBox* cmbRZ_CTRL;
        wxComboBox* cmbCMPLO_CTRL;
	public:
    lms7002_pnlSX_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
    bool mIsSXT;
protected:
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_PD_FBDIV = 2048,
			ID_PD_LOCH_T2RBUF,
			ID_PD_CP,
			ID_PD_FDIV,
			ID_PD_SDM,
			ID_PD_VCO_COMP,
			ID_PD_VCO,
			ID_EN_G,
			ID_EN_DIR_SXRSXT,
			ID_RESET_N,
			ID_SPDUP_VCO,
			ID_BYPLDO_VCO,
			ID_EN_COARSEPLL,
			ID_CURLIM_VCO,
			ID_REVPH_PFD,
			ID_EN_INTONLY_SDM,
			ID_EN_SDM_CLK,
			ID_REV_SDMCLK,
			ID_SX_DITHER_EN,
			ID_EN_DIV2_DIVPROGenabled,
			ID_TST_SX,
			ID_PW_DIV2_LOCH,
			ID_PW_DIV4_LOCH,
			ID_DIV_LOCH,
			ID_CP2_PLL,
			ID_CP3_PLL,
			ID_CZ,
			ID_SEL_VCO,
			ID_EN_DIV2_DIVPROG,
			ID_VCO_CMPHO,
			ID_VCO_CMPLO,
			ID_CSW_VCO,
			ID_ICT_VCO,
			ID_BTN_SHOW_VCO,
			ID_VDIV_VCO,
			ID_IPULSE_CP,
			ID_SEL_SDMCLK,
			ID_RSEL_LDO_VCO,
			ID_IOFFSET_CP
		};
		
		wxCheckBox* chkPD_FBDIV;
		wxCheckBox* chkPD_LOCH_T2RBUF;
		wxCheckBox* chkPD_CP;
		wxCheckBox* chkPD_FDIV;
		wxCheckBox* chkPD_SDM;
		wxCheckBox* chkPD_VCO_COMP;
		wxCheckBox* chkPD_VCO;
		wxCheckBox* chkEN_G;
		wxCheckBox* chkEN_DIR_SXRSXT;
		wxCheckBox* chkRESET_N;
		wxCheckBox* chkSPDUP_VCO;
		wxCheckBox* chkBYPLDO_VCO;
		wxCheckBox* chkEN_COARSEPLL;
		wxCheckBox* chkCURLIM_VCO;
		wxCheckBox* chkREVPH_PFD;
		wxCheckBox* chkEN_INTONLY_SDM;
		wxCheckBox* chkEN_SDM_CLK;
		wxCheckBox* chkREV_SDMCLK;
		wxCheckBox* chkSX_DITHER_EN;
		wxCheckBox* chkEN_DIV2_DIVPROGenabled;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbTST_SX;
		wxStaticText* ID_STATICTEXT4;
		wxSpinCtrl* ctrPW_DIV2_LOCH;
		wxStaticText* ID_STATICTEXT5;
		wxSpinCtrl* ctrPW_DIV4_LOCH;
		wxStaticText* ID_STATICTEXT6;
		wxSpinCtrl* ctrDIV_LOCH;
		wxStaticText* ID_STATICTEXT18;
		wxComboBox* cmbCP2_PLL;
		wxStaticText* ID_STATICTEXT19;
		wxComboBox* cmbCP3_PLL;
		wxStaticText* ID_STATICTEXT20;
		wxComboBox* cmbCZ;
		wxRadioBox* rgrSEL_VCO;
		wxTextCtrl* txtFrequency;
		wxButton* btnCalculate;
		wxButton* btnTune;
		wxButton* ID_BUTTON5;
		wxPanel* pnlRefClkSpur;
		wxStaticBoxSizer* RefClkSpurSizer;
		wxCheckBox* chkEnableRefSpurCancelation;
		wxStaticText* m_staticText359;
		wxTextCtrl* txtRefSpurBW;
		wxStaticText* ID_STATICTEXT2;
		wxStaticText* lblINT_SDM;
		wxStaticText* ID_STATICTEXT21;
		wxStaticText* lblFRAC_SDM;
		wxStaticText* ID_STATICTEXT25;
		wxStaticText* lblRealOutFrequency;
		wxStaticText* ID_STATICTEXT23;
		wxStaticText* lblDivider;
		wxStaticText* ID_STATICTEXT30;
		wxStaticText* lblEN_DIV2_DIVPROG;
		wxButton* btnChangeRefClk;
		wxStaticText* lblRefClk_MHz;
		wxStaticText* ID_STATICTEXT16;
		wxStaticText* lblVCO_CMPHO;
		wxStaticText* ID_STATICTEXT17;
		wxStaticText* lblVCO_CMPLO;
		wxButton* btnUpdateValues;
		wxStaticText* ID_STATICTEXT1;
		NumericSlider* ctrCSW_VCO;
		wxStaticText* ID_STATICTEXT10;
		NumericSlider* cmbICT_VCO;
		wxButton* btnShowVCOparams;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbVDIV_VCO;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbIPULSE_CP;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbSEL_SDMCLK;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbRSEL_LDO_VCO;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbIOFFSET_CP;
		wxFlexGridSizer* sizerR3;
};

#endif // __lms7002_pnlSX_view__
