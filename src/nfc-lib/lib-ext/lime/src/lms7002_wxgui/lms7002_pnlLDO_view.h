#ifndef __lms7002_pnlLDO_view__
#define __lms7002_pnlLDO_view__

#include "lms7002_wxgui.h"
#include <map>
#include "lime/LimeSuite.h"

class lms7002_pnlLDO_view : public wxPanel
{		
public:
	lms7002_pnlLDO_view(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void Initialize(lms_device_t* pControl);
    void UpdateGUI();
protected:
	void ParameterChangeHandler( wxCommandEvent& event );
    lms_device_t* lmsControl;
	std::map<wxWindow*, LMS7Parameter> wndId2Enum;

	enum
		{
			ID_EN_G_LDOP = 2048,
			ID_EN_G_LDO,
			ID_EN_LDO_DIVGN,
			ID_EN_LDO_DIGGN,
			ID_EN_LDO_CPGN,
			ID_EN_LDO_VCOGN,
			ID_EN_LDO_MXRFE,
			ID_EN_LDO_LNA14,
			ID_EN_LDO_LNA12,
			ID_EN_LDO_TIA14,
			ID_EN_LDO_DIVSXT,
			ID_EN_LDO_DIGSXT,
			ID_EN_LDO_CPSXT,
			ID_EN_LDO_VCOSXT,
			ID_EN_LDO_DIVSXR,
			ID_EN_LDO_DIGSXR,
			ID_EN_LDO_CPSXR,
			ID_EN_LDO_VCOSXR,
			ID_PD_LDO_DIGIp1,
			ID_PD_LDO_DIGIp2,
			ID_EN_LDO_TXBUF,
			ID_EN_LDO_RXBUF,
			ID_EN_LDO_TPAD,
			ID_EN_LDO_TLOB,
			ID_PD_LDO_SPIBUF,
			ID_RDIV_TXBUF,
			ID_EN_LDO_AFE,
			ID_EN_LDO_TBB,
			ID_EN_LDO_RBB,
			ID_EN_LDO_TIA12,
			ID_EN_LDO_DIG,
			ID_SPDUP_LDO_TBB,
			ID_SPDUP_LDO_TIA12,
			ID_SPDUP_LDO_TIA14,
			ID_SPDUP_LDO_TLOB,
			ID_SPDUP_LDO_TPAD,
			ID_SPDUP_LDO_TXBUF,
			ID_SPDUP_LDO_VCOGN,
			ID_SPDUP_LDO_DIVSXR,
			ID_SPDUP_LDO_DIVSXT,
			ID_SPDUP_LDO_AFE,
			ID_SPDUP_LDO_CPGN,
			ID_SPDUP_LDO_VCOSXR,
			ID_SPDUP_LDO_VCOSXT,
			ID_SPDUP_LDO_DIG,
			ID_SPDUP_LDO_DIGGN,
			ID_SPDUP_LDO_DIGSXR,
			ID_SPDUP_LDO_DIGSXT,
			ID_SPDUP_LDO_DIVGN,
			ID_SPDUP_LDO_CPSXR,
			ID_SPDUP_LDO_CPSXT,
			ID_SPDUP_LDO_LNA12,
			ID_SPDUP_LDO_LNA14,
			ID_SPDUP_LDO_MXRFE,
			ID_SPDUP_LDO_RBB,
			ID_SPDUP_LDO_RXBUF,
			ID_SPDUP_LDO_SPIBUF,
			ID_SPDUP_LDO_DIGIp2,
			ID_SPDUP_LDO_DIGIp1,
			ID_EN_LOADIMP_LDO_TBB,
			ID_EN_LOADIMP_LDO_TIA12,
			ID_EN_LOADIMP_LDO_TIA14,
			ID_EN_LOADIMP_LDO_TLOB,
			ID_EN_LOADIMP_LDO_TPAD,
			ID_EN_LOADIMP_LDO_TXBUF,
			ID_EN_LOADIMP_LDO_VCOGN,
			ID_EN_LOADIMP_LDO_VCOSXR,
			ID_EN_LOADIMP_LDO_VCOSXT,
			ID_EN_LOADIMP_LDO_AFE,
			ID_EN_LOADIMP_LDO_CPGN,
			ID_EN_LOADIMP_LDO_DIVSXR,
			ID_EN_LOADIMP_LDO_DIVSXT,
			ID_EN_LOADIMP_LDO_DIG,
			ID_EN_LOADIMP_LDO_DIGGN,
			ID_EN_LOADIMP_LDO_DIGSXR,
			ID_EN_LOADIMP_LDO_DIGSXT,
			ID_EN_LOADIMP_LDO_DIVGN,
			ID_EN_LOADIMP_LDO_CPSXR,
			ID_EN_LOADIMP_LDO_CPSXT,
			ID_EN_LOADIMP_LDO_LNA12,
			ID_EN_LOADIMP_LDO_LNA14,
			ID_EN_LOADIMP_LDO_MXRFE,
			ID_EN_LOADIMP_LDO_RBB,
			ID_EN_LOADIMP_LDO_RXBUF,
			ID_EN_LOADIMP_LDO_SPIBUF,
			ID_EN_LOADIMP_LDO_DIGIp2,
			ID_EN_LOADIMP_LDO_DIGIp1,
			ID_RDIV_VCOSXR,
			ID_RDIV_VCOSXT,
			ID_RDIV_VCOGN,
			ID_RDIV_TLOB,
			ID_RDIV_TPAD,
			ID_RDIV_TIA12,
			ID_RDIV_TIA14,
			ID_RDIV_RXBUF,
			ID_RDIV_TBB,
			ID_RDIV_MXRFE,
			ID_RDIV_RBB,
			ID_RDIV_LNA12,
			ID_RDIV_LNA14,
			ID_RDIV_DIVSXR,
			ID_RDIV_DIVSXT,
			ID_RDIV_DIGSXT,
			ID_RDIV_DIVGN,
			ID_RDIV_DIGGN,
			ID_RDIV_DIGSXR,
			ID_RDIV_CPSXT,
			ID_RDIV_DIG,
			ID_RDIV_CPGN,
			ID_RDIV_CPSXR,
			ID_RDIV_SPIBUF,
			ID_RDIV_AFE,
			ID_RDIV_DIGIp2,
			ID_RDIV_DIGIp1
		};
		
		wxNotebook* ID_NOTEBOOK1;
		wxPanel* ID_PANEL3;
		wxCheckBox* chkEN_G_LDOP;
		wxCheckBox* chkEN_G_LDO;
		wxCheckBox* chkEN_LDO_DIVGN;
		wxCheckBox* chkEN_LDO_DIGGN;
		wxCheckBox* chkEN_LDO_CPGN;
		wxCheckBox* chkEN_LDO_VCOGN;
		wxCheckBox* chkEN_LDO_MXRFE;
		wxCheckBox* chkEN_LDO_LNA14;
		wxCheckBox* chkEN_LDO_LNA12;
		wxCheckBox* chkEN_LDO_TIA14;
		wxCheckBox* chkEN_LDO_DIVSXT;
		wxCheckBox* chkEN_LDO_DIGSXT;
		wxCheckBox* chkEN_LDO_CPSXT;
		wxCheckBox* chkEN_LDO_VCOSXT;
		wxCheckBox* chkEN_LDO_DIVSXR;
		wxCheckBox* chkEN_LDO_DIGSXR;
		wxCheckBox* chkEN_LDO_CPSXR;
		wxCheckBox* chkEN_LDO_VCOSXR;
		wxCheckBox* chkPD_LDO_DIGIp1;
		wxCheckBox* chkPD_LDO_DIGIp2;
		wxCheckBox* chkEN_LDO_TXBUF;
		wxCheckBox* chkEN_LDO_RXBUF;
		wxCheckBox* chkEN_LDO_TPAD;
		wxCheckBox* chkEN_LDO_TLOB;
		wxCheckBox* chkPD_LDO_SPIBUF;
		wxStaticText* m_staticText349;
		wxComboBox* cmbISINK_SPIBUFF;
		wxCheckBox* chkEN_LDO_AFE;
		wxCheckBox* chkEN_LDO_TBB;
		wxCheckBox* chkEN_LDO_RBB;
		wxCheckBox* chkEN_LDO_TIA12;
		wxCheckBox* chkEN_LDO_DIG;
		wxCheckBox* chkSPDUP_LDO_TBB;
		wxCheckBox* chkSPDUP_LDO_TIA12;
		wxCheckBox* chkSPDUP_LDO_TIA14;
		wxCheckBox* chkSPDUP_LDO_TLOB;
		wxCheckBox* chkSPDUP_LDO_TPAD;
		wxCheckBox* chkSPDUP_LDO_TXBUF;
		wxCheckBox* chkSPDUP_LDO_VCOGN;
		wxCheckBox* chkSPDUP_LDO_DIVSXR;
		wxCheckBox* chkSPDUP_LDO_DIVSXT;
		wxCheckBox* chkSPDUP_LDO_AFE;
		wxCheckBox* chkSPDUP_LDO_CPGN;
		wxCheckBox* chkSPDUP_LDO_VCOSXR;
		wxCheckBox* chkSPDUP_LDO_VCOSXT;
		wxCheckBox* chkSPDUP_LDO_DIG;
		wxCheckBox* chkSPDUP_LDO_DIGGN;
		wxCheckBox* chkSPDUP_LDO_DIGSXR;
		wxCheckBox* chkSPDUP_LDO_DIGSXT;
		wxCheckBox* chkSPDUP_LDO_DIVGN;
		wxCheckBox* chkSPDUP_LDO_CPSXR;
		wxCheckBox* chkSPDUP_LDO_CPSXT;
		wxCheckBox* chkSPDUP_LDO_LNA12;
		wxCheckBox* chkSPDUP_LDO_LNA14;
		wxCheckBox* chkSPDUP_LDO_MXRFE;
		wxCheckBox* chkSPDUP_LDO_RBB;
		wxCheckBox* chkSPDUP_LDO_RXBUF;
		wxCheckBox* chkSPDUP_LDO_SPIBUF;
		wxCheckBox* chkSPDUP_LDO_DIGIp2;
		wxCheckBox* chkSPDUP_LDO_DIGIp1;
		wxCheckBox* chkEN_LOADIMP_LDO_TBB;
		wxCheckBox* chkEN_LOADIMP_LDO_TIA12;
		wxCheckBox* chkEN_LOADIMP_LDO_TIA14;
		wxCheckBox* chkEN_LOADIMP_LDO_TLOB;
		wxCheckBox* chkEN_LOADIMP_LDO_TPAD;
		wxCheckBox* chkEN_LOADIMP_LDO_TXBUF;
		wxCheckBox* chkEN_LOADIMP_LDO_VCOGN;
		wxCheckBox* chkEN_LOADIMP_LDO_VCOSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_VCOSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_AFE;
		wxCheckBox* chkEN_LOADIMP_LDO_CPGN;
		wxCheckBox* chkEN_LOADIMP_LDO_DIVSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_DIVSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_DIG;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGGN;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_DIVGN;
		wxCheckBox* chkEN_LOADIMP_LDO_CPSXR;
		wxCheckBox* chkEN_LOADIMP_LDO_CPSXT;
		wxCheckBox* chkEN_LOADIMP_LDO_LNA12;
		wxCheckBox* chkEN_LOADIMP_LDO_LNA14;
		wxCheckBox* chkEN_LOADIMP_LDO_MXRFE;
		wxCheckBox* chkEN_LOADIMP_LDO_RBB;
		wxCheckBox* chkEN_LOADIMP_LDO_RXBUF;
		wxCheckBox* chkEN_LOADIMP_LDO_SPIBUF;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGIp2;
		wxCheckBox* chkEN_LOADIMP_LDO_DIGIp1;
		wxPanel* ID_PANEL2;
		wxStaticText* ID_STATICTEXT1;
		wxComboBox* cmbRDIV_VCOSXR;
		wxStaticText* ID_STATICTEXT2;
		wxComboBox* cmbRDIV_VCOSXT;
		wxStaticText* ID_STATICTEXT3;
		wxComboBox* cmbRDIV_TXBUF;
		wxStaticText* ID_STATICTEXT4;
		wxComboBox* cmbRDIV_VCOGN;
		wxStaticText* ID_STATICTEXT5;
		wxComboBox* cmbRDIV_TLOB;
		wxStaticText* ID_STATICTEXT6;
		wxComboBox* cmbRDIV_TPAD;
		wxStaticText* ID_STATICTEXT7;
		wxComboBox* cmbRDIV_TIA12;
		wxStaticText* ID_STATICTEXT8;
		wxComboBox* cmbRDIV_TIA14;
		wxStaticText* ID_STATICTEXT9;
		wxComboBox* cmbRDIV_RXBUF;
		wxStaticText* ID_STATICTEXT10;
		wxComboBox* cmbRDIV_TBB;
		wxStaticText* ID_STATICTEXT11;
		wxComboBox* cmbRDIV_MXRFE;
		wxStaticText* ID_STATICTEXT12;
		wxComboBox* cmbRDIV_RBB;
		wxStaticText* ID_STATICTEXT13;
		wxComboBox* cmbRDIV_LNA12;
		wxStaticText* ID_STATICTEXT14;
		wxComboBox* cmbRDIV_LNA14;
		wxStaticText* ID_STATICTEXT15;
		wxComboBox* cmbRDIV_DIVSXR;
		wxStaticText* ID_STATICTEXT16;
		wxComboBox* cmbRDIV_DIVSXT;
		wxStaticText* ID_STATICTEXT17;
		wxComboBox* cmbRDIV_DIGSXT;
		wxStaticText* ID_STATICTEXT18;
		wxComboBox* cmbRDIV_DIVGN;
		wxStaticText* ID_STATICTEXT19;
		wxComboBox* cmbRDIV_DIGGN;
		wxStaticText* ID_STATICTEXT20;
		wxComboBox* cmbRDIV_DIGSXR;
		wxStaticText* ID_STATICTEXT21;
		wxComboBox* cmbRDIV_CPSXT;
		wxStaticText* ID_STATICTEXT22;
		wxComboBox* cmbRDIV_DIG;
		wxStaticText* ID_STATICTEXT23;
		wxComboBox* cmbRDIV_CPGN;
		wxStaticText* ID_STATICTEXT24;
		wxComboBox* cmbRDIV_CPSXR;
		wxStaticText* ID_STATICTEXT25;
		wxComboBox* cmbRDIV_SPIBUF;
		wxStaticText* ID_STATICTEXT26;
		wxComboBox* cmbRDIV_AFE;
		wxStaticText* ID_STATICTEXT27;
		wxComboBox* cmbRDIV_DIGIp2;
		wxStaticText* ID_STATICTEXT28;
		wxComboBox* cmbRDIV_DIGIp1;
};

#endif // __lms7002_pnlLDO_view__
