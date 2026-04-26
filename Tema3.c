#include <windows.h>
#include <stdio.h>

#define SERVICE_NAME "HelloWorldService"

SERVICE_STATUS g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;


void WriteToLog(const char* message) {
	HANDLE hFile = CreateFileA("C:\\Temp\\HelloWorldService_Log.txt",
		FILE_APPEND_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		char buffer[256];
		int len = snprintf(buffer, sizeof(buffer), "%s\r\n", message);
		WriteFile(hFile, buffer, (DWORD)len, &bytesWritten, NULL);
		CloseHandle(hFile);
	}
}


void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
	switch (CtrlCode) {
	case SERVICE_CONTROL_STOP:
		if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
			WriteToLog("Serviciul se opreste...");

			g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			g_ServiceStatus.dwWin32ExitCode = 0;
			SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

			SetEvent(g_ServiceStopEvent);
		}
		break;
	default:
		break;
	}
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
	g_StatusHandle = RegisterServiceCtrlHandlerA(SERVICE_NAME, ServiceCtrlHandler);
	if (g_StatusHandle == NULL) return;
	
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;

	
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL) {
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
		return;
	}

	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	WriteToLog("Hello World!");

	WaitForSingleObject(g_ServiceStopEvent, INFINITE);

	WriteToLog("Serviciul a fost oprit.");
}

int main() {
	SERVICE_TABLE_ENTRYA ServiceTable[] = {
		{(LPSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONA)ServiceMain},
		{NULL, NULL}
	};

	if (StartServiceCtrlDispatcherA(ServiceTable) == FALSE) {
		return GetLastError();
	}

	return 0;
} 