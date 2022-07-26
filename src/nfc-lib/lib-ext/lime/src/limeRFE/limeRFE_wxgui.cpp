#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "limeRFE_wxgui.h"
#include "limeRFE.h"
#include "lms7suiteAppFrame.h"
#include "RFE_Device.h"

#include <chrono>

limeRFE_wxgui::limeRFE_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
limeRFE_view(parent, id, title, pos, size)
{
    lmsControl = nullptr;

	rfe = nullptr;

	lastPortRXSelection = 1;
	lastPortTXSelection = 1;

	cbNotch->SetValue(RFE_NOTCH_DEFAULT);

	SetModeLabel();

	cbNotch->SetValue(RFE_NOTCH_DEFAULT);

	ReadPorts();

	SetChannelsChoices();

	cAttenuation->Clear();
	for (int i = 0; i <= 7; i++) {
		cAttenuation->AppendString(wxString::Format(wxT("%i dB"), i * 2));
	}
	cAttenuation->SetSelection(0);

	configured = false;

	activeMode = RFE_MODE_NONE;

	powerCellCalCorr = 0.0;
	powerCalCorr = 0.0;
	rlCalCorr = 0.0;

	cbTXasRX->SetValue(true);
	SetChannelsTypesTXRX(RFE_CHANNEL_RX);

	UpdateRFEForm();
}

void limeRFE_wxgui::Initialize(lms_device_t* lms)
{
    lmsControl = lms;
}

void limeRFE_wxgui::OnbtnOpenPort(wxCommandEvent& event) {

	if (rfe) {
		AddMssg("Port already opened.");
		return;
	}

	wxString PortName = cmbbPorts->GetValue();

        if (GetCommType() == RFE_USB)
            rfe = RFE_Open(PortName.mb_str(), lmsControl);
        else
            rfe = RFE_Open(nullptr, lmsControl);

	if (rfe == nullptr) {
		AddMssg("Error initializing serial port");
		return;
	}

	unsigned char cinfo[4];

	int result = RFE_GetInfo(rfe, cinfo);
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}

	boardInfo info;

	info.fw_ver = cinfo[0];
	info.hw_ver = cinfo[1];
	info.status1 = cinfo[2];
	info.status2 = cinfo[3];

	if (((info.fw_ver == 255) && (info.hw_ver == 0xff)) ||
		((info.fw_ver == 0) && (info.hw_ver == 0x00))) {
		AddMssg("Error initializing serial port");
		AddMssg("Please check the connection between the SDR and LimeRFE");
		RFE_Close(rfe);
		rfe = nullptr;
		return;
	}

	AddMssg("Port opened");

	char msg[200];
	sprintf(msg, "Firmware version: %d", (int)info.fw_ver);
	AddMssg(msg);
	sprintf(msg, "Hardware version: %#x", (int)info.hw_ver);
	AddMssg(msg);

	OnbtnBoard2GUI(event);
}

void limeRFE_wxgui::OnbtnClosePort(wxCommandEvent& event) {

	RFE_Close(rfe);
	rfe = nullptr;
	AddMssg("Port closed");
	configured = false;
	UpdateRFEForm();
}

void limeRFE_wxgui::AddMssg(const char* mssg) {
	wxString s(mssg, wxConvUTF8);
	AddMssg(s);
}

void limeRFE_wxgui::AddMssg(const wxString& mssg) {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	//add time stamp
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%H:%M:%S", timeinfo);
	wxString line(wxString::Format("[%s] %s", buffer, mssg));

	txtMessageField->AppendText(line + _("\n"));
}

void limeRFE_wxgui::ReadPorts() {

	cmbbPorts->Clear();

#ifdef _MSC_VER
	TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	DWORD test;
	bool gotPort = 0; // in case the port is not found

	char portName[100];

	for (int i = 0; i<255; i++) // checking ports from COM0 to COM255
	{
		sprintf(portName, "COM%d", i);

		test = QueryDosDeviceA((LPCSTR)portName, (LPSTR)lpTargetPath, 5000);

		// Test the return value and error if any
		if (test != 0) //QueryDosDevice returns zero if it didn't find an object
		{
			cmbbPorts->AppendString(portName);
		}

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			lpTargetPath[10000]; // in case the buffer got filled, increase size of the buffer.
			continue;
		}
	}
#endif // WIN

#ifdef __unix__
	//Maybe it would be better to follow this:
	//https://stackoverflow.com/questions/15342804/c-linux-detect-all-serial-ports
	//instead of trying to open port, as it is implemented now
	char portName[100];
	int fd;

	for (int i = 0; i < 256; ++i)
	{
		sprintf(portName, "/dev/ttyUSB%d", i);
		//	    fd = open(portName, O_RDWR | O_NOCTTY | O_DELAY);
		fd = open(portName, O_RDWR | O_NOCTTY);
		if (fd != -1)
		{
			cmbbPorts->AppendString(portName);
		}
	}
#endif // LINUX

	cmbbPorts->SetSelection(0);
}

void limeRFE_wxgui::OnbtnRefreshPorts(wxCommandEvent& event) {
	ReadPorts();
}

void limeRFE_wxgui::OnbtnReset(wxCommandEvent& event) {

	int result = RFE_Reset(rfe);

	if (result != RFE_SUCCESS){
		PrintError(result);
		return;
	}

	OnbtnBoard2GUI(event);
}

