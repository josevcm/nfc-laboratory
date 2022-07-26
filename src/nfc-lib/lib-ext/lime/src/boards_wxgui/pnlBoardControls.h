#ifndef __pnlAnalog_view__
#define __pnlAnalog_view__

/**	@brief GUI for writing and reading analog controls
	@author Lime Microsystems (www.limemicro.com)
*/

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/frame.h>
#include <wx/spinctrl.h>
#include "lime/LimeSuite.h"
#include <vector>
#include <string>
#include <stdint.h>
#include <LMSBoards.h>

namespace lime{
}

class wxTextCtrl;

class pnlBoardControls : public wxFrame
{
	public:
        struct ADC_DAC
        {
            std::string name;
            bool writable;
            double value;
            uint8_t channel;
            std::string units;
            int8_t powerOf10;
            int minValue;
            int maxValue;
        };

        class Param_GUI
        {
        public:
            Param_GUI() : title(nullptr), units(nullptr), rValue(nullptr),wValue(nullptr){};
            ~Param_GUI()
            {
                if (title)
                    title->Destroy();
                if (units)
                    units->Destroy();
                if (rValue)
                    rValue->Destroy();
                if (wValue)
                    wValue->Destroy();
            }
            wxStaticText* title;
            wxStaticText* units;
            wxStaticText* rValue;
            wxSpinCtrl* wValue;
        };

        pnlBoardControls(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &title = _(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL);
        ~pnlBoardControls();

        void UpdatePanel();
        void Initialize(lms_device_t* controlPort);
        lms_device_t* lmsControl;

        void SetupControls(const std::string &boardID);
        void OnSetDACvalues(wxSpinEvent &event);
        void OnSetDACvaluesENTER(wxCommandEvent &event);
        void OnDACWrite(wxCommandEvent &event);
        static std::vector<ADC_DAC> mParameters;
	protected:
        wxPanel* pnlCustomControls;
        wxPanel* pnlReadControls;
        wxSpinCtrl *spinCustomChannelRd;
        wxStaticText *txtCustomValueRd;
        wxStaticText *txtCustomUnitsRd;
        wxStaticText *txtCustomPowerOf10Rd;
        wxSpinCtrl *spinCustomChannelWr;
        wxSpinCtrl *spinCustomValueWr;
        wxChoice *cmbCustomUnitsWr;
        wxChoice *cmbCustomPowerOf10Wr;
        wxButton *btnCustomRd;
        wxButton *btnCustomWr;
        void OnCustomRead(wxCommandEvent& event);
        void OnCustomWrite(wxCommandEvent& event);

        wxFlexGridSizer* sizerAnalogRd;
        wxPanel* additionalControls;
        wxStaticText* txtDACTitle;
        wxTextCtrl* txtDACValue;
        wxButton* btnDAC;
        wxFlexGridSizer* sizerDAC;

        wxFlexGridSizer* sizerAdditionalControls;
        
        std::vector<ADC_DAC> getBoardParams(const std::string &boardID);

        void OnUserChangedBoardType(wxCommandEvent& event);
        void OnReadAll( wxCommandEvent& event );
        void OnWriteAll( wxCommandEvent& event );

        wxButton* btnReadAll;
        wxButton* btnWriteAll;
        wxStaticText* m_staticText349;
        wxChoice* cmbBoardSelection;



        std::vector<Param_GUI*> mGUI_widgets;
        static const std::vector<lime::eLMS_DEV> board_list;
};

#endif // __pnlAnalog_view__
