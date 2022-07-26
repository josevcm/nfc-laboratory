#include "pnlAPI.h"
#include "wx/wxprec.h"
#include "lime/LimeSuite.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP
#include <wx/spinctrl.h>

#include "IConnection.h"
#include "lms7suiteEvents.h"
#include "pnlLimeNetMicro.h"
#include "lms7suiteAppFrame.h"

using namespace std;
using namespace lime;

enum Buttons {
    btnInit,
    btnEnCh,
    btnSetRate,
    btnSetRateDir,
    btnSetFreq,
    btnSetAnt,
    btnSetGain,
    btnSetdB,
    btnSetTest,
    btnGetRate,
    btnGetFreq,
    btnGetAnt,
    btnGetGain,
    btnGetdB,
    btnGetTest,
    btn_COUNT
};

static const wxString test_signals[] = {"None", "NCO CLK/8", "NCO CLK/4", "NCO CLK/8 FS", "NCO CLK/8 FS", "DC"};

pnlAPI::pnlAPI(LMS7SuiteAppFrame* parent) :
    wxFrame(parent, wxID_ANY, "API Calls", wxDefaultPosition, wxDefaultSize,  wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    lmsControl(nullptr),
    lmsAppFrame(parent)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    wxFlexGridSizer* mainSizer;
    runButtons.resize(btn_COUNT);
    mainSizer = new wxFlexGridSizer( 0, 3, 1, 1);
    mainSizer->AddGrowableCol( 2 );
    mainSizer->SetFlexibleDirection( wxBOTH );
    mainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    auto dirChoice = [this](){
        auto choice = new wxChoice(this, wxID_ANY);
        choice->Append("RX");
        choice->Append("TX");
        choice->SetSelection(0);
        return choice;
    };

    auto functionEntry = [this, mainSizer](Buttons btn, wxString label, std::vector<wxWindow*>args){
        runButtons[btn] = new wxButton(this, wxID_ANY, wxT("Run"), wxDefaultPosition, wxDefaultSize, 0);
        mainSizer->Add( runButtons[btn], 0, wxALL, 1 );
        mainSizer->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
        runButtons[btn]->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlAPI::OnRun), NULL, this );
        auto paramSizer = new wxFlexGridSizer( 1, 0, 5, 5);
        for (auto a : args)
            paramSizer->Add(a, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1);
        mainSizer->Add(paramSizer, 0, wxALL, 1 );
        return paramSizer;
    };

    functionEntry(btnInit, _("LMS_Init"), {new wxStaticText(this, wxID_ANY, _(" "))});

    enChDir = dirChoice();
    enChCh = new ChWxChoice(this);
    enChEn = new wxChoice(this, wxID_ANY);
    enChEn->Append("Disabled");
    enChEn->SetSelection(enChEn->Append("Enabled"));
    functionEntry(btnEnCh, _("LMS_EnableChannel"), {enChDir, enChCh, enChEn});

    setRateRate = new wxTextCtrl(this, wxNewId(), _("10"), wxDefaultPosition, wxSize(64, -1));
    setRateOv = new wxChoice(this, wxID_ANY);
    for (int i = 32; i; i /= 2)
        setRateOv->Append(wxString::Format(_("%ix"), i));
    setRateOv->SetSelection(setRateOv->Append("Auto"));
    auto text = new wxStaticText(this, wxID_ANY, _("MSps      oversampling:"));
    functionEntry(btnSetRate, _("LMS_SetSampleRate"), {setRateRate, text, setRateOv});

    setRateDirDir = dirChoice();
    setRateDirRate = new wxTextCtrl(this, wxNewId(), _("10"), wxDefaultPosition, wxSize(64, -1));
    setRateDirOv = new wxChoice(this, wxID_ANY);
    for (int i = 32; i; i /= 2)
        setRateDirOv->Append(wxString::Format(_("%ix"), i));
    setRateDirOv->SetSelection(setRateDirOv->Append("Auto"));
    text = new wxStaticText(this, wxID_ANY, _("MSps      oversampling:"));
    functionEntry(btnSetRateDir, _("LMS_SetSampleRateDir"), {setRateDirRate, text, setRateDirOv, setRateDirDir});

    setFreqDir = dirChoice();
    setFreqCh = new ChWxChoice(this);
    setFreqFreq = new wxTextCtrl(this, wxNewId(), _("500"), wxDefaultPosition, wxSize(80, -1));
    text = new wxStaticText(this, wxID_ANY, _("MHz"));
    functionEntry(btnSetFreq, _("LMS_SetLOFrequency"), {setFreqDir, setFreqCh, setFreqFreq, text});

    setAntDir = dirChoice();
    setAntDir->Connect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(pnlAPI::OnAntDir), NULL, this);
    setAntCh = new ChWxChoice(this);
    setAntAnt = new wxChoice(this, wxID_ANY);
    functionEntry(btnSetAnt, _("LMS_SetAntenna"), {setAntDir, setAntCh, setAntAnt});

    setGainDir = dirChoice();
    setGainCh = new ChWxChoice(this);
    setGainGain = new wxTextCtrl(this, wxNewId(), _("0.7"), wxDefaultPosition, wxSize(48, -1));
    functionEntry(btnSetGain, _("LMS_SetNormalizedGain"), {setGainDir, setGainCh, setGainGain});

    setdBDir = dirChoice();
    setdBCh = new ChWxChoice(this);
    setdBGain = new wxSpinCtrl(this, wxNewId(), _(""), wxDefaultPosition, wxSize(56, -1), wxSP_ARROW_KEYS, 0, 100, 50);
    text = new wxStaticText(this, wxID_ANY, _("dB"));
    functionEntry(btnSetdB,  _("LMS_SetGaindB"), {setdBDir, setdBCh, setdBGain, text});

    setTestDir = dirChoice();
    setTestCh = new ChWxChoice(this);
    setTestSig = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 6 , test_signals);
    setTestSig->SetSelection(0);
    setTestI = new wxSpinCtrl(this, wxNewId(), _(""), wxDefaultPosition, wxSize(72, -1), wxSP_ARROW_KEYS, 0, 65535, 10000);
    setTestQ = new wxSpinCtrl(this, wxNewId(), _(""), wxDefaultPosition, wxSize(72, -1), wxSP_ARROW_KEYS, 0, 65535, 10000);
    text = new wxStaticText(this, wxID_ANY, _("DC_I"));
    auto text2 = new wxStaticText(this, wxID_ANY, _("DC_Q"));
    functionEntry(btnSetTest,  _("LMS_SetTestSignal"), {setTestDir, setTestCh, setTestSig, text, setTestI, text2, setTestQ});

    getRateDir = dirChoice();
    getRateCh = new ChWxChoice(this);
    getRateResult = new wxStaticText(this, wxID_ANY, _(""));
    functionEntry(btnGetRate,  _("LMS_GetSampleRate"), {getRateDir, getRateCh, getRateResult});

    getFreqDir = dirChoice();
    getFreqCh = new ChWxChoice(this);
    getFreqResult = new wxStaticText(this, wxID_ANY, _(""));
    functionEntry(btnGetFreq,  _("LMS_GetLOFrequency"), {getFreqDir, getFreqCh, getFreqResult});

    getAntDir = dirChoice();
    getAntCh = new ChWxChoice(this);
    getAntResult = new wxStaticText(this, wxID_ANY, _(""));
    functionEntry(btnGetAnt,  _("LMS_GetAntenna"), {getAntDir, getAntCh, getAntResult});

    getGainDir = dirChoice();
    getGainCh = new ChWxChoice(this);
    getGainResult = new wxStaticText(this, wxID_ANY, _(""));
    functionEntry(btnGetGain,  _("LMS_GetNormalizedGain"), {getGainDir, getGainCh, getGainResult});

    getdBDir = dirChoice();
    getdBCh = new ChWxChoice(this);
    getdBResult = new wxStaticText(this, wxID_ANY, _(""));
    functionEntry(btnGetdB,  _("LMS_GetGaindB"), {getdBDir, getdBCh, getdBResult});

    getTestDir = dirChoice();
    getTestCh = new ChWxChoice(this);
    getTestResult = new wxStaticText(this, wxID_ANY, _(""));
    functionEntry(btnGetTest,  _("LMS_GetTestSignal"), {getTestDir, getTestCh, getTestResult});

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );
}

