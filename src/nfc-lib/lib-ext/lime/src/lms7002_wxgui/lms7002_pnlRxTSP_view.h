#ifndef __lms7002_pnlRxTSP_view__
#define __lms7002_pnlRxTSP_view__

#include "lms7002_wxgui.h"

#include <map>
#include <vector>
#include "lime/LimeSuite.h"

class lms7002_pnlRxTSP_view : public wxPanel
{
protected:
    // Handlers for pnlRxTSP_view events.
    void ParameterChangeHandler(wxSpinEvent& event);
    void ParameterChangeHandler(wxCommandEvent& event);
    void OnNCOSelectionChange(wxCommandEvent& event);
    void OnbtnReadBISTSignature(wxCommandEvent& event);
    void OnbtnLoadDCIClick(wxCommandEvent& event);
    void OnbtnLoadDCQClick(wxCommandEvent& event);
    void onbtnGFIR1Coef(wxCommandEvent& event);
    void onbtnGFIR2Coef(wxCommandEvent& event);
    void onbtnGFIR3Coef(wxCommandEvent& event);
    void OnbtnUploadNCOClick(wxCommandEvent& event);
    void OnbtnReadRSSI(wxCommandEvent& event);
    void OnbtnSetLPFClick( wxCommandEvent& event );
public:
    lms7002_pnlRxTSP_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
    void UpdateNCOinputs();
protected:
    void PHOinputChanged(wxCommandEvent& event);
    void txtFCWPHOmodeAdditional_OnMouseWheel(wxMouseEvent& event);
    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;

    std::vector<wxStaticText*> lblNCOangles;
    std::vector<wxRadioButton*> rgrNCOselections;
    std::vector<wxTextCtrl*> txtNCOinputs;

    enum
        {
            ID_EN_RXTSP = 2048,
            ID_DC_BYP_RXTSP,
            ID_GC_BYP_RXTSP,
            ID_PH_BYP_RXTSP,
            ID_CMIX_BYP_RXTSP,
            ID_AGC_BYP_RXTSP,
            ID_GFIR1_BYP_RXTSP,
            ID_GFIR2_BYP_RXTSP,
            ID_GFIR3_BYP_RXTSP,
            ID_BSTART_RXTSP,
            ID_MODE_RX,
            ID_DTHBIT_RX,
            ID_TSGSWAPIQ_RXTSP,
            ID_TSGFCW_RXTSP,
            ID_TSGMODE_RXTSP,
            ID_INSEL_RXTSP,
            ID_TSGFC_RXTSP,
            ID_DCCORR_AVG,
            ID_DC_REG_RXTSP,
            ID_CMIX_SC_RXTSP,
            ID_CMIX_GAIN_RXTSP,
            ID_HBD_OVR_RXTSP,
            ID_GFIR1_L_RXTSP,
            ID_GFIR1_N_RXTSP,
            ID_GFIR2_L_RXTSP,
            ID_GFIR2_N_RXTSP,
            ID_GFIR3_L_RXTSP,
            ID_GFIR3_N_RXTSP,
            ID_IQCORR_RXTSP,
            ID_GCORRI_RXTSP,
            ID_GCORRQ_RXTSP,
            ID_AGC_MODE_RXTSP,
            ID_AGC_AVG_RXTSP,
            ID_AGC_K_RXTSP,
            ID_AGC_ADESIRED_RXTSP
        };
        
