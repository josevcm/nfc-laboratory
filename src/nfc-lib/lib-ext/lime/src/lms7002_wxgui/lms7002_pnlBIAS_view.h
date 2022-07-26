#ifndef __lms7002_pnlBIAS_view__
#define __lms7002_pnlBIAS_view__

#include "lms7002_wxgui.h"

#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlBIAS_view : public wxPanel
{
	protected:
		// Handlers for pnlBIAS_view events.
		void ParameterChangeHandler( wxCommandEvent& event );
		void OnCalibrateRP_BIAS( wxCommandEvent& event );
public:
	lms7002_pnlBIAS_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_PD_FRP_BIAS = 2048,
			ID_PD_F_BIAS,
			ID_PD_PTRP_BIAS,
			ID_PD_PT_BIAS,
			ID_PD_BIAS_MASTER,
			ID_MUX_BIAS_OUT,
			ID_RP_CALIB_BIAS
		};
		
		wxCheckBox* chkPD_FRP_BIAS;
		wxCheckBox* chkPD_F_BIAS;
		wxCheckBox* chkPD_PTRP_BIAS;
		wxCheckBox* chkPD_PT_BIAS;
		wxCheckBox* chkPD_BIAS_MASTER;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbMUX_BIAS_OUT;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbRP_CALIB_BIAS;
		wxButton* btnCalibrateRP_BIAS;
		
};

#endif // __lms7002_pnlBIAS_view__
