#define CORSAIR_LIGHTING_SDK_DISABLE_DEPRECATION_WARNINGS

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
CorsairLedColor stripLeds[10];
CorsairLedColor caseLeds[1];
CorsairLedColor fanTemp[1];
int stripIndex = -1;
int caseIndex = -1;
int fanIndex = -1;
char fanSpeed = 'd';

const char* toString(CorsairError error)
{
	switch (error)
	{
	case CE_Success:
		return "CE_Success";
	case CE_ServerNotFound:
		return "CE_ServerNotFound";
	case CE_NoControl:
		return "CE_NoControl";
	case CE_ProtocolHandshakeMissing:
		return "CE_ProtocolHandshakeMissing";
	case CE_IncompatibleProtocol:
		return "CE_IncompatibleProtocol";
	case CE_InvalidArguments:
		return "CE_InvalidArguments";
	default:
		return "unknown error";
	}
}

int getStripIndex() {
	for (int i = 0, size = CorsairGetDeviceCount(); i < size; i++)
	{
		if (CorsairGetDeviceInfo(i)->type == CDT_LightingNodePro)
		{
			return i;
		}
	}
	return -1;
}

int getFanIndex() {
	for (int i = 0, size = CorsairGetDeviceCount(); i < size; i++)
	{
		if (CorsairGetDeviceInfo(i)->type == CDT_CommanderPro)
		{
			return i;
		}
	}
	return -1;
}

int getCaseIndex() {
	for (int i = 0, size = CorsairGetDeviceCount(); i < size; i++)
	{
		if (CorsairGetDeviceInfo(i)->type == CDT_MouseMat)
		{
			return i;
		}
	}
	return -1;
}

bool InitLEDs()
{
	CorsairPerformProtocolHandshake();

	while (CorsairGetLastError() == CE_ServerNotFound)
	{
		std::cout << ".";
		Sleep(5000);
		CorsairPerformProtocolHandshake();
	}

	CorsairError error = CorsairGetLastError();

	if (error != CE_Success)
	{
		std::cout << "Handshake failed: " << toString(error) << std::endl;
		return false;
	}

	std::cout << "SDK connected" << std::endl;

	if (!arduino)
	{
		arduino = new Serial("\\\\.\\COM4");

		if (arduino->IsConnected())
			std::cout << "Arduino connected" << std::endl;
		else
		{
			std::cout << "Arduino not found" << std::endl;
			return false;
		}
	}

	stripIndex = getStripIndex();
	caseIndex = getCaseIndex();
	fanIndex = getFanIndex();

	if (stripIndex < 0)
	{
		std::cout << "LED strip not found " << std::endl;
		return false;
	}

	if (caseIndex < 0)
	{
		std::cout << "Case LED not found " << std::endl;
		return false;
	}

	if (fanIndex < 0)
	{
		std::cout << "FAN not found " << std::endl;
		return false;
	}

	CorsairLedPositions* p1 = CorsairGetLedPositionsByDeviceIndex(stripIndex);
	CorsairLedPositions* p2 = CorsairGetLedPositionsByDeviceIndex(caseIndex);
	CorsairLedPositions* p3 = CorsairGetLedPositionsByDeviceIndex(fanIndex);

	for (int i = 0; i < 10; i++)
		stripLeds[i].ledId = p1->pLedPosition[i].ledId;

	caseLeds[0].ledId = p2->pLedPosition[0].ledId;

	fanTemp[0].ledId = p3->pLedPosition[0].ledId;

	return true;
}

bool UpdateLEDs()
{
	CorsairError error = CorsairGetLastError();

	if (arduino->IsConnected() && error == CE_Success)
	{
		if (CorsairGetLedsColorsByDeviceIndex(stripIndex, 10, stripLeds))
		{
			for (int i = 1; i < 10; i++)
			{
				char buffer[5];

				buffer[0] = 'f';	//Fita
				buffer[1] = (char)i;
				buffer[2] = (char)stripLeds[i].r;
				buffer[3] = (char)stripLeds[i].g;
				buffer[4] = (char)stripLeds[i].b;

				if (!arduino->WriteData(buffer, 5)) break;
			}
		}
		else return false;

		if (CorsairGetLedsColorsByDeviceIndex(caseIndex, 1, caseLeds))
		{
			char buffer[4];

			buffer[0] = 'g';	//Gabinete
			buffer[1] = (char)caseLeds[0].r;
			buffer[2] = (char)caseLeds[0].g;
			buffer[3] = (char)caseLeds[0].b;

			arduino->WriteData(buffer, 4);
		}
		else return false;

		return true;
	}
	else 
	{
		std::cout << "Error" << std::endl;
		return false;
	}
}

bool UpdateTemp()
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
		std::cout << "Error" << std::endl;
		return false;
	}
}

void ChangeColorMode(char c) 
{
	char buffer[2];

	buffer[0] = 'c';		//Cor
	buffer[1] = c;

	if (!arduino->WriteData(buffer, 2))
	{
		std::cout << "Serial write failed" << std::endl;
	}
	else
	{
		std::cout << "Color mode changed" << std::endl;
	}
}
