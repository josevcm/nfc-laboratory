#ifndef lms7002_pnlRFE_view_h
#define lms7002_pnlRFE_view_h

#include "lms7002_wxgui.h"

//// end generated include
#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlRFE_view : public wxPanel
{
public:
    lms7002_pnlRFE_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
	wxCheckBox* NewCheckBox(wxWindow *parent, const LMS7Parameter &param, const wxString &label, const wxString &tooltip, const wxPoint &pos, const wxSize &size);
	wxComboBox* NewComboBox(wxWindow *parent, const LMS7Parameter &param, const wxString &tooltip, const wxPoint &pos, const wxSize &size);
    NumericSlider* NewNumericSlider(wxWindow* parent, const LMS7Parameter &param, const wxString &value, const wxPoint &pos, const wxSize &size, long style, int min, int max, int initial, const wxString &name);
    void ParameterChangeHandler( wxCommandEvent& event );
    void SpinParameterChangeHandler(wxSpinEvent& event);

    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;

    enum
        {
            ID_PD_LNA_RFE = 2048,
            ID_PD_RLOOPB_1_RFE,
            ID_PD_RLOOPB_2_RFE,
            ID_PD_MXLOBUF_RFE,
            ID_PD_QGEN_RFE,
            ID_PD_RSSI_RFE,
            ID_PD_TIA_RFE,
            ID_EN_G_RFE,
            ID_EN_DIR_RFE,
            ID_EN_INSHSW_LB1_RFE,
            ID_EN_INSHSW_LB2_RFE,
            ID_EN_INSHSW_L_RFE,
            ID_EN_INSHSW_W_RFE,
            ID_SEL_PATH_RFE,
            ID_CAP_RXMXO_RFE,
            ID_CGSIN_LNA_RFE,
            ID_RCOMP_TIA_RFE,
            ID_RFB_TIA_RFE,
            ID_EN_NEXTRX_RFE,
            ID_ICT_LNACMO_RFE,
            ID_ICT_LNA_RFE,
            ID_DCOFFI_RFE,
            ID_DCOFFQ_RFE,
            ID_ICT_LODC_RFE,
            ID_EN_DCOFF_RXFE_RFE,
            ID_CCOMP_TIA_RFE,
            ID_CFB_TIA_RFE,
            ID_G_LNA_RFE,
            ID_G_RXLOOPB_RFE,
            ID_G_TIA_RFE,
            ID_ICT_LOOPB_RFE,
            ID_ICT_TIAMAIN_RFE,
            ID_ICT_TIAOUT_RFE,
            ID_CDC_I_RFE,
            ID_CDC_Q_RFE
        };
        
        wxComboBox* cmbSEL_PATH_RFE;
        wxComboBox* cmbCAP_RXMXO_RFE;
        wxComboBox* cmbCGSIN_LNA_RFE;
        wxComboBox* cmbRCOMP_TIA_RFE;
        wxComboBox* cmbRFB_TIA_RFE;
        wxCheckBox* chkEN_NEXTRX_RFE;
        wxComboBox* cmbICT_LNACMO_RFE;
        wxComboBox* cmbICT_LNA_RFE;
        NumericSlider* cmbDCOFFI_RFE;
        NumericSlider* cmbDCOFFQ_RFE;
        wxComboBox* cmbICT_LODC_RFE;
        wxComboBox* cmbCCOMP_TIA_RFE;
        NumericSlider* cmbCFB_TIA_RFE;
        wxComboBox* cmbG_LNA_RFE;
        wxComboBox* cmbG_RXLOOPB_RFE;
        wxComboBox* cmbG_TIA_RFE;
        wxComboBox* cmbICT_LOOPB_RFE;
        NumericSlider* cmbICT_TIAMAIN_RFE;
        NumericSlider* cmbICT_TIAOUT_RFE;
        wxComboBox* cmbCDC_I_RFE;
        wxComboBox* cmbCDC_Q_RFE;
};

#endif