void limeRFE_wxgui::OnbtnOpen(wxCommandEvent& event){
	wxFileDialog dlg(this, _("Open config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	int result = RFE_LoadConfig(rfe, dlg.GetPath().To8BitData());
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}

	OnbtnBoard2GUI(event);

	rfe_boardState state;
	guiState stateGUI;
	ReadConfig(dlg.GetPath().To8BitData(), &state, &stateGUI);

	powerCellCalCorr = stateGUI.powerCellCorr;
	powerCalCorr = stateGUI.powerCorr;
	rlCalCorr = stateGUI.rlCorr;
}

void limeRFE_wxgui::OnbtnSave(wxCommandEvent& event) {
	if (!configured) {
		AddMssg("Error: The board has not been configured to the current settings. Please configure the board prior to saving the state.");
		return;
	}

	wxFileDialog dlg(this, _("Save config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	guiState stateGUI;
	configuredState.mode = activeMode;
	stateGUI.powerCellCorr = powerCellCalCorr;
	stateGUI.powerCorr = powerCalCorr;
	stateGUI.rlCorr = rlCalCorr;

	int status = SaveConfig(dlg.GetPath().To8BitData(), configuredState, stateGUI);
	if (status != 0)
		wxMessageBox(_("Failed to save file"), _("Warning"));
}

void limeRFE_wxgui::State2GUI(rfe_boardState state) {
	int stateChannelIDRX = state.channelIDRX;
	int stateChannelIDTX = state.channelIDTX;
	int typeIndexRX, channelIndexRX, typeIndexTX, channelIndexTX;
	int resultRX = GetChannelIndexes(stateChannelIDRX, &typeIndexRX, &channelIndexRX);
	int resultTX = GetChannelIndexes(stateChannelIDTX, &typeIndexTX, &channelIndexTX);
	if (resultRX != 0) {
		AddMssg("ERROR: Configuration channel RX not found.");
		return;
	}
	if (resultTX != 0) {
		AddMssg("ERROR: Configuration channel TX not found.");
		return;
	}

	lastTypeSelectionRX = typeIndexRX;
	lastTypeSelectionTX = typeIndexTX;

	configured = true;
	lastSelectionRX[typeIndexRX] = channelIndexRX;
	lastSelectionTX[typeIndexTX] = channelIndexTX;
	cbTXasRX->SetValue(stateChannelIDRX == stateChannelIDTX);
	lastPortRXSelection = state.selPortRX;
	lastPortTXSelection = state.selPortTX;
	cbEnableSWR->SetValue(state.enableSWR == 1);
	rbSWRext->SetValue(state.sourceSWR == 0);
	rbSWRcell->SetValue(state.sourceSWR == 1);

	UpdateRFEForm();

	cAttenuation->SetSelection(state.attValue);
	cbNotch->SetValue(state.notchOnOff);

	activeMode = state.mode;
	setTXRXBtns();

	SetModeLabel();
}

void limeRFE_wxgui::GUI2State(rfe_boardState* state) {
	int channelIDRX = GetChannelID(RFE_CHANNEL_RX);
	int channelIDTX = GetChannelID(RFE_CHANNEL_TX);
	int selPortTXindex = cPortTX->GetSelection();
	int selPortTX = portTXvals[selPortTXindex];
	int selPortRXindex = cPortRX->GetSelection();
	int selPortRX = portRXvals[selPortRXindex];
	int notch = cbNotch->GetValue();
	int attenuation = cAttenuation->GetSelection();
	int enableSWR = (cbEnableSWR->GetValue()) ? 1 : 0;
	int sourceSWR = rbSWRcell->GetValue();

	state->channelIDRX = channelIDRX;
	state->channelIDTX = channelIDTX;
	state->selPortRX = selPortRX;
	state->selPortTX = selPortTX;
	state->mode = activeMode;
	state->notchOnOff = notch;
	state->attValue = attenuation;
	state->enableSWR = enableSWR;
	state->sourceSWR = sourceSWR;
}

void limeRFE_wxgui::setTXRXBtns() {
	switch (activeMode) {
	case RFE_MODE_NONE:
		tbtnTXRX->SetValue(false);
		tbtnRX->SetValue(false);
		tbtnTX->SetValue(false);
		break;
	case RFE_MODE_RX:
		tbtnTXRX->SetValue(false);
		tbtnRX->SetValue(true);
		tbtnTX->SetValue(false);
		break;
	case RFE_MODE_TX:
		tbtnTXRX->SetValue(true);
		tbtnRX->SetValue(false);
		tbtnTX->SetValue(true);
		break;
	case RFE_MODE_TXRX:
		tbtnTXRX->SetValue(true);
		tbtnRX->SetValue(true);
		tbtnTX->SetValue(true);
		break;
	}
}

void limeRFE_wxgui::OnbtnBoard2GUI(wxCommandEvent& event) {
	rfe_boardState state;
	auto* dev = static_cast<RFE_Device*>(rfe);
	int result = Cmd_GetConfig(dev->sdrDevice, dev->com, &state);
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}

	configuredState = state;

	State2GUI(state);
}

void limeRFE_wxgui::OncTypeRX(wxCommandEvent& event) {
	configured = false;
	lastTypeSelectionRX = cTypeRX->GetSelection();
	if(cbTXasRX->GetValue())
		lastTypeSelectionTX = cTypeRX->GetSelection();
	UpdateRFEForm();
}

void limeRFE_wxgui::OncTypeTX(wxCommandEvent& event) {
	configured = false;
	lastTypeSelectionTX = cTypeTX->GetSelection();
	UpdateRFEForm();
}

void limeRFE_wxgui::OnbtnCalibrate(wxCommandEvent& event) {

	int adc1, adc2, result;
	result = RFE_ReadADC(rfe, RFE_ADC1, &adc1);
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}
	result = RFE_ReadADC(rfe, RFE_ADC2, &adc2);
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}

	double pin_dBm, rl_dB;

	CalculatePowerAndGamma(adc1, adc2, &pin_dBm, &rl_dB);

	wxString wxsCalPower = tcPowerCalibrate->GetValue();
	double calpower;

	char msg[200];

	if (!wxsCalPower.ToDouble(&calpower)) { // error!
		AddMssg("Error: Please enter the proper number in Calibrate Power filed.");
		return;
	}

	if (rbSWRcell->GetValue() == 1) { //Cellular
		powerCellCalCorr = calpower - pin_dBm + powerCellCalCorr;
		sprintf(msg, "Power correction = %f dBm", powerCellCalCorr);
		AddMssg(msg);
	} else {
		powerCalCorr = calpower - pin_dBm + powerCalCorr;
		sprintf(msg, "Power correction = %f dBm", powerCalCorr);
		AddMssg(msg);

		wxString wxsCalGamma = tcRLCalibrate->GetValue();
		double calRL;
		if (!wxsCalGamma.ToDouble(&calRL)) { // error!
			AddMssg("Error: Please enter the proper number in Calibrate Gamma filed.");
			return;
		}
		rlCalCorr = calRL - rl_dB + rlCalCorr;
		sprintf(msg, "Gamma correction = %f dB", rlCalCorr);
		AddMssg(msg);
	}
	AddMssg("Calibration successful.");
}

