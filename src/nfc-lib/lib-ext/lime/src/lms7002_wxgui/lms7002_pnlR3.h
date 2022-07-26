#ifndef __lms7002_R3__
#define __lms7002_R3__

#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include "numericSlider.h"

#include <map>
#include <vector>
#include "lime/LimeSuite.h"
namespace lime{

}

class lms7002_pnlR3_view : public wxPanel
{
    protected:
		wxComboBox* cmbISINK_SPIBUFF;
		wxButton* btnReadADC;
		wxStaticText* dccal_statuses[8];
		wxStaticText* dccal_cmpstatuses[8];
		wxCheckBox* dccal_cmpcfg[8];
		wxCheckBox* dccal_start[8];
		wxStaticText* rssi_vals[2];
		wxStaticText* pdet_vals[2];
		wxStaticText* tref_val;
		wxStaticText* tvptat_val;
		wxStaticText* rssidc_cmpstatus;
                NumericSlider* cmbRSSIDC_HYSCMP;
                wxCheckBox* chkRSSI_PD;
                NumericSlider* spinDCO1;
                NumericSlider* spinDCO2;
                wxComboBox* cmbRSEL;
		std::vector<NumericSlider*> cmbDCControlsRx;
		std::vector<NumericSlider*> cmbDCControlsTx;

        wxStaticText* rssiCMPSTATUS[6];
        wxCheckBox* rssiCMPCFG[6];

		// Virtual event handlers, overide them in your derived class
		void ParameterChangeHandler( wxSpinEvent& event );
        void ParameterChangeHandler( wxCommandEvent& event );
        void OnRSSICMPCFGChanged(wxCommandEvent& event);
        void OnDCCMPCFGChanged(wxCommandEvent& event);
        void OnReadRSSICMP(wxCommandEvent& event);
        void OnReadDCCMP(wxCommandEvent& event);
        void OnCalibrateAnalogRSSI( wxCommandEvent& event );

        void OnWriteRxDC( wxCommandEvent& event );
        void OnWriteTxDC( wxCommandEvent& event );
        void OnDCCMPCFGRead();
        void OnReadDC( wxCommandEvent& event );
        void UpdateGUISlow();
        void ParameterChangeHandlerCMPRead( wxCommandEvent& event );
        void MCU_RunProcedure(uint8_t id);
        uint8_t MCU_WaitForStatus(uint16_t timeout_ms);
    public:
        lms7002_pnlR3_view( wxWindow* parent );
        lms7002_pnlR3_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
        ~lms7002_pnlR3_view();
        void Initialize(lms_device_t* pControl);
        void UpdateGUI();
protected:
    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;
};

#endif // __lms7002_pnlAFE_view__
