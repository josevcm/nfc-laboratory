#include "pnlUltimateEVB.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"

#include <ciso646>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "pnlBoardControls.h"


using namespace std;

BEGIN_EVENT_TABLE(pnlUltimateEVB, wxPanel)
END_EVENT_TABLE()

pnlUltimateEVB::pnlUltimateEVB(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size, int style, wxString name)
{
    lmsControl = nullptr;

    Create(parent, id, pos, size, style, name);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);

    SetSizer(mainSizer);

    auto powerBox = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Voltage/Current/Power") ), wxVERTICAL );
    auto sizerPowerRd = new wxFlexGridSizer( 0, 4, 2, 2 );
    sizerPowerRd->SetFlexibleDirection( wxBOTH );
    sizerPowerRd->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Name")), 1, wxALL, 5);
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Voltage")), 1, wxALL, 5);
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Current")), 1, wxALL, 5);
    sizerPowerRd->Add(new wxStaticText(this, wxID_ANY, _(" Power")), 1, wxALL, 5);

    const std::vector<std::string> names = {"VCC1P25_LMS_DIG", "VCC1P25_LMS", "VCC1P4_LMS", "VCC1P8_LMS", "VDIO_LMS", "USER"};

    for (unsigned i = 0; i<names.size(); i++)
    {
        const int offset = 36+i;
        pwrParams.push_back(ADCparam(offset,names[i]));
        pwrParams.push_back(ADCparam(offset+names.size(),names[i]));
        pwrParams.push_back(ADCparam(offset+2*names.size(),names[i]));
    }

    for (unsigned i = 0; i<pwrParams.size(); i++)
    {
        if (i%3 == 0)
        {
            pwrParams[i].title = new wxStaticText(this, wxID_ANY, pwrParams[i].name);
            sizerPowerRd->Add(pwrParams[i].title, 1, wxALL, 5);
        }
        pwrParams[i].value = new wxStaticText(this, wxID_ANY, _("- - - - -"));
        sizerPowerRd->Add(pwrParams[i].value, 1, wxALL, 5);
    }
    powerBox->Add( sizerPowerRd, 1, wxEXPAND, 5 );
    auto sizertotalPwr = new wxFlexGridSizer( 1, 2, 10, 10 );
    totalPwr = new wxStaticText(this, wxID_ANY, _("- - - W"));
    sizertotalPwr->Add(new wxStaticText(this, wxID_ANY, _("Total LMS power:")), 1, wxALL, 5);
    sizertotalPwr->Add( totalPwr, 1, wxALL, 5 );
    powerBox->Add( sizertotalPwr, 1, wxEXPAND, 5 );

    auto voltageBox = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Voltages") ), wxVERTICAL );
    auto sizerVoltageRd = new wxFlexGridSizer( 0, 4, 2, 2 );
    sizerVoltageRd->SetFlexibleDirection( wxBOTH );
    sizerVoltageRd->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _("Name")), 1, wxALL, 5);
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _(" Voltage")), 1, wxALL, 5);
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _("Name")), 1, wxALL, 5);
    sizerVoltageRd->Add(new wxStaticText(this, wxID_ANY, _("Voltage")), 1, wxALL, 5);

    const std::vector<std::string> voltages = {
        "VDD_AFE", "VDD12_RXBUF", "VDD12_VCO_SXR", "DVDD_SXR",
        "VDD_DIV_SXR", "VDD_CP_SXR", "VDD18_SXR", "VDD_MXLOBUF_RFE",
        "VDD12_TIA_RFE", "VDD14_LNA_RFE", "VDD14_TIA_RFE", "DIGPRVDD1_1",
        "VDD12_LNA_RFE", "VDD14_RBB", "DVDD_SXT", "VDD_TBB",
        "VDD_TPAD_TRF", "VDD12_DIG", "VDD_CP_SXT", "VDD12_VCO_SXT",
        "DIGPRVDD1_2", "DVDD_CGEN", "VDD_DIV_CGEN", "VDD_SPI_BUF",
        "VDD12_TXBUF", "VDD_DIV_SXT", "VDD_TLOBUF_TRF", "VDD14_VCO_CGEN",
        "VDD_CP_CGEN", "LMS_TSTAO", "LMS_TSTDO1", "LMS_TSTDO0",
        "VDIO_LMS", "VCC_EXT"
    };

    for (unsigned i = 0; i<voltages.size(); i++)
        vltgParams.push_back(ADCparam(2+i,voltages[i]));


    for (unsigned i = 0; i<vltgParams.size(); i++)
    {
        vltgParams[i].title = new wxStaticText(this, wxID_ANY, vltgParams[i].name);
        sizerVoltageRd->Add(vltgParams[i].title, 1, wxALL, 5);
        vltgParams[i].value = new wxStaticText(this, wxID_ANY, _("- - - mV"));
        sizerVoltageRd->Add(vltgParams[i].value, 1, wxALL, 5);
    }
    voltageBox->Add( sizerVoltageRd, 1, wxEXPAND, 5 );

    mainSizer->Add( voltageBox, 1, wxEXPAND, 5 );
    mainSizer->Add( powerBox, 1, wxEXPAND, 5 );
    auto button = new wxButton(this, wxNewId(), _("Save to CSV"));
    mainSizer->Add(button, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
    Connect(button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(pnlUltimateEVB::OnSave), NULL, this);
    mainSizer->Fit(this);
    //mainSizer->SetSizeHints(this);
    Layout();
    Fit();
    Bind(READ_ALL_VALUES, &pnlUltimateEVB::OnReadAll, this, this->GetId());
}

