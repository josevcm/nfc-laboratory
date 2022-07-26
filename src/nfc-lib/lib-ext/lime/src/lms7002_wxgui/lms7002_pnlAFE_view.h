#ifndef __lms7002_pnlAFE_view__
#define __lms7002_pnlAFE_view__

#include "lms7002_wxgui.h"
#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlAFE_view : public wxPanel
{
protected:
		void ParameterChangeHandler( wxCommandEvent& event );
public:
	lms7002_pnlAFE_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;

    enum
		{
			ID_PD_AFE = 2048,
			ID_PD_RX_AFE1,
			ID_PD_RX_AFE2,
			ID_PD_TX_AFE1,
			ID_PD_TX_AFE2,
			ID_EN_G_AFE,
			ID_ISEL_DAC_AFE,
			ID_MUX_AFE_1,
			ID_MUX_AFE_2,
			ID_MODE_INTERLEAVE_AFE
		};
		
		wxCheckBox* chkPD_AFE;
		wxCheckBox* chkPD_RX_AFE1;
		wxCheckBox* chkPD_RX_AFE2;
		wxCheckBox* chkPD_TX_AFE1;
		wxCheckBox* chkPD_TX_AFE2;
		wxCheckBox* chkEN_G_AFE;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbISEL_DAC_AFE;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbMUX_AFE_1;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbMUX_AFE_2;
		wxRadioBox* rgrMODE_INTERLEAVE_AFE;
};

#endif // __lms7002_pnlAFE_view__
