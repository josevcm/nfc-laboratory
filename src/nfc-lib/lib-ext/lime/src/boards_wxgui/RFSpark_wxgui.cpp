/**
@file 	RFSpark_wxgui.cpp
@author Lime Microsystems
@brief 	wxWidgets panel for interacting with RFSpark board
*/

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "lms7suiteEvents.h"

#include "RFSpark_wxgui.h"
#include <vector>

const long RFSpark_wxgui::ID_BTNREADADC = wxNewId();
const long RFSpark_wxgui::ID_BTNREADALLADC = wxNewId();
const long RFSpark_wxgui::ID_BTNWRITEGPIO = wxNewId();
const long RFSpark_wxgui::ID_BTNREADGPIO = wxNewId();
const long RFSpark_wxgui::ID_CMBSELECTADC = wxNewId();

BEGIN_EVENT_TABLE(RFSpark_wxgui, wxPanel)
END_EVENT_TABLE()

RFSpark_wxgui::RFSpark_wxgui(wxWindow* parent,wxWindowID id, const wxString& title, const wxPoint& pos,const wxSize& size, long style)
{
    lmsControl = nullptr;
    Create(parent, id, wxDefaultPosition, wxDefaultSize, style, title);
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif

    //ADC values
    wxFlexGridSizer* sizerADCs = new wxFlexGridSizer(0, 6, 5, 10);

    ADCdataGUI adcElement;
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Channel")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Value")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Units")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Channel")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Value")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    sizerADCs->Add(new wxStaticText(this, wxNewId(), _("Units")), 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    wxArrayString adcList;
    wxString adcNames[] = { "VDD12_TIA_RFE", "VDD_TBB", "VDD14_RBB", "DVDD_SXT", "VDD_DIC_SXT", "VDD12_TXBUF", "VDD12_VCO_SXT", "VDD14_LNA_RFE",
        "DVDD_SXR", "VDD_DIV_SXR", "VDD_CP_SXR", "VDD18_SXR", "VD_MXLOBUF_RFE", "VDD14_TIA_RFE", "VDD12_LNA_RFE", "VDD_TLOBUF_TRF",
        "VDD12_RXBIF", "VDD_AFE", "DIGPRVDD1", "VDD_SPI_BUF", "VDD18_TPAD_TRF", "DVDD_CGEN", "VDD_DIV_CGEN", "VDD_CP_CGEN", "VDD12_DIG", "VDD14_VCO_CGEN",
        "TSTAO", "TSTDO 0", "TSTDO 1", "3V3", "VDIO", "VDIO_FMC" };
    for (int i = 0; i < mADCcount; ++i)
    {
        adcElement.title = new wxStaticText(this, wxNewId(), wxString::Format("%s", adcNames[i]));
        adcList.push_back(adcElement.title->GetLabel());
        adcElement.value = new wxStaticText(this, wxNewId(), _("????"));
        adcElement.units = new wxStaticText(this, wxNewId(), _("????"));
        sizerADCs->Add(adcElement.title, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        sizerADCs->Add(adcElement.value, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        sizerADCs->Add(adcElement.units, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mADCgui.push_back(adcElement);
        ADCdata data;
        data.channel = i;
        //data.powerOf10coefficient = 0;
        //data.units = 0;
        data.value = 0;
        mADCdata.push_back(data);
    }

    wxStaticBoxSizer* boxADC = new wxStaticBoxSizer(wxVERTICAL, this, _T("ADC values"));
    boxADC->Add(sizerADCs, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxFlexGridSizer* sizerGPIOs = new wxFlexGridSizer(0, 2, 2, 4);

    wxString gpios7_0[] = { "ADCinQ2_N 15", "ADCinQ2_P", "ADCinI2_N", "ADCinI2_P", "ADCinQ1_N", "ADCinQ1_P", "ADCinI1_N", "ADCinI1_P" };
    for (int j = 0; j < 8; ++j)
    {
        long id = wxNewId();
        wxCheckBox* chkgpio = new wxCheckBox(this, id, wxString::Format("%s", gpios7_0[j]));
        this->Connect(id, wxEVT_CHECKBOX, (wxObjectEventFunction)&RFSpark_wxgui::OnWriteGPIO);
        sizerGPIOs->Add(chkgpio, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mGPIOboxes.push_back(chkgpio);
    }

    wxString gpios15_8[] = { "GPIO 15", "DIG_RST", "CORE_LDO_EN", "IQSELEN2RX_DIR", "IQSELEN1TX_DIR", "DIO_BUF_OE", "DIQ2RX_DIR", "DIQ1TX_DIR" };
    for (int j = 0; j < 8; ++j)
    {
        long id = wxNewId();
        wxCheckBox* chkgpio = new wxCheckBox(this, id, wxString::Format("%s", gpios15_8[j]));
        this->Connect(id, wxEVT_CHECKBOX, (wxObjectEventFunction)&RFSpark_wxgui::OnWriteGPIO);
        sizerGPIOs->Add(chkgpio, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mGPIOboxes.push_back(chkgpio);
    }
    wxString gpios23_16[] = { "GPIO 23", "GPIO 22", "GPIO 21", "GPIO 20", "GPIO 19", "GPIO 18", "TXEN_LMS", "RXEN_LMS" };
    for (int j = 0; j < 8; ++j)
    {
        long id = wxNewId();
        wxCheckBox* chkgpio = new wxCheckBox(this, id, wxString::Format("%s", gpios23_16[j]));
        this->Connect(id, wxEVT_CHECKBOX, (wxObjectEventFunction)&RFSpark_wxgui::OnWriteGPIO);
        sizerGPIOs->Add(chkgpio, wxALIGN_LEFT | wxALIGN_TOP, 0);
        mGPIOboxes.push_back(chkgpio);
    }

    wxFlexGridSizer* sizerBoxGPIO = new wxFlexGridSizer(0, 1, 5, 5);
    sizerBoxGPIO->Add(sizerGPIOs, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxStaticBoxSizer* boxGPIO = new wxStaticBoxSizer(wxVERTICAL, this, _T("GPIO states"));
    boxGPIO->Add(sizerBoxGPIO, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    wxFlexGridSizer* mainGrid = new wxFlexGridSizer(0, 2, 5, 5);
    mainGrid->Add(boxADC, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    mainGrid->Add(boxGPIO, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

    SetSizer(mainGrid);
    mainGrid->Fit(this);
    mainGrid->SetSizeHints(this);
    Layout();

    Bind(READ_ALL_VALUES, &RFSpark_wxgui::OnReadAll, this, this->GetId());
    Bind(WRITE_ALL_VALUES, &RFSpark_wxgui::OnWriteAll, this, this->GetId());
}

void RFSpark_wxgui::Initialize(lms_device_t* pSerPort)
{
    lmsControl = pSerPort;
    wxCommandEvent evt;
}

RFSpark_wxgui::~RFSpark_wxgui()
{

}

void RFSpark_wxgui::UpdateADClabels()
{
	for (unsigned i = 0; i < mADCdata.size(); ++i)
	{
        mADCgui[i].value->SetLabelText(wxString::Format("%1.3f", mADCdata[i].value));
        mADCgui[i].units->SetLabelText(wxString::Format("%s", mADCdata[i].units));
    }
}

void RFSpark_wxgui::OnRefreshAllADC(wxCommandEvent& event)
{
    for (size_t i = 0; i < mADCdata.size(); ++i)
    {
        float_type val;
        lms_name_t units;
        if (LMS_ReadCustomBoardParam(lmsControl,i,&val,units)!=0)
        {
            wxMessageBox(_("Error reading ADC values"), _("Warning"));
            return;
        }
        mADCdata[i].channel = i;
        mADCdata[i].units = units;
        mADCdata[i].value = val;
    }
    UpdateADClabels();
}

void RFSpark_wxgui::OnWriteGPIO(wxCommandEvent& event)
{
    std::vector<uint8_t> values(mGPIOboxes.size()/8, 0);
    int gpioIndex = 0;
    for (size_t i = 0; i < mGPIOboxes.size()/8; ++i)
    {
        unsigned char value = 0;
        for (int j = 7; j >= 0; --j)
            value |= mGPIOboxes[gpioIndex++]->IsChecked() << j;
        values[i] = value;
    }
    if (LMS_GPIOWrite(lmsControl, values.data(), mGPIOboxes.size()/8) != 0)
        wxMessageBox(_("GPIO write failed"), _("Warning"));
}

void RFSpark_wxgui::OnReadGPIO(wxCommandEvent& event)
{
    std::vector<uint8_t> values(mGPIOboxes.size()/8);
    if (LMS_GPIORead(lmsControl, values.data(), mGPIOboxes.size()/8) != 0)
    {
        wxMessageBox(_("GPIO read failed"), _("Warning"));
        return;
    }
    size_t gpioIndex = 0;
    int gpioByte = 0;
    for (size_t i = 0; i < mGPIOboxes.size() / 8; ++i)
    {
        for (int j = 7; j >= 0 && gpioIndex < mGPIOboxes.size(); --j)
            mGPIOboxes[gpioIndex++]->SetValue( values[gpioByte] & (0x1 << j) );
        ++gpioByte;
    }
}

void RFSpark_wxgui::OnReadAll(wxCommandEvent &event)
{
    OnReadGPIO(event);
    OnRefreshAllADC(event);
}

void RFSpark_wxgui::OnWriteAll(wxCommandEvent &event)
{
    OnWriteGPIO(event);
}