pnlAPI::~pnlAPI()
{
    // Disconnect Events
    setAntDir->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlAPI::OnAntDir ), NULL, this );
    for (auto btn : runButtons)
        btn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( pnlAPI::OnRun ), NULL, this );
}

void pnlAPI::OnRun( wxCommandEvent& event )
{
    auto obj = event.GetEventObject();
    if (obj == runButtons[btnInit])
    {
        LMS_Init(lmsControl);
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnEnCh])
    {
        LMS_EnableChannel(lmsControl, enChDir->GetSelection(), enChCh->GetSelection(), enChEn->GetSelection());
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnSetdB])
    {
        LMS_SetGaindB(lmsControl, setdBDir->GetSelection(), setdBCh->GetSelection(), setdBGain->GetValue());
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnSetTest])
    {
        LMS_SetTestSignal(lmsControl, setTestDir->GetSelection(), setTestCh->GetSelection(), lms_testsig_t(setTestSig->GetSelection()), setTestI->GetValue(), setTestQ->GetValue());
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnSetFreq])
    {
        double freq = 0;
        setFreqFreq->GetValue().ToDouble(&freq);
        LMS_SetLOFrequency(lmsControl, setFreqDir->GetSelection(), setFreqCh->GetSelection(), freq*1e6);
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnSetRate])
    {
        double freq = 0;
        setRateRate->GetValue().ToDouble(&freq);
        LMS_SetSampleRate(lmsControl, freq*1e6, 32>>setRateOv->GetSelection());
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnSetRateDir])
    {
        double freq = 0;
        setRateDirRate->GetValue().ToDouble(&freq);
        LMS_SetSampleRateDir(lmsControl, setRateDirDir->GetSelection(), freq*1e6, 32>>setRateDirOv->GetSelection());
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnSetAnt])
    {
        double freq = 0;
        setRateRate->GetValue().ToDouble(&freq);
        LMS_SetAntenna(lmsControl, setAntDir->GetSelection(), setAntCh->GetSelection(), setAntAnt->GetSelection());
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnSetGain])
    {
        double gain = 0;
        setGainGain->GetValue().ToDouble(&gain);
        LMS_SetNormalizedGain(lmsControl, setGainDir->GetSelection(), setGainCh->GetSelection(), gain);
        lmsAppFrame->UpdateVisiblePanel();
    }
    else if (obj == runButtons[btnGetRate])
    {
        double host, rf;
        LMS_GetSampleRate(lmsControl, getRateDir->GetSelection(), getRateCh->GetSelection(), &host, &rf);
        getRateResult->SetLabel(wxString::Format(_("Sample Rate: %.6f MSps  (RF: %.3f MSps)"), host/1e6, rf/1e6));
    }
    else if (obj == runButtons[btnGetFreq])
    {
        double freq;
        LMS_GetLOFrequency(lmsControl, getFreqDir->GetSelection(), getFreqCh->GetSelection(), &freq);
        getFreqResult->SetLabel(wxString::Format(_("RF Frequency: %.6f MHz"), freq/1e6));
    }
    else if (obj == runButtons[btnGetAnt])
    {
        int index = LMS_GetAntenna(lmsControl, getAntDir->GetSelection(), getAntCh->GetSelection());
        if (index < 0)
            return;
        lms_name_t list[16];
        int cnt = LMS_GetAntennaList(lmsControl, getAntDir->GetSelection(), getAntCh->GetSelection(), list);
        if (index >= cnt)
            return;
        getAntResult->SetLabel(wxString::Format(_("%s"), list[index]));
    }
    else if (obj == runButtons[btnGetGain])
    {
        double gain;
        LMS_GetNormalizedGain(lmsControl, getGainDir->GetSelection(), getGainCh->GetSelection(), &gain);
        getGainResult->SetLabel(wxString::Format(_("%.2f"), gain));
    }
    else if (obj == runButtons[btnGetdB])
    {
        unsigned gain;
        LMS_GetGaindB(lmsControl, getdBDir->GetSelection(), getdBCh->GetSelection(), &gain);
        getdBResult->SetLabel(wxString::Format(_("%d dB"), gain));
    }
    else if (obj == runButtons[btnGetTest])
    {
        lms_testsig_t sig = LMS_TESTSIG_NONE;
        LMS_GetTestSignal(lmsControl, getTestDir->GetSelection(), getTestCh->GetSelection(), &sig);
        getTestResult->SetLabel(wxString::Format(_("%s"), test_signals[sig].c_str()));
    }

}

void pnlAPI::OnAntDir( wxCommandEvent& event )
{
    lms_name_t list[16];
    int cnt = LMS_GetAntennaList(lmsControl, event.GetInt(), 0, list);
    setAntAnt->Clear();
    for (int i = 0; i < cnt; i++)
        setAntAnt->Append(list[i]);
    setAntAnt->SetSelection(0);
}

void pnlAPI::Initialize(lms_device_t* controlPort)
{
    lmsControl = controlPort;

    if (lmsControl)
    {
        for (auto ctrl : chControls)
            ctrl->Clear();
        for (int i = 0; i< LMS_GetNumChannels(lmsControl, false); i++)
        {
            auto txt = wxString::Format(_("ch %i"), i);
            for (auto ctrl : chControls)
                ctrl->Append(txt);
        }
        for (auto ctrl : chControls)
            ctrl->SetSelection(0);
        {
            wxCommandEvent evt;
            evt.SetInt(0);
            OnAntDir(evt);
        }
    }
}

