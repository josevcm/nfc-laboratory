#ifndef __lms7002_pnlLimeLightPAD_view__
#define __lms7002_pnlLimeLightPAD_view__

#include "lms7002_wxgui.h"

#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlLimeLightPAD_view : public wxPanel
{
		
public:
	lms7002_pnlLimeLightPAD_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
	void ParameterChangeHandler( wxCommandEvent& event );
    void ParameterChangeHandler(wxSpinEvent& event);
	void onbtnReadVerRevMask( wxCommandEvent& event );
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_SDA_PE = 2048,
			ID_SCL_PE,
			ID_RX_CLK_PE,
			ID_SDIO_PE,
			ID_SDO_PE,
			ID_TX_CLK_PE,
			ID_SEN_PE,
			ID_DIQ1_PE,
			ID_TXNRX1_PE,
			ID_SCLK_PE,
			ID_DIQ2_PE,
			ID_TXNRX2_PE,
			ID_IQ_SEL_EN_1_PE,
			ID_FCLK1_PE,
			ID_MCLK1_PE,
			ID_IQ_SEL_EN_2_PE,
			ID_FCLK2_PE,
			ID_MCLK2_PE,
			ID_SRST_RXFIFO,
			ID_SRST_TXFIFO,
			ID_LRST_TX_A,
			ID_LRST_TX_B,
			ID_LRST_RX_A,
			ID_LRST_RX_B,
			ID_MRST_TX_A,
			ID_MRST_TX_B,
			ID_MRST_RX_A,
			ID_MRST_RX_B,
			ID_RXEN_A,
			ID_TXEN_A,
			ID_RXEN_B,
			ID_TXEN_B,
			ID_MIMO_SISO,
			ID_MOD_EN,
			ID_LML_FIDM1,
			ID_LML_FIDM2,
			ID_LML_MODE1,
			ID_LML_MODE2,
			ID_LML_TXNRXIQ1,
			ID_LML_TXNRXIQ2,
			ID_SDA_DS,
			ID_SCL_DS,
			ID_SDIO_DS,
			ID_DIQ2_DS,
			ID_DIQ1_DS,
			ID_RX_MUX,
			ID_TX_MUX,
			ID_SPIMODE,
			ID_VER,
			ID_REV,
			ID_MASK,
			ID_LML1_S3S,
			ID_LML1_S2S,
			ID_LML1_S1S,
			ID_LML1_S0S,
			ID_LML2_S3S,
			ID_LML2_S2S,
			ID_LML2_S1S,
			ID_LML2_S0S,
			ID_LML1_BQP,
			ID_LML1_BIP,
			ID_LML1_AQP,
			ID_LML1_AIP,
			ID_LML2_BQP,
			ID_LML2_BIP,
			ID_LML2_AQP,
			ID_LML2_AIP,
			ID_TXRDCLK_MUX,
			ID_TXWRCLK_MUX,
			ID_RXRDCLK_MUX,
			ID_RXWRCLK_MUX,
			ID_MCLK2SRC,
			ID_MCLK1SRC,
			ID_TXDIVEN,
			ID_TXTSPCLKA_DIV,
			ID_RXDIVEN,
			ID_RXTSPCLKA_DIV,
			ID_MCLK1DLY,
			ID_MCLK2DLY,
			ID_FCLK1_INV,
			ID_FCLK2_INV,
			ID_FCLK1DLY,
			ID_FCLK2DLY,
			ID_LML1_TX_PST,
			ID_LML1_TX_PRE,
			ID_LML1_RX_PST,
			ID_LML1_RX_PRE,
			ID_LML2_TX_PST,
			ID_LML2_TX_PRE,
			ID_LML2_RX_PST,
			ID_LML2_RX_PRE,
			ID_DIQDIRCTR1,
			ID_DIQDIR1,
			ID_ENABLEDIRCTR1,
			ID_ENABLEDIR1,
			ID_DIQDIRCTR2,
			ID_DIQDIR2,
			ID_ENABLEDIRCTR2,
			ID_ENABLEDIR2
		};
		
		wxNotebook* ID_NOTEBOOK1;
		wxPanel* ID_PANEL1;
		wxCheckBox* chkSDA_PE;
		wxCheckBox* chkSCL_PE;
		wxCheckBox* chkRX_CLK_PE;
		wxCheckBox* chkSDIO_PE;
		wxCheckBox* chkSDO_PE;
		wxCheckBox* chkTX_CLK_PE;
		wxCheckBox* chkSEN_PE;
		wxCheckBox* chkDIQ1_PE;
		wxCheckBox* chkTXNRX1_PE;
		wxCheckBox* chkSCLK_PE;
		wxCheckBox* chkDIQ2_PE;
		wxCheckBox* chkTXNRX2_PE;
		wxCheckBox* chkIQ_SEL_EN_1_PE;
		wxCheckBox* chkFCLK1_PE;
		wxCheckBox* chkMCLK1_PE;
		wxCheckBox* chkIQ_SEL_EN_2_PE;
		wxCheckBox* chkFCLK2_PE;
		wxCheckBox* chkMCLK2_PE;
		wxCheckBox* chkSRST_RXFIFO;
		wxCheckBox* chkSRST_TXFIFO;
		wxCheckBox* chkLRST_TX_A;
		wxCheckBox* chkLRST_TX_B;
		wxCheckBox* chkLRST_RX_A;
		wxCheckBox* chkLRST_RX_B;
		wxCheckBox* chkMRST_TX_A;
		wxCheckBox* chkMRST_TX_B;
		wxCheckBox* chkMRST_RX_A;
		wxCheckBox* chkMRST_RX_B;
		wxCheckBox* chkRXEN_A;
		wxCheckBox* chkTXEN_A;
		wxCheckBox* chkRXEN_B;
		wxCheckBox* chkTXEN_B;
		wxCheckBox* chkMIMO_SISO;
		wxCheckBox* chkMOD_EN;
		wxRadioBox* rgrLML_FIDM1;
		wxRadioBox* rgrLML_FIDM2;
		wxRadioBox* rgrLML_MODE1;
		wxRadioBox* rgrLML_MODE2;
		wxRadioBox* rgrLML_TXNRXIQ1;
		wxRadioBox* rgrLML_TXNRXIQ2;
		wxCheckBox* chkLML1_TRXIQPULSE;
		wxCheckBox* chkLML2_TRXIQPULSE;
		wxCheckBox* chkLML1_SISODDR;
		wxCheckBox* chkLML2_SISODDR;
		wxRadioBox* rgrSDA_DS;
		wxRadioBox* rgrSCL_DS;
		wxRadioBox* rgrSDIO_DS;
		wxRadioBox* rgrDIQ2_DS;
		wxRadioBox* rgrDIQ1_DS;
		wxRadioBox* rgrRX_MUX;
		wxRadioBox* rgrTX_MUX;
		wxRadioBox* rgrSPIMODE;
		wxStaticText* ID_STATICTEXT36;
		wxStaticText* lblVER;
		wxStaticText* ID_STATICTEXT38;
		wxStaticText* lblREV;
		wxStaticText* ID_STATICTEXT40;
		wxStaticText* lblMASK;
		wxButton* btnReadVerRevMask;
		wxPanel* ID_PANEL2;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbLML1_S3S;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbLML1_S2S;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbLML1_S1S;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbLML1_S0S;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbLML2_S3S;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbLML2_S2S;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbLML2_S1S;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbLML2_S0S;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbLML1_BQP;
		wxStaticText* ID_STATICTEXT6;
		wxComboBox* cmbLML1_BIP;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbLML1_AQP;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbLML1_AIP;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbLML2_BQP;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbLML2_BIP;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbLML2_AQP;
		wxStaticText* ID_STATICTEXT16;
		wxComboBox* cmbLML2_AIP;
		wxStaticText* ID_STATICTEXT25;
		wxComboBox* cmbTXRDCLK_MUX;
		wxStaticText* ID_STATICTEXT26;
		wxComboBox* cmbTXWRCLK_MUX;
		wxStaticText* ID_STATICTEXT27;
		wxComboBox* cmbRXRDCLK_MUX;
		wxStaticText* ID_STATICTEXT28;
		wxComboBox* cmbRXWRCLK_MUX;
		wxStaticText* ID_STATICTEXT29;
		wxComboBox* cmbMCLK2SRC;
		wxStaticText* ID_STATICTEXT30;
		wxComboBox* cmbMCLK1SRC;
		wxCheckBox* chkTXDIVEN;
		wxSpinCtrl* cmbTXTSPCLKA_DIV;
		wxCheckBox* chkRXDIVEN;
		wxSpinCtrl* cmbRXTSPCLKA_DIV;
		wxStaticText* ID_STATICTEXT37;
		wxStaticText* ID_STATICTEXT39;
		wxCheckBox* chkFCLK1_INV;
		wxCheckBox* chkFCLK2_INV;
		wxCheckBox* chkMCLK1_INV;
		wxCheckBox* chkMCLK2_INV;
		wxStaticText* ID_STATICTEXT301;
		wxComboBox* cmbFCLK1_DLY;
		wxStaticText* ID_STATICTEXT302;
		wxComboBox* cmbFCLK2_DLY;
		wxStaticText* ID_STATICTEXT303;
		wxComboBox* cmbMCLK1_DLY;
		wxStaticText* ID_STATICTEXT304;
		wxComboBox* cmbMCLK2_DLY;
		wxStaticText* ID_STATICTEXT17;
		wxSpinCtrl* cmbLML1_TX_PST;
		wxStaticText* ID_STATICTEXT18;
		wxSpinCtrl* cmbLML1_TX_PRE;
		wxStaticText* ID_STATICTEXT19;
		wxSpinCtrl* cmbLML1_RX_PST;
		wxStaticText* ID_STATICTEXT20;
		wxSpinCtrl* cmbLML1_RX_PRE;
		wxStaticText* ID_STATICTEXT21;
		wxSpinCtrl* cmbLML2_TX_PST;
		wxStaticText* ID_STATICTEXT22;
		wxSpinCtrl* cmbLML2_TX_PRE;
		wxStaticText* ID_STATICTEXT23;
		wxSpinCtrl* cmbLML2_RX_PST;
		wxStaticText* ID_STATICTEXT24;
		wxSpinCtrl* cmbLML2_RX_PRE;
		wxRadioBox* rgrDIQDIRCTR1;
		wxRadioBox* rgrDIQDIR1;
		wxRadioBox* rgrENABLEDIRCTR1;
		wxRadioBox* rgrENABLEDIR1;
		wxRadioBox* rgrDIQDIRCTR2;
		wxRadioBox* rgrDIQDIR2;
		wxRadioBox* rgrENABLEDIRCTR2;
		wxRadioBox* rgrENABLEDIR2;
};

#endif // __lms7002_pnlLimeLightPAD_view__
