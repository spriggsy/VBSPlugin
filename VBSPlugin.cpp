#include <windows.h>
#include "StdAfx.h"
#include "VBSPlugin.h"
#include <tchar.h>
#include "Serial.h"						//Used fo the Serial port
#include <vector>
#include <algorithm>					//used for string replace function
#include <string>
#include "whereami.h"					// find path of DLL
#include "SimpleIni.h"					// used to read and write ini file
#include <sstream>
#include <iostream>						// needed for file access/creation
#include <fstream>
 
CSerial serial;
CSimpleIniA ini;

LONG    lLastError = ERROR_SUCCESS;

using namespace std;

string dataString;
string pathString;
bool synced;
bool calib = false;

float X = 0.0;
float Y = 0.0;
float Z = 0.0;
float bty = 100.0;
 
bool connected = false;  //is the 

int AX = 0;
int AY = 0;
int B1 = 0;
int B2 = 0;
int B3 = 0;
int B4 = 0;
int B5 = 0;
int Calibrating = 0;     // int sent from head-tracker

int HEADING_OFF = 0;
string com;
int baud;



//Function to split string at deliminator
void split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}


// Command function declaration
typedef int (WINAPI * ExecuteCommandType)(const char *command, char *result, int resultLength);

// Command function definition
ExecuteCommandType ExecuteCommand = NULL;

const char *sendX(const char *input)
{
	// The output result
	static char result[128];

	X = X + HEADING_OFF;
	if (X > 360)  { X = X - 360; };

	// The output result

	sprintf_s(result, "['%f']", X);

	// Return whatever is in the result
	return result;
};

const char *sendAX(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%i']", AX);

	// Return whatever is in the result
	return result;
};
const char *sendAY(const char *input)
{
	// The output result
	static char result[128];


	// The output result

	sprintf_s(result, "['%i']", AY);

	// Return whatever is in the result
	return result;
};
const char *sendY(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%f']", Y);

	// Return whatever is in the result
	return result;
}
const char *sendZ(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%f']", Z);

	// Return whatever is in the result
	return result;
}
const char *sendA(const char *input)
{
	// The output result
	static char result[128];

	X = X + HEADING_OFF;
	if (X > 360)  { X = X - 360; };

	sprintf_s(result, "['%f,%f,%f']", X, Y, Z);

	// Return whatever is in the result
	return result;
}
const char *sendB1(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%i']", B1);

	// Return whatever is in the result
	return result;
}
const char *sendB2(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%i']", B2);

	// Return whatever is in the result
	return result;
}
const char *sendB3(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%i']", B3);

	// Return whatever is in the result
	return result;
}
const char *sendB4(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%i']", B4);

	// Return whatever is in the result
	return result;
}
const char *sendB5(const char *input)
{
	// The output result
	static char result[128];

	sprintf_s(result, "['%i']", B5);

	// Return whatever is in the result
	return result;
}


void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}


void checkIni()
{
	//find path of DLL
	char* path;
	int length, dirname_length;
	length = wai_getModulePath(NULL, 0, &dirname_length);
	if (length > 0)
	{
		path = (char*)malloc(length + 1);
		if (!path)
			abort();
		wai_getModulePath(path, length, &dirname_length);
		path[length] = '\0';
		string str(path);
		pathString = str + ".ini";
		replaceAll(pathString, "\\", "\\\\");

	}


	// if the ini file does not exist, create it
	if (!ifstream(pathString))
	{

		ofstream outfile(pathString.c_str());

		outfile << "[SETTINGS]" << endl;
		outfile << "PORT = COM3" << endl;
		outfile << "BAUD = 115200" << endl;
		outfile << "HEADING_OFF = 0" << endl;
		outfile << "GPS = 0" << endl;
		outfile << "JPAD = 0" << endl;
		outfile.close();
	}

	free(path);

	// load settings from ini file
	ini.LoadFile(pathString.c_str());

	com = ini.GetValue("SETTINGS", "PORT", NULL);
	baud = atoi(ini.GetValue("SETTINGS", "BAUD", NULL));
	HEADING_OFF = atoi(ini.GetValue("SETTINGS", "HEADING_OFF", NULL));
};
const char *CALIBRATE(const char *input)
{
	int nBytesSent = serial.Write("1");  
	cout << "CALIBRATE" << endl;
	// Return whatever is in the result
	return NULL;
}

