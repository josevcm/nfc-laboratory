#include "SPI_wxgui.h"
#include <vector>


SPI_wxgui::SPI_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
SPI_view( parent )
{
    ctrPort = nullptr;
}

void SPI_wxgui::Initialize(lms_device_t* pCtrPort, const size_t devIndex)
{
    ctrPort = pCtrPort;
    if (ctrPort != nullptr)
    {
        //m_rficSpiAddr = ctrPort->GetDeviceInfo().addrsLMS7002M.at(devIndex);
    }
}

void SPI_wxgui::onLMSwrite( wxCommandEvent& event )
{
    const std::vector<wxObject*> wrbtn = { btnLMSwrite, btnLMSwrite1, btnLMSwrite2, btnLMSwrite3,
                                            btnLMSwrite4, btnLMSwrite5, btnLMSwrite6, btnLMSwrite7 };
    const std::vector<wxTextCtrl*> wrAddr = { txtLMSwriteAddr, txtLMSwriteAddr1, txtLMSwriteAddr2, txtLMSwriteAddr3,
                                                txtLMSwriteAddr4, txtLMSwriteAddr5, txtLMSwriteAddr6, txtLMSwriteAddr7 };
    const std::vector<wxTextCtrl*> wrVal = { txtLMSwriteValue, txtLMSwriteValue1, txtLMSwriteValue2, txtLMSwriteValue3,
                                             txtLMSwriteValue4, txtLMSwriteValue5, txtLMSwriteValue6, txtLMSwriteValue7 };
    const std::vector<wxStaticText*> wrStatus = { lblLMSwriteStatus, lblLMSwriteStatus1, lblLMSwriteStatus2, lblLMSwriteStatus3,
                                                lblLMSwriteStatus4, lblLMSwriteStatus5, lblLMSwriteStatus6, lblLMSwriteStatus7};
    auto object = event.GetEventObject();
    unsigned int index;
    for (index = 0; index < wrbtn.size(); index++)
        if (object == wrbtn[index])
            break;
    wxString address = wrAddr[index]->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);
    wxString value = wrVal[index]->GetValue();
    long data = 0;
    value.ToLong(&data, 16);

    if (ctrPort == nullptr)
        return;

    int status;
    status = LMS_WriteLMSReg(ctrPort,addr,data);

    if (status == 0)
        wrStatus[index]->SetLabel(_("Write success"));
    else
        wrStatus[index]->SetLabel(_("Write failed"));
}

void SPI_wxgui::onLMSread( wxCommandEvent& event )
{
    const std::vector<wxObject*> rdbtn = { btnLMSread, btnLMSread1, btnLMSread2, btnLMSread3,
        btnLMSread4, btnLMSread5, btnLMSread6, btnLMSread7 };
    const std::vector<wxTextCtrl*> rdAddr = { txtLMSwriteAddr, txtLMSwriteAddr1, txtLMSwriteAddr2, txtLMSwriteAddr3,
        txtLMSwriteAddr4, txtLMSwriteAddr5, txtLMSwriteAddr6, txtLMSwriteAddr7 };
    const std::vector<wxTextCtrl*> rdVal = { txtLMSwriteValue, txtLMSwriteValue1, txtLMSwriteValue2, txtLMSwriteValue3,
        txtLMSwriteValue4, txtLMSwriteValue5, txtLMSwriteValue6, txtLMSwriteValue7 };
    const std::vector<wxStaticText*> rdStatus = { lblLMSwriteStatus, lblLMSwriteStatus1, lblLMSwriteStatus2, lblLMSwriteStatus3,
        lblLMSwriteStatus4, lblLMSwriteStatus5, lblLMSwriteStatus6, lblLMSwriteStatus7 };
    auto object = event.GetEventObject();
    unsigned int index;
    for (index = 0; index < rdbtn.size(); index++)
        if (object == rdbtn[index])
            break;

    wxString address = rdAddr[index]->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);

    if (ctrPort == nullptr)
        return;

    uint16_t dataRd = 0;
    int status;
    status = LMS_ReadLMSReg(ctrPort,addr,&dataRd);

    if (status == 0)
    {
        rdStatus[index]->SetLabel(_("Read success"));
        unsigned short value = dataRd & 0xFFFF;
        rdVal[index]->SetValue(wxString::Format(_("%04X"), value));
    }
    else
        rdStatus[index]->SetLabel(_("Read failed"));
}

