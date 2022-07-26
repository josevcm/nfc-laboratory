#include "pnlLimeNetMicro.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <vector>
#include "lms7suiteEvents.h"
#include "pnlGPIO.h"

using namespace std;

BEGIN_EVENT_TABLE(pnlLimeNetMicro, wxPanel)
END_EVENT_TABLE()

static const std::vector<uint16_t> rxSwVal  = {0x0502, 0x0602, 0x0101, 0x0201, 0x0600, 0x0500};
static const std::vector<uint16_t> rxSwMask = {0x0702, 0x0702, 0x0701, 0x0701, 0x0702, 0x0702};
static const std::vector<uint16_t> txSwVal  = {0x5000, 0x6000, 0x6001, 0x5001, 0x1000, 0x2000};
static const std::vector<uint16_t> txSwMask = {0x7001, 0x7001, 0x7001, 0x7001, 0x7002, 0x7002};

pnlLimeNetMicro::pnlLimeNetMicro(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    SetSizer(mainSizer);
    wxFlexGridSizer* lbSizer = new wxFlexGridSizer(0, 2, 5, 5);
    lbSizer->Add(new wxStaticText(this, wxID_ANY, _("RX RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString rxChoices;
    rxChoices.push_back(_("LNAL"));
    rxChoices.push_back(_("LNAH"));
    cmbRxPath = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, rxChoices, 1);
    cmbRxPath->SetSelection(0);
    Connect(cmbRxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), NULL, this);
    lbSizer->Add(cmbRxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    lbSizer->Add(new wxStaticText(this, wxID_ANY, _("TX RF port path:")), 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString txChoices;
    txChoices.push_back(_("Band 1"));
    txChoices.push_back(_("Band 2"));
    cmbTxPath = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, txChoices, 1);
    cmbTxPath->SetSelection(0);
    Connect(cmbTxPath->GetId(), wxEVT_CHOICE, wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), NULL, this);
    lbSizer->Add(cmbTxPath, 1, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

    mainSizer->Add(lbSizer, 1, wxEXPAND | wxALL, 5);

    pnl_gpio = new pnlGPIO(this, wxNewId());
    mainSizer->Add(pnl_gpio, 1, wxEXPAND | wxALL, 5);
    Bind(READ_ALL_VALUES, &pnlLimeNetMicro::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &pnlLimeNetMicro::OnLoopbackChange, this, this->GetId());
}

void pnlLimeNetMicro::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
    cmbRxPath->Clear();
    cmbRxPath->Append(wxString(_("LNAL RX")));
    cmbRxPath->Append(wxString(_("LNAH RX")));
    cmbTxPath->Clear();
    cmbTxPath->Append(wxString(_("Band 1 TX")));
    cmbTxPath->Append(wxString(_("Band 2 TX")));
    if (pControl)
    {
        uint16_t reg3 = 0;
        if (LMS_ReadFPGAReg(lmsControl, 0x3, &reg3) == LMS_SUCCESS)
        {
            int bom_ver = reg3>>4;
            int hw_ver = reg3 & 0xF;
            if (hw_ver == 3 && bom_ver != 1)
            {
                cmbRxPath->Append(wxString(_("LNAL Duplexer A")));
                cmbRxPath->Append(wxString(_("LNAH Duplexer A")));
                cmbRxPath->Append(wxString(_("LNAL Duplexer B")));
                cmbRxPath->Append(wxString(_("LNAH Duplexer B")));
                cmbTxPath->Append(wxString(_("Band 1 Duplexer A")));
                cmbTxPath->Append(wxString(_("Band 2 Duplexer A")));
                cmbTxPath->Append(wxString(_("Band 1 Duplexer B")));
                cmbTxPath->Append(wxString(_("Band 2 Duplexer B")));
            }
        }
    }
    pnl_gpio->Initialize(lmsControl);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);
    Layout();
}