const char *CONNECT(const char *input)
{
	static char result[128];
	checkIni();

	// Attempt to open the serial port 
	// Add "\\\\.\\" to the beginning of the read com port from the ini file,
	//	this allows the plugin to use com ports greater than 9
	//string preTxt = "\\\\.\\";
	//string port = preTxt + com;
	lLastError = serial.Open((com.c_str()), 0, 0, false);
	//lLastError = serial.Open((preTxt.c_str()), 0, 0, false);
	if (lLastError != ERROR_SUCCESS)
	{
		sprintf_s(result, "['Unable to open port %s']",com.c_str());
		// Return whatever is in the result
		return result;
	};
	// Setup the serial port (9600,8N1, which is the default setting)
	lLastError = serial.Setup(CSerial::EBaudrate(baud), CSerial::EData8, CSerial::EParNone, CSerial::EStop1);
	if (lLastError != ERROR_SUCCESS)
	{
		sprintf_s(result, "['Unable to set Baud rate']");

		// Return whatever is in the result
		return result;
	};

	// Register only for the receive event
	lLastError = serial.SetMask(CSerial::EEventBreak |
		CSerial::EEventCTS |
		CSerial::EEventDSR |
		CSerial::EEventError |
		CSerial::EEventRing |
		CSerial::EEventRLSD |
		CSerial::EEventRecv);
	if (lLastError != ERROR_SUCCESS)
		{

			sprintf_s(result, "['Unable to set COM-port event mask']");

			// Return whatever is in the result
			return result;
		};

	// Use 'non-blocking' reads, because we don't know how many bytes
	// will be received. This is normally the most convenient mode
	// (and also the default mode for reading data).
	lLastError = serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);
	if (lLastError != ERROR_SUCCESS)
	{
		sprintf_s(result, "['Unable to set COM-port read timeout.']");

		// Return whatever is in the result
		return result;
	};


	//check if we managed to connect
	if (serial.IsOpen() == true)
	{
		sprintf_s(result, "['true']");
		// Return whatever is in the result
		return result;
		
	}
	else
	{
		sprintf_s(result, "['false']");
		// Return whatever is in the result
		return result;
	}
	return NULL;
}
const char *DISCONNECT(const char *input)
{
	static char result[128];
	// Close the port again
	serial.Close();
	
	if (serial.IsOpen() == true)
	{
		sprintf_s(result, "['false']");
		// Return whatever is in the result
		return result;

	}
	else
	{
		sprintf_s(result, "['true']");
		// Return whatever is in the result
		return result;
	}
	return NULL;

 
}

const char *sendCHK(const char *input)
{
	// The output result
	static char result[128];

	if (serial.IsOpen() == true)
	{
		sprintf_s(result, "['true']");
		// Return whatever is in the result
		return result;

	}
	else
	{
		sprintf_s(result, "['false']");
		// Return whatever is in the result
		return result;
	}
	return NULL;

	// Return whatever is in the result
	return result;
}


const char *sendBTY(const char *input)
{
	// The output result
	static char result[56];

	sprintf_s(result, "['%f']", bty);

	// Return whatever is in the result
	return result;
}


// Function that will register the ExecuteCommand function of the engine
VBSPLUGIN_EXPORT void WINAPI RegisterCommandFnc(void *executeCommandFnc)
{
	ExecuteCommand = (ExecuteCommandType)executeCommandFnc;
}

// This function will be executed every simulation step (every frame) and took a part in the simulation procedure.
// We can be sure in this function the ExecuteCommand registering was already done.
// deltaT is time in seconds since the last simulation step
VBSPLUGIN_EXPORT void WINAPI OnSimulationStep(float deltaT)
{
	//try reading com port here

	if (serial.IsOpen())
	{
		// Wait for an event
		lLastError = serial.WaitEvent();
		if (lLastError != ERROR_SUCCESS){

		}
		// Save event
		const CSerial::EEvent eEvent = serial.GetEventType();

		// Handle data receive event
		if (eEvent & CSerial::EEventRecv)
		{
			// Read data, until there is nothing left
			DWORD dwBytesRead = 0;
			char szBuffer[2];
			do
			{
				// Read data from the COM-port
				lLastError = serial.Read(szBuffer, sizeof(szBuffer)-1, &dwBytesRead);
				if (lLastError != ERROR_SUCCESS)
				{
 				}

				if (dwBytesRead > 0)
				{
					// Finalize the data, so it is a valid string
					szBuffer[dwBytesRead] = '\0';

					if (_stricmp("<", szBuffer) == 0)

					{
						synced = true;
					};

					if ((_stricmp("<", szBuffer) == 0) || (_stricmp(">", szBuffer) == 0) || (_stricmp(",", szBuffer) == 0) || (_stricmp(".", szBuffer) == 0) || (_stricmp("-", szBuffer) == 0) || (_stricmp("1", szBuffer) == 0) || (_stricmp("2", szBuffer) == 0) || (_stricmp("3", szBuffer) == 0) || (_stricmp("4", szBuffer) == 0) || (_stricmp("5", szBuffer) == 0) || (_stricmp("6", szBuffer) == 0) || (_stricmp("7", szBuffer) == 0) || (_stricmp("8", szBuffer) == 0) || (_stricmp("9", szBuffer) == 0) || (_stricmp("0", szBuffer) == 0) && (synced == true))
					{
						//add ">" to dataString 
						dataString += szBuffer;
						if ((_stricmp(">", szBuffer) == 0))
						{
							// end of packet 
							synced = false;

							vector<string> x = split(dataString, ',');
							if (x.size() > 12)
							{
								try
								{
									X = stof(x[1]);
									Y = stof(x[2]);
									Z = stof(x[3]);
									AX = stoi(x[4]);
									AY = stoi(x[5]);
									B1 = stoi(x[6]);
									B2 = stoi(x[7]);
									B3 = stoi(x[8]);
									B4 = stoi(x[9]);
									B5 = stoi(x[10]);
									bty = stof(x[11]);
									Calibrating = stoi(x[12]);
								}
								catch (...) {
								};
							}
							dataString.clear();
						};
					}
				}
			} while (dwBytesRead == sizeof(szBuffer)-1);
		}
	}
}
 


