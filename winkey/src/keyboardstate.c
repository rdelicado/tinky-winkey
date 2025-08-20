#include "winkey.h"

// Sets or clears the state bits for Shift (uppercase)
static void SetShiftState(BYTE ks[256]) {
    // If Shift is pressed
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        ks[VK_SHIFT]   |= 0x80;   // Set generic Shift as pressed
        ks[VK_LSHIFT]  |= 0x80;   // Set left Shift as pressed
        ks[VK_RSHIFT]  |= 0x80;   // Set right Shift as pressed
    } else {
        ks[VK_SHIFT]   &= ~0x80;  // Clear generic Shift
        ks[VK_LSHIFT]  &= ~0x80;  // Clear left Shift
        ks[VK_RSHIFT]  &= ~0x80;  // Clear right Shift
    }
}

// Sets or clears the state bits for Control (Ctrl)
static void SetCtrlState(BYTE ks[256]) {
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        ks[VK_CONTROL]  |= 0x80;
        ks[VK_LCONTROL] |= 0x80;
        ks[VK_RCONTROL] |= 0x80;
    } else {
        ks[VK_CONTROL]  &= ~0x80;
        ks[VK_LCONTROL] &= ~0x80;
        ks[VK_RCONTROL] &= ~0x80;
    }
}

// Sets or clears the state bits for Alt (left and right Alt)
static void SetAltState(BYTE ks[256]) {
    if (GetAsyncKeyState(VK_MENU) & 0x8000) {
        ks[VK_MENU]  |= 0x80;
        ks[VK_LMENU] |= 0x80;
        ks[VK_RMENU] |= 0x80;
    } else {
        ks[VK_MENU]  &= ~0x80;
        ks[VK_LMENU] &= ~0x80;
        ks[VK_RMENU] &= ~0x80;
    }
}

// Sets the necessary bits for AltGr (right Alt + right Ctrl)
static void SetAltGrState(BYTE ks[256]) {
    if (GetAsyncKeyState(VK_RMENU) & 0x8000) {
        ks[VK_RMENU]   |= 0x80;
        ks[VK_MENU]    |= 0x80;
        ks[VK_CONTROL] |= 0x80;
        ks[VK_RCONTROL]|= 0x80;
    }
}

// Sets or clears the state bit for CapsLock
static void SetCapsLockState(BYTE ks[256]) {
    if (GetKeyState(VK_CAPITAL) & 0x0001)
        ks[VK_CAPITAL] |= 0x01;
    else
        ks[VK_CAPITAL] &= ~0x01;
}

// Builds the complete keyboard state for ToUnicodeEx
void BuildKeyboardState(BYTE ks[256]) {
    if (!GetKeyboardState(ks))
        memset(ks, 0, 256);
    SetShiftState(ks);
    SetCtrlState(ks);
    SetAltState(ks);
    SetAltGrState(ks);
    SetCapsLockState(ks);
}