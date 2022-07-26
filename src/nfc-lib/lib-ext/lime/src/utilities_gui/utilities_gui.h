///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __UTILITIES_GUI_H__
#define __UTILITIES_GUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/statbox.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class dlgConnectionSettings_view
///////////////////////////////////////////////////////////////////////////////
class dlgConnectionSettings_view : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText472;
		wxListBox* mListLMS7ports;
		wxButton* btnConnect;
		wxButton* btnCancel;
		wxButton* btnDisconnect;
		
		// Virtual event handlers, overide them in your derived class
		virtual void GetDeviceList( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnConnect( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDisconnect( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dlgConnectionSettings_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Connection Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~dlgConnectionSettings_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgFullMessageLog_view
///////////////////////////////////////////////////////////////////////////////
class dlgFullMessageLog_view : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* txtMessageField;
	
	public:
		
		dlgFullMessageLog_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Message log"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~dlgFullMessageLog_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgDeviceInfo_view
///////////////////////////////////////////////////////////////////////////////
class dlgDeviceInfo_view : public wxDialog 
{
	private:
	
	protected:
		wxButton* btnGetInfo;
		wxStaticText* m_staticText6;
		wxStaticText* lblDeviceCtr;
		wxStaticText* m_staticText8;
		wxStaticText* lblExpansionCtr;
		wxStaticText* m_staticText10;
		wxStaticText* lblFirmwareCtr;
		wxStaticText* m_staticText12;
		wxStaticText* lblHardwareCtr;
		wxStaticText* m_staticText14;
		wxStaticText* lblProtocolCtr;
		wxStaticText* m_staticText41;
		wxStaticText* lblGatewareCtr;
		wxStaticText* m_staticText45;
		wxStaticText* lblGatewareTargetCtr;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnGetInfo( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		dlgDeviceInfo_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Device Info"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~dlgDeviceInfo_view();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SPI_view
///////////////////////////////////////////////////////////////////////////////
class SPI_view : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* ID_STATICTEXT1;
		wxTextCtrl* txtLMSwriteAddr;
		wxStaticText* ID_STATICTEXT9;
		wxTextCtrl* txtLMSwriteValue;
		wxButton* btnLMSwrite;
		wxButton* btnLMSread;
		wxStaticText* ID_STATICTEXT5;
		wxStaticText* lblLMSwriteStatus;
		wxStaticText* ID_STATICTEXT11;
		wxTextCtrl* txtLMSwriteAddr1;
		wxStaticText* ID_STATICTEXT91;
		wxTextCtrl* txtLMSwriteValue1;
		wxButton* btnLMSwrite1;
		wxButton* btnLMSread1;
		wxStaticText* ID_STATICTEXT51;
		wxStaticText* lblLMSwriteStatus1;
		wxStaticText* ID_STATICTEXT12;
		wxTextCtrl* txtLMSwriteAddr2;
		wxStaticText* ID_STATICTEXT92;
		wxTextCtrl* txtLMSwriteValue2;
		wxButton* btnLMSwrite2;
		wxButton* btnLMSread2;
		wxStaticText* ID_STATICTEXT52;
		wxStaticText* lblLMSwriteStatus2;
		wxStaticText* ID_STATICTEXT13;
		wxTextCtrl* txtLMSwriteAddr3;
		wxStaticText* ID_STATICTEXT93;
		wxTextCtrl* txtLMSwriteValue3;
		wxButton* btnLMSwrite3;
		wxButton* btnLMSread3;
		wxStaticText* ID_STATICTEXT53;
		wxStaticText* lblLMSwriteStatus3;
		wxStaticText* ID_STATICTEXT14;
		wxTextCtrl* txtLMSwriteAddr4;
		wxStaticText* ID_STATICTEXT94;
		wxTextCtrl* txtLMSwriteValue4;
		wxButton* btnLMSwrite4;
		wxButton* btnLMSread4;
		wxStaticText* ID_STATICTEXT54;
		wxStaticText* lblLMSwriteStatus4;
		wxStaticText* ID_STATICTEXT18;
		wxTextCtrl* txtLMSwriteAddr5;
		wxStaticText* ID_STATICTEXT95;
		wxTextCtrl* txtLMSwriteValue5;
		wxButton* btnLMSwrite5;
		wxButton* btnLMSread5;
		wxStaticText* ID_STATICTEXT55;
		wxStaticText* lblLMSwriteStatus5;
		wxStaticText* ID_STATICTEXT19;
		wxTextCtrl* txtLMSwriteAddr6;
		wxStaticText* ID_STATICTEXT96;
		wxTextCtrl* txtLMSwriteValue6;
		wxButton* btnLMSwrite6;
		wxButton* btnLMSread6;
		wxStaticText* ID_STATICTEXT56;
		wxStaticText* lblLMSwriteStatus6;
		wxStaticText* ID_STATICTEXT110;
		wxTextCtrl* txtLMSwriteAddr7;
		wxStaticText* ID_STATICTEXT97;
		wxTextCtrl* txtLMSwriteValue7;
		wxButton* btnLMSwrite7;
		wxButton* btnLMSread7;
		wxStaticText* ID_STATICTEXT57;
		wxStaticText* lblLMSwriteStatus7;
		wxStaticText* ID_STATICTEXT15;
		wxTextCtrl* txtBoardwriteAddr;
		wxStaticText* ID_STATICTEXT16;
		wxTextCtrl* txtBoardwriteValue;
		wxButton* ID_BUTTON24;
		wxButton* ID_BUTTON25;
		wxStaticText* ID_STATICTEXT17;
		wxStaticText* lblBoardwriteStatus;
		wxStaticText* ID_STATICTEXT151;
		wxTextCtrl* txtBoardwriteAddr1;
		wxStaticText* ID_STATICTEXT161;
		wxTextCtrl* txtBoardwriteValue1;
		wxButton* ID_BUTTON241;
		wxButton* ID_BUTTON251;
		wxStaticText* ID_STATICTEXT171;
		wxStaticText* lblBoardwriteStatus1;
		wxStaticText* ID_STATICTEXT152;
		wxTextCtrl* txtBoardwriteAddr2;
		wxStaticText* ID_STATICTEXT162;
		wxTextCtrl* txtBoardwriteValue2;
		wxButton* ID_BUTTON242;
		wxButton* ID_BUTTON252;
		wxStaticText* ID_STATICTEXT172;
		wxStaticText* lblBoardwriteStatus2;
		wxStaticText* ID_STATICTEXT153;
		wxTextCtrl* txtBoardwriteAddr3;
		wxStaticText* ID_STATICTEXT163;
		wxTextCtrl* txtBoardwriteValue3;
		wxButton* ID_BUTTON243;
		wxButton* ID_BUTTON253;
		wxStaticText* ID_STATICTEXT173;
		wxStaticText* lblBoardwriteStatus3;
		wxStaticText* ID_STATICTEXT154;
		wxTextCtrl* txtBoardwriteAddr4;
		wxStaticText* ID_STATICTEXT164;
		wxTextCtrl* txtBoardwriteValue4;
		wxButton* ID_BUTTON244;
		wxButton* ID_BUTTON254;
		wxStaticText* ID_STATICTEXT174;
		wxStaticText* lblBoardwriteStatus4;
		wxStaticText* ID_STATICTEXT155;
		wxTextCtrl* txtBoardwriteAddr5;
		wxStaticText* ID_STATICTEXT165;
		wxTextCtrl* txtBoardwriteValue5;
		wxButton* ID_BUTTON245;
		wxButton* ID_BUTTON255;
		wxStaticText* ID_STATICTEXT175;
		wxStaticText* lblBoardwriteStatus5;
		wxStaticText* ID_STATICTEXT156;
		wxTextCtrl* txtBoardwriteAddr6;
		wxStaticText* ID_STATICTEXT166;
		wxTextCtrl* txtBoardwriteValue6;
		wxButton* ID_BUTTON246;
		wxButton* ID_BUTTON256;
		wxStaticText* ID_STATICTEXT176;
		wxStaticText* lblBoardwriteStatus6;
		wxStaticText* ID_STATICTEXT157;
		wxTextCtrl* txtBoardwriteAddr7;
		wxStaticText* ID_STATICTEXT167;
		wxTextCtrl* txtBoardwriteValue7;
		wxButton* ID_BUTTON247;
		wxButton* ID_BUTTON257;
		wxStaticText* ID_STATICTEXT177;
		wxStaticText* lblBoardwriteStatus7;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onLMSwrite( wxCommandEvent& event ) { event.Skip(); }
		virtual void onLMSread( wxCommandEvent& event ) { event.Skip(); }
		virtual void onBoardWrite( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBoardRead( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		SPI_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SPI"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~SPI_view();
	
};

#endif //__UTILITIES_GUI_H__
