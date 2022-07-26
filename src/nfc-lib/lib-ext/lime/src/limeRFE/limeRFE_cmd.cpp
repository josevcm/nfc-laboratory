#include "limeRFE_constants.h"
#include "INI.h"
#include <chrono>

/*********************************************************************************************
* USB Communication
**********************************************************************************************/

int my_read(RFE_COM com, char* buffer, int count) {
	int result;
#ifdef __unix__
	result = read(com.fd, buffer, count);
#else
	int rc = 0;
	int ret;

	DWORD rc_dw = 0;
	ret = ReadFile(com.hComm, buffer, count, &rc_dw, NULL);
	rc = rc_dw;

	result = (ret == 0)? -1 : rc;

#endif // LINUX
	return result;
}

int my_write(RFE_COM com, char* buffer, int count) {
	int result;
#ifdef __unix__
	result = write(com.fd, buffer, count);
#else
	int rc = 0;
	int ret;

	DWORD rc_dw = 0;
	ret = WriteFile(com.hComm, buffer, count, &rc_dw, NULL);
	rc = rc_dw;

	result = (ret == 0)? -1 : rc;

#endif // LINUX
	return result;
}

int serialport_write(RFE_COM com, const char* str, int len)
{
	char* cstr = (char*)str;
	return my_write(com, cstr, len);
}

int serialport_read(RFE_COM com, char* buff, int len)
{
	int n = my_read(com, buff, len);
	return n;
}


// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serialport_init(const char* serialport, int baud, RFE_COM* com)
{

	char* cserialport = (char*)serialport;

	int result = 0;
#ifdef __unix__

	struct termios toptions;

	int fd = open(cserialport, O_RDWR | O_NOCTTY);
	if (fd == -1)
		return -1;

	com->fd = fd;

	int res;

	res = tcgetattr(com->fd, &toptions);

	if (res < 0) {
		perror("init_serialport: Couldn't get term attributes");
		return -1;
	}

	speed_t brate = baud; // let you override switch below if needed
	switch (baud) {
	case 4800:   brate = B4800;   break;
	case 9600:   brate = B9600;   break;
		// if you want these speeds, uncomment these and set #defines if Linux
		//#ifndef OSNAME_LINUX
		//    case 14400:  brate=B14400;  break;
		//#endif
	case 19200:  brate = B19200;  break;
		//#ifndef OSNAME_LINUX
		//    case 28800:  brate=B28800;  break;
		//#endif
		//case 28800:  brate=B28800;  break;
	case 38400:  brate = B38400;  break;
	case 57600:  brate = B57600;  break;
	case 115200: brate = B115200; break;
	}
	cfsetispeed(&toptions, brate);
	cfsetospeed(&toptions, brate);

	// 8N1
	toptions.c_cflag &= ~PARENB;
	toptions.c_cflag &= ~CSTOPB;
	toptions.c_cflag &= ~CSIZE;
	toptions.c_cflag |= CS8;
	// no flow control
	toptions.c_cflag &= ~CRTSCTS;
	toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
	toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
	toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
	toptions.c_oflag &= ~OPOST; // make raw
								// see: http://unixwiz.net/techtips/termios-vmin-vtime.html
	toptions.c_cc[VMIN] = 0;
	toptions.c_cc[VTIME] = 20;

	res = tcsetattr(com->fd, TCSANOW, &toptions);

	if (res < 0) {
		perror("init_serialport: Couldn't set term attributes");
		return -1;
	}

#else
	HANDLE hComm;
	char* port;

	if (strlen(serialport) < 4) return -1;

	//COMxx/COMxxx
	if (strlen(serialport) > 4) {
		port = (char*)calloc(1, sizeof(char) * strlen("\\\\.\\COM100") + 1);
		strncat(port, "\\\\.\\", strlen("\\\\.\\"));
	}
	//COMx
	else {
		port = (char*)calloc(1, sizeof(char) * 5);
	}
	strncat(port, serialport, strlen(serialport));

#ifdef _UNICODE
	wchar_t wport[20];
	mbstowcs(wport, port, strlen(port) + 1);//Plus null

	hComm = CreateFileW(wport, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
#else
	hComm = CreateFileA(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
#endif
	
	free(port);

	if (hComm == INVALID_HANDLE_VALUE) {
		result = -1;
	}

	DCB dcbSerialParams = { 0 }; // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	//	After that retrieve the current settings of the serial port using the GetCommState() function.
	bool status = GetCommState(hComm, &dcbSerialParams);

	//	and set the values for Baud rate, Byte size, Number of start / Stop bits etc.
	dcbSerialParams.BaudRate = CBR_9600;	// Setting BaudRate = 9600
	dcbSerialParams.ByteSize = 8;			// Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;	// Setting StopBits = 1
	dcbSerialParams.Parity = NOPARITY;      // Setting Parity = None

	dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;  //Disable DTR, because in Windows each time the port opens the Arduino is reset
	SetCommState(hComm, &dcbSerialParams);

	// Set timeouts
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50; // in milliseconds
	timeouts.ReadTotalTimeoutConstant = 50; // in milliseconds
	timeouts.ReadTotalTimeoutMultiplier = 10; // in milliseconds
	timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
	timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds

	if (!SetCommTimeouts(hComm, &timeouts)) {
		return -1;
	}

	com->hComm = hComm;
	com->fd = 0; //Set to a value greater than -1, so the direct USB connection can be checked by if(com.fd >= 0)

#endif // LINUX

	return 0;
}

int serialport_close(RFE_COM com) {
	int result;
#ifdef __unix__
	result = close(com.fd);
#else
	int ret = CloseHandle(com.hComm); // Closing the Serial Port
	result = (ret != 0)? 0 : -1;

#endif // LINUX
	return result;
}

int write_buffer(lms_device_t *dev, RFE_COM com, unsigned char* data, int size) {
	if (com.fd >= 0) {  //prioritize direct connection
		return write_buffer_fd(com, data, size);
	}
	else if (dev != NULL){
		return i2c_write_buffer(dev, data, size);
	}
	return -1; //error: both dev and fd are invalid
}

int write_buffer_fd(RFE_COM com, unsigned char* c, int size)
{
	int actual_length;
	actual_length = serialport_write(com, (char*)c, size);
	if (actual_length != size) {
		return -1;
	}
	return 0;
}

int read_buffer(lms_device_t * dev, RFE_COM com, unsigned char * data, int size)
{
	if (com.fd >= 0) { //prioritize direct connection
		return read_buffer_fd(com, data, size);
	}
	else if(dev != NULL){
		return i2c_read_buffer(dev, data, size);
	}
	return -1; //error: both dev and fd are invalid
}

int read_buffer_fd(RFE_COM com, unsigned char * data, int size)
{
    memset(data, 0, size);
    int received = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    do
    {
        int count = serialport_read(com, (char*)data+received, size - received);
        if (count > 0)
            received += count;
        if (received >= size)
            break;
    }while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count() < 1.0); //timeout
    return received;
}


//******* Command Definitions *******
int Cmd_GetInfo(lms_device_t *dev, RFE_COM com, boardInfo* info) {
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_GET_INFO;
	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len != RFE_BUFFER_SIZE)
		return(RFE_ERROR_COMM);

	info->fw_ver = buf[1];     // FW_VER
	info->hw_ver = buf[2];     // HW_VER
	info->status1 = buf[3];    // Status
	info->status2 = buf[4];    // Status

	return RFE_SUCCESS;
}

int ReadConfig(const char *filename, rfe_boardState *stateBoard, guiState *stateGUI) {
	typedef INI<string, string, string> ini_t;
	ini_t parser(filename, true);

	if (parser.select("LimeRFE_Board_Settings") == false)
		return RFE_ERROR_CONF_FILE;

	stateBoard->channelIDRX = parser.get("channelIDRX", 0);
	stateBoard->channelIDTX = parser.get("channelIDTX", 0);
	stateBoard->selPortRX = parser.get("selPortRX", 0);
	stateBoard->selPortTX = parser.get("selPortTX", 0);
	stateBoard->notchOnOff = parser.get("notchOnOff", 0);
	stateBoard->mode = parser.get("mode", 0);
	stateBoard->attValue = parser.get("attValue", 0);
	stateBoard->enableSWR = parser.get("enableSWR", 0);
	stateBoard->sourceSWR = parser.get("sourceSWR", 0);

	if (parser.select("LimeRFE_GUI_Settings")) {
		stateGUI->powerCellCorr = parser.get("CellularPowerCorrection", 0);
		stateGUI->powerCorr = parser.get("PowerCorrection", 0);
		stateGUI->rlCorr = parser.get("GammaCorrection", 0);
	}

	return RFE_SUCCESS;
}

