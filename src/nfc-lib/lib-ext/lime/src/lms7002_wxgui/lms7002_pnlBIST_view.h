#ifndef __lms7002_pnlBIST_view__
#define __lms7002_pnlBIST_view__

#include "lms7002_wxgui.h"
#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlBIST_view : public wxPanel
{
	protected:
		// Handlers for pnlBIST_view events.
		void ParameterChangeHandler( wxCommandEvent& event );
		void onbtnReadSignature( wxCommandEvent& event );
	public:
	lms7002_pnlBIST_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_BENC = 2048,
			ID_SDM_TSTO_CGEN,
			ID_BENR,
			ID_SDM_TSTO_SXR,
			ID_BENT,
			ID_SDM_TSTO_SXT,
			ID_BSTART,
			ID_BSTATE,
			ID_BSIGT,
			ID_BSIGR,
			ID_BSIGC,
			ID_BTN_READ_SIGNATURE
		};
		
		wxCheckBox* chkBENC;
		wxCheckBox* chkSDM_TSTO_CGEN;
		wxCheckBox* chkBENR;
		wxCheckBox* chkSDM_TSTO_SXR;
		wxCheckBox* chkBENT;
		wxCheckBox* chkSDM_TSTO_SXT;
		wxCheckBox* chkBSTART;
		wxStaticText* ID_STATICTEXT2;
		wxStaticText* lblBSTATE;
		wxStaticText* ID_STATICTEXT1;
		wxStaticText* lblBSIGT;
		wxStaticText* ID_STATICTEXT3;
		wxStaticText* lblBSIGR;
		wxStaticText* ID_STATICTEXT4;
		wxStaticText* lblBSIGC;
		wxButton* btnReadSignature;
};

#endif // __lms7002_pnlBIST_view__
