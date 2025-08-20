#include "winkey.h"

// Opens the log file at C:\Users\Public\winkey.log
BOOL OpenLogFile(void)
{
    char logPath[MAX_PATH];
    time_t currentTime;
    struct tm timeInfo;
    long fileSize = 0;

    // Build full path to log file
    snprintf(logPath, sizeof(logPath), "C:\\Users\\Public\\winkey.log");

    // Open in append read/write mode to detect if empty
    if (fopen_s(&g_winkeyState.logFile, logPath, "ab+") != 0 || !g_winkeyState.logFile)
        return FALSE;

    // If file is empty, write UTF-8 BOM so Notepad detects encoding
    if (fseek(g_winkeyState.logFile, 0, SEEK_END) == 0) {
        fileSize = ftell(g_winkeyState.logFile);
        if (fileSize == 0) {
            unsigned char bom[3] = {0xEF, 0xBB, 0xBF};
            fwrite(bom, 1, 3, g_winkeyState.logFile);
        }
    }

    // Get current time and write start header
    time(&currentTime);
    localtime_s(&timeInfo, &currentTime);
    
    fprintf(g_winkeyState.logFile, "\n=============== Winkey Started: %02d/%02d/%04d %02d:%02d:%02d ===============\n",
            timeInfo.tm_mday, timeInfo.tm_mon + 1, timeInfo.tm_year + 1900,
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    
    fflush(g_winkeyState.logFile);

    return TRUE;
}