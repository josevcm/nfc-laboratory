#ifndef __lms7002_pnlCLKGEN_view__
#define __lms7002_pnlCLKGEN_view__

#include "lms7002_wxgui.h"

#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlCLKGEN_view : public wxPanel
{
	protected:
		// Handlers for pnlCLKGEN_view events.
		void ParameterChangeHandler( wxCommandEvent& event );
        void ParameterChangeHandler(wxSpinEvent& event);
		void onbtnTuneClick( wxCommandEvent& event );
        void OnbtnReadComparators(wxCommandEvent& event);
        void OnbtnUpdateCoarse(wxCommandEvent& event);
        void OnShowVCOclicked(wxCommandEvent& event);

        wxComboBox *cmbCMPLO_CTRL;
	public:
        void onbtnCalculateClick(wxCommandEvent& event);
        void onbtnCalculateClick(wxSpinEvent& event);
        void OnAutoPhase(wxCommandEvent& event);

	lms7002_pnlCLKGEN_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
    void UpdateInterfaceFrequencies();
    wxSpinCtrl* rxPhase;
	wxSpinCtrl* txPhase;
protected:
    void UpdateCLKL();
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_PD_CP_CGEN = 2048,
			ID_PD_FDIV_FB_CGEN,
			ID_PD_FDIV_O_CGEN,
			ID_PD_SDM_CGEN,
			ID_PD_VCO_CGEN,
			ID_PD_VCO_COMP_CGEN,
			ID_EN_G_CGEN,
			ID_SPDUP_VCO_CGEN,
			ID_RESET_N_CGEN,
			ID_EN_COARSE_CKLGEN,
			ID_EN_INTONLY_SDM_CGEN,
			ID_EN_SDM_CLK_CGEN,
			ID_REV_SDMCLK_CGEN,
			ID_SX_DITHER_EN_CGEN,
			ID_REV_CLKDAC_CGEN,
			ID_REV_CLKADC_CGEN,
			ID_REVPH_PFD_CGEN,
			ID_COARSE_START_CGEN,
			ID_SEL_SDMCLK_CGEN,
			ID_TST_CGEN,
			ID_CP2_CGEN,
			ID_CP3_CGEN,
			ID_CZ_CGEN,
			ID_EN_ADCCLKH_CLKGN,
			ID_CLKH_OV_CLKL_CGEN,
			ID_GFIR1_N_TXTSP,
			ID_AUTO_PHASE,
			ID_INT_SDM_CGEN,
			ID_FRAC_SDM_CGEN,
			ID_VCO_CMPHO_CGEN,
			ID_VCO_CMPLO_CGEN,
			ID_COARSE_STEPDONE_CGEN,
			ID_COARSEPLL_COMPO_CGEN,
			ID_CSW_VCO_CGEN,
			ID_ICT_VCO_CGEN,
			ID_IOFFSET_CP_CGEN,
			ID_IPULSE_CP_CGEN
		};
		
		wxCheckBox* chkPD_CP_CGEN;
		wxCheckBox* chkPD_FDIV_FB_CGEN;
		wxCheckBox* chkPD_FDIV_O_CGEN;
		wxCheckBox* chkPD_SDM_CGEN;
		wxCheckBox* chkPD_VCO_CGEN;
		wxCheckBox* chkPD_VCO_COMP_CGEN;
		wxCheckBox* chkEN_G_CGEN;
		wxCheckBox* chkSPDUP_VCO_CGEN;
		wxCheckBox* chkRESET_N_CGEN;
		wxCheckBox* chkEN_COARSE_CKLGEN;
		wxCheckBox* chkEN_INTONLY_SDM_CGEN;
		wxCheckBox* chkEN_SDM_CLK_CGEN;
		wxCheckBox* chkREV_SDMCLK_CGEN;
		wxCheckBox* chkSX_DITHER_EN_CGEN;
		wxCheckBox* chkREV_CLKDAC_CGEN;
		wxCheckBox* chkREV_CLKADC_CGEN;
		wxCheckBox* chkREVPH_PFD_CGEN;
		wxCheckBox* chkCOARSE_START_CGEN;
		wxRadioBox* rgrSEL_SDMCLK_CGEN;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbTST_CGEN;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbCP2_CGEN;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbCP3_CGEN;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbCZ_CGEN;
		wxRadioBox* cmbEN_ADCCLKH_CLKGN;
		wxStaticText* m_staticText369;
		wxStaticText* lblRxTSPfreq;
		wxStaticText* m_staticText367;
		wxStaticText* lblTxTSPfreq;
		wxStaticText* ID_STATICTEXT10;
		wxStaticText* ID_STATICTEXT18;
		wxStaticText* ID_STATICTEXT3;
		wxTextCtrl* txtFrequency;
		wxComboBox* cmbCLKH_OV_CLKL_CGEN;
		wxTextCtrl* txtFrequencyCLKL;
		wxStaticText* ID_STATICTEXT101;
		wxStaticText* ID_STATICTEXT102;
		wxCheckBox* chkAutoPhase;
		wxButton* btnCalculate;
		wxButton* btnTune;
		wxStaticText* ID_STATICTEXT7;
		wxStaticText* lblINT_SDM_CGEN;
		wxStaticText* ID_STATICTEXT23;
		wxStaticText* lblRefClk_MHz;
		wxStaticText* ID_STATICTEXT21;
		wxStaticText* lblFRAC_SDM_CGEN;
		wxStaticText* ID_STATICTEXT2;
		wxStaticText* lblDivider;
		wxStaticText* ID_STATICTEXT25;
		wxStaticText* lblRealOutFrequency;
		wxStaticText* ID_STATICTEXT161;
		wxStaticText* lblVCO_CMPHO_CGEN;
		wxStaticText* ID_STATICTEXT171;
		wxStaticText* lblVCO_CMPLO_CGEN;
		wxButton* btnUpdateValues1;
		wxStaticText* ID_STATICTEXT5;
		wxStaticText* lblCOARSE_STEPDONE_CGEN;
		wxStaticText* ID_STATICTEXT6;
		wxStaticText* lblCOARSEPLL_COMPO_CGEN;
		wxButton* btnUpdateCoarse;
		wxStaticText* ID_STATICTEXT20;
		NumericSlider* cmbCSW_VCO_CGEN;
		wxStaticText* ID_STATICTEXT9;
		NumericSlider* cmbICT_VCO_CGEN;
		wxButton* btnShowVCO;
		wxStaticText* ID_STATICTEXT16;
		NumericSlider* cmbIOFFSET_CP_CGEN;
		wxStaticText* ID_STATICTEXT17;
		NumericSlider* cmbIPULSE_CP_CGEN;
		wxFlexGridSizer* sizerR3;
};

#endif // __lms7002_pnlCLKGEN_view__
