/*

    Throttle Control Example Modified By Yo Han Joo

    Press Z to set throttle to 75%
    Press X to set throttle to 40%

*/

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <strsafe.h>

#include "SimConnect.h"

int     quit = 0;
HANDLE  hSimConnect = NULL;

static enum GROUP_ID{
    GROUP_KEYS,
};
static enum INPUT_ID {
    INPUT_KEYS,
};

static enum EVENT_ID {
    EVENT_SIM_START,
    EVENT_Z,
	EVENT_X
};

static enum DATA_DEFINE_ID {
    DEFINITION_THROTTLE,
};

static enum DATA_REQUEST_ID {
    REQUEST_THROTTLE,
};

struct structThrottleControl 
{
	double throttlePercent;
};

structThrottleControl		tc;

void CALLBACK MyDispatchProcTC(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
    HRESULT hr;
    
    switch(pData->dwID)
    {
        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
        {
            SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
            
            switch(pObjData->dwRequestID)
            {
                case REQUEST_THROTTLE:
                {
					// Read and set the initial throttle control value
					structThrottleControl *pS = (structThrottleControl*)&pObjData->dwData;

					tc.throttlePercent	= pS->throttlePercent;
					
					printf("\nREQUEST_USERID received, throttle = %2.1f", pS->throttlePercent);

					// Now turn the input events on
					hr = SimConnect_SetInputGroupState(hSimConnect, INPUT_KEYS, SIMCONNECT_STATE_ON);
                }

                default:
                   break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

            switch(evt->uEventID)
            {

		        case EVENT_SIM_START:
                    {
			            // Send this request to get the user aircraft id
		                hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_THROTTLE, DEFINITION_THROTTLE, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);
                        hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQUEST_THROTTLE, DEFINITION_THROTTLE, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_ONCE);
                    }
			        break;
						
			    case EVENT_Z:
                    {
					    // Set throttle to 75%
                        tc.throttlePercent = 75.0f;

					    hr = SimConnect_SetDataOnSimObject(hSimConnect, DEFINITION_THROTTLE, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(tc), &tc);
                    }
                    break;

                case EVENT_X:
                    {
					    // Set throttle to 40%
                        tc.throttlePercent = 40.0f;

					    hr = SimConnect_SetDataOnSimObject(hSimConnect, DEFINITION_THROTTLE, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(tc), &tc);
                    }
                    break;

                default:
                    break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_QUIT:
        {
            quit = 1;
            break;
        }

        default:
            printf("\nReceived:%d",pData->dwID);
            break;
    }
}

void testThrottleControl()
{
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Throttle Control", NULL, 0, 0, 0)))
    {
        printf("\nConnected to Flight Simulator!");   
        
        // Set up a data definition for the throttle control
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_THROTTLE,
			"GENERAL ENG THROTTLE LEVER POSITION:1", "percent");

        // Request a simulation started event
        hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");

        // Create two private key events to control the throttle
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_X);
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_Z);

        // Link the events to some keyboard keys
        hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_KEYS, "x", EVENT_X);
        hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_KEYS, "z", EVENT_Z);

		// Ensure the input events are off until the sim is up and running
        hr = SimConnect_SetInputGroupState(hSimConnect, INPUT_KEYS, SIMCONNECT_STATE_OFF);

        // Sign up for notifications
        hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYS, EVENT_X);
        hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYS, EVENT_Z);
    
        printf("\nPlease launch a flight");

        while( 0 == quit )
        {
            SimConnect_CallDispatch(hSimConnect, MyDispatchProcTC, NULL);
            Sleep(1);
        } 

        hr = SimConnect_Close(hSimConnect);
    }
}

void ProgramStart() 
{
    int userChoice;
    do {
        std::cout << "Hello, Type 1 to continue: " << std::endl;
        std::cin >> userChoice;
    } while (userChoice != 1);

    testThrottleControl();
}


int __cdecl _tmain(int argc, _TCHAR* argv[])
{
    ProgramStart();
	return 0;
}
