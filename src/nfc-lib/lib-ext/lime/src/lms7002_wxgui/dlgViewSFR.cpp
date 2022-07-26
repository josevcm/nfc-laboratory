/**
@file	dlgViewSFR.cpp
@author Lime Microsystems
@brief	dialog for viewing MCU SFR data
*/

#include "dlgViewSFR.h"

#include <wx/sizer.h>
#include <wx/grid.h>
#include <wx/intl.h>
#include <wx/string.h>

const long dlgViewSFR::ID_GRID1 = wxNewId();

BEGIN_EVENT_TABLE(dlgViewSFR,wxDialog)
END_EVENT_TABLE()

dlgViewSFR::dlgViewSFR(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	wxBoxSizer* BoxSizer1;
    Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
	Grid1->CreateGrid(16,8);
	Grid1->EnableEditing(false);
	Grid1->EnableGridLines(true);
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	BoxSizer1->Add(Grid1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
}

dlgViewSFR::~dlgViewSFR()
{	
}

void dlgViewSFR::StrArrayIni() 
{
    int i=0;
    for (i=0; i<256; i++) strArray.Add("");

	strArray[0x80]="P0: ";
	strArray[0x81]="SP: ";
	strArray[0x82]="DPL0: ";
	strArray[0x83]="DPH0: ";
	strArray[0x84]="DPL1: ";
	strArray[0x85]="DPH1: ";
	strArray[0x86]="DPS: ";
	strArray[0x87]="PCON: ";

	strArray[0x88]="TCON: ";
	strArray[0x89]="TMOD: ";
	strArray[0x8A]="TL0: ";
	strArray[0x8B]="TL1: ";
	strArray[0x8C]="TH0: ";
	strArray[0x8D]="TH1: ";
	strArray[0x8E]="PMSR: ";

	strArray[0x90]="P1: ";
	strArray[0x91]="DIR1: ";

	strArray[0x98]="SCON: ";
	strArray[0x99]="SBUF: ";


	strArray[0xA0]="P2: ";
	strArray[0xA1]="DIR2: ";
	strArray[0xA2]="DIR0: ";

	strArray[0xA8]="IEN0: ";
	strArray[0xA9]="IEN1: ";

	strArray[0xB0]="EECTRL: ";
	strArray[0xB1]="EEDATA: ";

	strArray[0xB8]="IP0: ";
	strArray[0xB9]="IP1: ";

	strArray[0xBF]="USR2: ";

	strArray[0xC0]="IRCON: ";

	strArray[0xC8]="T2CON: ";

	strArray[0xCA]="RCAP2L: ";
	strArray[0xCB]="RCAP2H: ";
	strArray[0xCC]="TL2: ";
	strArray[0xCD]="TH2: ";

	strArray[0xD0]="PSW: ";

	strArray[0xE0]="ACC: ";

	strArray[0xF0]="B: ";

	strArray[0xEC]="REG0: ";
	strArray[0xED]="REG1: ";
	strArray[0xEE]="REG2: ";
	strArray[0xEF]="REG3: ";
	strArray[0xF4]="REG4: ";
	strArray[0xF5]="REG5: ";
	strArray[0xF6]="REG6: ";
	strArray[0xF7]="REG7: ";

	strArray[0xFC]="REG8: ";
	strArray[0xFD]="REG9: ";
}

void dlgViewSFR::InitGridData(const unsigned char *data)
{
    StrArrayIni();
	for (int i=0; i<16; i++)
        Grid1->SetRowLabelValue(i, wxString::Format(_("Row: 0x%02X"), 0xF8 - i * 8));

	for (int j=0; j<8; j++)
        Grid1->SetColLabelValue(j, wxString::Format(_("Col.: 0x%02X"), j));

	for (int i=0x0080; i<=0x00FF; i++) 
    {
        int row=((i-0x080)/8);
		int col=((i-0x080)-row*8);
		row=15-row;
		if (!(strArray[i]==""))
            Grid1->SetCellValue(row, col, strArray[i] + wxString::Format(_("0x%02X"), data[i]));
        else
            Grid1->SetCellValue(row,col,_(""));
	}
}
