/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   impersonation.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 13:31:21 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/15 18:05:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "svc.h"

	#include <stdio.h>
	#include <tlhelp32.h>

#pragma endregion

#pragma region "Methods"

	#pragma region "GetProcessIdByName"

		DWORD GetProcessIdByName(const char* name) {
			HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			PROCESSENTRY32 pe32 = { sizeof(pe32) };
			if (Process32First(handle, &pe32)) do {
				if (!_stricmp(pe32.szExeFile, name)) {
					CloseHandle(handle);
					return (pe32.th32ProcessID);
				}
			} while (Process32Next(handle, &pe32));
			CloseHandle(handle);
			return (0);
		}

	#pragma endregion

	#pragma region "Compare Tokens"

		BOOL CompareTokens(char *process1, char *process2) {
			HANDLE				hProcess1 = NULL,	hProcess2 = NULL;
			HANDLE				hToken1 = NULL,		hToken2 = NULL;
			TOKEN_USER			*pUser1 = NULL,		*pUser2 = NULL;
			BOOL				result = FALSE;

			do {
				DWORD len1 = 0, len2 = 0;

				// Try to open first process with different permission levels
				hProcess1 = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetProcessIdByName(process1));
				if (!hProcess1) hProcess1 = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetProcessIdByName(process1));
				if (!hProcess1)																									{ printf("[!] Failed to open %s process\n", process1);						break; }
				if (!OpenProcessToken(hProcess1, TOKEN_QUERY, &hToken1))														{ printf("[!] Failed to open %s process token\n", process1);				break; }
				if (!GetTokenInformation(hToken1, TokenUser, NULL, 0, &len1) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)	{ printf("[!] Failed to get token info size for %s process\n", process1);	break; }
				if (!(pUser1 = (TOKEN_USER*)malloc(len1)))																		{ printf("[!] Memory allocation failed\n");									break; }
				if (!GetTokenInformation(hToken1, TokenUser, pUser1, len1, &len1))												{ printf("[!] Failed to get token info for %s process\n", process1);		break; }

				// Try to open second process with different permission levels
				hProcess2 = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetProcessIdByName(process2));
				if (!hProcess2) hProcess2 = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetProcessIdByName(process2));
				if (!hProcess2)																									{ printf("[!] Failed to open %s process\n", process2);						break; }
				if (!OpenProcessToken(hProcess2, TOKEN_QUERY, &hToken2))														{ printf("[!] Failed to open %s process token\n", process2);				break; }
				if (!GetTokenInformation(hToken2, TokenUser, NULL, 0, &len2) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)	{ printf("[!] Failed to get token info size for %s process\n", process2);	break; }
				if (!(pUser2 = (TOKEN_USER*)malloc(len2)))																		{ printf("[!] Memory allocation failed\n");									break; }
				if (!GetTokenInformation(hToken2, TokenUser, pUser2, len2, &len2))												{ printf("[!] Failed to get token info for %s process\n", process2);		break; }

				char name[256], domain[256];
				DWORD nameLen = sizeof(name), domainLen = sizeof(domain); SID_NAME_USE sidType;
				if (LookupAccountSidA(NULL, pUser1->User.Sid, name, &nameLen, domain, &domainLen, &sidType))					printf("Token:               %s", name); else printf("Token:               UNKNOWN");
				nameLen = sizeof(name); domainLen = sizeof(domain);
				if (LookupAccountSidA(NULL, pUser2->User.Sid, name, &nameLen, domain, &domainLen, &sidType))					printf("       %s\n", name); else printf("       UNKNOWN\n");

				if (EqualSid(pUser1->User.Sid, pUser2->User.Sid)) { result = TRUE;

					// Integrity Level
					DWORD intLen1 = 0, intLen2 = 0;
					GetTokenInformation(hToken1, TokenIntegrityLevel, NULL, 0, &intLen1);
					GetTokenInformation(hToken2, TokenIntegrityLevel, NULL, 0, &intLen2);
					TOKEN_MANDATORY_LABEL *pIntegrity1 = (TOKEN_MANDATORY_LABEL*)malloc(intLen1);
					TOKEN_MANDATORY_LABEL *pIntegrity2 = (TOKEN_MANDATORY_LABEL*)malloc(intLen2);
					if (!pIntegrity1 || !pIntegrity2) { printf("Integrity Level: Memory allocation failed\n"); }

					if (pIntegrity1 && pIntegrity2 &&
						GetTokenInformation(hToken1, TokenIntegrityLevel, pIntegrity1, intLen1, &intLen1) &&
						GetTokenInformation(hToken2, TokenIntegrityLevel, pIntegrity2, intLen2, &intLen2)) {
						DWORD intLevel1 = *GetSidSubAuthority(pIntegrity1->Label.Sid, *GetSidSubAuthorityCount(pIntegrity1->Label.Sid) - 1);
						DWORD intLevel2 = *GetSidSubAuthority(pIntegrity2->Label.Sid, *GetSidSubAuthorityCount(pIntegrity2->Label.Sid) - 1);
						printf("Integrity Level:     %lu        %lu\n", intLevel1, intLevel2);
						if (intLevel1 != intLevel2) result = FALSE;
					}
					free(pIntegrity1);
					free(pIntegrity2);

					// Elevation Type
					TOKEN_ELEVATION_TYPE elevType1, elevType2;
					DWORD elevSize = sizeof(TOKEN_ELEVATION_TYPE);
					if (GetTokenInformation(hToken1, TokenElevationType, &elevType1, elevSize, &elevSize) &&
						GetTokenInformation(hToken2, TokenElevationType, &elevType2, elevSize, &elevSize)) {
						printf("Elevation Type:        %d            %d\n", elevType1, elevType2);
						if (elevType1 != elevType2) result = FALSE;
					}

					// Elevated
					TOKEN_ELEVATION elev1, elev2;
					elevSize = sizeof(TOKEN_ELEVATION);
					if (GetTokenInformation(hToken1, TokenElevation, &elev1, elevSize, &elevSize) &&
						GetTokenInformation(hToken2, TokenElevation, &elev2, elevSize, &elevSize)) {
						printf("Elevated:              %ld            %ld\n", elev1.TokenIsElevated, elev2.TokenIsElevated);
						if (elev1.TokenIsElevated != elev2.TokenIsElevated) result = FALSE;
					}

					// Session ID
					DWORD session1, session2;
					DWORD sessionSize = sizeof(DWORD);
					if (GetTokenInformation(hToken1, TokenSessionId, &session1, sessionSize, &sessionSize) &&
						GetTokenInformation(hToken2, TokenSessionId, &session2, sessionSize, &sessionSize)) {
						printf("Session ID:            %lu            %lu\n", session1, session2);
						if (session1 != session2) result = FALSE;
					}

					// Token Type
					TOKEN_TYPE tokenType1, tokenType2; DWORD size;
					if (GetTokenInformation(hToken1, TokenType, &tokenType1, sizeof(tokenType1), &size) &&
						GetTokenInformation(hToken2, TokenType, &tokenType2, sizeof(tokenType2), &size)) {
						printf("Token Type:            %d            %d\n", tokenType1, tokenType2);
						if (tokenType1 != tokenType2) result = FALSE;
					}

					// Privileges
					DWORD privLen1 = 0, privLen2 = 0;
					GetTokenInformation(hToken1, TokenPrivileges, NULL, 0, &privLen1);
					GetTokenInformation(hToken2, TokenPrivileges, NULL, 0, &privLen2);
					TOKEN_PRIVILEGES *priv1 = (TOKEN_PRIVILEGES*)malloc(privLen1);
					TOKEN_PRIVILEGES *priv2 = (TOKEN_PRIVILEGES*)malloc(privLen2);
					if (!priv1 || !priv2) { printf("Privileges:      Memory allocation failed\n"); }

					if (priv1 && priv2 &&
						GetTokenInformation(hToken1, TokenPrivileges, priv1, privLen1, &privLen1) &&
						GetTokenInformation(hToken2, TokenPrivileges, priv2, privLen2, &privLen2)) {
						printf("Privileges:            %lu           %lu\n", priv1->PrivilegeCount, priv2->PrivilegeCount);
						if (priv1->PrivilegeCount != priv2->PrivilegeCount) result = FALSE;
					}
					free(priv1);
					free(priv2);
				} else {
					printf("\nSIDs are different\n");
				}

			} while (FALSE);

			if (pUser1)		free(pUser1);
			if (pUser2)		free(pUser2);
			if (hToken1)	CloseHandle(hToken1);
			if (hToken2)	CloseHandle(hToken2);
			if (hProcess1)	CloseHandle(hProcess1);
			if (hProcess2)	CloseHandle(hProcess2);

			return (result);
		}

	#pragma endregion

	#pragma region "Impersonate"

		HANDLE impersonate() {
			HANDLE dup_token_handle = NULL;
			HANDLE token_handle = NULL;
			DWORD pid = GetProcessIdByName("winlogon.exe");

			if (pid == 0) {
				// Error finding winlogon.exe
				ReportStatus(SERVICE_STOPPED, GetLastError(), 0);
				CloseHandle(g_ServiceStopEvent);
				return (NULL);
			}

			HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
			if (!processHandle) {
				// Error opening the process
				ReportStatus(SERVICE_STOPPED, GetLastError(), 0);
				CloseHandle(g_ServiceStopEvent);
				return (NULL);
			}

			if (!OpenProcessToken(processHandle, TOKEN_ALL_ACCESS, &token_handle)) {
				// Error obtaining the token
				CloseHandle(processHandle);
				ReportStatus(SERVICE_STOPPED, GetLastError(), 0);
				CloseHandle(g_ServiceStopEvent);
				return (NULL);
			}

			if (!DuplicateTokenEx(token_handle, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &dup_token_handle)) {
				// Error duplicating the token
				CloseHandle(token_handle);
				CloseHandle(processHandle);
				ReportStatus(SERVICE_STOPPED, GetLastError(), 0);
				CloseHandle(g_ServiceStopEvent);
				return (NULL);
			}

			return (dup_token_handle);
		}

	#pragma endregion

#pragma endregion
