/**
@file	numericSlider.cpp
@brief	Slider control with text field for integer entry
@author	Lime Microsystems (www.limemicro.com)
*/

#include "numericSlider.h"
#include <wx/event.h>
#include <wx/spinctrl.h>

IMPLEMENT_DYNAMIC_CLASS(NumericSlider, wxPanel)

NumericSlider::NumericSlider()
{
}

NumericSlider::NumericSlider(
		wxWindow* parent,
		wxWindowID id,
		const wxString &value,
		const wxPoint &pos,
		const wxSize &size,
		long style,
		int min,
		int max,
		int initial,
		const wxString &name
	)
{
    Create(parent, id, pos, size, style);

    wxFlexGridSizer* mainSizer;
    mainSizer = new wxFlexGridSizer(0, 2, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->SetFlexibleDirection(wxHORIZONTAL);

    mScroll = new wxScrollBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);
    mScroll->SetMinSize(wxSize(128, -1));
    mainSizer->Add(mScroll, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND, 0);
    int sliderStep = (max - min) / 20;
    if (sliderStep == 0)
        sliderStep = 1;
    mScroll->SetScrollbar(initial, 1, max - min + 1, sliderStep);
    mScroll->Connect(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(NumericSlider::OnScrollChange), NULL, this);

    mSpinner = new wxSpinCtrl(this, wxNewId(), wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, min, max, initial);
    //mSpinner->SetMinSize(wxSize(112, -1));
    mainSizer->Add(mSpinner, 0, wxALIGN_CENTER_VERTICAL, 0);
    mSpinner->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(NumericSlider::OnSpinnerChange), NULL, this);
    mSpinner->Connect(wxEVT_TEXT_ENTER, wxSpinEventHandler(NumericSlider::OnSpinnerChangeEnter), NULL, this);

    mScroll->SetThumbPosition(mSpinner->GetValue() - mSpinner->GetMin());

    this->SetSizer(mainSizer);
    this->Layout();
    mainSizer->Fit(this);
}

NumericSlider::~NumericSlider()
{

}

void NumericSlider::OnSpinnerChangeEnter(wxSpinEvent &event)
{
    wxCommandEvent evt(wxEVT_COMMAND_SPINCTRL_UPDATED);
    evt.SetId(this->GetId());
    std::string test = event.GetString().ToStdString();
    long value = 0;
    event.GetString().ToLong(&value);
    evt.SetInt(value);
    evt.SetEventObject(this);
    mScroll->SetThumbPosition(event.GetInt() - mSpinner->GetMin());
    wxPostEvent(this, evt);
}

void NumericSlider::OnSpinnerChange(wxSpinEvent &event)
{
    wxCommandEvent evt(wxEVT_COMMAND_SPINCTRL_UPDATED);
    evt.SetId(this->GetId());
    evt.SetInt(event.GetInt());
    evt.SetEventObject(this);
    mScroll->SetThumbPosition(event.GetInt() - mSpinner->GetMin());
    wxPostEvent(this, evt);
}

void NumericSlider::OnScrollChange(wxScrollEvent &event)
{
    wxCommandEvent evt(wxEVT_COMMAND_SPINCTRL_UPDATED);
    evt.SetId(this->GetId());
    evt.SetEventObject(this);
    mSpinner->SetValue(event.GetInt() + mSpinner->GetMin());
    evt.SetInt(event.GetInt() + mSpinner->GetMin());
    wxPostEvent(this, evt);
}

void NumericSlider::SetValue(int integer)
{
    if (integer < mSpinner->GetMin())
        integer = mSpinner->GetMin();
    else if (integer > mSpinner->GetMax())
        integer = mSpinner->GetMax();
    mSpinner->SetValue(integer);
    mScroll->SetThumbPosition(integer - mSpinner->GetMin());
}

int NumericSlider::GetValue()
{
    return mSpinner->GetValue();
}

void NumericSlider::SetToolTip(const wxString &tipString)
{
#if wxUSE_TOOLTIPS
    mSpinner->UnsetToolTip();
    mSpinner->SetToolTip(tipString);
    mScroll->SetToolTip(tipString);
#endif
}

BEGIN_EVENT_TABLE(NumericSlider, wxPanel)
END_EVENT_TABLE()

