/**
@file 	dlgMarkers.h
@author Lime Microsystems
*/
#ifndef DLGMARKERS_H
#define DLGMARKERS_H

//(*Headers(dlgMarkers)
#include <wx/dialog.h>
class wxFlexGridSizer;
class wxButton;
//*)
#include <wx/timer.h>

class wxStaticText;
class wxTextCtrl;
class wxCheckBox;
class wxChoice;
class wxButton;
#include <vector>

class OpenGLGraph;

class dlgMarkers: public wxDialog
{
	public:

		dlgMarkers(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~dlgMarkers();
                OpenGLGraph* parent_graph;
		void AddMarker(int id);
		std::vector<float> marker_valuesA;
                std::vector<float> marker_valuesB;
		std::vector<wxStaticText*> labels;
		std::vector<wxTextCtrl*> freqs;
		std::vector<wxCheckBox*> enables;
		std::vector<wxCheckBox*> shows;
		void UpdateValues();
		void AddDeltas();

		std::vector<wxChoice*> deltaSrc;
		std::vector<wxStaticText*> deltaValues;

		//(*Declarations(dlgMarkers)
		wxButton* btnClose;
		wxFlexGridSizer* sizerMarkerList;
		wxFlexGridSizer* sizerDeltasList;
		wxFlexGridSizer* FlexGridSizer1;
		//*)

		//(*Identifiers(dlgMarkers)
		static const long ID_BUTTON1;
		//*)

		//(*Handlers(dlgMarkers)
		void OnbtnCloseClick(wxCommandEvent& event);
		void OnbtnAddMarkerClick(wxCommandEvent& event);
		void OnbtnDeleteClick(wxCommandEvent& event);
		void OnMarkerChange(wxCommandEvent& event);
		void OnButton1Click(wxCommandEvent& event);
		void OnInit(wxInitDialogEvent& event);
		void OnButton1Click1(wxCommandEvent& event);
		//*)
		void OnEnableMarker(wxCommandEvent& event);
        bool refreshMarkFreq;

	protected:
	    bool initialized;
		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

		DECLARE_EVENT_TABLE()
};

#endif
