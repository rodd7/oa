#ifndef OA_MKI_H
#define OA_MKI_H

#include <windows.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <unicode/uchar.h>
#include <iostream>
#include <sstream>
#include <locale>
#include <cwctype>

// Global variables
HHOOK g_hKeyboardHook;
std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>> > orthographicDictionary;

std::string firstKey = "";
std::string secondKey = "";
bool isUpper = false;
bool falsePattern = false;
bool mnemonicMode = false;


// Function declarations
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void SetKeyboardHook(); // Sets the keyboard hook
void RemoveKeyboardHook(); //Removes the keyboard hook

// Helpers
void clear(); //resets buffers and flag conditions

#endif // OA_MKI_H