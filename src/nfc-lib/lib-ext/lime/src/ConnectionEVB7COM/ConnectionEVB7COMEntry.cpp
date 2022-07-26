/**
    @file ConnectionEVB7COM.cpp
    @author Lime Microsystems
    @brief Implementation of EVB7 connection of serial COM port.
*/

#include "ConnectionEVB7COM.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace lime;

static const int comBaudrate = 9600;

//! make a static-initialized entry in the registry
void __loadConnectionEVB7COMEntry(void) //TODO fixme replace with LoadLibrary/dlopen
{
static ConnectionEVB7COMEntry EVB7COMEntry;
}

ConnectionEVB7COMEntry::ConnectionEVB7COMEntry(void):
    ConnectionRegistryEntry("EVB7COM")
{
    return;
}

std::vector<ConnectionHandle> ConnectionEVB7COMEntry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> result;
    auto comPortList = this->FindAllComPorts();
    auto availableComms = this->FilterDeviceList(comPortList);
    for (const auto &comName : availableComms)
    {
        ConnectionHandle handle;
        handle.media = "COM";
        handle.name = "EVB7 ("+comName+")";
        handle.addr = comName;
        if(hint.addr.length() == 0 || hint.addr == handle.addr)
            result.push_back(handle);
    }
    return result;
}

IConnection *ConnectionEVB7COMEntry::make(const ConnectionHandle &handle)
{
    return new ConnectionEVB7COM(handle.addr.c_str(), comBaudrate);
}

std::vector<std::string> ConnectionEVB7COMEntry::FindAllComPorts()
{
    std::vector<std::string> comPortList;
    #ifndef __unix__
	HKEY hSERIALCOMM;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),	0, KEY_QUERY_VALUE, &hSERIALCOMM) == ERROR_SUCCESS)
	{
		// Get the max value name and max value lengths
		DWORD dwMaxValueNameLen;
		DWORD dwMaxValueLen;
		DWORD dwQueryInfo = RegQueryInfoKey(hSERIALCOMM, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL);
		if (dwQueryInfo == ERROR_SUCCESS)
		{
			DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1;
			// Include space for the NULL terminator
			DWORD dwMaxValueNameSizeInBytes = dwMaxValueNameSizeInChars*sizeof(TCHAR);
			DWORD dwMaxValueDataSizeInChars = dwMaxValueLen / sizeof(TCHAR) + 1;
			// Include space for the NULL terminator
			DWORD dwMaxValueDataSizeInBytes = dwMaxValueDataSizeInChars*sizeof(TCHAR);

			// Allocate some space for the value name and value data
			TCHAR* szValueName = new TCHAR[dwMaxValueNameSizeInChars];
			TCHAR* byValue = new TCHAR[dwMaxValueDataSizeInBytes];
			if (szValueName && byValue)
			{
				// Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
				DWORD dwIndex = 0;
				DWORD dwType;
				DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
				DWORD dwDataSize = dwMaxValueDataSizeInBytes;
				memset(szValueName, 0, dwMaxValueNameSizeInBytes);
				memset(byValue, 0, dwMaxValueDataSizeInBytes);
				LONG nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, (LPBYTE)byValue, &dwDataSize);
				while (nEnum == ERROR_SUCCESS)
				{
					// If the value is of the correct type, then add it to the array
					if (dwType == REG_SZ)
					{
					    char portname[512];
						TCHAR* szPort = byValue;
						int nUserNameLenUnicode = lstrlen( szPort ); // Convert all UNICODE characters
						int nUserNameLen = WideCharToMultiByte( CP_ACP, // ANSI Code Page
                        0, // No special handling of unmapped chars
                        (LPCWSTR)szPort, // wide-character string to be converted
                        nUserNameLenUnicode,
                        NULL, 0, // No output buffer since we are calculating length
                        NULL, NULL ); // Unrepresented char replacement - Use Default
                        TCHAR* pszUserName = new TCHAR[ nUserNameLen ]; // nUserNameLen includes the NULL character
                        WideCharToMultiByte( CP_ACP, // ANSI Code Page
                        0, // No special handling of unmapped chars
						(LPCWSTR)szPort, // wide-character string to be converted
                        nUserNameLenUnicode,
                        portname,
                        nUserNameLen,
                        NULL, NULL ); // Unrepresented char replacement - Use Default
						portname[nUserNameLen] = 0;
#ifdef UNICODE
                        comPortList.push_back(portname);
#else
                        comPortList.push_back(szPort);
#endif
					}
					// Prepare for the next time around
					dwValueNameSize = dwMaxValueNameSizeInChars;
					dwDataSize = dwMaxValueDataSizeInBytes;
					memset(szValueName, 0, dwMaxValueNameSizeInBytes);
					memset(byValue, 0, dwMaxValueDataSizeInBytes);
					++dwIndex;
					nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName,	&dwValueNameSize, NULL, &dwType, (LPBYTE)byValue, &dwDataSize);
				}
			}
			delete szValueName;
			delete byValue;
		}
		// Close the registry key now that we are finished with it
		RegCloseKey(hSERIALCOMM);

		if (dwQueryInfo != ERROR_SUCCESS)
			SetLastError(dwQueryInfo);
	}
#else
    char tempBuffer[256];
    string result = "";
    if (system( "ls /dev | grep ttyACM > /tmp/foundSerialPorts.txt") == -1)
        return comPortList;

    fstream fin;
    fin.open("/tmp/foundSerialPorts.txt", ios::in);
    while(!fin.eof())
    {
        fin.getline(tempBuffer, 256);
        result = "/dev/";
        result.append(tempBuffer);
        if( strlen(tempBuffer) > 3 ) //longer than tty
            comPortList.push_back(result);
    }
    fin.close();
#endif

    return comPortList;
}

std::vector<std::string> ConnectionEVB7COMEntry::FilterDeviceList(const std::vector<std::string> &comPortList)
{
    std::vector<std::string> deviceNames;

    string comName;
    for(unsigned int i=0; i<comPortList.size(); i++)
    {
        comName = comPortList[i];

        //when not on unix, perform a test open to validate the port
#ifndef __unix__
        ConnectionEVB7COM testConn(comName.c_str(), comBaudrate);
        if (testConn.IsOpen()) deviceNames.push_back(comName);
#else
        deviceNames.push_back(comName);
#endif
    }
    return deviceNames;
}
