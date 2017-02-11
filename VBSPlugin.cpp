#include <windows.h>
#include "VBSPlugin.h"

#include "Serial.h"	

#include <thread>
#include <string>
#include "SimpleIni.h"					// used to read and write ini file


#include <iostream>
#include <fstream>
#include <conio.h>
#include <stdio.h>


using namespace std;


bool calib = false;
float X = 0.0;
float Y = 0.0;
float Z = 0.0;

int   Data_Of_Thread_1;

CSimpleIniA ini;
CSerial test;




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


DWORD WINAPI XYZ(LPVOID lpParam)
{

	bool synced = false;

	string dataString;

	char* buffer = new char[1]();



	while (true){

		if (test.IsOpened()){

			test.ReadData(buffer, 1);

			if (_stricmp("<", buffer) == 0)
			{
				synced = true;
				dataString.clear();
				
				


			};

			if (((_stricmp("\r", buffer) == 0) || (_stricmp("\n", buffer) == 0) || (_stricmp("<", buffer) == 0) || (_stricmp(">", buffer) == 0) || (_stricmp(",", buffer) == 0) || (_stricmp(".", buffer) == 0) || (_stricmp("-", buffer) == 0) || (_stricmp("1", buffer) == 0) || (_stricmp("2", buffer) == 0) || (_stricmp("3", buffer) == 0) || (_stricmp("4", buffer) == 0) || (_stricmp("5", buffer) == 0) || (_stricmp("6", buffer) == 0) || (_stricmp("7", buffer) == 0) || (_stricmp("8", buffer) == 0) || (_stricmp("9", buffer) == 0) || (_stricmp("0", buffer) == 0)) && synced == true)
			{
				dataString += buffer;

				if ((_stricmp(">", buffer) == 0))
				{
					// end of packet
					synced = false;
					cout << "dataString: " << dataString << std::endl;

					dataString.clear();
				};
				
			}
 






		}

		else{

			cout << "Not Connected" << endl;
			dataString.clear();
		}

	}

	return 0;
}

 


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


	if (test.Open(5, 115200))
	{
		connected = true;
		

	}

	return NULL;
}
const char *DISCONNECT(const char *input)
{
	
	if (test.Close()){
		connected = false;
	}
	// Return whatever is in the result
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
  //{ Sample code:
  //!}
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
							  HANDLE   Hthread1 = CreateThread(NULL, 0, XYZ, &Data_Of_Thread_1, 0, NULL);

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
