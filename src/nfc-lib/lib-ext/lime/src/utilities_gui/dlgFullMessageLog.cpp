#include "dlgFullMessageLog.h"

dlgFullMessageLog::dlgFullMessageLog( wxWindow* parent )
:
dlgFullMessageLog_view( parent )
{

}

void dlgFullMessageLog::AddMessages(const std::deque<wxString> &messages)
{
    for (auto msg : messages)
        txtMessageField->AppendText(msg + _("\n"));
}