        wxCheckBox* chkEN_RXTSP;
        wxCheckBox* chkDC_BYP_RXTSP;
        wxCheckBox* chkDC_LOOP_RXTSP;
        wxCheckBox* chkGC_BYP_RXTSP;
        wxCheckBox* chkPH_BYP_RXTSP;
        wxCheckBox* chkCMIX_BYP_RXTSP;
        wxCheckBox* chkAGC_BYP_RXTSP;
        wxCheckBox* chkGFIR1_BYP_RXTSP;
        wxCheckBox* chkGFIR2_BYP_RXTSP;
        wxCheckBox* chkGFIR3_BYP_RXTSP;
        wxStaticText* ID_STATICTEXT29;
        wxStaticText* lblBISTI;
        wxStaticText* ID_STATICTEXT30;
        wxStaticText* lblBSTATE_I;
        wxStaticText* ID_STATICTEXT31;
        wxStaticText* lblBISTQ;
        wxStaticText* ID_STATICTEXT32;
        wxStaticText* lblBSTATE_Q;
        wxCheckBox* chkBSTART_RXTSP;
        wxButton* btnReadBIST;
        wxStaticText* ID_STATICTEXT71;
        wxStaticText* lblADCI;
        wxStaticText* ID_STATICTEXT28;
        wxStaticText* lblADCQ;
        wxStaticText* ID_STATICTEXT51;
        wxStaticText* lblRSSI;
        wxCheckBox* chkCAPSEL_ADC_RXTSP;
        wxButton* ID_BUTTON1;
        wxFlexGridSizer* sizerNCOgrid;
        wxStaticText* tableTitleCol1;
        wxStaticText* tableTitleCol2;
        wxRadioButton* rgrSEL0;
        wxTextCtrl* txtFCWPHO0;
        wxStaticText* txtAnglePHO0;
        wxRadioButton* rgrSEL01;
        wxTextCtrl* txtFCWPHO01;
        wxStaticText* txtAnglePHO01;
        wxRadioButton* rgrSEL02;
        wxTextCtrl* txtFCWPHO02;
        wxStaticText* txtAnglePHO02;
        wxRadioButton* rgrSEL03;
        wxTextCtrl* txtFCWPHO03;
        wxStaticText* txtAnglePHO03;
        wxRadioButton* rgrSEL04;
        wxTextCtrl* txtFCWPHO04;
        wxStaticText* txtAnglePHO04;
        wxRadioButton* rgrSEL05;
        wxTextCtrl* txtFCWPHO05;
        wxStaticText* txtAnglePHO05;
        wxRadioButton* rgrSEL06;
        wxTextCtrl* txtFCWPHO06;
        wxStaticText* txtAnglePHO06;
        wxRadioButton* rgrSEL07;
        wxTextCtrl* txtFCWPHO07;
        wxStaticText* txtAnglePHO07;
        wxRadioButton* rgrSEL08;
        wxTextCtrl* txtFCWPHO08;
        wxStaticText* txtAnglePHO08;
        wxRadioButton* rgrSEL09;
        wxTextCtrl* txtFCWPHO09;
        wxStaticText* txtAnglePHO09;
        wxRadioButton* rgrSEL10;
        wxTextCtrl* txtFCWPHO10;
        wxStaticText* txtAnglePHO10;
        wxRadioButton* rgrSEL11;
        wxTextCtrl* txtFCWPHO11;
        wxStaticText* txtAnglePHO11;
        wxRadioButton* rgrSEL12;
        wxTextCtrl* txtFCWPHO12;
        wxStaticText* txtAnglePHO12;
        wxRadioButton* rgrSEL13;
        wxTextCtrl* txtFCWPHO13;
        wxStaticText* txtAnglePHO13;
        wxRadioButton* rgrSEL14;
        wxTextCtrl* txtFCWPHO14;
        wxStaticText* txtAnglePHO14;
        wxRadioButton* rgrSEL15;
        wxTextCtrl* txtFCWPHO15;
        wxStaticText* txtAnglePHO15;
        wxStaticText* ID_STATICTEXT21;
        wxStaticText* lblRefClk;
        wxButton* btnUploadNCO;
        wxRadioBox* rgrMODE_RX;
        wxStaticText* lblFCWPHOmodeName;
        wxTextCtrl* txtFCWPHOmodeAdditional;
        wxStaticText* ID_STATICTEXT25;
        wxComboBox* cmbDTHBIT_RX;
        wxCheckBox* chkTSGSWAPIQ_RXTSP;
        wxRadioBox* rgrTSGFCW_RXTSP;
        wxRadioBox* rgrTSGMODE_RXTSP;
        wxRadioBox* rgrINSEL_RXTSP;
        wxRadioBox* rgrTSGFC_RXTSP;
        wxComboBox* cmbDCCORR_AVG;
        wxStaticText* ID_STATICTEXT23;
        wxTextCtrl* txtDC_REG_RXTSP;
        wxButton* btnLoadDCI;
        wxButton* btnLoadDCQ;
        wxComboBox* cmbCMIX_SC_RXTSP;
        wxStaticText* ID_STATICTEXT24;
        wxComboBox* cmbCMIX_GAIN_RXTSP;
        wxStaticText* ID_STATICTEXT7;
        wxComboBox* cmbHBD_OVR_RXTSP;
        wxStaticText* ID_STATICTEXT72;
        wxComboBox* cmbHBD_DLY;
        wxStaticText* ID_STATICTEXT10;
        wxComboBox* cmbGFIR1_L_RXTSP;
        wxStaticText* ID_STATICTEXT11;
        wxSpinCtrl* cmbGFIR1_N_RXTSP;
        wxButton* btnGFIR1Coef;
        wxStaticText* ID_STATICTEXT12;
        wxComboBox* cmbGFIR2_L_RXTSP;
        wxStaticText* ID_STATICTEXT13;
        wxSpinCtrl* cmbGFIR2_N_RXTSP;
        wxButton* btnGFIR2Coef;
        wxStaticText* ID_STATICTEXT14;
        wxComboBox* cmbGFIR3_L_RXTSP;
        wxStaticText* ID_STATICTEXT15;
        wxSpinCtrl* cmbGFIR3_N_RXTSP;
        wxButton* btnGFIR3Coef;
        wxStaticText* txtBW;
        wxTextCtrl* txtLPFBW;
        wxButton* btnSetLPF;
        wxStaticText* txtRate;
        wxStaticText* txtRateVal;
        NumericSlider* cmbIQCORR_RXTSP;
        wxStaticText* ID_STATICTEXT16;
        wxStaticText* txtPhaseAlpha;
        wxStaticText* ID_STATICTEXT5;
        NumericSlider* cmbGCORRI_RXTSP;
        wxStaticText* ID_STATICTEXT4;
        NumericSlider* cmbGCORRQ_RXTSP;
        wxButton* ID_BUTTON10;
        wxStaticText* ID_STATICTEXT91;
        wxComboBox* cmbAGC_MODE_RXTSP;
        wxStaticText* ID_STATICTEXT3;
        wxComboBox* cmbAGC_AVG_RXTSP;
        wxStaticText* ID_STATICTEXT1;
        NumericSlider* spinAGC_K_RXTSP;
        wxStaticText* ID_STATICTEXT2;
        NumericSlider* cmbAGC_ADESIRED_RXTSP;
};

#endif // __lms7002_pnlRxTSP_view__
