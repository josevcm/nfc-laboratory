///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __FFTVIEWER_WXGUI_H__
#define __FFTVIEWER_WXGUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
class OpenGLGraph;

#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class frFFTviewer
///////////////////////////////////////////////////////////////////////////////
class frFFTviewer : public wxFrame 
{
	private:
	
	protected:
		wxSplitterWindow* mPlotsSplitter;
		wxPanel* mTimeConstellationPanel;
		OpenGLGraph* mTimeDomainPanel;
		OpenGLGraph* mConstelationPanel;
		OpenGLGraph* mFFTpanel;
		wxStaticText* m_staticText6;
		wxTextCtrl* txtNyquistFreqMHz;
		wxStaticText* m_staticText7;
		wxSpinCtrl* spinFFTsize;
		wxChoice* cmbStreamType;
		wxChoice* cmbFmt;
		wxCheckBox* chkEnTx;
		wxCheckBox* chkEnSync;
		wxButton* btnStartStop;
		wxStaticText* m_staticText8;
		wxStaticText* lblRxDataRate;
		wxStaticText* m_staticText18;
		wxStaticText* lblTxDataRate;
		wxChoice* cmbWindowFunc;
		wxCheckBox* chkCaptureToFile;
		wxStaticText* m_staticText12;
		wxSpinCtrl* spinCaptureCount;
		wxCheckBox* chkFreezeTimeDomain;
		wxCheckBox* chkFreezeConstellation;
		wxCheckBox* chkFreezeFFT;
		wxStaticText* m_staticText11;
		wxChoice* cmbChannelVisibility;
		wxStaticText* m_staticText23;
		wxSpinCtrl* spinAvgCount;
		wxStaticText* m_staticText15;
		wxGauge* gaugeRxBuffer;
		wxStaticText* m_staticText16;
		wxGauge* gaugeTxBuffer;
		wxCheckBox* chkEnPwr;
		wxStaticText* m_staticText13;
		wxStaticText* m_staticText14;
		wxStaticText* m_staticText151;
		wxTextCtrl* txtCenterOffset1;
		wxTextCtrl* txtCenterOffset2;
		wxStaticText* m_staticText161;
		wxTextCtrl* txtBW1;
		wxTextCtrl* txtBW2;
		wxStaticText* m_staticText17;
		wxStaticText* lblPower1;
		wxStaticText* lblPower2;
		wxStaticText* m_staticText20;
		wxStaticText* lbldBc;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnFFTsamplesCountChanged( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnStreamChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFmtChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEnTx( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnbtnStartStop( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnWindowFunctionChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnChannelVisibilityChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAvgChange( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnAvgChangeEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEnPwr( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		frFFTviewer( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("FFT viewer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~frFFTviewer();
		
		void mPlotsSplitterOnIdle( wxIdleEvent& )
		{
			mPlotsSplitter->SetSashPosition( 0 );
			mPlotsSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( frFFTviewer::mPlotsSplitterOnIdle ), NULL, this );
		}
	
};

#endif //__FFTVIEWER_WXGUI_H__
