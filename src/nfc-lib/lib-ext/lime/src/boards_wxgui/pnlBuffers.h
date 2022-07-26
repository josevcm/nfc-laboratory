#ifndef PNL_BUFFERS_H
#define PNL_BUFFERS_H

#include "lime/LimeSuite.h"
#include <wx/panel.h>
class wxCheckBox;

class pnlBuffers : public wxPanel
{
public:
    pnlBuffers(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = 0, wxString name = wxEmptyString);
    void Initialize(lms_device_t *pControl);
    virtual ~pnlBuffers();
    virtual void UpdatePanel();
    void OnGPIOchanged(wxCommandEvent &event);

    void OnReadAll(wxCommandEvent &event);
    void OnWriteAll(wxCommandEvent &event);

protected:
    wxCheckBox* chkDIO_DIR_CTRL1;
    wxCheckBox* chkDIO_DIR_CTRL2;
    wxCheckBox* chkDIO_BUFF_OE;
    wxCheckBox* chkIQ_SEL1_DIR;
    wxCheckBox* chkIQ_SEL2_DIR;
    wxCheckBox* chkG_PWR_DWN;
    lms_device_t *lmsControl;
    DECLARE_EVENT_TABLE()
};

#endif
