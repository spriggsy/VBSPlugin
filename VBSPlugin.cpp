#include <windows.h>
#include "VBSPlugin.h"

#include <tchar.h>
#include "Serial.h"	

#include <thread>
#include <string>
#include "SimpleIni.h"					// used to read and write ini file

#include <iostream>
#include <fstream>
#include <conio.h>
#include <stdio.h>

enum { EOF_Char = 27 };

CSerial serial;
LONG    lLastError = ERROR_SUCCESS;


using namespace std;


bool calib = false;
float X = 0.0;
float Y = 0.0;
float Z = 0.0;

int   Data_Of_Thread_1;

CSimpleIniA ini;

bool connected = false;  //is the 

string pathString;
int B1 = 0;
int B2 = 0;
int B3 = 0;
int B4 = 0;
int B5 = 0;
int Calibrating = 0;     // int sent from head-tracker

int com;
int baud;
int HEADING_OFF = 0;



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
const char *sendY(const char *input)
{
	// The output result
	static char result[256];

	sprintf_s(result, "['%f']", Y);

	// Return whatever is in the result
	return result;
}
const char *sendZ(const char *input)
{
	// The output result
	static char result[256];

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
 


	sprintf_s(result, "['%i,%i,%i']", X, Y, Z);

	// Return whatever is in the result
	return result;
}
const char *sendB1(const char *input)
{
	// The output result
	static char result[512];

	sprintf_s(result, "['%i']", B1);

	// Return whatever is in the result
	return result;
}
const char *sendB2(const char *input)
{
	// The output result
	static char result[512];

	sprintf_s(result, "['%i']", B2);

	// Return whatever is in the result
	return result;
}
const char *sendB3(const char *input)
{
	// The output result
	static char result[512];

	sprintf_s(result, "['%i']", B3);

	// Return whatever is in the result
	return result;
}
const char *sendB4(const char *input)
{
	// The output result
	static char result[512];

	sprintf_s(result, "['%i']", B4);

	// Return whatever is in the result
	return result;
}
const char *sendB5(const char *input)
{
	// The output result
	static char result[512];

	sprintf_s(result, "['%i']", B5);

	// Return whatever is in the result
	return result;
}
const char *CALIBRATE(const char *input)
{
	//int nBytesSent = serial.SendData("1", 1);
	
	// Return whatever is in the result
	return NULL;
}
const char *CONNECT(const char *input)
{

	

	// Attempt to open the serial port (COM1)
	lLastError = serial.Open(_T("COM5"), 0, 0, false);
	if (lLastError != ERROR_SUCCESS)
		cout << serial.GetLastError() << " : " << _T("Unable to open COM 5-port") << endl;
		

	// Setup the serial port (9600,8N1, which is the default setting)
	lLastError = serial.Setup(CSerial::EBaud115200, CSerial::EData8, CSerial::EParNone, CSerial::EStop1);
	if (lLastError != ERROR_SUCCESS)
		cout << serial.GetLastError() << " : " << _T("Unable to set COM-port setting") << endl;

	// Register only for the receive event
	lLastError = serial.SetMask(CSerial::EEventBreak |
		CSerial::EEventCTS |
		CSerial::EEventDSR |
		CSerial::EEventError |
		CSerial::EEventRing |
		CSerial::EEventRLSD |
		CSerial::EEventRecv);
	if (lLastError != ERROR_SUCCESS)
		cout << serial.GetLastError() << " : " << _T("Unable to set COM-port event mask") << endl;

	// Use 'non-blocking' reads, because we don't know how many bytes
	// will be received. This is normally the most convenient mode
	// (and also the default mode for reading data).
	lLastError = serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);
	if (lLastError != ERROR_SUCCESS)
		cout << serial.GetLastError() << " : " << _T("Unable to set COM-port read timeout.") << endl;
	 

	//thread t1(task1, "Hello");
	//t1.join();


	
	return NULL;
}
const char *DISCONNECT(const char *input)
{

	// Close the port again
	serial.Close();
	printf("Close serial port\n");
	return NULL;
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
		cout << "serial is open: "<< endl;

		// Wait for an event
		lLastError = serial.WaitEvent();
		if (lLastError != ERROR_SUCCESS){


			printf("\n### Unable to wait for a COM-port event. %s ###\n", serial.GetLastError());
		}

		// Save event
		const CSerial::EEvent eEvent = serial.GetEventType();

		// Handle break event
		if (eEvent & CSerial::EEventBreak)
		{
			printf("\n### BREAK received ###\n");
		}

		// Handle CTS event
		if (eEvent & CSerial::EEventCTS)
		{
			printf("\n### Clear to send %s ###\n", serial.GetCTS() ? "on" : "off");
		}

		// Handle DSR event
		if (eEvent & CSerial::EEventDSR)
		{
			printf("\n### Data set ready %s ###\n", serial.GetDSR() ? "on" : "off");
		}

		// Handle error event
		if (eEvent & CSerial::EEventError)
		{
			printf("\n### ERROR: ");
			switch (serial.GetError())
			{
			case CSerial::EErrorBreak:		printf("Break condition");			break;
			case CSerial::EErrorFrame:		printf("Framing error");			break;
			case CSerial::EErrorIOE:		printf("IO device error");			break;
			case CSerial::EErrorMode:		printf("Unsupported mode");			break;
			case CSerial::EErrorOverrun:	printf("Buffer overrun");			break;
			case CSerial::EErrorRxOver:		printf("Input buffer overflow");	break;
			case CSerial::EErrorParity:		printf("Input parity error");		break;
			case CSerial::EErrorTxFull:		printf("Output buffer full");		break;
			default:						printf("Unknown");					break;
			}
			printf(" ###\n");
		}

		// Handle ring event
		if (eEvent & CSerial::EEventRing)
		{
			printf("\n### RING ###\n");
		}

		// Handle RLSD/CD event
		if (eEvent & CSerial::EEventRLSD)
		{
			printf("\n### RLSD/CD %s ###\n", serial.GetRLSD() ? "on" : "off");
		}

		// Handle data receive event
		if (eEvent & CSerial::EEventRecv)
		{
			// Read data, until there is nothing left
			DWORD dwBytesRead = 0;
			char szBuffer[101];
			do
			{
				// Read data from the COM-port
				lLastError = serial.Read(szBuffer, sizeof(szBuffer)-1, &dwBytesRead);
				if (lLastError != ERROR_SUCCESS)
				{
			 
				printf("\nUnable to read from COM-port. %s ###\n", serial.GetLastError());

			}

				if (dwBytesRead > 0)
				{
					// Finalize the data, so it is a valid string
					szBuffer[dwBytesRead] = '\0';

					// Display the data
					printf("heres the string: %s", szBuffer);

					
				}
			} while (dwBytesRead == sizeof(szBuffer)-1);
		}
	}





	}
  //!}
 

