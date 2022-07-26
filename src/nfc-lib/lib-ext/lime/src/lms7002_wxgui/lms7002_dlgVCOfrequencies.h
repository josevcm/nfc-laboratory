#ifndef __lms7002_dlgVCOfrequencies__
#define __lms7002_dlgVCOfrequencies__

#include "lime/LimeSuite.h"
#include "lms7002_wxgui.h"

class lms7002_dlgVCOfrequencies : public wxDialog
{
	protected:
		// Handlers for dlgVCOfrequencies events.
		void OnBtnOkClick( wxCommandEvent& event );
		void OnBtnCancelClick( wxCommandEvent& event );
        void OnSaveFile(wxCommandEvent& event);
        void OnLoadFile(wxCommandEvent& event);
	public:
		/** Constructor */
		lms7002_dlgVCOfrequencies( wxWindow* parent, lms_device_t *plmsControl);
	//// end generated class members
protected:
    lms_device_t* lmsControl;
	wxStaticText* m_staticText341;
	wxTextCtrl* txtVCOH_low;
	wxStaticText* m_staticText342;
	wxTextCtrl* txtVCOH_high;
	wxStaticText* m_staticText3411;
	wxTextCtrl* txtVCOM_low;
	wxStaticText* m_staticText3421;
	wxTextCtrl* txtVCOM_high;
	wxStaticText* m_staticText3412;
	wxTextCtrl* txtVCOL_low;
	wxStaticText* m_staticText3422;
	wxTextCtrl* txtVCOL_high;
	wxStaticText* m_staticText3413;
	wxTextCtrl* txtVCOCGEN_low;
	wxStaticText* m_staticText3423;
	wxTextCtrl* txtVCOCGEN_high;
	wxButton* btnOk;
	wxButton* btnCancel;
	wxButton* btnLoadFile;
	wxButton* btnSaveFile;	
};

#endif // __lms7002_dlgVCOfrequencies__