pnlLimeNetMicro::~pnlLimeNetMicro()
{
    cmbRxPath->Disconnect(wxEVT_CHOICE, cmbRxPath->GetId(), wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), 0, this);
    cmbTxPath->Disconnect(wxEVT_CHOICE, cmbTxPath->GetId(), wxCommandEventHandler(pnlLimeNetMicro::OnLoopbackChange), 0, this);
}

void pnlLimeNetMicro::OnLoopbackChange(wxCommandEvent& event)
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;

    uint16_t reg3 = 0;
    LMS_ReadFPGAReg(lmsControl, 0x3, &reg3);
    int bom_ver = reg3>>4;
    int hw_ver = reg3 & 0xF;

    if (hw_ver == 3 && bom_ver != 1)
    {
        if (event.GetEventObject() == cmbRxPath)
        {
            value = txSwVal[cmbTxPath->GetSelection()];
            value &= ~rxSwMask[cmbRxPath->GetSelection()];
            value |= rxSwVal[cmbRxPath->GetSelection()];
            if ((value & txSwMask[cmbTxPath->GetSelection()]) != txSwVal[cmbTxPath->GetSelection()])
            {
                for (unsigned index = 0; index < txSwVal.size(); index++)
                    if (txSwVal[index] == (value&txSwMask[index]))
                    {
                        cmbTxPath->SetSelection(index);
                        break;
                    }
            }
        }
        else
        {
            value = rxSwVal[cmbRxPath->GetSelection()];
            value &= ~txSwMask[cmbTxPath->GetSelection()];
            value |= txSwVal[cmbTxPath->GetSelection()];
            if ((value & rxSwMask[cmbRxPath->GetSelection()]) != rxSwVal[cmbRxPath->GetSelection()])
            {
                for (unsigned index = 0; index < rxSwVal.size(); index++)
                    if (rxSwVal[index] == (value&rxSwMask[index]))
                    {
                        cmbRxPath->SetSelection(index);
                        break;
                    }
            }
        }

    }
    else
    {
        value |= cmbRxPath->GetSelection() == 1 ? 1<<9 : 1<<8;
        value |= cmbTxPath->GetSelection() == 1 ? 1<<13 : 1<<12;
    }
    if(LMS_WriteFPGAReg(lmsControl,addr, value))
        wxMessageBox(_("Failed to write FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
}

void pnlLimeNetMicro::UpdatePanel()
{
    uint16_t addr = 0x0017;
    uint16_t value = 0;
    if(LMS_ReadFPGAReg(lmsControl,addr, &value))
    {
        wxMessageBox(_("Failed to read FPGA registers"), _("Error"), wxICON_ERROR | wxOK);
        return;
    }
    uint16_t reg3 = 0;
    LMS_ReadFPGAReg(lmsControl, 0x3, &reg3);
    int bom_ver = reg3>>4;
    int hw_ver = reg3 & 0xF;

    if (hw_ver == 3 && bom_ver != 1)
    {
        for (unsigned index = 0; index < txSwVal.size(); index++)
        if (txSwVal[index] == (value&txSwMask[index]))
            {
                cmbTxPath->SetSelection(index);
                break;
            }
        for (unsigned index = 0; index < rxSwVal.size(); index++)
        if (rxSwVal[index] == (value&rxSwMask[index]))
            {
                cmbRxPath->SetSelection(index);
                break;
            }
    }
    else
    {
        cmbRxPath->SetSelection((value >> 9) & 0x1);
        cmbTxPath->SetSelection((value >> 13) & 0x1);
    }
    pnl_gpio->UpdatePanel();
}

void pnlLimeNetMicro::OnReadAll(wxCommandEvent &event)
{
    UpdatePanel();
}

void pnlLimeNetMicro::OnWriteAll(wxCommandEvent &event)
{
    OnLoopbackChange(event);
    pnl_gpio->OnUsrGPIODirChange(event);
    pnl_gpio->OnUsrGPIOChange(event);
}