void limeRFE_wxgui::OnrbI2CrbUSB(wxCommandEvent& event) {
	UpdateRFEForm();
}

void limeRFE_wxgui::OnrbSWRext(wxCommandEvent& event) {
	rbSWRext->SetValue(true);
	rbSWRcell->SetValue(false);
	configured = false;
	UpdateRFEForm();
}

void limeRFE_wxgui::OnrbSWRcell(wxCommandEvent& event) {
	rbSWRext->SetValue(false);
	rbSWRcell->SetValue(true);
	configured = false;
	UpdateRFEForm();
}

void limeRFE_wxgui::UpdateRFEForm() {
	SetChannelsTypesTXRX(RFE_CHANNEL_RX);
	SetChannelsTypesTXRX(RFE_CHANNEL_TX);

	int typeRX = cTypeRX->GetSelection();

	if (typeRX == RFE_TYPE_INDEX_CELL) {
		cbTXasRX->SetValue(true);
		cbTXasRX->Enable(false);
	}
	else {
		cbTXasRX->Enable(true);
	}

	if (cbTXasRX->GetValue()) {
		cTypeTX->SetSelection(cTypeRX->GetSelection());
	}
	SetChannelsChoices();
	if (cbTXasRX->GetValue()) {
		cChannelTX->SetSelection(cChannelRX->GetSelection());
	}
	SetConfigurationOptions();
	btnConfigure->Enable(!configured);

	int channelRX = GetChannelID(RFE_CHANNEL_RX);

	pnlTXRXMode->Hide();
	pnlTXRXModeEN->Hide();

	if (configured) {
		tbtnTXRX->Enable(true);
		tbtnRX->Enable(true);
		tbtnTX->Enable(true);
	}

	pnlTXRX->Hide();
	pnlTXRXEN->Show();
	if (configured) pnlTXRXModeEN->Show();
	if ((typeRX == RFE_TYPE_INDEX_CELL) & (channelRX != RFE_CID_CELL_BAND38)) {
		tbtnRX->Enable(false);
		tbtnTX->Enable(false);
	}

	if (!configured) {
		tbtnTXRX->Enable(false);
		tbtnTXRX->SetValue(false);
		tbtnRX->Enable(false);
		tbtnRX->SetValue(false);
		tbtnTX->Enable(false);
		tbtnTX->SetValue(false);
	}

	bool isI2C = (rbI2C->GetValue() == 1);
	cmbbPorts->Enable(!isI2C);
	btnRefreshPorts->Enable(!isI2C);
	//btnOpenPort->Enable(!isI2C);
	//btnClosePort->Enable(!isI2C);

	if (configuredState.sourceSWR == 1) { //Cellular
		pnlADC2->Hide();
	}
	if (configuredState.sourceSWR == 0) { //External
		pnlADC2->Show();
	}

	bool txasrx = cbTXasRX->GetValue();
	cTypeTX->Enable(!txasrx);
	cChannelTX->Enable(!txasrx);

	rbSWRext->Enable(cbEnableSWR->GetValue());
	rbSWRcell->Enable(cbEnableSWR->GetValue());

	if (configuredState.enableSWR == 1)
		pnlPowerMeter->Enable();
	else
		pnlPowerMeter->Disable();

	GetSizer()->Layout();
	this->Fit();
}

void limeRFE_wxgui::SetChannelsChoices() {
	SetChannelsChoicesTXRX(RFE_CHANNEL_RX);
	SetChannelsChoicesTXRX(RFE_CHANNEL_TX);
}