// This function will be executed every time the script in the engine calls the script function "pluginFunction"
// We can be sure in this function the ExecuteCommand registering was already done.
// Note that the plugin takes responsibility for allocating and deleting the returned string
VBSPLUGIN_EXPORT const char* WINAPI PluginFunction(const char *input)
{
	static const char cmdX[] = "X";
	static const char cmdY[] = "Y";
	static const char cmdZ[] = "Z";
	static const char cmdXYZ[] = "A";
	static const char cmdAX[] = "AX";
	static const char cmdAY[] = "AY";
	static const char cmdB1[] = "B1";
	static const char cmdB2[] = "B2";
	static const char cmdB3[] = "B3";
	static const char cmdB4[] = "B4";
	static const char cmdB5[] = "B5";
	static const char cmdCONNECT[] = "CONNECT";	 
	static const char cmdDISCONNECT[] = "DISCONNECT";	 
	static const char cmdCALIBRATE[] = "CALIBRATE";	
	static const char cmdCHK[] = "CHK";
	static const char cmdBTY[] = "BTY";


	// _strnicmp returns 0 (which is TRUE when using this command) if strings X == Y up to the character length of X, so Toss==Toss, Toss==Tossy, etc.
	if (_strnicmp(input, cmdX, strlen(cmdX)) == 0) return sendX(&input[strlen(cmdX)]);
	if (_strnicmp(input, cmdAX, strlen(cmdAX)) == 0) return sendAX(&input[strlen(cmdAX)]);
	if (_strnicmp(input, cmdAY, strlen(cmdAY)) == 0) return sendAY(&input[strlen(cmdAY)]);
	if (_strnicmp(input, cmdY, strlen(cmdY)) == 0) return sendY(&input[strlen(cmdY)]);
	if (_strnicmp(input, cmdZ, strlen(cmdZ)) == 0) return sendZ(&input[strlen(cmdZ)]);
	if (_strnicmp(input, cmdXYZ, strlen(cmdXYZ)) == 0) return sendA(&input[strlen(cmdXYZ)]);
	if (_strnicmp(input, cmdB1, strlen(cmdB1)) == 0) return sendB1(&input[strlen(cmdB1)]);
	if (_strnicmp(input, cmdB2, strlen(cmdB2)) == 0) return sendB2(&input[strlen(cmdB2)]);
	if (_strnicmp(input, cmdB3, strlen(cmdB3)) == 0) return sendB3(&input[strlen(cmdB3)]);
	if (_strnicmp(input, cmdB4, strlen(cmdB4)) == 0) return sendB4(&input[strlen(cmdB4)]);
	if (_strnicmp(input, cmdB5, strlen(cmdB5)) == 0) return sendB5(&input[strlen(cmdB5)]);
	if (_strnicmp(input, cmdCONNECT, strlen(cmdCONNECT)) == 0) return CONNECT(&input[strlen(cmdCONNECT)]);
	if (_strnicmp(input, cmdDISCONNECT, strlen(cmdDISCONNECT)) == 0) return DISCONNECT(&input[strlen(cmdDISCONNECT)]);
	if (_strnicmp(input, cmdCALIBRATE, strlen(cmdCALIBRATE)) == 0) return CALIBRATE(&input[strlen(cmdCALIBRATE)]);
	if (_strnicmp(input, cmdCHK, strlen(cmdCHK)) == 0) return sendCHK(&input[strlen(cmdCHK)]);

	if (_strnicmp(input, cmdBTY, strlen(cmdBTY)) == 0) return sendBTY(&input[strlen(cmdBTY)]);

	// Report error; should return in VBS2 as ["{error} Unrecognized command"] -- string in an array
	static const char err[] = "['Unrecognized command Use X,Y,Z,B1,B2,B3,B4,B5,A,CONNECT,DISCONNECT,BTY']";

	return err;
}

// DllMain
BOOL WINAPI DllMain(HINSTANCE hDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		OutputDebugString("Called DllMain with DLL_PROCESS_ATTACH\n");
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugString("Called DllMain with DLL_PROCESS_DETACH\n");
		break;
	case DLL_THREAD_ATTACH:
		OutputDebugString("Called DllMain with DLL_THREAD_ATTACH\n");
		break;
	case DLL_THREAD_DETACH:
		OutputDebugString("Called DllMain with DLL_THREAD_DETACH\n");
		break;
	}
	return TRUE;
}
