/**
@file 	dlgViewIRAM.h
@author Lime Microsystems
*/

#ifndef DLGVIEWIRAM_H
#define DLGVIEWIRAM_H

#include <wx/dialog.h>
#include <vector>
class wxGrid;
class wxBoxSizer;
class wxGridEvent;

class dlgViewIRAM: public wxDialog
{
	public:

		dlgViewIRAM(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~dlgViewIRAM();

		wxGrid* Grid1;
		void InitGridData(const unsigned char *data);
	protected:
		static const long ID_GRID1;

	private:
		DECLARE_EVENT_TABLE()
};

#endif
