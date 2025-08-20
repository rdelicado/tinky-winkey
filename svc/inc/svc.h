/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   svc.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 16:22:13 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/14 16:22:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <windows.h>

#pragma endregion

#pragma region "Variables"

	#define Name		"Tinky"
	#define Version		"1.0.0"

	extern char			g_WinkeyPath[MAX_PATH];
	extern HANDLE		g_ServiceStopEvent;

#pragma endregion

#pragma region "Methods"

	void WinkeyPath(char *Path);

	// Service Manager
	void ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
	void WINAPI ServiceMain(DWORD dwArgc, LPSTR *lpszArgv);

	// Service Control
	int control(int argc, char **argv);

	// Impersonation
	DWORD GetProcessIdByName(const char* name);
	BOOL CompareTokens(char *process1, char *process2);
	HANDLE impersonate();

#pragma endregion
