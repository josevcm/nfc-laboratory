#include "lms7002_pnlRFE_view.h"
#include <wx/msgdlg.h>
#include <map>
#include <vector>
#include <assert.h>
#include "lms7002_gui_utilities.h"
#include "numericSlider.h"
#include "lms7suiteEvents.h"
#include "LMS7002M_parameters.h"
#include <tuple>

using namespace std;
using namespace lime;
using namespace LMS7002_WXGUI;

static indexValueMap g_lna_rfe_IndexValuePairs;
static indexValueMap g_tia_rfe_IndexValuePairs;

wxCheckBox* lms7002_pnlRFE_view::NewCheckBox(wxWindow *parent, const LMS7Parameter &param, const wxString &label, const wxString &tooltip=wxEmptyString, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize)
{
    int id = wxNewId();
    wxCheckBox* item = new wxCheckBox(parent, id, label, pos, size, 0);
    wndId2Enum[item] = param;
    item->SetToolTip(tooltip);
    item->Bind( wxEVT_COMMAND_CHECKBOX_CLICKED, &lms7002_pnlRFE_view::ParameterChangeHandler, this);
    return item;
}

wxComboBox* lms7002_pnlRFE_view::NewComboBox(wxWindow *parent, const LMS7Parameter &param, const wxString &tooltip=wxEmptyString, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize)
{
    int id = wxNewId();
    wxComboBox* item = new wxComboBox(parent, id, wxEmptyString, pos, size, 0, NULL, 0);
    wndId2Enum[item] = param;
    item->SetToolTip(tooltip);
    item->Bind( wxEVT_COMMAND_COMBOBOX_SELECTED, &lms7002_pnlRFE_view::ParameterChangeHandler, this);
    return item;
}

