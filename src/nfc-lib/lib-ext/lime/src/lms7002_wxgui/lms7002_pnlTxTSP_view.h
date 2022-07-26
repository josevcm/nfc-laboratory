#ifndef __lms7002_pnlTxTSP_view__
#define __lms7002_pnlTxTSP_view__

#include "lms7002_wxgui.h"

#include <map>
#include <vector>
#include "lime/LimeSuite.h"

class lms7002_pnlTxTSP_view : public wxPanel
{
public:
    lms7002_pnlTxTSP_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
    void UpdateNCOinputs();
protected:
    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;

    std::vector<wxStaticText*> lblNCOangles;
    std::vector<wxRadioButton*> rgrNCOselections;
    std::vector<wxTextCtrl*> txtNCOinputs;

    enum
        {
            ID_EN_TXTSP = 2048,
            ID_DC_BYP_TXTSP,
            ID_GC_BYP_TXTSP,
            ID_PH_BYP_TXTSP,
            ID_CMIX_BYP_TXTSP,
            ID_ISINC_BYP_TXTSP,
            ID_GFIR1_BYP_TXTSP,
            ID_GFIR2_BYP_TXTSP,
            ID_GFIR3_BYP_TXTSP,
            ID_BSTART_TXTSP,
            ID_BSTATE_TXTSP,
            ID_BSIGI_TXTSP,
            ID_BSIGQ_TXTSP,
            ID_MODE_TX,
            ID_DTHBIT_TX,
            ID_TSGSWAPIQ_TXTSP,
            ID_TSGFCW_TXTSP,
            ID_TSGMODE_TXTSP,
            ID_INSEL_TXTSP,
            ID_TSGFC_TXTSP,
            ID_DC_REG_TXTSP,
            ID_CMIX_SC_TXTSP,
            ID_CMIX_GAIN_TXTSP,
            ID_HBI_OVR_TXTSP,
            ID_GFIR1_L_TXTSP,
            ID_GFIR1_N_TXTSP,
            ID_GFIR2_L_TXTSP,
            ID_GFIR2_N_TXTSP,
            ID_GFIR3_L_TXTSP,
            ID_GFIR3_N_TXTSP,
            ID_IQCORR_TXTSP,
            ID_GCORRI_TXTSP,
            ID_GCORRQ_TXTSP,
            ID_DCCORRI_TXTSP,
            ID_DCCORRQ_TXTSP
        };
        
        wxCheckBox* chkEN_TXTSP;
        wxCheckBox* chkDC_BYP_TXTSP;
        wxCheckBox* chkGC_BYP_TXTSP;
        wxCheckBox* chkPH_BYP_TXTSP;
        wxCheckBox* chkCMIX_BYP_TXTSP;
        wxCheckBox* chkISINC_BYP_TXTSP;
        wxCheckBox* chkGFIR1_BYP_TXTSP;
        wxCheckBox* chkGFIR2_BYP_TXTSP;
        wxCheckBox* chkGFIR3_BYP_TXTSP;
        wxCheckBox* chkBSTART_TXTSP;
        wxStaticText* ID_STATICTEXT2;
        wxStaticText* lblBSTATE_TXTSP;
        wxStaticText* ID_STATICTEXT1;
        wxStaticText* lblBSIGI_TXTSP;
        wxStaticText* ID_STATICTEXT3;
        wxStaticText* lblBSIGQ_TXTSP;
        wxButton* btnReadBIST;
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
        wxRadioBox* rgrMODE_TX;
        wxStaticText* lblFCWPHOmodeName;
        wxTextCtrl* txtFCWPHOmodeAdditional;
        wxStaticText* ID_STATICTEXT25;
        wxComboBox* cmbDTHBIT_TX;
        wxCheckBox* chkTSGSWAPIQ_TXTSP;
        wxRadioBox* rgrTSGFCW_TXTSP;
        wxRadioBox* rgrTSGMODE_TXTSP;
        wxRadioBox* rgrINSEL_TXTSP;
        wxRadioBox* rgrTSGFC_TXTSP;
        wxStaticText* ID_STATICTEXT23;
        wxTextCtrl* txtDC_REG_TXTSP;
        wxButton* btnLoadDCI;
        wxButton* btnLoadDCQ;
        wxComboBox* cmbCMIX_SC_TXTSP;
        wxStaticText* ID_STATICTEXT24;
        wxComboBox* cmbCMIX_GAIN_TXTSP;
        wxStaticText* ID_STATICTEXT7;
        wxComboBox* cmbHBI_OVR_TXTSP;
        wxStaticText* ID_STATICTEXT10;
        wxComboBox* cmbGFIR1_L_TXTSP;
        wxStaticText* ID_STATICTEXT11;
        wxSpinCtrl* cmbGFIR1_N_TXTSP;
        wxButton* btnGFIR1Coef;
        wxStaticText* ID_STATICTEXT12;
        wxComboBox* cmbGFIR2_L_TXTSP;
        wxStaticText* ID_STATICTEXT13;
        wxSpinCtrl* cmbGFIR2_N_TXTSP;
        wxButton* btnGFIR2Coef;
        wxStaticText* ID_STATICTEXT14;
        wxComboBox* cmbGFIR3_L_TXTSP;
        wxStaticText* ID_STATICTEXT15;
        wxSpinCtrl* cmbGFIR3_N_TXTSP;
        wxButton* btnGFIR3Coef;
        wxStaticText* txtBW;
        wxTextCtrl* txtLPFBW;
        wxButton* btnSetLPF;
        wxStaticText* txtRATE;
        wxStaticText* txtRATEVAL;
        NumericSlider* cmbIQCORR_TXTSP;
        wxStaticText* ID_STATICTEXT16;
        wxStaticText* txtPhaseAlpha;
        wxStaticText* ID_STATICTEXT5;
        NumericSlider* cmbGCORRI_TXTSP;
        wxStaticText* ID_STATICTEXT4;
        NumericSlider* cmbGCORRQ_TXTSP;
        wxButton* ID_BUTTON10;
        wxStaticText* ID_STATICTEXT8;
        NumericSlider* cmbDCCORRI_TXTSP;
        wxStaticText* ID_STATICTEXT9;
        NumericSlider* cmbDCCORRQ_TXTSP;
        
        // Virtual event handlers, overide them in your derived class
        virtual void ParameterChangeHandler( wxCommandEvent& event );
        virtual void onbtnReadBISTSignature( wxCommandEvent& event );
        virtual void OnNCOSelectionChange( wxCommandEvent& event );
        virtual void PHOinputChanged( wxCommandEvent& event );
        virtual void OnbtnUploadNCOClick( wxCommandEvent& event );
        virtual void txtFCWPHOmodeAdditional_OnMouseWheel( wxMouseEvent& event );
        virtual void OnbtnLoadDCIClick( wxCommandEvent& event );
        virtual void OnbtnLoadDCQClick( wxCommandEvent& event );
        virtual void ParameterChangeHandler( wxSpinEvent& event );
        virtual void onbtnGFIR1Coef( wxCommandEvent& event );
        virtual void onbtnGFIR2Coef( wxCommandEvent& event );
        virtual void onbtnGFIR3Coef( wxCommandEvent& event );
        virtual void OnbtnSetLPFClick( wxCommandEvent& event );
};

#endif // __lms7002_pnlTxTSP_view__