int SaveConfig(const char *filename, rfe_boardState state, guiState stateGUI) {
	FILE *fout;
	fout = fopen(filename, "w");

	if (fout == NULL) {
		fclose(fout);
		return 1;
	}

	fprintf(fout, "[LimeRFE_Board_Settings]\n");

	fprintf(fout, "channelIDRX=%d\n", state.channelIDRX);
	fprintf(fout, "channelIDTX=%d\n", state.channelIDTX);
	fprintf(fout, "selPortRX=%d\n", state.selPortRX);
	fprintf(fout, "selPortTX=%d\n", state.selPortTX);
	fprintf(fout, "mode=%d\n", state.mode);
	fprintf(fout, "notchOnOff=%d\n", state.notchOnOff);
	fprintf(fout, "attValue=%d\n", state.attValue);
	fprintf(fout, "enableSWR=%d\n", state.enableSWR);
	fprintf(fout, "sourceSWR=%d\n", state.sourceSWR);

	fprintf(fout, "[LimeRFE_GUI_Settings]\n");

	fprintf(fout, "CellularPowerCorrection=%f\n", stateGUI.powerCellCorr);
	fprintf(fout, "PowerCorrection=%f\n", stateGUI.powerCorr);
	fprintf(fout, "GammaCorrection=%f\n", stateGUI.rlCorr);

	fclose(fout);
	return 0;
}

int Cmd_GetConfig(lms_device_t *dev, RFE_COM com, rfe_boardState *state) {
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_GET_CONFIG;
	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	state->channelIDRX = buf[1];
	state->channelIDTX = buf[2];

	state->selPortRX = buf[3];
	state->selPortTX = buf[4];

	state->mode = buf[5];

	state->notchOnOff = buf[6];
	state->attValue = buf[7];

	state->enableSWR = buf[8];
	state->sourceSWR = buf[9];

	return 0;
}

