//#define CORSAIR_LIGHTING_SDK_DISABLE_DEPRECATION_WARNINGS

#include <iostream>
#include<string>
#include <atomic>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "CUESDK.h"
#include "Serial.h"
#include "iCUE.h"

Serial* arduino;
CorsairLedColor leds[20];
//CorsairLedColor caseLeds[1];
//CorsairLedColor fanTemp[1];
int index = -1;
//int caseIndex = -1;
//int fanIndex = -1;
//char fanSpeed = 'd';

const char* ToString(CorsairError error)
{
	switch (error)
	{
		case CE_Success:
			return "Success";
		case CE_ServerNotFound:
			return "Server Not Found";
		case CE_NoControl:
			return "No Control";
		case CE_ProtocolHandshakeMissing:
			return "Protocol Handshake Missing";
		case CE_IncompatibleProtocol:
			return "Incompatible Protocol";
		case CE_InvalidArguments:
			return "Invalid Arguments";
		default:
			return "Unknown Error";
	}
}

int GetDevideIndex(CorsairDeviceType device)
{
	for (int i = 0, size = CorsairGetDeviceCount(); i < size; i++)
		if (CorsairGetDeviceInfo(i)->type == device)
			return i;
	
	return -1;
}

bool InitLEDs()
{
	if (!arduino)		//It means the program has been initialized before (e.g. on windows wake up)
	{
		arduino = new Serial("\\\\.\\COM6");

		if (!arduino->IsConnected())
		{
			std::cerr << "Arduino not found" << std::endl;
			return false;
		}
		else
		{
			std::cout << "Arduino on COM6" << std::endl;
		}
	}

	CorsairPerformProtocolHandshake();

	std::cout << "Connecting to iCUE";

	while (const auto error = CorsairGetLastError())
	{
		std::cout << ".";
		Sleep(5000);
		CorsairPerformProtocolHandshake();
	}

	std::cout << std::endl;

	CorsairError error = CorsairGetLastError();

	if (error != CE_Success)
	{
		std::cerr << "Handshake failed: " << ToString(error) << std::endl;
		return false;
	}
	else
	{
		std::cout << "Connected" << std::endl;
	}

	std::cout << "Getting devices";

	index = GetDevideIndex(CDT_LightingNodePro);

	while (index < 0)
	{
		index = GetDevideIndex(CDT_LightingNodePro);
		//fanIndex = GetDevideIndex(CDT_CommanderPro);
		std::cout << ".";
		Sleep(1000);
	}

	std::cout << std::endl << "Done" << std::endl;
	
	CorsairLedPositions* p1 = CorsairGetLedPositionsByDeviceIndex(index);
	//CorsairLedPositions* p3 = CorsairGetLedPositionsByDeviceIndex(fanIndex);

	for (int i = 0; i < 20; i++)
		leds[i].ledId = p1->pLedPosition[i].ledId;

	//fanTemp[0].ledId = p3->pLedPosition[0].ledId;

	return true;
}

bool UpdateLEDs()
{
	CorsairError error = CorsairGetLastError();

	if (arduino->IsConnected() && error == CE_Success)
	{
		if (CorsairGetLedsColorsByDeviceIndex(index, 20, leds))
		{
			for (int i = 1; i < 10; i++)
			{
				char buffer1[5];

				buffer1[0] = 'f';	//Fita
				buffer1[1] = (char)i;
				buffer1[2] = (char)leds[i].r;
				buffer1[3] = (char)leds[i].g;
				buffer1[4] = (char)leds[i].b;

				arduino->WriteData(buffer1, 5);
			}

			char buffer2[4];

			buffer2[0] = 'g';	//Gabinete
			buffer2[1] = (char)leds[10].r;
			buffer2[2] = (char)leds[10].g;
			buffer2[3] = (char)leds[10].b;

			arduino->WriteData(buffer2, 4);
		}
		else
		{
			std::cerr << "Serial writing failed" << std::endl;
			return false;
		}

		return true;
	}
	else 
	{
		std::cerr << "Error in getting device" << std::endl;
		return false;
	}
}

/*bool UpdateTemp()
{
	CorsairError error = CorsairGetLastError();

	if (arduino->IsConnected() && error == CE_Success)
	{
		if (CorsairGetLedsColorsByDeviceIndex(fanIndex, 1, fanTemp))
		{
			int temp = fanTemp[0].r;
			char s = 'd';
			char* m;

			if (temp < 40)
			{
				s = 'd';	//Off
				m = "Fan off";
			}
			else if (temp >= 40 && temp < 60)
			{
				s = 'a';	//Min
				m = "Fan at min speed";
			}
			else if (temp >= 60 && temp < 70)
			{
				s = 'b';	//Med
				m = "Fan at medium speed";
			}
			else if (temp >= 70)
			{
				s = 'c';	//Max
				m = "Fan at max speed";
			}

			if (s != fanSpeed) 
			{
				fanSpeed = s;

				std::cout << m << std::endl;

				char buffer[2];

				buffer[0] = 't';	//Temperatura
				buffer[1] = s;

				arduino->WriteData(buffer, 2);
			}
		}
		else return false;
		
		return true;
	}
	else
	{
		std::cerr << "Error temperature" << std::endl;
		return false;
	}
}*/

void ChangeColorMode(char c) 
{
	char buffer[2];

	buffer[0] = 'c';		//Cor
	buffer[1] = c;

	std::cerr << (arduino->WriteData(buffer, 2) ? 
		"Color mode changed" : "Serial write failed") << std::endl;
}