void limeRFE_wxgui::SetChannelsTypesTXRX(int channelTXRX) {
	wxChoice* cTypeTXRX;
	int* lastTypeSelectionTXRX;

	if (channelTXRX == RFE_CHANNEL_RX) {
		cTypeTXRX = cTypeRX;
		lastTypeSelectionTXRX = &lastTypeSelectionRX;
	}
	else { //RFE_CHANNEL_TX
		cTypeTXRX = cTypeTX;
		lastTypeSelectionTXRX = &lastTypeSelectionTX;
	}

	cTypeTXRX->Clear();
	for (int i = 0; i < RFE_TYPE_INDEX_COUNT; i++) {
		if (i == RFE_TYPE_INDEX_WB) cTypeTXRX->AppendString("Wideband");
		if (i == RFE_TYPE_INDEX_HAM) cTypeTXRX->AppendString("HAM");
		if (!((channelTXRX == RFE_CHANNEL_TX) & (cTypeRX->GetSelection() != RFE_TYPE_INDEX_CELL)))
			if (i == RFE_TYPE_INDEX_CELL) cTypeTXRX->AppendString("Cellular");
	}

	if (cTypeTXRX->GetCount() < unsigned(*lastTypeSelectionTXRX + 1))
		*lastTypeSelectionTXRX = 0;

	cTypeTXRX->SetSelection(*lastTypeSelectionTXRX);
}


void limeRFE_wxgui::SetChannelsChoicesTXRX(int channelTXRX) {
	wxChoice* cTypeTXRX;
	wxChoice* cChannelTXRX;
	int* lastSelectionTXRX;

	if (channelTXRX == RFE_CHANNEL_RX) {
		cTypeTXRX = cTypeRX;
		cChannelTXRX = cChannelRX;
		lastSelectionTXRX = lastSelectionRX;
	}
	else { //RFE_CHANNEL_TX
		cTypeTXRX = cTypeTX;
		cChannelTXRX = cChannelTX;
		lastSelectionTXRX = lastSelectionTX;
	}

	int type = cTypeTXRX->GetSelection();

	wxArrayString channelItems;
	FindChannelChoices(type, &channelItems);
	cChannelTXRX->Clear();
	cChannelTXRX->Append(channelItems);
	cChannelTXRX->SetSelection(lastSelectionTXRX[type]);
}

void limeRFE_wxgui::FindChannelChoices(int type, wxArrayString* channelItems) {
	if (type == 0) { // Wideband
		channelItems->Clear();
		for (int i = 0; i < RFE_CHANNEL_INDEX_WB_COUNT; i++) {
			if (i == RFE_CHANNEL_INDEX_WB_1000) channelItems->Add("1 - 1000 MHz");
			if (i == RFE_CHANNEL_INDEX_WB_4000) channelItems->Add("1000 - 4000 MHz");
		}
	}

	if (type == 1) { // HAM
		channelItems->Clear();
		for (int i = 0; i < RFE_CHANNEL_INDEX_HAM_COUNT; i++) {
			if (i == RFE_CHANNEL_INDEX_HAM_0030) channelItems->Add("30 MHz (HF)");
			if (i == RFE_CHANNEL_INDEX_HAM_0070) channelItems->Add("50-70 MHz (6 & 4 m)");
			if (i == RFE_CHANNEL_INDEX_HAM_0145) channelItems->Add("144-146 MHz (2 m)");
			if (i == RFE_CHANNEL_INDEX_HAM_0220) channelItems->Add("220-225 MHz (1.25 m)");
			if (i == RFE_CHANNEL_INDEX_HAM_0435) channelItems->Add("430-440 MHz (70 cm)");
			if (i == RFE_CHANNEL_INDEX_HAM_0920) channelItems->Add("902-928 MHz (33 cm)");
			if (i == RFE_CHANNEL_INDEX_HAM_1280) channelItems->Add("1240-1325 MHz (23 cm)");
			if (i == RFE_CHANNEL_INDEX_HAM_2400) channelItems->Add("2300-2450 MHz (13 cm)");
			if (i == RFE_CHANNEL_INDEX_HAM_3500) channelItems->Add("3300-3500 MHz");
		}
		if (strlen(LMS_GetDeviceInfo(lmsControl)->deviceName))
			channelItems->Add("Auto");
	}

	if (type == 2) { // Cellular
		channelItems->Clear();
		for (int i = 0; i < RFE_CHANNEL_INDEX_CELL_COUNT; i++) {
			if (i == RFE_CHANNEL_INDEX_CELL_BAND01) channelItems->Add("Band 1");
			if (i == RFE_CHANNEL_INDEX_CELL_BAND02) channelItems->Add("Band 2/PCS-1900");
			if (i == RFE_CHANNEL_INDEX_CELL_BAND03) channelItems->Add("Band 3/DCS-1800");
			if (i == RFE_CHANNEL_INDEX_CELL_BAND07) channelItems->Add("Band 7");
			if (i == RFE_CHANNEL_INDEX_CELL_BAND38) channelItems->Add("Band 38");
		}
	}
}

void limeRFE_wxgui::OncChannelRX(wxCommandEvent& event) {
	configured = false;
	int type = cTypeRX->GetSelection();
	lastSelectionRX[type] = cChannelRX->GetSelection();
	UpdateRFEForm();
}

void limeRFE_wxgui::OncChannelTX(wxCommandEvent& event) {
	configured = false;
	int type = cTypeTX->GetSelection();
	lastSelectionTX[type] = cChannelTX->GetSelection();
	UpdateRFEForm();
}

