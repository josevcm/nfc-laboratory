#ifndef __lms7002_pnlTRF_view__
#define __lms7002_pnlTRF_view__

#include "lms7002_wxgui.h"

#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlTRF_view : public wxPanel
{
public:
	lms7002_pnlTRF_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
	void ParameterChangeHandler( wxCommandEvent& event );
	void OnBandChange( wxCommandEvent& event );
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_PD_PDET_TRF = 2048,
			ID_PD_TLOBUF_TRF,
			ID_PD_TXPAD_TRF,
			ID_EN_G_TRF,
			ID_EN_DIR_TRF,
			ID_LOADR_PDET_TRF,
			ID_ICT_LIN_TXPAD_TRF,
			ID_ICT_MAIN_TXPAD_TRF,
			ID_CDC_I_TRF,
			ID_CDC_Q_TRF,
			ID_GCAS_GNDREF_TXPAD_TRF,
			ID_EN_NEXTTX_TRF,
			ID_EN_LOOPB_TXPAD_TRF,
			ID_TXFEoutput,
			ID_EN_LOWBWLOMX_TMX_TRF,
			ID_EN_AMPHF_PDET_TRF,
			ID_F_TXPAD_TRF,
			ID_L_LOOPB_TXPAD_TRF,
			ID_LOSS_LIN_TXPAD_TRF,
			ID_LOSS_MAIN_TXPAD_TRF,
			ID_VGCAS_TXPAD_TRF,
			ID_LOBIASN_TXM_TRF,
			ID_LOBIASP_TXX_TRF
		};
		
		wxCheckBox* chkPD_PDET_TRF;
		wxCheckBox* chkPD_TLOBUF_TRF;
		wxCheckBox* chkPD_TXPAD_TRF;
		wxCheckBox* chkEN_G_TRF;
		wxCheckBox* chkEN_DIR_TRF;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbLOADR_PDET_TRF;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbICT_LIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbICT_MAIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbCDC_I_TRF;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbCDC_Q_TRF;
		wxRadioBox* rgrGCAS_GNDREF_TXPAD_TRF;
		wxCheckBox* chkEN_NEXTTX_TRF;
		wxCheckBox* chkEN_LOOPB_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT16;
		wxComboBox* cmbTXFEoutput;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbEN_LOWBWLOMX_TMX_TRF;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbEN_AMPHF_PDET_TRF;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbF_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbL_LOOPB_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT6;
		wxComboBox* cmbLOSS_LIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbLOSS_MAIN_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbVGCAS_TXPAD_TRF;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbLOBIASN_TXM_TRF;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbLOBIASP_TXX_TRF;
};

#endif // __lms7002_pnlTRF_view__
