#ifndef PNL_CORESDR_H
#define PNL_CORESDR_H

#include "lime/LimeSuite.h"
#include <map>
#include <wx/panel.h>
#include <wx/choice.h>
class wxStaticText;
class wxFlexGridSizer;
class wxCheckBox;

class pnlCoreSDR : public wxPanel
{
public:
    pnlCoreSDR(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t *pControl);
    virtual ~pnlCoreSDR();
    virtual void UpdatePanel();
    void OnLoopbackChange(wxCommandEvent &event);

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxChoice* cmbRx1Path;
    wxChoice* cmbTx1Path;
    wxChoice* cmbRx2Path;
    wxChoice* cmbTx2Path;
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};

#endif