void limeRFE_wxgui::SetConfigurationOptions() {
	int selChannelIDRX = GetChannelID(RFE_CHANNEL_RX);
	int selChannelIDTX = GetChannelID(RFE_CHANNEL_TX);
	int typeTX = cTypeTX->GetSelection();

	if ((selChannelIDRX == RFE_CID_HAM_0030) ||
		(selChannelIDRX == RFE_CID_HAM_0070) ||
		(selChannelIDRX == RFE_CID_HAM_0145) ||
		(selChannelIDRX == RFE_CID_HAM_0220) ||
		(selChannelIDRX == RFE_CID_HAM_0435) ||
		(selChannelIDRX == RFE_CID_WB_1000)) {
		cbNotch->Show();
	}
	else {
		cbNotch->SetValue(RFE_NOTCH_DEFAULT);
		cbNotch->Hide();
	}

	cPortTX->Clear();
	for (int y = 0; y < 10; y++) portTXvals[y] = 0;
	if (typeTX == RFE_TYPE_INDEX_CELL) {
		cPortTX->AppendString(RFE_PORT_1_NAME);
		portTXvals[0] = RFE_PORT_1;
	}
	else if ((selChannelIDTX == RFE_CID_HAM_0030) ||
		     (selChannelIDTX == RFE_CID_HAM_0070)) {
		cPortTX->AppendString(RFE_PORT_3_NAME);
		portTXvals[0] = RFE_PORT_3;
	}
        else if (selChannelIDTX == RFE_CID_AUTO){
		cPortTX->AppendString(RFE_PORT_1_NAME);
		portTXvals[0] = RFE_PORT_1;
		cPortTX->AppendString(RFE_PORT_2_NAME);
		portTXvals[1] = RFE_PORT_2;
                cPortTX->AppendString(RFE_PORT_3_NAME);
		portTXvals[2] = RFE_PORT_3;
	}
	else {
		cPortTX->AppendString(RFE_PORT_1_NAME);
		portTXvals[0] = RFE_PORT_1;
		cPortTX->AppendString(RFE_PORT_2_NAME);
		portTXvals[1] = RFE_PORT_2;
	}
	SelectPort(RFE_CHANNEL_TX);

	cPortRX->Clear();
	for (int y = 0; y < 10; y++) portRXvals[y] = 0;
	if ((selChannelIDRX == RFE_CID_HAM_0030) ||  //HAM Low channels
        (selChannelIDRX == RFE_CID_HAM_0070) ||
	         (selChannelIDRX == RFE_CID_HAM_0145) ||
		(selChannelIDRX == RFE_CID_HAM_0220) ||
		     (selChannelIDRX == RFE_CID_HAM_0435) ||
		     (selChannelIDRX == RFE_CID_WB_1000) ||
                     (selChannelIDRX == RFE_CID_AUTO))
	{
		cPortRX->AppendString(RFE_PORT_1_NAME);
		portRXvals[0] = RFE_PORT_1;
		cPortRX->AppendString(RFE_PORT_3_NAME);
		portRXvals[1] = RFE_PORT_3;
	}
	else {
		cPortRX->AppendString(RFE_PORT_1_NAME);
		portRXvals[0] = RFE_PORT_1;
	}
	SelectPort(RFE_CHANNEL_RX);
}

bool limeRFE_wxgui::SelectPort(int channelTXRX) {
	wxChoice* cPortTXRX;
	int* portTXRXvals;
	int* lastPortTXRXSelection;

	if (channelTXRX == RFE_CHANNEL_RX) {
		cPortTXRX = cPortRX;
		portTXRXvals = portRXvals;
		lastPortTXRXSelection = &lastPortRXSelection;
	}
	else { //RFE_CHANNEL_TX
		cPortTXRX = cPortTX;
		portTXRXvals = portTXvals;
		lastPortTXRXSelection = &lastPortTXSelection;
	}
	bool found = false;
	int index = 0;
	for (int i = 0; i < 10; i++) {
		if (portTXRXvals[i] == *lastPortTXRXSelection) {
			index = i;
			found = true;
			break;
		}
	}
	cPortTXRX->SetSelection(index);

	return found;
}

