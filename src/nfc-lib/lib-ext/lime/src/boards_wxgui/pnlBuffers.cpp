#include "pnlBuffers.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"

#include <ciso646>


using namespace std;

BEGIN_EVENT_TABLE(pnlBuffers, wxPanel)
END_EVENT_TABLE()

pnlBuffers::pnlBuffers(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* fgSizer239;
    fgSizer239 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer239->SetFlexibleDirection( wxBOTH );
    fgSizer239->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticBoxSizer* sbSizer128;
    sbSizer128 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Buffers") ), wxVERTICAL );

    chkDIO_DIR_CTRL1 = new wxCheckBox( sbSizer128->GetStaticBox(), wxID_ANY, wxT("DIO_DIR_CTRL1"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer128->Add( chkDIO_DIR_CTRL1, 0, 0, 5 );

    chkDIO_DIR_CTRL2 = new wxCheckBox( sbSizer128->GetStaticBox(), wxID_ANY, wxT("DIO_DIR_CTRL2"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer128->Add( chkDIO_DIR_CTRL2, 0, 0, 5 );

    chkDIO_BUFF_OE = new wxCheckBox( sbSizer128->GetStaticBox(), wxID_ANY, wxT("DIO_BUFF_OE"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer128->Add( chkDIO_BUFF_OE, 0, wxRIGHT, 5 );

    chkIQ_SEL1_DIR = new wxCheckBox( sbSizer128->GetStaticBox(), wxID_ANY, wxT("IQ_SEL1_DIR"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer128->Add( chkIQ_SEL1_DIR, 0, 0, 5 );

    chkIQ_SEL2_DIR = new wxCheckBox( sbSizer128->GetStaticBox(), wxID_ANY, wxT("IQ_SEL2_DIR"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer128->Add( chkIQ_SEL2_DIR, 0, 0, 5 );

    chkG_PWR_DWN = new wxCheckBox( sbSizer128->GetStaticBox(), wxID_ANY, wxT("G_PWR_DWN"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizer128->Add( chkG_PWR_DWN, 0, 0, 5 );


    fgSizer239->Add( sbSizer128, 1, wxEXPAND, 5 );

    this->SetSizer( fgSizer239 );
    this->Layout();
    fgSizer239->Fit( this );

    // Connect Events
    chkDIO_DIR_CTRL1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkDIO_DIR_CTRL2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkDIO_BUFF_OE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkIQ_SEL1_DIR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkIQ_SEL2_DIR->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkG_PWR_DWN->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );

    Bind(READ_ALL_VALUES, &pnlBuffers::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlBuffers::OnWriteAll, this, this->GetId());
}

void pnlBuffers::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
}

pnlBuffers::~pnlBuffers()
{
    // Disconnect Events
    chkDIO_DIR_CTRL1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkDIO_DIR_CTRL2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkDIO_BUFF_OE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkIQ_SEL1_DIR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkIQ_SEL2_DIR->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
    chkG_PWR_DWN->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( pnlBuffers::OnGPIOchanged ), NULL, this );
}

static unsigned char setbit(const unsigned char src, const int pos, const bool value)
{
    int val = src;
    val = val & ~(0x1 << pos);
    val |= value << pos;
    return val;
}

void pnlBuffers::OnGPIOchanged( wxCommandEvent& event )
{
    uint8_t value = 0;
    value = setbit(value, 0, chkDIO_DIR_CTRL1->GetValue());
    value = setbit(value, 1, chkDIO_DIR_CTRL2->GetValue());
    value = setbit(value, 2, chkDIO_BUFF_OE->GetValue());
    value = setbit(value, 3, chkIQ_SEL1_DIR->GetValue());
    value = setbit(value, 4, chkIQ_SEL2_DIR->GetValue());
    value = setbit(value, 5, chkG_PWR_DWN->GetValue());

    LMS_GPIOWrite(lmsControl,&value,1);
    {
        wxMessageBox(wxString(_("GPIO Write Failed")));
    }
}

static bool getbit(const unsigned char src, const int pos)
{
    return (src >> pos) & 0x01;
}

void pnlBuffers::UpdatePanel()
{
    uint8_t value;
    if (LMS_GPIORead(lmsControl,&value,1)==0)
    {
        chkDIO_BUFF_OE->SetValue(getbit(value, 2));
        chkDIO_DIR_CTRL1->SetValue(getbit(value, 0));
        chkDIO_DIR_CTRL2->SetValue(getbit(value, 1));
        chkIQ_SEL1_DIR->SetValue(getbit(value, 3));
        chkIQ_SEL2_DIR->SetValue(getbit(value, 4));
        chkG_PWR_DWN->SetValue(getbit(value, 5));
    }
    else wxMessageBox(wxString::Format(_("GPIO Read: Failed")));
}

void pnlBuffers::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnlBuffers::OnWriteAll(wxCommandEvent &event)
{
    OnGPIOchanged(event);
}
