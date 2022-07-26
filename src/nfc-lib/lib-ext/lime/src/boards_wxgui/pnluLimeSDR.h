#ifndef PNL_ULIMESDR_H
#define PNL_ULIMESDR_H

#include "lime/LimeSuite.h"
#include <wx/panel.h>

class wxChoice;
class wxStaticText;
class wxCheckBox;
class wxFlexGridSizer;
class pnlGPIO;

class pnluLimeSDR : public wxPanel
{
public:
    pnluLimeSDR(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t *pControl);
    virtual ~pnluLimeSDR();
    virtual void UpdatePanel();
    void OnLoopbackChange(wxCommandEvent &event);

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxFlexGridSizer* mainSizer;
    pnlGPIO* pnl_gpio;
    wxCheckBox* chkTxLBSH;
    wxCheckBox* chkTxLBAT;
    wxChoice* cmbRxPath;
    wxChoice* cmbTxPath;
    wxStaticText* txtLB;
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};

#endif
