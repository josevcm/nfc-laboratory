#ifndef PNL_GPIO_H
#define PNL_GPIO_H

#include "lime/LimeSuite.h"
#include <wx/panel.h>
class wxStaticText;
class wxCheckBox;

namespace lime
{
class IConnection;
}

class pnlGPIO : public wxPanel
{
public:
    pnlGPIO(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t* pControl);
    virtual ~pnlGPIO();
    virtual void UpdatePanel();
    void OnUsrGPIODirChange(wxCommandEvent &event);
    void OnUsrGPIOChange(wxCommandEvent &event);
protected:
    wxStaticText* gpioIn[16];
    wxCheckBox* gpioDir[16];
    wxCheckBox* gpioOut[16];
    lms_device_t *lmsControl;
    int gpioCnt;
};

#endif