void pnlUltimateEVB::Initialize(lms_device_t* pControl)
{
    lmsControl = pControl;
}

pnlUltimateEVB::~pnlUltimateEVB()
{
}

void pnlUltimateEVB::OnReadAll(wxCommandEvent &event)
{
    double sumPwr = 0;
    float_type value;
    lms_name_t units;
    for (size_t i = 0; i < pwrParams.size(); ++i)
    {
        int status = LMS_ReadCustomBoardParam(lmsControl,pwrParams[i].channel,&value,units);
        if (status != 0)
        {
            wxMessageBox(_("Failed to read board parameters"), _("Warning"));
            return;
        }
        if (pwrParams[i].channel >= 48 && pwrParams[i].channel < 53)
            sumPwr += value;
        pwrParams[i].value->SetLabel(wxString::Format(_("%1.0f %s"), value, units));
    }
    totalPwr->SetLabel(wxString::Format(_("%1.0f mW"), sumPwr));

    for (size_t i = 0; i < vltgParams.size(); ++i)
    {
        int status = LMS_ReadCustomBoardParam(lmsControl,vltgParams[i].channel,&value,units);
        if (status != 0)
        {
            wxMessageBox(_("Failed to read board parameters"), _("Warning"));
            return;
        }
        vltgParams[i].value->SetLabel(wxString::Format(_("%1.0f %s"), value, units));
    }
    this->GetParent()->Layout();
    this->GetParent()->Fit();
}

void pnlUltimateEVB::OnWriteAll(wxCommandEvent &event)
{

}

void pnlUltimateEVB::OnSave(wxCommandEvent &event)
{

    wxFileDialog dlg(this, _("Save samples file"), "", "", "CSV (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    std::string captureFilename = std::string(dlg.GetPath().mb_str());
    std::ofstream ofile(captureFilename);

    if (ofile)
    {
        ofile << "Temperature;" << pnlBoardControls::mParameters[1].value << ";" << pnlBoardControls::mParameters[1].units << std::endl;
        ofile << "Name;Voltage;;Current;;Power;;" << std::endl;
        for (size_t i = 0; i < vltgParams.size(); ++i)
        {
            std::string str = std::string(vltgParams[i].value->GetLabel().mb_str());
            str.replace(str.find(' '), 1, 1, ';');
            ofile << vltgParams[i].name << ";" << str << ";" << std::endl;
        }
        for (size_t i = 0; i < pwrParams.size(); ++i)
        {
            ofile << pwrParams[i].name << ";";
            std::string str = std::string(pwrParams[i++].value->GetLabel().mb_str());
            str.replace(str.find(' '), 1, 1, ';');
            ofile << str << ";";
            str = pwrParams[i++].value->GetLabel().mb_str();
            str.replace(str.find(' '), 1, 1, ';');
            ofile << str << ";";
            str = pwrParams[i].value->GetLabel().mb_str();
            str.replace(str.find(' '), 1, 1, ';');
            ofile << str << ";" << std::endl;
        }
        {
            std::string str = std::string(totalPwr->GetLabel().mb_str());
            str.replace(str.find(' '), 1, 1, ';');
            ofile << "Total LMS power;;;;;" << str << ";" << std::endl;
        }
    }
}