int limeRFE_wxgui::GetChannelID(int channelTXRX) {
	wxChoice* cTypeTXRX;
	wxChoice* cChannelTXRX;

	if (channelTXRX == RFE_CHANNEL_RX) {
		cTypeTXRX = cTypeRX;
		cChannelTXRX = cChannelRX;
	}
	else { //RFE_CHANNEL_TX
		cTypeTXRX = cTypeTX;
		cChannelTXRX = cChannelTX;
	}

	int type = cTypeTXRX->GetSelection();
	int channel = cChannelTXRX->GetSelection();
	int selChannelID = -1;

	switch (type) {
	case RFE_TYPE_INDEX_WB:
		switch (channel) {
		case RFE_CHANNEL_INDEX_WB_1000: selChannelID = RFE_CID_WB_1000; break;
		case RFE_CHANNEL_INDEX_WB_4000: selChannelID = RFE_CID_WB_4000; break;
		} break;
	case RFE_TYPE_INDEX_HAM:
		switch (channel) {
		case RFE_CHANNEL_INDEX_HAM_0030: selChannelID = RFE_CID_HAM_0030; break;
		case RFE_CHANNEL_INDEX_HAM_0070: selChannelID = RFE_CID_HAM_0070; break;
		case RFE_CHANNEL_INDEX_HAM_0145: selChannelID = RFE_CID_HAM_0145; break;
		case RFE_CHANNEL_INDEX_HAM_0220: selChannelID = RFE_CID_HAM_0220; break;
		case RFE_CHANNEL_INDEX_HAM_0435: selChannelID = RFE_CID_HAM_0435; break;
		case RFE_CHANNEL_INDEX_HAM_0920: selChannelID = RFE_CID_HAM_0920; break;
		case RFE_CHANNEL_INDEX_HAM_1280: selChannelID = RFE_CID_HAM_1280; break;
		case RFE_CHANNEL_INDEX_HAM_2400: selChannelID = RFE_CID_HAM_2400; break;
		case RFE_CHANNEL_INDEX_HAM_3500: selChannelID = RFE_CID_HAM_3500; break;
                default: selChannelID = RFE_CID_AUTO;
		} break;
	case RFE_TYPE_INDEX_CELL:
		switch (channel) {
		case RFE_CHANNEL_INDEX_CELL_BAND01: selChannelID = RFE_CID_CELL_BAND01; break;
		case RFE_CHANNEL_INDEX_CELL_BAND02: selChannelID = RFE_CID_CELL_BAND02; break;
		case RFE_CHANNEL_INDEX_CELL_BAND03: selChannelID = RFE_CID_CELL_BAND03; break;
		case RFE_CHANNEL_INDEX_CELL_BAND07: selChannelID = RFE_CID_CELL_BAND07; break;
		case RFE_CHANNEL_INDEX_CELL_BAND38: selChannelID = RFE_CID_CELL_BAND38; break;
		} break;
	}
	return selChannelID;
}

int limeRFE_wxgui::GetChannelIndexes(int channelID, int* typeIndex, int* channelIndex) {
	int result = 0;
	switch (channelID) {
	case RFE_CID_WB_1000: *typeIndex = RFE_TYPE_INDEX_WB; *channelIndex = RFE_CHANNEL_INDEX_WB_1000; break;
	case RFE_CID_WB_4000: *typeIndex = RFE_TYPE_INDEX_WB; *channelIndex = RFE_CHANNEL_INDEX_WB_4000; break;
	case RFE_CID_HAM_0030: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_0030; break;
	case RFE_CID_HAM_0070: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_0070; break;
	case RFE_CID_HAM_0145: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_0145; break;
	case RFE_CID_HAM_0220: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_0220; break;
	case RFE_CID_HAM_0435: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_0435; break;
	case RFE_CID_HAM_0920: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_0920; break;
	case RFE_CID_HAM_1280: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_1280; break;
	case RFE_CID_HAM_2400: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_2400; break;
	case RFE_CID_HAM_3500: *typeIndex = RFE_TYPE_INDEX_HAM; *channelIndex = RFE_CHANNEL_INDEX_HAM_3500; break;
	case RFE_CID_CELL_BAND01: *typeIndex = RFE_TYPE_INDEX_CELL; *channelIndex = RFE_CHANNEL_INDEX_CELL_BAND01; break;
	case RFE_CID_CELL_BAND02: *typeIndex = RFE_TYPE_INDEX_CELL; *channelIndex = RFE_CHANNEL_INDEX_CELL_BAND02; break;
	case RFE_CID_CELL_BAND03: *typeIndex = RFE_TYPE_INDEX_CELL; *channelIndex = RFE_CHANNEL_INDEX_CELL_BAND03; break;
	case RFE_CID_CELL_BAND07: *typeIndex = RFE_TYPE_INDEX_CELL; *channelIndex = RFE_CHANNEL_INDEX_CELL_BAND07; break;
	case RFE_CID_CELL_BAND38: *typeIndex = RFE_TYPE_INDEX_CELL; *channelIndex = RFE_CHANNEL_INDEX_CELL_BAND38; break;

	default: result = 1;
	}
	return result;
}

void limeRFE_wxgui::OncPortRX(wxCommandEvent& event) {
	configured = false;
	lastPortRXSelection = portRXvals[cPortRX->GetSelection()];
	UpdateRFEForm();
}

void limeRFE_wxgui::OncPortTX(wxCommandEvent& event) {
	configured = false;
	lastPortTXSelection = portTXvals[cPortTX->GetSelection()];
	UpdateRFEForm();
}

void limeRFE_wxgui::OntbtnTXRX(wxCommandEvent& event) {
	int mode = RFE_MODE_RX;
	int tbtnTXRXValue = tbtnTXRX->GetValue();
	if (tbtnTXRXValue == RFE_TXRX_VALUE_RX)
		mode = RFE_MODE_RX;
	else if (tbtnTXRXValue == RFE_TXRX_VALUE_TX)
		mode = RFE_MODE_TX;

	activeMode = mode;

	RFE_Mode(rfe, mode);

	activeMode = mode;

	SetModeLabel();
}

void limeRFE_wxgui::SetModeLabel() {
	wxString modeText;

	switch (activeMode) {
	case RFE_MODE_NONE: modeText = "None"; break;
	case RFE_MODE_RX: modeText = "RX"; break;
	case RFE_MODE_TX: modeText = "TX"; break;
	case RFE_MODE_TXRX: modeText = "TX RX"; break;
	default: modeText = "Unknown";
	}
	txtTXRX->SetLabel(modeText);
	txtTXRXEN->SetLabel(modeText);
}

