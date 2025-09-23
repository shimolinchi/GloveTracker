#ifndef _CLIENT_PLATFORM_SPECIFIC_TYPES_HPP_
#define _CLIENT_PLATFORM_SPECIFIC_TYPES_HPP_

// Virtual key code characters for WasKeyPressed().
// Based on key codes used with GetAsyncKeyState().
// Manually checked what values getch() returns for these keys.
// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
enum MicrosoftVirtualKeyCodes
{
 MVK_TAB    = 9, // ASCII tab character value
 MVK_RETURN = 13, // ASCII carriage return character value
 MVK_ESCAPE = 27, // ASCII ESC character value
 MVK_DOWN   = 258,
 MVK_UP     = 259,
 MVK_LEFT   = 260,
 MVK_RIGHT  = 261,
 MVK_HOME   = 262,
 MVK_BACK   = 263,
 MVK_F1     = 265,
 MVK_F2     = 266,
 MVK_F3     = 267,
 MVK_F4     = 268,
 MVK_F5     = 269,
 MVK_F6     = 270,
 MVK_F7     = 271,
 MVK_F8     = 272,
 MVK_F9     = 273,
 MVK_F10    = 274,
 MVK_F11    = 275,
 MVK_F12    = 276,
 MVK_DELETE = 330,
 MVK_INSERT = 331,
 MVK_NEXT   = 338,
 MVK_PRIOR  = 339,
 MVK_END    = 360
};

#endif
