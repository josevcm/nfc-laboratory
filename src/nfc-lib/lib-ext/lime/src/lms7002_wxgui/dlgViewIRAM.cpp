/**
@file	dlgViewIRAM.cpp
@author Lime Microsystems
@brief	dialog for viewing MCU IRAM data
*/

#include "dlgViewIRAM.h"

#include <wx/sizer.h>
#include <wx/grid.h>
#include <wx/intl.h>
#include <wx/string.h>

const long dlgViewIRAM::ID_GRID1 = wxNewId();

BEGIN_EVENT_TABLE(dlgViewIRAM,wxDialog)
END_EVENT_TABLE()

dlgViewIRAM::dlgViewIRAM(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	wxBoxSizer* BoxSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
	Grid1->CreateGrid(32,8);
	Grid1->EnableEditing(false);
	Grid1->EnableGridLines(true);
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	BoxSizer1->Add(Grid1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
}

dlgViewIRAM::~dlgViewIRAM()
{
}

void dlgViewIRAM::InitGridData(const unsigned char *data)
{
	for (int i=0; i<=31; i++)
        Grid1->SetRowLabelValue(i, wxString::Format(_("Row: 0x%02X"), 0xF8 - i * 8));

	for (int j=0; j<8; j++)     //columns
        Grid1->SetColLabelValue(j, wxString::Format(_("Col.: 0x%02X"), j));

	for (int i=0x0000; i<=0x00FF; i++)
    {
		int row=(i/8);
		int col=(i-row*8);
		row=31-row;
        Grid1->SetCellValue(row, col, wxString::Format(_("0x%02X"), data[i]));
	}
}