void limeRFE_wxgui::OnbtnConfigure(wxCommandEvent& event) {
	int type = cTypeRX->GetSelection();

	rfe_boardState newState;
	GUI2State(&newState);

	newState.mode = RFE_MODE_RX;
	if ((type == RFE_TYPE_INDEX_CELL) && (newState.channelIDRX != RFE_CID_CELL_BAND38)) {
		newState.mode = RFE_MODE_TXRX;
	}

	int result = RFE_Configure(rfe, newState.channelIDRX, newState.channelIDTX, newState.selPortRX, newState.selPortTX, newState.mode, newState.notchOnOff, newState.attValue, newState.enableSWR, newState.sourceSWR);
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}

	auto* dev = static_cast<RFE_Device*>(rfe);
	rfe_boardState state = dev->GetState();
	int getChannelResult;
	int rxTypeIndex;
	int rxChannelIndex;
	getChannelResult = GetChannelIndexes(state.channelIDRX, &rxTypeIndex, &rxChannelIndex);
	int txTypeIndex;
	int txChannelIndex;
	getChannelResult = GetChannelIndexes(state.channelIDTX, &txTypeIndex, &txChannelIndex);

	wxArrayString channelItems;
	wxString wxRXTypeDescription = cTypeRX->GetString(rxTypeIndex);
	FindChannelChoices(rxTypeIndex, &channelItems);
	wxString wxRXChannelDescription = channelItems.Item(rxChannelIndex);
//	wxString wxRXChannelDescription = cChannelRX->GetString(rxChannelIndex);
	wxString wxTXTypeDescription = cTypeTX->GetString(txTypeIndex);
	FindChannelChoices(txTypeIndex, &channelItems);
	wxString wxTXChannelDescription = channelItems.Item(txChannelIndex);
//	wxString wxTXChannelDescription = cChannelTX->GetString(txChannelIndex);

	AddMssg(wxString("RX - Type: ") + wxRXTypeDescription + wxString("; Channel: ") + wxRXChannelDescription + wxString(";"));
	AddMssg(wxString("TX - Type: ") + wxTXTypeDescription + wxString("; Channel: ") + wxTXChannelDescription + wxString(";"));

	activeMode = newState.mode;

	setTXRXBtns();

	SetModeLabel();

	configured = true;

	configuredState = newState;

	UpdateRFEForm();
}

void limeRFE_wxgui::PrintError(int errorCode) {
	switch (errorCode) {
	case RFE_ERROR_COMM:
		AddMssg("ERROR: Communication error.");
		break;
	case RFE_ERROR_TX_CONN:
		AddMssg("ERROR: Wrong TX connector.");
		break;
	case RFE_ERROR_RX_CONN:
		AddMssg("Error: Wrong RX connector.");
		break;
	case RFE_ERROR_RXTX_SAME_CONN:
		AddMssg("ERROR: Mode RX & TX not allowed when RX and TX are on the same port.");
		break;
	case RFE_ERROR_CELL_WRONG_MODE:
		AddMssg("ERROR: Wrong mode for cellular channel.");
		break;
	case RFE_ERROR_CELL_TX_NOT_EQUAL_RX:
		AddMssg("ERROR: For cellular channels RX and TX channels must be the same.");
		break;
	case RFE_ERROR_WRONG_CHANNEL_CODE:
		AddMssg("ERROR: Wrong channel code.");
		break;
	case RFE_ERROR_CONF_FILE:
		AddMssg("ERROR: Problem with .ini configuration file.");
		break;
	case RFE_ERROR_GPIO_PIN:
		AddMssg("ERROR: Non-configurable GPIO pin specified. Only pins 4 and 5 are configurable.");
		break;
	case RFE_ERROR_COMM_SYNC:
		AddMssg("ERROR: Communication synchronization error.");
		break;


	default:
		AddMssg("Error: Unknown error.");
	}
}

void limeRFE_wxgui::OncbNotch(wxCommandEvent& event) {
	configured = false;
	UpdateRFEForm();
}

void limeRFE_wxgui::OncAttenuation(wxCommandEvent& event) {
	configured = false;
	UpdateRFEForm();
}

void limeRFE_wxgui::OncbTXasRX(wxCommandEvent& event) {
	configured = false;
	UpdateRFEForm();
}

void limeRFE_wxgui::OnbtnReadADC(wxCommandEvent& event) {
	int adc1, adc2, result;

	result = RFE_ReadADC(rfe, RFE_ADC1, &adc1);
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}
	result = RFE_ReadADC(rfe, RFE_ADC2, &adc2);
	if (result != RFE_SUCCESS){
		PrintError(result);
		return;
	}

	double vADC1 = RFE_ADC_VREF * (adc1 / 1024.0);
	double vADC2 = RFE_ADC_VREF * (adc2 / 1024.0);

	txtADC1->SetLabel(wxString::Format(wxT("%i"), adc1));
	txtADC1_V->SetLabel(wxString::Format(wxT("%.2f"), vADC1));
	txtADC2->SetLabel(wxString::Format(wxT("%i"), adc2));
	txtADC2_V->SetLabel(wxString::Format(wxT("%.2f"), vADC2));

	double pin_dBm, pin_W;

	double gamma, rl_dB, swr;

	CalculatePowerAndGamma(adc1, adc2, &pin_dBm, &rl_dB);

	txtRL_dB->SetLabel(wxString::Format(wxT("%.2f"), rl_dB));

	gamma = pow(10, -rl_dB / 20.0);
	if (gamma < (1.0 - 1E-6)) {
		swr = (1 + gamma) / (1 - gamma);
		txtSWR->SetLabel(wxString::Format(wxT("%.2f"), swr));
	}
	else {
		txtSWR->SetLabel("inf");
	}

	txtPFWD_dBm->SetLabel(wxString::Format(wxT("%.1f"), pin_dBm));

	pin_W = pow(10, (pin_dBm - 30) / 10.0);
	txtPFWD_W->SetLabel(wxString::Format(wxT("%.2f"), pin_W));
}

