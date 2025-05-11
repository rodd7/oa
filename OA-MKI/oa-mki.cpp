
#include "oa-mki.h"

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

        if (pKeyboard->flags & LLKHF_INJECTED) { // !This blocks out the spamming of multiple diacritics
            return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
        }

        std::vector<int> unicodeValues;

        if (!falsePattern) {
            unicodeValues = orthographicDictionary[firstKey][secondKey];
        }
        else {
            unicodeValues = { static_cast<int>(firstKey[0]), static_cast<int>(secondKey[0]) };
        }

        // Pre-condition
        if (!firstKey.empty() && !secondKey.empty() && !unicodeValues.empty()) {

            if (isUpper) {
                unicodeValues[0] = u_toupper(unicodeValues[0]);
            }

            std::cout << unicodeValues[0] << " and " << unicodeValues[1] << std::endl;

            INPUT inputs[4] = {0};

            inputs[0].type = INPUT_KEYBOARD;
            inputs[0].ki.wScan = unicodeValues[0];
            inputs[0].ki.wVk = 0;
            inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;

            inputs[1] = inputs[0];
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP; // Need to simluate a key up before entering second key (if needed)

            if (unicodeValues.size() == 2) { // Diacritic cases
                inputs[2].type = INPUT_KEYBOARD;
                inputs[2].ki.wScan = unicodeValues[1];
                inputs[2].ki.wVk = 0;
                inputs[2].ki.dwFlags = KEYEVENTF_UNICODE;

                inputs[3] = inputs[2];
                inputs[3].ki.dwFlags |= KEYEVENTF_KEYUP;
            }

            SendInput(static_cast<UINT>(unicodeValues.size() * 2), inputs, sizeof(INPUT)); // Actual sending

            // Clearing stuff
            clear();

            return 1; // Stops from spamming multiple prints for some reason
        }

        // Now proceed with 2-keycode system - ctrl functions should still be allowed
        if (wParam == WM_KEYDOWN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0) {
            DWORD code = pKeyboard->vkCode;

            bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
            bool capsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001);

            if (shiftPressed ^ capsLockOn) { // XOR
                isUpper = true;
            }

            if (firstKey.empty()) {
                for (const auto& outer : orthographicDictionary) {
                    if (code == std::toupper(outer.first[0])) {
                        return firstKey = std::string(1, std::tolower(static_cast<char>(code))), std::cout << "1: " << firstKey << std::endl, 1;
                    }
                }
                clear(); // Picks up caps/shift if not
            }

            if (!firstKey.empty() && secondKey.empty()) {
                for (const auto& inner : orthographicDictionary[firstKey]) {
                    if (code == std::toupper(inner.first[0])) {
                        return secondKey = std::string(1, std::tolower(static_cast<char>(code))), std::cout << "2: " << secondKey << std::endl, 1;
                    }
                }
                return secondKey = secondKey = std::string(1, std::tolower(static_cast<char>(code))), falsePattern = true, std::cout << "2: " << secondKey << std::endl, 1; // Character doesn't match the pattern of firstKey, need to output both anyways
            }
        }
    }

    // Pass the event to the next hook in the hook chain
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

void clear() {
    firstKey.clear();
    secondKey.clear();
    isUpper = false;
    falsePattern = false;
}

void SetKeyboardHook() {
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (g_hKeyboardHook == NULL) {
        std::cerr << "Failed to install hook!" << std::endl;
    }
    else {
        std::cout << "Keyboard hook installed." << std::endl;
    }
}

void RemoveKeyboardHook() {
    if (g_hKeyboardHook != NULL) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        std::cout << "Keyboard hook removed." << std::endl;
    }
}


int main() {

    orthographicDictionary["a"]["h"] = { 0x0101 }; //ā
    orthographicDictionary["a"]["a"] = { 0x0101 }; //ā //2nd
    orthographicDictionary["a"]["w"] = { 0x00F4 }; //ô
    orthographicDictionary["a"]["y"] = { 0x0065, 0x030A }; //e̊
    orthographicDictionary["c"]["h"] = { 0x010D }; //č
    orthographicDictionary["e"]["e"] = { 0x0113 }; //ē
    orthographicDictionary["e"]["y"] = { 0x0065, 0x030A }; //e̊ //2nd
    orthographicDictionary["g"]["h"] = { 0x01E7 }; //ǧ
    orthographicDictionary["i"]["i"] = { 0x012B }; //ī
    orthographicDictionary["i"]["y"] = { 0x0069, 0x030A }; //i̊
    orthographicDictionary["o"]["e"] = { 0x00E5 }; //å
    orthographicDictionary["o"]["o"] = { 0x016B }; //ū
    orthographicDictionary["o"]["y"] = { 0x006F, 0x030A }; //o̊
    orthographicDictionary["o"]["i"] = { 0x006F, 0x030A }; //o̊ //2nd
    orthographicDictionary["r"]["r"] = { 0x0155 }; //ŕ
    orthographicDictionary["s"]["h"] = { 0x0161 }; //š
    orthographicDictionary["t"]["h"] = { 0x0163 }; //ţ
    orthographicDictionary["u"]["h"] = { 0x0259 }; //ə
    orthographicDictionary["u"]["u"] = { 0x016B }; //ū
    orthographicDictionary["u"]["i"] = { 0x016F }; //ů //2nd
    orthographicDictionary["u"]["y"] = { 0x016F }; //ů
    orthographicDictionary["u"]["i"] = { 0x016F }; //ů //2nd

    // Set up the low-level keyboard hook
    SetKeyboardHook();


    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Remove the hook before exiting
    RemoveKeyboardHook();
    return 0;
}