// This function will be executed every time the script in the engine calls the script function "pluginFunction"
// We can be sure in this function the ExecuteCommand registering was already done.
// Note that the plugin takes responsibility for allocating and deleting the returned string
VBSPLUGIN_EXPORT const char* WINAPI PluginFunction(const char *input)
{
	static const char cmdX[] = "X";
	static const char cmdY[] = "Y";
	static const char cmdZ[] = "Z";
	static const char cmdXYZ[] = "A";
	static const char cmdB1[] = "B1";
	static const char cmdB2[] = "B2";
	static const char cmdB3[] = "B3";
	static const char cmdB4[] = "B4";
	static const char cmdB5[] = "B5";
	
	static const char cmdCONNECT[] = "CONNECT";	//show the GUI window
	static const char cmdDISCONNECT[] = "DISCONNECT";	//show the GUI window
	static const char cmdCALIBRATE[] = "CALIBRATE";	//show the GUI window

	// _strnicmp returns 0 (which is TRUE when using this command) if strings X == Y up to the character length of X, so Toss==Toss, Toss==Tossy, etc.
	if (_strnicmp(input, cmdX, strlen(cmdX)) == 0) return sendX(&input[strlen(cmdX)]);
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

	// Report error; should return in VBS2 as ["{error} Unrecognized command"] -- string in an array
	static const char err[] = "['Unrecognized command Use X,Y,Z,B1,B2,B3,B4,B5,A,CONNECT']";

	return err;
}

// DllMain
BOOL WINAPI DllMain(HINSTANCE hDll, DWORD fdwReason, LPVOID lpvReserved)
{
   switch(fdwReason)
   {
   case DLL_PROCESS_ATTACH:
   {
 
							  AllocConsole();
							  freopen("CONOUT$", "w", stdout);

							  
							 
							

   }
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