int limeRFE_wxgui::CalculatePowerAndGamma(int adc1, int adc2, double * pin_dBm, double * rl_dB) {
	double vADC1 = RFE_ADC_VREF * (adc1 / 1024.0);
	double vADC2 = RFE_ADC_VREF * (adc2 / 1024.0);

	double intercept, slope, k, correction, calCorr;

	if (rbSWRcell->GetValue() == 1) { //Cellular
		intercept = -89; // f = 2140 MHz from ADL5513 datasheet
		slope = 21E-3;
		k = 2;
		correction = 25;
		calCorr = powerCellCalCorr;
	}

	if (rbSWRext->GetValue() == 1) { //External
		intercept = -87; // f = 100 MHz from ADL5513 datasheet
		slope = 21E-3;
		k = 2;
		correction = 21;
		calCorr = powerCalCorr;

		*rl_dB = vADC2 / (k*slope) + rlCalCorr;
	}

	*pin_dBm = intercept + vADC1 / (slope * k) + correction + calCorr;

	return 0;
}

void limeRFE_wxgui::OntbtnRXEN(wxCommandEvent& event) {
	int selPortTXindex = cPortTX->GetSelection();
	int selPortTX = portTXvals[selPortTXindex];
	int selPortRXindex = cPortRX->GetSelection();
	int selPortRX = portRXvals[selPortRXindex];

	bool tbtnRXValue = tbtnRX->GetValue();
	bool tbtnTXValue = tbtnTX->GetValue();

	if ((selPortTX == selPortRX) & tbtnRXValue & tbtnTXValue)
		tbtnTX->SetValue(false);

	// If cellular band38 then mode None is not allowed
	int channelIDRTX = GetChannelID(RFE_CHANNEL_TX);
	if (channelIDRTX == RFE_CID_CELL_BAND38) {
		if ((!tbtnRXValue) && (!tbtnTXValue)) {
			tbtnRX->SetValue(true);
			return;
		}
	}

	OntbtnTXRXEN(event);
}
void limeRFE_wxgui::OntbtnTXEN(wxCommandEvent& event) {
	int selPortTXindex = cPortTX->GetSelection();
	int selPortTX = portTXvals[selPortTXindex];
	int selPortRXindex = cPortRX->GetSelection();
	int selPortRX = portRXvals[selPortRXindex];

	bool tbtnRXValue = tbtnRX->GetValue();
	bool tbtnTXValue = tbtnTX->GetValue();

	if ((selPortTX == selPortRX) & tbtnRXValue & tbtnTXValue)
		tbtnRX->SetValue(false);

	// If cellular band38 then mode None is not allowed
	int channelIDRTX = GetChannelID(RFE_CHANNEL_TX);
	if (channelIDRTX == RFE_CID_CELL_BAND38) {
		if ((!tbtnRXValue) && (!tbtnTXValue)) {
			tbtnTX->SetValue(true);
			return;
		}
	}

	OntbtnTXRXEN(event);
}

void limeRFE_wxgui::OntbtnTXRXEN(wxCommandEvent& event) {
	int mode = RFE_MODE_NONE;
	int tbtnRXValue = tbtnRX->GetValue();
	int tbtnTXValue = tbtnTX->GetValue();
	if ((tbtnRXValue == 0) && (tbtnTXValue == 0))
		mode = RFE_MODE_NONE;
	if ((tbtnRXValue == 1) && (tbtnTXValue == 0))
		mode = RFE_MODE_RX;
	if ((tbtnRXValue == 0) && (tbtnTXValue == 1))
		mode = RFE_MODE_TX;
	if ((tbtnRXValue == 1) && (tbtnTXValue == 1))
		mode = RFE_MODE_TXRX;

	int result = RFE_Mode(rfe, mode);
	if (result != RFE_SUCCESS) {
		PrintError(result);
		return;
	}

	activeMode = mode;

	SetModeLabel();
}

int limeRFE_wxgui::GetCommType() {
	int commType;

	if (rbI2C->GetValue() == 1)
		commType = RFE_I2C;
	else
		commType = RFE_USB;

	return commType;
}

void limeRFE_wxgui::OnQuit(wxCommandEvent& event) {
	LMS7SuiteAppFrame* parentFrame;
	parentFrame = (LMS7SuiteAppFrame*)this->GetParent();
	wxCloseEvent closeEvent;
	parentFrame->OnLimeRFEClose(closeEvent);
}

void limeRFE_wxgui::OncbEnableSWR(wxCommandEvent& event) {
	configured = false;
	UpdateRFEForm();
}

void limeRFE_wxgui::OnbtnClearMessages(wxCommandEvent& event) {
	txtMessageField->Clear();
}
