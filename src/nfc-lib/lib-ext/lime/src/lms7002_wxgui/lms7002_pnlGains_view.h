#ifndef __lms7002_pnlGains_view__
#define __lms7002_pnlGains_view__

#include "lms7002_wxgui.h"


#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlGains_view : public wxPanel
{
	protected:
	// Handlers for pnlGains_view events.
	void ParameterChangeHandler(wxSpinEvent& event);
        void ParameterChangeHandler(wxCommandEvent& event);
	public:
	lms7002_pnlGains_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
    void OnAGCStateChange(wxCommandEvent& event);
protected:
    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;

    enum
	{
		ID_EN_NEXTRX_RFE = 2048,
		ID_G_LNA_RFE,
		ID_G_TIA_RFE,
		ID_G_PGA_RBB,
		ID_C_CTL_PGA_RBB,
		ID_LOSS_LIN_TXPAD_TRF,
		ID_LOSS_MAIN_TXPAD_TRF,
		ID_CG_IAMP_TBB
	};
	
	wxCheckBox* chkTRX_GAIN_SRC;
	wxComboBox* cmbG_LNA_RFE;
	wxComboBox* cmbG_TIA_RFE;
	wxComboBox* cmbG_PGA_RBB;
	NumericSlider* cmbC_CTL_PGA_RBB;
	wxComboBox* cmbLOSS_LIN_TXPAD_TRF;
	wxComboBox* cmbLOSS_MAIN_TXPAD_TRF;
	NumericSlider* cmbCG_IAMP_TBB;
	wxCheckBox* chkAGC;
	wxStaticText* m_staticText360;
	wxTextCtrl* txtCrestFactor;
};

#endif // __lms7002_pnlGains_view__
