#ifndef __limeRFE_wxgui__
#define __limeRFE_wxgui__

#include "limeRFE_constants.h"
#include "limeRFE_gui.h"

#include <iostream>
#include <fstream>

#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <vector>

class limeRFE_wxgui : public limeRFE_view
{
	protected:
		lms_device_t *lmsControl;

		// Handlers for AppFrame events.
		void OnbtnOpenPort(wxCommandEvent& event);
		void OnbtnClosePort(wxCommandEvent& event);
		void AddMssg(const char* mssg);
		void AddMssg(const wxString& mssg);
		void ReadPorts();
		void OnbtnRefreshPorts(wxCommandEvent& event);
		void OnbtnReset(wxCommandEvent& event);
		void OnbtnOpen(wxCommandEvent& event);
		void OnbtnSave(wxCommandEvent& event);
		void State2GUI(rfe_boardState state);
		void GUI2State(rfe_boardState *state);
		void setTXRXBtns();
		void OnbtnBoard2GUI(wxCommandEvent& event);
		void OncTypeRX(wxCommandEvent& event);
		void OncTypeTX(wxCommandEvent& event);
		void SetChannelsChoices();
		void SetChannelsChoicesTXRX(int channelTXRX);
		void SetChannelsTypesTXRX(int channelTXRX);
		void OncChannelRX(wxCommandEvent& event);
		void OncChannelTX(wxCommandEvent& event);
		void SetConfigurationOptions();
		int GetChannelID(int channelTXRX);
		int GetChannelIndexes(int channelID, int* typeIndex, int* channelIndex);

		int lastSelectionRX[3] = { 0, 0, 0 };
		int lastSelectionTX[3] = { 0, 0, 0 };

		int portRXvals[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int portTXvals[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		bool SelectPort(int channelTXRX);

		int lastTypeSelectionRX = 0;
		int lastTypeSelectionTX = 0;

		int lastPortRXSelection = 0;
		int lastPortTXSelection = 0;

		void OncPortRX(wxCommandEvent& event);
		void OncPortTX(wxCommandEvent& event);

		void OntbtnTXRX(wxCommandEvent& event);
		void SetModeLabel();

		void OnbtnConfigure(wxCommandEvent& event);

		bool configured;

		rfe_boardState configuredState;

		int activeMode;

		void UpdateRFEForm();

		void OncbNotch(wxCommandEvent& event);
		void OncAttenuation(wxCommandEvent& event);

		void OncbTXasRX(wxCommandEvent& event);

		void OncbEnableSWR(wxCommandEvent& event);

		void OnbtnReadADC(wxCommandEvent& event);

		void OntbtnRXEN(wxCommandEvent& event);
		void OntbtnTXEN(wxCommandEvent& event);
		void OntbtnTXRXEN(wxCommandEvent& event);

		void OnrbI2CrbUSB(wxCommandEvent& event);

		void OnrbSWRext(wxCommandEvent& event);
		void OnrbSWRcell(wxCommandEvent& event);

		void OnbtnCalibrate(wxCommandEvent& event);

		void OnQuit(wxCommandEvent& event);

		void OnmiPowerMeter(wxCommandEvent& event);

		int CalculatePowerAndGamma(int adc1, int adc2, double * pin_dBm, double * gamma_dB);

		int GetCommType();

		rfe_dev_t* rfe; //Port handle

		double powerCellCalCorr;
		double powerCalCorr;
		double rlCalCorr;

		void PrintError(int errorCode);

		void FindChannelChoices(int type, wxArrayString* channelItems);

		void OnbtnClearMessages(wxCommandEvent& event);

	public:
		/** Constructor */
		limeRFE_wxgui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long styles = 0);
        void Initialize(lms_device_t* lms);
};

#endif // __limeRFE_wxgui__
