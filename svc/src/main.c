/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 13:00:34 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/14 17:06:38 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

    #include "svc.h"

	#include <stdio.h>

#pragma endregion

#pragma region "Variables"

	char g_WinkeyPath[MAX_PATH];

#pragma endregion

#pragma region "Winkey Path"

	void WinkeyPath(char *Path) {
		char *lastSlash;
		if (!GetModuleFileName(NULL, Path, MAX_PATH) || !(lastSlash = strrchr(Path, '\\'))) return;
		strcpy_s(lastSlash + 1, Path + MAX_PATH - (lastSlash + 1), "winkey.exe");
	}

#pragma endregion

#pragma region "Main"

	int main(int argc, char **argv) {
		// Set Winkey.exe fullpath
		WinkeyPath(g_WinkeyPath);

		// Executed as a service
		if (StartServiceCtrlDispatcher((SERVICE_TABLE_ENTRY[]) {{ Name, (LPSERVICE_MAIN_FUNCTION)ServiceMain }, { NULL, NULL }})) return (0);

		// Executed as a process, check if it has administrator privileges
		DWORD dwSize; HANDLE hToken; TOKEN_ELEVATION elevation = {0};
		if (!(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)
			&& GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)
			&& (CloseHandle(hToken), elevation.TokenIsElevated)))
			return (printf("\nAdministrator privileges are required\n"), 1);

		// Process actions
		return (control(argc, argv));
	}

#pragma endregion
