#pragma once

#include "lime/LimeSuite.h"
#include <wx/panel.h>

class wxChoice;
class wxStaticText;
class wxFlexGridSizer;
class pnlGPIO;

class pnlLimeNetMicro : public wxPanel
{
public:
    pnlLimeNetMicro(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t *pControl);
    virtual ~pnlLimeNetMicro();
    virtual void UpdatePanel();
    void OnLoopbackChange(wxCommandEvent &event);
    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxFlexGridSizer* mainSizer;
    pnlGPIO* pnl_gpio;
    wxChoice* cmbRxPath;
    wxChoice* cmbTxPath;
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};
