#include "dlgDeviceInfo.h"


dlgDeviceInfo::dlgDeviceInfo(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
dlgDeviceInfo_view( parent, id, title, pos, size, styles)
{
    lmsControl = nullptr;

}

void dlgDeviceInfo::Initialize(lms_device_t* lms)
{
    lmsControl = lms;
}

void dlgDeviceInfo::OnGetInfo( wxCommandEvent& event )
{
    const lms_dev_info_t* info;

    if ((info = LMS_GetDeviceInfo(lmsControl))!=nullptr)
    {
        lblDeviceCtr->SetLabel(info->deviceName);
        lblExpansionCtr->SetLabel(info->expansionName);
        lblFirmwareCtr->SetLabel(info->firmwareVersion);
        lblHardwareCtr->SetLabel(info->hardwareVersion);
        lblProtocolCtr->SetLabel(info->protocolVersion);
        lblGatewareCtr->SetLabel(info->gatewareVersion);
        lblGatewareTargetCtr->SetLabel(info->gatewareTargetBoard);
    }
    else
    {
        lblDeviceCtr->SetLabel(_("???"));
        lblExpansionCtr->SetLabel(_("???"));
        lblFirmwareCtr->SetLabel(_("???"));
        lblHardwareCtr->SetLabel(_("???"));
        lblProtocolCtr->SetLabel(_("???"));
        lblGatewareCtr->SetLabel(_("???"));
        lblGatewareTargetCtr->SetLabel(_("???"));
    }


}
