/**
@file 	dlgViewSFR.h
@author Lime Microsystems
*/

#ifndef DLGVIEWSFR_H
#define DLGVIEWSFR_H

#include <wx/dialog.h>
class wxGrid;
class wxBoxSizer;
class wxGridEvent;

#include <wx/arrstr.h>
#include <vector>

class dlgViewSFR: public wxDialog
{
	public:

		dlgViewSFR(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~dlgViewSFR();

		wxGrid* Grid1;
        void InitGridData(const unsigned char *data);
	protected:
        static const long ID_GRID1;
		wxArrayString strArray;
		void StrArrayIni();
	private:
		DECLARE_EVENT_TABLE()
};

#endif