void SPI_wxgui::onBoardWrite( wxCommandEvent& event )
{
    const std::vector<wxObject*> wrbtn = { ID_BUTTON24, ID_BUTTON241, ID_BUTTON242, ID_BUTTON243,
        ID_BUTTON244, ID_BUTTON245, ID_BUTTON246, ID_BUTTON247 };
    const std::vector<wxTextCtrl*> wrAddr = { txtBoardwriteAddr, txtBoardwriteAddr1, txtBoardwriteAddr2, txtBoardwriteAddr3,
        txtBoardwriteAddr4, txtBoardwriteAddr5, txtBoardwriteAddr6, txtBoardwriteAddr7 };
    const std::vector<wxTextCtrl*> wrVal = { txtBoardwriteValue, txtBoardwriteValue1, txtBoardwriteValue2, txtBoardwriteValue3,
        txtBoardwriteValue4, txtBoardwriteValue5, txtBoardwriteValue6, txtBoardwriteValue7 };
    const std::vector<wxStaticText*> wrStatus = { lblBoardwriteStatus, lblBoardwriteStatus1, lblBoardwriteStatus2, lblBoardwriteStatus3,
        lblBoardwriteStatus4, lblBoardwriteStatus5, lblBoardwriteStatus6, lblBoardwriteStatus7 };

    auto object = event.GetEventObject();
    unsigned int index;
    for (index = 0; index < wrbtn.size(); index++)
        if (object == wrbtn[index])
            break;

    wxString address = wrAddr[index]->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);
    wxString value = wrVal[index]->GetValue();
    long data = 0;
    value.ToLong(&data, 16);

    assert(ctrPort != nullptr);
    if (ctrPort == nullptr)
        return;

    int status;
    status = LMS_WriteFPGAReg(ctrPort,addr,data);

    if (status == 0)
        wrStatus[index]->SetLabel(_("Write success"));
    else
        wrStatus[index]->SetLabel(_("Write failed"));
}

void SPI_wxgui::OnBoardRead( wxCommandEvent& event )
{
    const std::vector<wxObject*> rdbtn = { ID_BUTTON25, ID_BUTTON251, ID_BUTTON252, ID_BUTTON253,
        ID_BUTTON254, ID_BUTTON255, ID_BUTTON256, ID_BUTTON257 };
    const std::vector<wxTextCtrl*> rdAddr = { txtBoardwriteAddr, txtBoardwriteAddr1, txtBoardwriteAddr2, txtBoardwriteAddr3,
        txtBoardwriteAddr4, txtBoardwriteAddr5, txtBoardwriteAddr6, txtBoardwriteAddr7 };
    const std::vector<wxTextCtrl*> rdVal = { txtBoardwriteValue, txtBoardwriteValue1, txtBoardwriteValue2, txtBoardwriteValue3,
        txtBoardwriteValue4, txtBoardwriteValue5, txtBoardwriteValue6, txtBoardwriteValue7 };
    const std::vector<wxStaticText*> rdStatus = { lblBoardwriteStatus, lblBoardwriteStatus1, lblBoardwriteStatus2, lblBoardwriteStatus3,
        lblBoardwriteStatus4, lblBoardwriteStatus5, lblBoardwriteStatus6, lblBoardwriteStatus7 };

    auto object = event.GetEventObject();
    unsigned int index;
    for (index = 0; index < rdbtn.size(); index++)
        if (object == rdbtn[index])
            break;

    wxString address = rdAddr[index]->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);

    assert(ctrPort != nullptr);
    if (ctrPort == nullptr)
        return;

    uint16_t dataRd = 0;
    int status = LMS_ReadFPGAReg(ctrPort,addr,&dataRd);

    if (status == 0)
    {
        rdStatus[index]->SetLabel(_("Read success"));
        unsigned short value = dataRd & 0xFFFF;
        rdVal[index]->SetValue(wxString::Format(_("%04X"), value));
    }
    else
        rdStatus[index]->SetLabel(_("Read failed"));
}
