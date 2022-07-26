#include "pnlCoreSDR.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"

#include <ciso646>


using namespace std;

BEGIN_EVENT_TABLE(pnlCoreSDR, wxPanel)
END_EVENT_TABLE()

pnlCoreSDR::pnlCoreSDR(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RX1 RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString rxChoices;
    rxChoices.push_back(_("Shut Down"));
    rxChoices.push_back(_("LNAH"));
    rxChoices.push_back(_("LNAW"));
    rxChoices.push_back(_("LNAL"));
    cmbRx1Path = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, rxChoices, 1);
    cmbRx1Path->SetSelection(0);
    Connect(cmbRx1Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), NULL, this);
    mainSizer->Add(cmbRx1Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("RX2 RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbRx2Path = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, rxChoices, 1);
    cmbRx2Path->SetSelection(0);
    Connect(cmbRx2Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), NULL, this);
    mainSizer->Add(cmbRx2Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("TX1 RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString txChoices;
    txChoices.push_back(_("Band 2"));
    txChoices.push_back(_("Band 1"));
    cmbTx1Path = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, txChoices, 1);
    cmbTx1Path->SetSelection(0);
    Connect(cmbTx1Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), NULL, this);
    mainSizer->Add(cmbTx1Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("TX2 RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    cmbTx2Path = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, txChoices, 1);
    cmbTx2Path->SetSelection(0);
    Connect(cmbTx2Path->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), NULL, this);
    mainSizer->Add(cmbTx2Path, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);


    wxStaticBoxSizer* mainBoxSizer;
    mainBoxSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("RF switch controls") ), wxVERTICAL );
    mainBoxSizer->Add(mainSizer, 0, 0, 5);
    mainBoxSizer->Fit(this);
    mainBoxSizer->SetSizeHints(this);
    SetSizer(mainBoxSizer);
    Layout();

    Bind(READ_ALL_VALUES, &pnlCoreSDR::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlCoreSDR::OnLoopbackChange, this, this->GetId());
}

void pnlCoreSDR::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
}

pnlCoreSDR::~pnlCoreSDR()
{
    cmbRx1Path->Disconnect(wxEVT_CHOICE, cmbRx1Path->GetId(), wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), 0, this);
    cmbTx1Path->Disconnect(wxEVT_CHOICE, cmbTx1Path->GetId(), wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), 0, this);
    cmbRx2Path->Disconnect(wxEVT_CHOICE, cmbRx2Path->GetId(), wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), 0, this);
    cmbTx2Path->Disconnect(wxEVT_CHOICE, cmbTx2Path->GetId(), wxCommandEventHandler(pnlCoreSDR::OnLoopbackChange), 0, this);
}

void pnlCoreSDR::OnLoopbackChange(wxCommandEvent& event)
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;

    value |= cmbTx1Path->GetSelection() << 8;
    value |= cmbTx2Path->GetSelection() << 12;

    value |= cmbRx1Path->GetSelection() ? 0x01 << (cmbRx1Path->GetSelection()-1) : 0;
    value |= cmbRx2Path->GetSelection() ? 0x10 << (cmbRx2Path->GetSelection()-1) : 0;

    if(LMS_WriteFPGAReg(lmsControl,addr, value))
        wxMessageBox(_("Failed to write FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
}

void pnlCoreSDR::UpdatePanel()
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    if(LMS_ReadFPGAReg(lmsControl,addr, &value))
    {
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }

    cmbTx1Path->SetSelection((value >> 8) & 0x1);
    cmbTx2Path->SetSelection((value >> 12) & 0x1);

    {
        int path = value&0x7;
        path = path == 4 ? 3 : path == 2 ? 2 : path == 1 ? 1 : 0;
        cmbRx1Path->SetSelection(path);
    }

    {
        int path = (value>>4)&0x7;
        path = path == 4 ? 3 : path == 2 ? 2 : path == 1 ? 1 : 0;
        cmbRx2Path->SetSelection(path);
    }
}

void pnlCoreSDR::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnlCoreSDR::OnWriteAll(wxCommandEvent &event)
{
    OnLoopbackChange(event);
}
