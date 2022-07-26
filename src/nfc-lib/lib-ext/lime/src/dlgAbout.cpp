#include "dlgAbout.h"
#include "VersionInfo.h"
#include "resources/splash.h"
dlgAbout::dlgAbout( wxWindow* parent )
:
dlgAbout_view( parent )
{

}

void dlgAbout::OnInit( wxInitDialogEvent& event )
{
    wxBitmap splashBitmap = wxBITMAP_PNG_FROM_DATA(splash);
    imgLogo->SetBitmap(splashBitmap);
	wxString msg;
    msg << "Version: " << lime::GetLibraryVersion() << "\n";
    msg << "Build date: " << lime::GetBuildTimestamp();
	txtVersion->SetLabel(msg);
	this->Fit();
	this->Layout();
}

void dlgAbout::OnbtnClose( wxCommandEvent& event )
{
	EndModal(wxID_OK);
	Destroy();
}
