#ifndef __lms7002_pnlRBB_view__
#define __lms7002_pnlRBB_view__

#include "lms7002_wxgui.h"
#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlRBB_view : public wxPanel
{
    protected:
        void ParameterChangeHandler( wxCommandEvent& event );
        void ParameterChangeHandler(wxSpinEvent& event);
        void OncmbBBLoopbackSelected( wxCommandEvent& event );        
        void OnbtnTuneFilter(wxCommandEvent& event);
    public:
    lms7002_pnlRBB_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
    lms_device_t* lmsControl;
    std::map<wxWindow*, LMS7Parameter> wndId2Enum;

    enum
        {
            ID_PD_LPFH_RBB = 2048,
            ID_PD_LPFL_RBB,
            ID_PD_PGA_RBB,
            ID_EN_G_RBB,
            ID_EN_DIR_RBB,
            ID_BBLoopback,
            ID_INPUT_CTL_PGA_RBB,
            ID_G_PGA_RBB,
            ID_C_CTL_PGA_RBB,
            ID_OSW_PGA_RBB,
            ID_R_CTL_LPF_RBB,
            ID_C_CTL_LPFH_RBB,
            ID_C_CTL_LPFL_RBB,
            ID_TXT_LOWBW,
            ID_BTN_TUNE_FILTER,
            ID_RCC_CTL_LPFH_RBB,
            ID_RCC_CTL_LPFL_RBB,
            ID_ICT_LPF_IN_RBB,
            ID_ICT_LPF_OUT_RBB,
            ID_ICT_PGA_OUT_RBB,
            ID_ICT_PGA_IN_RBB,
            ID_RCC_CTL_PGA_RBB
        };
        
        wxCheckBox* chkPD_LPFH_RBB;
        wxCheckBox* chkPD_LPFL_RBB;
        wxCheckBox* chkPD_PGA_RBB;
        wxCheckBox* chkEN_G_RBB;
        wxCheckBox* chkEN_DIR_RBB;
        wxStaticText* ID_STATICTEXT14;
        wxComboBox* cmbBBLoopback;
        wxStaticText* ID_STATICTEXT1;
        wxComboBox* cmbINPUT_CTL_PGA_RBB;
        wxStaticText* ID_STATICTEXT2;
        wxComboBox* cmbG_PGA_RBB;
        wxStaticText* ID_STATICTEXT3;
        NumericSlider* cmbC_CTL_PGA_RBB;
        wxRadioBox* rgrOSW_PGA_RBB;
        wxStaticText* ID_STATICTEXT4;
        wxComboBox* cmbR_CTL_LPF_RBB;
        wxStaticText* ID_STATICTEXT5;
        NumericSlider* cmbC_CTL_LPFH_RBB;
        wxStaticText* ID_STATICTEXT6;
        NumericSlider* cmbC_CTL_LPFL_RBB;
        wxStaticText* m_staticText309;
        wxTextCtrl* txtLowBW_MHz;
        wxButton* btnTuneFilter;
        wxStaticText* ID_STATICTEXT7;
        wxComboBox* cmbRCC_CTL_LPFH_RBB;
        wxStaticText* ID_STATICTEXT8;
        wxComboBox* cmbRCC_CTL_LPFL_RBB;
        wxStaticText* ID_STATICTEXT9;
        wxComboBox* cmbICT_LPF_IN_RBB;
        wxStaticText* ID_STATICTEXT10;
        wxComboBox* cmbICT_LPF_OUT_RBB;
        wxStaticText* ID_STATICTEXT11;
        wxComboBox* cmbICT_PGA_OUT_RBB;
        wxStaticText* ID_STATICTEXT12;
        wxComboBox* cmbICT_PGA_IN_RBB;
        wxStaticText* ID_STATICTEXT13;
        wxComboBox* cmbRCC_CTL_PGA_RBB;
};

#endif // __lms7002_pnlRBB_view__
