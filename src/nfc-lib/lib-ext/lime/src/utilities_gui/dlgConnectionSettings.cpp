#include "dlgConnectionSettings.h"
#include <wx/msgdlg.h>
#include <vector>
#include "lms7suiteEvents.h"
#include "ConnectionHandle.h"
#include "lms7_device.h"
#include <iso646.h> // alternative operators for visual c++: not, and, or...
using namespace std;

dlgConnectionSettings::dlgConnectionSettings( wxWindow* parent )
	: dlgConnectionSettings_view( parent )
{
	lmsOpenedIndex = -1;
	lmsControl = nullptr;
}

void dlgConnectionSettings::SetConnectionManagers(lms_device_t** lms)
{
    lmsControl = lms;
}

void dlgConnectionSettings::GetDeviceList( wxInitDialogEvent& event )
{
    mListLMS7ports->Clear();
    lms_info_str_t list[32];

    //select the currently opened index
    lmsOpenedIndex = -1;
    int ret = LMS_GetDeviceList(list);
    if (ret <= 0)
        return;
    for (int i = 0; i<ret; ++i)
    {
        const auto handle = lime::ConnectionHandle(list[i]);
        mListLMS7ports->Append(handle.ToString());
    }
    if (lmsOpenedIndex >= 0 && lmsOpenedIndex < int(mListLMS7ports->GetCount()))
        mListLMS7ports->SetSelection(lmsOpenedIndex);
}

void dlgConnectionSettings::OnConnect( wxCommandEvent& event )
{
    auto list = lime::LMS7_Device::GetDeviceList();

    const int selection = mListLMS7ports->GetSelection();
    if(selection != wxNOT_FOUND && (size_t)selection < list.size())
    {
        if (list[selection].module == "Z_Remote")
        {
            wxTextEntryDialog *dlg = new wxTextEntryDialog(this, _("Enter remote IP address"), _("Remote IP address"));
            dlg->SetTextValidator(wxFILTER_NUMERIC);
            dlg->SetValue("127.0.0.1");
            if (dlg->ShowModal() == wxID_OK)
                list[selection].addr = dlg->GetValue().mb_str() ;
            delete dlg;
        }

        auto dev = lime::LMS7_Device::CreateDevice(list[selection], (lime::LMS7_Device*)*lmsControl);
        if (!dev)
        {
            wxMessageBox(wxString(_("Failed to open device")));
            return;
        }
        *lmsControl = dev;
        wxCommandEvent evt;
        evt.SetEventType(CONTROL_PORT_CONNECTED);
        if(GetParent())
            wxPostEvent(GetParent(), evt);
    }
    EndModal(wxID_OK);
    Destroy();
}

void dlgConnectionSettings::OnCancel( wxCommandEvent& event )
{
	EndModal(wxID_CANCEL);
	Destroy();
}

void dlgConnectionSettings::OnDisconnect( wxCommandEvent& event )
{
    *lmsControl = new lime::LMS7_Device((lime::LMS7_Device*)*lmsControl);
    wxCommandEvent evt;
    evt.SetEventType(CONTROL_PORT_DISCONNECTED);
    if(GetParent())
        wxPostEvent(GetParent(), evt);
    wxInitDialogEvent tmp_evt;
    GetDeviceList(tmp_evt);
    mListLMS7ports->SetSelection(-1);
}
