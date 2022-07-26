#ifndef PNL_ULTIMATEEVB_H
#define PNL_ULTIMATEEVB_H

#include "lime/LimeSuite.h"
#include <map>
#include <vector>
#include <wx/panel.h>
#include <wx/stattext.h>

class wxStaticText;
class wxFlexGridSizer;
class wxCheckBox;

class pnlUltimateEVB : public wxPanel
{
    class ADCparam
    {
    public:
        ADCparam(int ch, std::string nm = ""): title(nullptr), value(nullptr)
        {
            channel = ch;
            name = nm;
        };
        ~ADCparam()
        {
            if (title) title->Destroy();
            if (value) value->Destroy();
        }
        int channel;
        std::string name;
        wxStaticText* title;
        wxStaticText* value;
    };
public:
    pnlUltimateEVB(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t *pControl);
    virtual ~pnlUltimateEVB();

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);
    void OnSave(wxCommandEvent &event);

protected:
    std::vector<ADCparam> pwrParams;
    std::vector<ADCparam> vltgParams;
    wxStaticText* totalPwr;
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};

#endif
