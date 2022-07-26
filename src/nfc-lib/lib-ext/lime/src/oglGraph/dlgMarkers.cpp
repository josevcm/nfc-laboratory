/**
@file 	dlgMarkers.cpp
@author Lime Microsystems
@brief 	dialog for setting plot markers
*/

#include "dlgMarkers.h"

//(*InternalHeaders(dlgMarkers)
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(dlgMarkers)
const long dlgMarkers::ID_BUTTON1 = wxNewId();
//*)
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/checkbox.h"
#include "wx/choice.h"
#include "wx/button.h"
#include "OpenGLGraph.h"

BEGIN_EVENT_TABLE(dlgMarkers,wxDialog)
	//(*EventTable(dlgMarkers)
	//*)
END_EVENT_TABLE()

dlgMarkers::dlgMarkers(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    initialized = false;
    BuildContent(parent,id,pos,size);
}

void dlgMarkers::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    parent_graph = (OpenGLGraph*)parent;
	//(*Initialize(dlgMarkers)
	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer1->AddGrowableRow(2);
	sizerMarkerList = new wxFlexGridSizer(0, 1, 0, 0);
	sizerMarkerList->AddGrowableCol(0);
	FlexGridSizer1->Add(sizerMarkerList, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
	sizerDeltasList = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->Add(sizerDeltasList, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
	btnClose = new wxButton(this, ID_BUTTON1, _T("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(btnClose, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&dlgMarkers::OnbtnCloseClick);
	//*)
}

dlgMarkers::~dlgMarkers()
{
	//(*Destroy(dlgMarkers)
	//*)
}

void dlgMarkers::AddMarker(int id)
{
    int index = id;
    int markerId = id+1000;
    wxPanel *pnl = new wxPanel(this, markerId);
    wxFlexGridSizer* FlexGridSizer3;
    FlexGridSizer3 = new wxFlexGridSizer(0, 6, 0, 0);
    wxCheckBox* chk = new wxCheckBox(pnl, markerId, wxString::Format("M%i", id));
    FlexGridSizer3->Add(chk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    enables.push_back(chk);
    chk = new wxCheckBox(pnl, markerId, "display");
    FlexGridSizer3->Add(chk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    shows.push_back(chk);
    wxStaticText *StaticText2 = new wxStaticText(pnl, markerId, wxString::Format("Marker%i freq(MHz):", id), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxTextCtrl *TextCtrl1 = new wxTextCtrl(pnl, markerId, _T("0.0"), wxDefaultPosition, wxSize(48,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    freqs.push_back(TextCtrl1);
    FlexGridSizer3->Add(TextCtrl1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxStaticText* StaticText3 = new wxStaticText(pnl, markerId, _T("value (dB):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxStaticText* StaticText4 = new wxStaticText(pnl, markerId, _T("0.0"), wxDefaultPosition, wxSize(168,-1), 0, _T("ID_STATICTEXT4"));
    labels.push_back(StaticText4);
    marker_valuesA.push_back(0);
    marker_valuesB.push_back(0);
    FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Connect(markerId,wxEVT_CHECKBOX,(wxObjectEventFunction)&dlgMarkers::OnMarkerChange);
    Connect(markerId,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&dlgMarkers::OnMarkerChange);
    pnl->SetSizer(FlexGridSizer3);
    sizerMarkerList->Add(pnl, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    pnl->Layout();
    FlexGridSizer1->Layout();
    Fit();
    parent_graph->markers[index].used = false;
}

void dlgMarkers::AddDeltas()
{
    wxArrayString selections;
    for(unsigned i=0; i<parent_graph->markers.size(); ++i)
    {
        selections.push_back(wxString::Format("M%u", i));
    }
    for(int i=0; i<5; ++i)
    {
        wxPanel *pnl = new wxPanel(this, wxNewId());
        wxFlexGridSizer* FlexGridSizer3;
        FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
        wxStaticText* StaticText1 = new wxStaticText(pnl, wxNewId(), wxString::Format("Delta%i", i), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
        FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
        wxChoice* dsrc0 = new wxChoice(pnl, wxNewId(), wxDefaultPosition, wxSize(64,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
        dsrc0->Append(selections);
        dsrc0->SetSelection(0);
        deltaSrc.push_back(dsrc0);
        FlexGridSizer3->Add(dsrc0, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
        wxChoice* dsrc1 = new wxChoice(pnl, wxNewId(), wxDefaultPosition, wxSize(64,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
        dsrc1->Append(selections);
        dsrc1->SetSelection(0);
        deltaSrc.push_back(dsrc1);
        FlexGridSizer3->Add(dsrc1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
        wxStaticText* lblDelta0 = new wxStaticText(pnl, wxNewId(), _T("0.0 dB"), wxDefaultPosition, wxSize(180,-1), 0, _T("ID_STATICTEXT2"));
        deltaValues.push_back(lblDelta0);
        FlexGridSizer3->Add(lblDelta0, 1, wxALL|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10);
        pnl->SetSizer(FlexGridSizer3);
        sizerDeltasList->Add(pnl, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
        pnl->Layout();
    }
    FlexGridSizer1->Layout();
    Fit();
}

void dlgMarkers::OnbtnCloseClick(wxCommandEvent& event)
{
    Hide();
}

void dlgMarkers::OnMarkerChange(wxCommandEvent& event)
{
    double freq = 0;
    int markerIndex = event.GetId() - 1000;
    if (event.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED)
        event.GetString().ToDouble(&freq);
    else
        freqs[markerIndex]->GetValue().ToDouble(&freq);
    freq *= 1000000;
    parent_graph->SetMarker(markerIndex, freq, enables[markerIndex]->IsChecked(), shows[markerIndex]->IsChecked());
}

void dlgMarkers::UpdateValues()
{
    if(IsShownOnScreen())
    {
        for(size_t i=0; i<labels.size(); ++i)
        {
            if(parent_graph == NULL)
                return;
            if (!parent_graph->markers[i].used)
                continue;
            char text[128];
            double valueA = 0;
            double valueB = 0;
            int cnt=0;

            if(parent_graph->series[0]->size > 0 && parent_graph->series[0]->visible)
            {
                valueA = parent_graph->series[0]->values[parent_graph->markers[i].dataValueIndex+1];
                cnt = sprintf(text, "%3.1f (ChA) ; ",valueA);
            }

            if(parent_graph->series[1]->size > 0 && parent_graph->series[1]->visible)
            {
                valueB = parent_graph->series[1]->values[parent_graph->markers[i].dataValueIndex+1];
                sprintf(text+cnt, "%3.1f (ChB) ;",valueB);
            }

            labels[i]->SetLabel(wxString(text));
            marker_valuesA[i] = valueA;
            marker_valuesB[i] = valueB;
            enables[i]->SetValue(parent_graph->markers[i].used);
            shows[i]->SetValue(parent_graph->markers[i].show);
        }

        for(int i=0; i<5; ++i)
        {
            int src1 = deltaSrc[2*i]->GetSelection();
            int src2 = deltaSrc[2*i+1]->GetSelection();
            char text[128]={0};
            int cnt = 0;

            if(parent_graph->series[0]->size > 0 && parent_graph->series[0]->visible)
            {
                float deltaValue = marker_valuesA[src1]-marker_valuesA[src2];
                cnt = sprintf(text, "%.3f (ChA) ; ",deltaValue);
            }

            if(parent_graph->series[1]->size > 0 && parent_graph->series[1]->visible)
            {
                float deltaValue = marker_valuesB[src1]-marker_valuesB[src2];
                sprintf(text+cnt, "%.3f (ChB)",deltaValue);
            }
            deltaValues[i]->SetLabel(wxString(text));
        }
        if (refreshMarkFreq)
        {
            for (size_t i = 0; i < labels.size(); ++i)
            {
                double freq = parent_graph->markers[i].posX / 1000000;
                freqs[i]->SetValue(wxString::Format("%3.2f", freq));
            }
            refreshMarkFreq = false;
        }
    }
}

void dlgMarkers::OnButton1Click1(wxCommandEvent& event)
{
}