NumericSlider* lms7002_pnlRFE_view::NewNumericSlider(wxWindow* parent,
        const LMS7Parameter &param,
        const wxString &tooltip=wxEmptyString,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = wxSP_ARROW_KEYS,
        int min = 0,
        int max = 100,
        int initial = 0,
        const wxString &name = "numericSlider"
        )
{
    int id = wxNewId();
    NumericSlider* item = new NumericSlider(parent, id, wxEmptyString, pos, size, style, min, max, initial);
    wndId2Enum[item] = param;
    item->SetToolTip(tooltip);
    item->Bind( wxEVT_COMMAND_SPINCTRL_UPDATED, &lms7002_pnlRFE_view::ParameterChangeHandler, this);
    return item;
}
void SizerAddTextAndControl(wxSizer* sizer, const wxString& text, wxWindow* ctrl)
{
    sizer->Add( new wxStaticText( ctrl->GetParent(), wxID_ANY, text), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    sizer->Add( ctrl, 0, wxEXPAND, 5 );
}

lms7002_pnlRFE_view::lms7002_pnlRFE_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style), lmsControl(nullptr)
{
    wxFlexGridSizer* fgMainSizer;
    fgMainSizer = new wxFlexGridSizer( 0, 3, 0, 5 );
    fgMainSizer->SetFlexibleDirection( wxBOTH );
    fgMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    { // power downs and input shorts
        wxFlexGridSizer* fgSizer190;
        fgSizer190 = new wxFlexGridSizer( 0, 1, 5, 0 );
        fgSizer190->SetFlexibleDirection( wxBOTH );
        fgSizer190->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        
        wxStaticBoxSizer* sbSizerPowerDowns;
        {
            sbSizerPowerDowns = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Power down controls") ), wxVERTICAL );
            auto rows = {
                make_tuple( LMS7param(PD_LNA_RFE), wxT("LNA_RFE"), wxT("Power control signal for LNA_RFE")),
                make_tuple( LMS7param(PD_RLOOPB_1_RFE), wxT("Loopback 1"), wxT("Power control signal for RXFE loopback 1")),
                make_tuple( LMS7param(PD_RLOOPB_2_RFE), wxT("Loopback 2"), wxT("Power control signal for RXFE loopback 2")),
                make_tuple( LMS7param(PD_MXLOBUF_RFE), wxT("Mixer LO buffer"), wxT("Power control signal for RXFE mixer lo buffer")),
                make_tuple( LMS7param(PD_QGEN_RFE), wxT("Quadrature LO generator"), wxT("Power control signal for RXFE Quadrature LO generator")),
                make_tuple( LMS7param(PD_RSSI_RFE), wxT("RSSI"), wxT("Power control signal for RXFE RSSI")),
                make_tuple( LMS7param(PD_TIA_RFE), wxT("TIA"), wxT("Power control signal for RXFE TIA")),
                make_tuple( LMS7param(EN_G_RFE), wxT("Enable RFE module"), wxT("Enable control for all the RFE_1 power downs"))
            };

            for(const auto row : rows)
                sbSizerPowerDowns->Add( NewCheckBox(sbSizerPowerDowns->GetStaticBox(), get<0>(row), get<1>(row), get<2>(row)), wxALIGN_LEFT);

            
            wxStaticBoxSizer* sbSizer23 = new wxStaticBoxSizer( new wxStaticBox( sbSizerPowerDowns->GetStaticBox(), wxID_ANY, wxT("Direct control") ), wxHORIZONTAL );
            sbSizer23->Add( NewCheckBox(sbSizer23->GetStaticBox(), LMS7param(EN_DIR_RFE), wxT("Direct control of PDs and ENs"), wxT("Enables direct control of PDs and ENs for RFE module")), wxALIGN_LEFT);
            sbSizerPowerDowns->Add( sbSizer23, 0, wxALL, 5 );
        }
        fgSizer190->Add( sbSizerPowerDowns, 1, wxEXPAND, 5 );
        
        wxStaticBoxSizer* sbSizerInputShorting;
        {
            sbSizerInputShorting = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Input shorting switches") ), wxVERTICAL );
            auto rows = {
                make_tuple( LMS7param(EN_INSHSW_LB1_RFE), wxT("input of loopback 1"), wxT("Enables the input shorting switch at the input  of the loopback 1 (in parallel with LNAL mixer)")),
                make_tuple( LMS7param(EN_INSHSW_LB2_RFE), wxT("input of loopback 2"), wxT("Enables the input shorting switch at the input  of the loopback 2 (in parallel with LNAW mixer)")),
                make_tuple( LMS7param(EN_INSHSW_L_RFE), wxT("input of LNAL"), wxT("Enables the input shorting switch at the input  of the LNAL")),
                make_tuple( LMS7param(EN_INSHSW_W_RFE), wxT("input of LNAW"), wxT("Enables the input shorting switch at the input  of the LNAW"))
            };        
            for(const auto row : rows)
                sbSizerInputShorting->Add( NewCheckBox(sbSizerInputShorting->GetStaticBox(), get<0>(row), get<1>(row), get<2>(row)), 0, wxALIGN_LEFT, 0);
        }
        fgSizer190->Add( sbSizerInputShorting, 1, wxEXPAND, 5 );
        fgMainSizer->Add( fgSizer190, 0, wxEXPAND, 5 );    
    }
    
    wxFlexGridSizer* fgSizer41;
    fgSizer41 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer41->SetFlexibleDirection( wxBOTH );
    fgSizer41->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    {
        wxFlexGridSizer* fgSizer43;
        fgSizer43 = new wxFlexGridSizer( 0, 2, 0, 5 );
        fgSizer43->SetFlexibleDirection( wxBOTH );
        fgSizer43->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        
        wxComboBox* cmbSEL_PATH_RFE = NewComboBox(this, LMS7param(SEL_PATH_RFE));
        cmbSEL_PATH_RFE->Append( wxT("No path active") );
        cmbSEL_PATH_RFE->Append( wxT("LNAH") );
        cmbSEL_PATH_RFE->Append( wxT("LNAL") );
        cmbSEL_PATH_RFE->Append( wxT("LNAW") );
        cmbSEL_PATH_RFE->SetToolTip( wxT("Selects the active path of the RXFE") );
        SizerAddTextAndControl(fgSizer43, wxT("Active path to the RXFE"), cmbSEL_PATH_RFE);
        
        cmbCAP_RXMXO_RFE = NewComboBox( this, LMS7param(CAP_RXMXO_RFE));
        cmbCAP_RXMXO_RFE->SetToolTip( wxT("Control the decoupling cap at the output of the RX Mixer") );    
        SizerAddTextAndControl(fgSizer43, wxT("Decoupling cap at output of RX mixer"), cmbCAP_RXMXO_RFE);
        
        cmbCGSIN_LNA_RFE = NewComboBox( this, LMS7param(CGSIN_LNA_RFE)); 
        cmbCGSIN_LNA_RFE->SetToolTip( wxT("Controls the cap parallel with the LNA input input NMOS CGS to control the Q of the maching circuit and provides trade off between gain/NF and IIP. The higher the frequency, the lower CGSIN_LNA_RFE should be. Also, the higher CGSIN, the lower the Q, The lower the gain, the higher the NF, and the higher the IIP3") );
        SizerAddTextAndControl(fgSizer43, wxT("Controls cap parallel with the LNA input"), cmbCGSIN_LNA_RFE);
        
        cmbRCOMP_TIA_RFE = NewComboBox( this, LMS7param(RCOMP_TIA_RFE));
        cmbRCOMP_TIA_RFE->SetToolTip( wxT("Controls the compensation resistors of the TIA opamp") );
        SizerAddTextAndControl(fgSizer43, wxT("Compensation resistor of TIA opamp"), cmbRCOMP_TIA_RFE);
        
        cmbRFB_TIA_RFE = NewComboBox( this, LMS7param(RFB_TIA_RFE));
        cmbRFB_TIA_RFE->SetToolTip( wxT("Sets the feedback resistor to the nominal value") );
        SizerAddTextAndControl(fgSizer43, wxT("Sets feedback resistor to nominal value"), cmbRFB_TIA_RFE);
        
        fgSizer43->Add( chkEN_NEXTRX_RFE = NewCheckBox(this, LMS7param(EN_NEXTRX_RFE), wxT("Enable Rx MIMO mode"), wxT("Enables the daisy chain LO buffer going from RXFE1  to RXFE2")), 1, wxALIGN_LEFT|wxALIGN_TOP, 5 );
        fgSizer41->Add( fgSizer43, 0, wxEXPAND, 0 );
    }
    wxStaticBoxSizer* sbSizerCurrentControl;
    sbSizerCurrentControl = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Current control") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer47;
    fgSizer47 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer47->AddGrowableCol( 1 );
    fgSizer47->SetFlexibleDirection( wxBOTH );
    fgSizer47->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxComboBox* cmbICT_LNACMO_RFE = NewComboBox( sbSizerCurrentControl->GetStaticBox(), LMS7param(ICT_LNACMO_RFE));
    cmbICT_LNACMO_RFE->SetToolTip( wxT("Controls the current generating LNA output common mode voltage") );
    SizerAddTextAndControl(fgSizer47, wxT("LNA output common mode voltage"), cmbICT_LNACMO_RFE);
    
    wxComboBox* cmbICT_LNA_RFE = NewComboBox( sbSizerCurrentControl->GetStaticBox(), LMS7param(ICT_LNA_RFE)); 
    cmbICT_LNA_RFE->SetToolTip( wxT("Controls the current of the LNA core") );
    SizerAddTextAndControl(fgSizer47, wxT("LNA core"), cmbICT_LNA_RFE);
    
    sbSizerCurrentControl->Add( fgSizer47, 0, wxEXPAND, 0 );
    
    
    fgSizer41->Add( sbSizerCurrentControl, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerDC;
    sbSizerDC = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("DC") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer45;
    fgSizer45 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer45->AddGrowableCol( 1 );
    fgSizer45->SetFlexibleDirection( wxBOTH );
    fgSizer45->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    cmbDCOFFI_RFE = NewNumericSlider( sbSizerDC->GetStaticBox(), LMS7param(DCOFFI_RFE), wxT("Controls DC offset at the output of the TIA by injecting current to the input of the TIA (I side)"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
    SizerAddTextAndControl(fgSizer45, wxT("Offset I"), cmbDCOFFI_RFE);
    
    cmbDCOFFQ_RFE = NewNumericSlider( sbSizerDC->GetStaticBox(), LMS7param(DCOFFQ_RFE), wxT("Controls DC offset at the output of the TIA by injecting current to the input of the TIA (Q side)"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -63, 63, 0 );
    SizerAddTextAndControl(fgSizer45, wxT("Offset Q"), cmbDCOFFQ_RFE);
    
    cmbICT_LODC_RFE = NewComboBox( sbSizerDC->GetStaticBox(), LMS7param(ICT_LODC_RFE)); 
    cmbICT_LODC_RFE->SetToolTip( wxT("Controls the DC of the mixer LO signal at the gate of the mixer switches") );
    SizerAddTextAndControl(fgSizer45, wxT("Mixer LO signal"), cmbICT_LODC_RFE);
    
    fgSizer45->Add( 0, 0, 1, wxEXPAND, 5 );
    wxCheckBox* chkEN_DCOFF_RXFE_RFE = new wxCheckBox( sbSizerDC->GetStaticBox(), ID_EN_DCOFF_RXFE_RFE, wxT("Enable DC offset"), wxDefaultPosition, wxDefaultSize, 0 );
    chkEN_DCOFF_RXFE_RFE->SetToolTip( wxT("Enables the DCOFFSET block for the RXFE") );
    fgSizer45->Add( chkEN_DCOFF_RXFE_RFE, 0, wxEXPAND|wxLEFT, 5 );
    
    
    sbSizerDC->Add( fgSizer45, 0, wxALL|wxEXPAND, 0 );
    fgSizer41->Add( sbSizerDC, 1, wxEXPAND, 5 );
    
    fgMainSizer->Add( fgSizer41, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer191;
    fgSizer191 = new wxFlexGridSizer( 0, 1, 5, 0 );
    fgSizer191->SetFlexibleDirection( wxBOTH );
    fgSizer191->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizerCapacitorControls;
    sbSizerCapacitorControls = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Capacitor controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer49;
    fgSizer49 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer49->AddGrowableCol( 1 );
    fgSizer49->SetFlexibleDirection( wxBOTH );
    fgSizer49->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    fgSizer49->Add( new wxStaticText( sbSizerCapacitorControls->GetStaticBox(), wxID_ANY, wxT("Compensation TIA")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCCOMP_TIA_RFE = NewComboBox( sbSizerCapacitorControls->GetStaticBox(), LMS7param(CCOMP_TIA_RFE)); 
    cmbCCOMP_TIA_RFE->SetToolTip( wxT("Compensation capacitor for TIA") );
    
    fgSizer49->Add( cmbCCOMP_TIA_RFE, 1, wxEXPAND, 0 );
    
    fgSizer49->Add( new wxStaticText( sbSizerCapacitorControls->GetStaticBox(), wxID_ANY, wxT("Feedback TIA")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0 );
    
    cmbCFB_TIA_RFE = NewNumericSlider( sbSizerCapacitorControls->GetStaticBox(), LMS7param(CFB_TIA_RFE), wxT("Feedback capacitor for TIA. Controls the 3dB BW of the TIA. Should be set with calibration through digital baseband."), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 4095, 0 );
    fgSizer49->Add( cmbCFB_TIA_RFE, 1, wxEXPAND, 5 );
    
    
    sbSizerCapacitorControls->Add( fgSizer49, 0, wxEXPAND, 0 );
    
    
    fgSizer191->Add( sbSizerCapacitorControls, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerGainControls;
    sbSizerGainControls = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Gain controls") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer50;
    fgSizer50 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer50->AddGrowableCol( 0 );
    fgSizer50->SetFlexibleDirection( wxBOTH );
    fgSizer50->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    cmbG_LNA_RFE = NewComboBox( sbSizerGainControls->GetStaticBox(), LMS7param(G_LNA_RFE)); 
    cmbG_LNA_RFE->SetToolTip( wxT("Controls the gain of the LNA") );
    SizerAddTextAndControl(fgSizer50, wxT("LNA"), cmbG_LNA_RFE);

    cmbG_RXLOOPB_RFE = NewComboBox( sbSizerGainControls->GetStaticBox(), LMS7param(G_RXLOOPB_RFE)); 
    cmbG_RXLOOPB_RFE->SetToolTip( wxT("Controls RXFE loopback gain") );
    SizerAddTextAndControl(fgSizer50, wxT("Loopback"), cmbG_RXLOOPB_RFE);
    
    cmbG_TIA_RFE = NewComboBox( sbSizerGainControls->GetStaticBox(), LMS7param(G_TIA_RFE)); 
    cmbG_TIA_RFE->SetToolTip( wxT("Controls the Gain of the TIA") );
    SizerAddTextAndControl(fgSizer50, wxT("TIA"), cmbG_TIA_RFE);
    
    sbSizerGainControls->Add( fgSizer50, 0, wxALIGN_LEFT, 0 );
    
    
    fgSizer191->Add( sbSizerGainControls, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerRefCurrent;
    sbSizerRefCurrent = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Reference current") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer46;
    fgSizer46 = new wxFlexGridSizer( 0, 2, 0, 5 );
    fgSizer46->AddGrowableCol( 1 );
    fgSizer46->SetFlexibleDirection( wxBOTH );
    fgSizer46->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    cmbICT_LOOPB_RFE = NewComboBox( sbSizerRefCurrent->GetStaticBox(), LMS7param(ICT_LOOPB_RFE)); 
    cmbICT_LOOPB_RFE->SetToolTip( wxT("Controls the reference current of the RXFE loopback amplifier") );
    SizerAddTextAndControl(fgSizer46, wxT("Loopback amplifier"), cmbICT_LOOPB_RFE);
    
    cmbICT_TIAMAIN_RFE = NewNumericSlider( sbSizerRefCurrent->GetStaticBox(), LMS7param(ICT_TIAMAIN_RFE), wxT("Controls the reference current of the RXFE TIA first stage"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
    SizerAddTextAndControl(fgSizer46, wxT("TIA 1st stage"), cmbICT_TIAMAIN_RFE);

    cmbICT_TIAOUT_RFE = NewNumericSlider( sbSizerRefCurrent->GetStaticBox(), LMS7param(ICT_TIAOUT_RFE), wxT("Controls the reference current of the RXFE TIA first stage"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 31, 0 );
    SizerAddTextAndControl(fgSizer46, wxT("TIA 2nd stage"), cmbICT_TIAOUT_RFE);
    
    sbSizerRefCurrent->Add( fgSizer46, 0, wxEXPAND, 0 );
    
    
    fgSizer191->Add( sbSizerRefCurrent, 0, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizerTrimDuty;
    sbSizerTrimDuty = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Trim duty cycle") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer51;
    fgSizer51 = new wxFlexGridSizer( 2, 2, 0, 0 );
    fgSizer51->AddGrowableCol( 0 );
    fgSizer51->AddGrowableCol( 1 );
    fgSizer51->SetFlexibleDirection( wxBOTH );
    fgSizer51->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    fgSizer51->Add( new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("I channel:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbCDC_I_RFE = NewComboBox( sbSizerTrimDuty->GetStaticBox(), LMS7param(CDC_I_RFE));
    fgSizer51->Add( cmbCDC_I_RFE, 0, wxEXPAND, 5 );
    
    fgSizer51->Add( new wxStaticText( sbSizerTrimDuty->GetStaticBox(), wxID_ANY, wxT("Q channel:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5 );
    
    cmbCDC_Q_RFE = NewComboBox( sbSizerTrimDuty->GetStaticBox(), LMS7param(CDC_Q_RFE)); 
    fgSizer51->Add( cmbCDC_Q_RFE, 0, wxEXPAND, 5 );
    
    
    sbSizerTrimDuty->Add( fgSizer51, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgSizer191->Add( sbSizerTrimDuty, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5 );
    
    
    fgMainSizer->Add( fgSizer191, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( fgMainSizer );
    this->Layout();
    fgMainSizer->Fit( this );

    wxArrayString temp;
    temp.clear();
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i fF"), (i + 1) * 80));
    cmbCAP_RXMXO_RFE->Set(temp);

    for (int i = 0; i < 15; ++i)
        g_lna_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-30"));
    temp.push_back(_("Gmax-27"));
    temp.push_back(_("Gmax-24"));
    temp.push_back(_("Gmax-21"));
    temp.push_back(_("Gmax-18"));
    temp.push_back(_("Gmax-15"));
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-9"));
    temp.push_back(_("Gmax-6"));
    temp.push_back(_("Gmax-5"));
    temp.push_back(_("Gmax-4"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax-2"));
    temp.push_back(_("Gmax-1"));
    temp.push_back(_("Gmax"));
    cmbG_LNA_RFE->Set(temp);

    temp.clear();
    temp.push_back(_("Gmax-40"));
    temp.push_back(_("Gmax-24"));
    temp.push_back(_("Gmax-17"));
    temp.push_back(_("Gmax-14"));
    temp.push_back(_("Gmax-11"));
    temp.push_back(_("Gmax-9"));
    temp.push_back(_("Gmax-7.5"));
    temp.push_back(_("Gmax-6.2"));
    temp.push_back(_("Gmax-5"));
    temp.push_back(_("Gmax-4"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax-2.4"));
    temp.push_back(_("Gmax-1.6"));
    temp.push_back(_("Gmax-1"));
    temp.push_back(_("Gmax-0.5"));
    temp.push_back(_("Gmax"));
    cmbG_RXLOOPB_RFE->Set(temp);


    for (int i = 0; i < 3; ++i)
        g_tia_rfe_IndexValuePairs.push_back(indexValuePair(i, i + 1));
    temp.clear();
    temp.push_back(_("Gmax-12"));
    temp.push_back(_("Gmax-3"));
    temp.push_back(_("Gmax"));
    cmbG_TIA_RFE->Set(temp);

    temp.clear();
    float nominalCurrent = 500; //uA
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.1f uA"), nominalCurrent*(i / 12.0)));
    cmbICT_LNA_RFE->Set(temp);

    temp.clear();
    float Vth = 0.440; //V
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.3f V"), Vth + 3500 * 0.000020*(i / 12.0)));
    cmbICT_LODC_RFE->Set(temp);

    temp.clear();
    float IsupplyNominal = 1.80; //uA
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%.3f uA"), IsupplyNominal*(i / 12.0)));
    cmbICT_LOOPB_RFE->Set(temp);

    temp.clear();
    for (int i = 0; i<32; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbRFB_TIA_RFE->Set(temp);
    cmbICT_LNACMO_RFE->Set(temp);
    cmbCGSIN_LNA_RFE->Set(temp);

    temp.clear();
    for (int i = 0; i<16; ++i)
        temp.push_back(wxString::Format(_("%i"), i));
    cmbRCOMP_TIA_RFE->Set(temp);
    cmbCDC_I_RFE->Set(temp);
    cmbCDC_Q_RFE->Set(temp);
    cmbCCOMP_TIA_RFE->Set(temp);

    LMS7002_WXGUI::UpdateTooltips(wndId2Enum, true);
}

void lms7002_pnlRFE_view::Initialize(lms_device_t* pControl)
{
	lmsControl = pControl;
	assert(lmsControl != nullptr);
}

void lms7002_pnlRFE_view::UpdateGUI()
{
    LMS7002_WXGUI::UpdateControlsByMap(this, lmsControl, wndId2Enum);

    uint16_t value;
    LMS_ReadParam(lmsControl,LMS7param(G_LNA_RFE),&value);
    cmbG_LNA_RFE->SetSelection( value2index(value, g_lna_rfe_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(G_TIA_RFE),&value);
    cmbG_TIA_RFE->SetSelection( value2index(value, g_tia_rfe_IndexValuePairs));

    LMS_ReadParam(lmsControl,LMS7param(DCOFFI_RFE),&value);
    int16_t dcvalue = value & 0x3F;
    if((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFI_RFE->SetValue(dcvalue);
    LMS_ReadParam(lmsControl,LMS7param(DCOFFQ_RFE),&value);
    dcvalue = value & 0x3F;
    if((value & 0x40) != 0)
        dcvalue *= -1;
    cmbDCOFFQ_RFE->SetValue(dcvalue);

    //check if B channel is enabled
    uint16_t macBck;
    LMS_ReadParam(lmsControl,LMS7param(MAC),&macBck);
    if (macBck >= 2)
        chkEN_NEXTRX_RFE->Hide();
    else
        chkEN_NEXTRX_RFE->Show();

    LMS_ReadParam(lmsControl,LMS7param(TRX_GAIN_SRC),&value);
    cmbG_LNA_RFE->Enable(!value);
    cmbG_TIA_RFE->Enable(!value);
}

void lms7002_pnlRFE_view::ParameterChangeHandler( wxCommandEvent& event )
{
    assert(lmsControl != nullptr);
    LMS7Parameter parameter;
    try
    {
        parameter = wndId2Enum.at(reinterpret_cast<wxWindow*>(event.GetEventObject()));
    }
    catch (std::exception & e)
    {
        std::cout << "Control element(ID = " << event.GetId() << ") don't have assigned LMS parameter." << std::endl;
        return;
    }
    long value = event.GetInt();
    if (event.GetEventObject() == cmbG_LNA_RFE)
    {
        value = index2value(value, g_lna_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbG_TIA_RFE)
    {
        value = index2value(value, g_tia_rfe_IndexValuePairs);
    }
    else if (event.GetEventObject() == cmbDCOFFI_RFE || event.GetEventObject() == cmbDCOFFQ_RFE)
    {
        uint16_t valToSend = 0;
        if (value < 0)
            valToSend |= 0x40;
        valToSend |= labs(value);
        LMS_WriteParam(lmsControl,parameter, valToSend);
        return;
    }
    LMS_WriteParam(lmsControl,parameter,value);
}
