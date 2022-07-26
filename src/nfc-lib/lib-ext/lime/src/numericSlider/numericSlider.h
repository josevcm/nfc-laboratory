/**
@file	ComboBoxMap.h
@brief	Class for integer input using slider or text field
@author Lime Microsystems (www.limemicro.com)
*/

#ifndef NUMERIC_SLIDER_H
#define NUMERIC_SLIDER_H

#include <wx/wx.h>
#include <wx/panel.h>

class wxSpinCtrl;
class wxScrollBar;

class NumericSlider : public wxPanel
{
public:
    NumericSlider();
    NumericSlider(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString &value = wxEmptyString,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = wxSP_ARROW_KEYS,
        int min = 0,
        int max = 100,
        int initial = 0,
        const wxString &name = "numericSlider"
        );
   ~NumericSlider();

   void SetValue(int integer);
   int GetValue();

   virtual void SetToolTip(const wxString &tipString);
protected:
    void OnSpinnerChangeEnter(wxSpinEvent &event);
    void OnSpinnerChange(wxSpinEvent &event);
    void OnScrollChange(wxScrollEvent &event);
    wxSpinCtrl* mSpinner;
    wxScrollBar* mScroll;
private:
	DECLARE_DYNAMIC_CLASS(NumericSlider)
	DECLARE_EVENT_TABLE()
};

#endif
