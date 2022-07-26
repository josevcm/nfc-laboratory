#ifndef __lms7002_pnlXBUF_view__
#define __lms7002_pnlXBUF_view__

#include "lms7002_wxgui.h"

#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlXBUF_view : public wxPanel
{
	protected:
		// Handlers for pnlXBUF_view events.
		void ParameterChangeHandler( wxCommandEvent& event );
public:
	lms7002_pnlXBUF_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_PD_XBUF_RX = 2048,
			ID_PD_XBUF_TX,
			ID_EN_G_XBUF,
			ID_SLFB_XBUF_RX,
			ID_SLFB_XBUF_TX,
			ID_BYP_XBUF_RX,
			ID_BYP_XBUF_TX,
			ID_EN_OUT2_XBUF_TX,
			ID_EN_TBUFIN_XBUF_RX
		};
		
		wxCheckBox* chkPD_XBUF_RX;
		wxCheckBox* chkPD_XBUF_TX;
		wxCheckBox* chkEN_G_XBUF;
		wxCheckBox* chkSLFB_XBUF_RX;
		wxCheckBox* chkSLFB_XBUF_TX;
		wxCheckBox* chkBYP_XBUF_RX;
		wxCheckBox* chkBYP_XBUF_TX;
		wxCheckBox* chkEN_OUT2_XBUF_TX;
		wxCheckBox* chkEN_TBUFIN_XBUF_RX;
};

#endif // __lms7002_pnlXBUF_view__
