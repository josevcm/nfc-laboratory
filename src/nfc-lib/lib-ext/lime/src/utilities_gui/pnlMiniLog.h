#ifndef __pnlMiniLog__
#define __pnlMiniLog__

#include "utilities_gui.h"
#include <deque>
#include <utility> //pair
#include "Logger.h"

class pnlMiniLog : public wxPanel
{
public:
    pnlMiniLog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
    void OnBtnClearClicked(wxCommandEvent& event);
    void OnShowFullLog(wxCommandEvent& event);
    void onLogLvlChange(wxCommandEvent& event);
    //// end generated class members
    void HandleMessage(wxCommandEvent &event);
    void OnUpdateGUI(wxUpdateUIEvent& event);
    void OnLogDataClicked( wxCommandEvent& event );
    wxCheckBox* chkLogData;
private:
    wxTextAttr mDefaultStyle;
    std::deque<wxString> mAllMessages;
    std::deque<std::pair<lime::LogLevel, wxString>> mMessageList;
    int log_level;

protected:
    wxTextCtrl* txtMessageField;
    wxButton* btnClear;
    wxButton* btnFullLog;
    wxChoice* choiceLogLvl;
};

#endif // __pnlMiniLog__
