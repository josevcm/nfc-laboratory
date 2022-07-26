#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/frame.h>
#include <wx/spinctrl.h>
#include "lime/LimeSuite.h"
#include <vector>
#include <string>
#include <stdint.h>
#include <LMSBoards.h>


class wxTextCtrl;
class LMS7SuiteAppFrame;

class pnlAPI : public wxFrame
{
public:

    pnlAPI(LMS7SuiteAppFrame* parent);
    ~pnlAPI();
    void Initialize(lms_device_t* controlPort);
    lms_device_t* lmsControl;

private:           
    void OnRun( wxCommandEvent& event );
    void OnAntDir( wxCommandEvent& event );

    std::vector<wxChoice*> chControls;
    std::vector<wxButton*> runButtons;    
    const LMS7SuiteAppFrame* lmsAppFrame;       

    wxChoice* enChDir;
    wxChoice* enChCh;
    wxChoice* enChEn;

    wxTextCtrl* setRateRate;
    wxChoice* setRateOv;

    wxChoice* setRateDirDir;
    wxTextCtrl* setRateDirRate;
    wxChoice* setRateDirOv;

    wxTextCtrl* setFreqFreq;
    wxChoice* setFreqDir;
    wxChoice* setFreqCh;

    wxChoice* setAntDir;
    wxChoice* setAntCh;
    wxChoice* setAntAnt;

    wxChoice* setGainDir;
    wxChoice* setGainCh;
    wxTextCtrl* setGainGain;

    wxChoice* setdBDir;
    wxChoice* setdBCh;
    wxSpinCtrl* setdBGain;

    wxChoice* setTestDir;
    wxChoice* setTestCh;
    wxChoice* setTestSig;
    wxSpinCtrl* setTestI;
    wxSpinCtrl* setTestQ;
    
    wxChoice* getRateDir;
    wxChoice* getRateCh;
    wxStaticText* getRateResult;
    
    wxChoice* getFreqDir;
    wxChoice* getFreqCh;
    wxStaticText* getFreqResult;
    
    wxChoice* getAntDir;
    wxChoice* getAntCh;
    wxStaticText* getAntResult;
    
    wxChoice* getGainDir;
    wxChoice* getGainCh;
    wxStaticText* getGainResult;
    
    wxChoice* getdBDir;
    wxChoice* getdBCh;
    wxStaticText* getdBResult;
    
    wxChoice* getTestDir;
    wxChoice* getTestCh;
    wxStaticText* getTestResult;
    
    class ChWxChoice : public wxChoice
    {
        friend pnlAPI;
        ChWxChoice(pnlAPI *parent) : wxChoice(parent, wxID_ANY)
        {
            parent->chControls.push_back(this);
        };
    };
};