void mySleep(int sleepms)
{
#ifdef __unix__
	usleep(sleepms * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef _WIN32
	Sleep(sleepms);
#endif
}

int Cmd_Hello(RFE_COM com) {
	int result = 0;
	unsigned char buf[1];
	int len;

	buf[0] = RFE_CMD_HELLO;

	int attempts = 0;
	bool connected = false;

	while (!connected && (attempts < RFE_MAX_HELLO_ATTEMPTS)) {
		write_buffer_fd(com, buf, 1);
		mySleep(RFE_TIME_BETWEEN_HELLO_MS);
#ifdef __unix__
		len = read_buffer_fd(com, buf, 1);
#else
		DWORD dwlen;
		ReadFile(com.hComm, buf, 1, &dwlen, NULL);
		len = dwlen;
#endif
		if ((len == 1) && (buf[0] == RFE_CMD_HELLO))
			connected = true;
		attempts++;
	}

	result = (connected) ? 0 : RFE_ERROR_COMM;
	return result;
}

int Cmd_LoadConfig(lms_device_t *dev, RFE_COM com, const char *filename) {
	int result = 0;
	rfe_boardState state;
	guiState stateGUI;
	result = ReadConfig(filename, &state, &stateGUI);
	if (result != 0)
		return result;

	result = Cmd_Configure(dev, com, state.channelIDRX, state.channelIDTX, state.selPortRX, state.selPortTX, state.mode, state.notchOnOff, state.attValue, state.enableSWR, state.sourceSWR);

	return result;
}

int Cmd_Reset(lms_device_t *dev, RFE_COM com) {
	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_RESET;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_ConfigureState(lms_device_t* dev, RFE_COM com, rfe_boardState state)
{
    return Cmd_Configure(dev, com, state.channelIDRX, state.channelIDTX, state.selPortRX, state.selPortTX, state.mode, state.notchOnOff, state.attValue, state.enableSWR, state.sourceSWR);
}

int Cmd_Configure(lms_device_t *dev, RFE_COM com, int channelIDRX, int channelIDTX, int selPortRX, int selPortTX, int mode, int notch, int attenuation, int enableSWR, int sourceSWR) {

	int result = 0;

	if (channelIDTX == -1)
		channelIDTX = channelIDRX;

	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_CONFIG;

	buf[1] = channelIDRX;
	buf[2] = channelIDTX;

	buf[3] = selPortRX;
	buf[4] = selPortTX;

	buf[5] = mode;

	buf[6] = notch;

	buf[7] = attenuation;

	buf[8] = enableSWR;
	buf[9] = sourceSWR;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	result = buf[1]; // buf[0] is the command, buf[1] is the result
	return result;
}

int Cmd_Mode(lms_device_t *dev, RFE_COM com, int mode) {
	int result = 0;

	unsigned char buf[RFE_BUFFER_SIZE_MODE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE_MODE);

	buf[0] = RFE_CMD_MODE;

	buf[1] = mode;

	if(write_buffer(dev, com, buf, RFE_BUFFER_SIZE_MODE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE_MODE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	result = buf[1]; // buf[0] is the command, buf[1] is the result
	return result;
}

int Cmd_ReadADC(lms_device_t *dev, RFE_COM com, int adcID, int* value) {
	int result = RFE_SUCCESS;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	if (adcID == RFE_ADC1)
		buf[0] = RFE_CMD_READ_ADC1;
	else
		buf[0] = RFE_CMD_READ_ADC2;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1) {
		*value = 0;
		return(RFE_ERROR_COMM);
	}

	*value = buf[2] * pow(2, 8) + buf[1];

	return result;
}

int Cmd_Cmd(lms_device_t *dev, RFE_COM com, unsigned char* buf) {
	int result = 0;
	int len;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_ConfGPIO(lms_device_t *dev, RFE_COM com, int gpioNum, int direction) {
	if ((gpioNum != RFE_GPIO4) & (gpioNum != RFE_GPIO5))
		return RFE_ERROR_GPIO_PIN;

	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_CONFGPIO45;
	buf[1] = gpioNum;
	buf[2] = direction;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_SetGPIO(lms_device_t *dev, RFE_COM com, int gpioNum, int val) {
	if ((gpioNum != RFE_GPIO4) & (gpioNum != RFE_GPIO5))
		return RFE_ERROR_GPIO_PIN;

	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_SETGPIO45;
	buf[1] = gpioNum;
	buf[2] = val;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_GetGPIO(lms_device_t *dev, RFE_COM com, int gpioNum, int * val) {
	if ((gpioNum != RFE_GPIO4) & (gpioNum != RFE_GPIO5))
		return RFE_ERROR_GPIO_PIN;

	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_GETGPIO45;
	buf[1] = gpioNum;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	*val = buf[1];

	return result;
}

/******************************************************************************
* I2C Communications
*******************************************************************************/

void mySleep(double sleepms)
{
#ifdef __unix__
	usleep(sleepms * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef _WIN32
	Sleep(sleepms);
#endif
}

void i2c_dly(void)
{
	double sleepms = 0.5 * 1 / RFE_I2C_FSCL * 1000; //for example 0.5 * period - Check this!!!
	mySleep(sleepms);
}

int i2c_setVal(lms_device_t* lms, int bitGPIO, int value) {
	uint8_t gpio_dir = 0;
	int direction;
	if (LMS_GPIODirRead(lms, &gpio_dir, 1) != 0)
		return -1;
	// Direction: 1-output, 0-input
	// If writting 0, then output => direction = 1
	// If writting 1, then input (high Z, and pull-up will do the trick) => direction = 0
	(value == 1) ? direction = 0 : direction = 1;

	(direction == 1) ? gpio_dir |= (1 << (bitGPIO)) : gpio_dir &= ~(1 << (bitGPIO));

	if (LMS_GPIODirWrite(lms, &gpio_dir, 1) != 0)
		return -1;

	uint8_t gpio_val = 0;
	if (LMS_GPIORead(lms, &gpio_val, 1) != 0)
		return -1;

	(value == 1) ? gpio_val |= (1 << (bitGPIO)) : gpio_val &= ~(1 << (bitGPIO));

	if (LMS_GPIOWrite(lms, &gpio_val, 1) != 0)
		return -1;

	i2c_dly();

	return 0;
}

int i2c_getVal(lms_device_t* lms, int bitGPIO) {
	uint8_t gpio_val = 0;
	if (LMS_GPIORead(lms, &gpio_val, 1) != 0)
		return -1;

	int result = 0;

	result = gpio_val & (1 << (bitGPIO));

	return result;
}

int i2c_start(lms_device_t* lms)
{
	if(i2c_setVal(lms, GPIO_SDA, 1) != 0)
		return -1;
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_setVal(lms, GPIO_SDA, 0);
	i2c_setVal(lms, GPIO_SCL, 0);
	return 0;
}

int i2c_stop(lms_device_t* lms)
{
	if(i2c_setVal(lms, GPIO_SDA, 0) != 0)
		return -1;
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_setVal(lms, GPIO_SDA, 1);
	return 0;
}

int i2c_rx(lms_device_t* lms, char ack, unsigned char* d)
{
	char x = 0;

	i2c_setVal(lms, GPIO_SDA, 1);
	for (x = 0; x<8; x++) {
		(*d) <<= 1;

		int attempt = 0;
		do {
			i2c_setVal(lms, GPIO_SCL, 1);
			attempt++;
			if (attempt > 100) { // Is this limit OK?
				return RFE_ERROR_COMM;
			}
		} while (i2c_getVal(lms, GPIO_SCL) == 0);    // wait for any SCL clock stretching
		i2c_dly();
		if (i2c_getVal(lms, GPIO_SDA)) (*d) |= 1;
		i2c_setVal(lms, GPIO_SCL, 0);
	}
	if (ack) i2c_setVal(lms, GPIO_SDA, 0);
	else i2c_setVal(lms, GPIO_SDA, 1);
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_dly();             // send (N)ACK bit
	i2c_setVal(lms, GPIO_SCL, 0);
	i2c_setVal(lms, GPIO_SDA, 1);
//	return d;
	return RFE_SUCCESS;
}

int i2c_tx(lms_device_t* lms, unsigned char d)
{
	char x;
	int b;

	for (x = 8; x; x--) {
		if (d & 0x80)
			i2c_setVal(lms, GPIO_SDA, 1);
		else
			i2c_setVal(lms, GPIO_SDA, 0);
		i2c_setVal(lms, GPIO_SCL, 1);
		d <<= 1;
		i2c_setVal(lms, GPIO_SCL, 0);
	}
	i2c_setVal(lms, GPIO_SDA, 1);
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_dly();
	b = i2c_getVal(lms, GPIO_SDA);          // possible ACK bit
	i2c_setVal(lms, GPIO_SCL, 0);
	return b;
}

int i2c_write_buffer(lms_device_t* lms, unsigned char* c, int size) {
	unsigned char addressI2C = RFE_I2C_ADDRESS;
	unsigned char addressByte = addressI2C << 1;
	unsigned char addressByteW = addressByte & ~1;

	if(i2c_start(lms) != 0) // send start sequence
		return -1;
	i2c_tx(lms, addressByteW);	// I2C address with R/W bit clear

	for (int i = 0; i < size; i++) {
		i2c_tx(lms, c[i]);
	}

	i2c_stop(lms);	// send stop sequence

	return 0;
}

int i2c_read_buffer(lms_device_t* lms, unsigned char* c, int size) {
	unsigned char addressI2C = RFE_I2C_ADDRESS;
	unsigned char addressByte = addressI2C << 1;
	unsigned char addressByteR = addressByte | 1;

	i2c_start(lms);	// send a restart sequence
	i2c_tx(lms, addressByteR);	// I2C address with R/W bit set

	int i;
	for (i = 0; i < size; i++) {
		char ack = 1;
		if (i == (size - 1))
			ack = 0;
		int res = i2c_rx(lms, ack, &(c[i]));
		if (res != RFE_SUCCESS)
			return RFE_ERROR_COMM;
	}
	i2c_stop(lms);	// send stop sequence
	return i;
}

int Cmd_Fan(lms_device_t *dev, RFE_COM com, int enable) {
	int result = 0;

	unsigned char buf[RFE_BUFFER_SIZE_MODE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE_MODE);

	buf[0] = RFE_CMD_FAN;

	buf[1] = enable;

	if (write_buffer(dev, com, buf, RFE_BUFFER_SIZE_MODE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, com, buf, RFE_BUFFER_SIZE_MODE);
	if (len == -1)
		return(RFE_ERROR_COMM);

//	result = buf[1]; // buf[0] is the command, buf[1] is the result
	return result;
}
