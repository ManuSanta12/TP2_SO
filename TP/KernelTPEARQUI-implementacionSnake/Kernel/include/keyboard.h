#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "lib.h"

void keyboard_handler(uint8_t keyPressed);

/* Returns character detected from keyboard, Scancodes without ascii symbols will return 0 */
char getCharFromKeyboard();

/* Returns the scanCode -> use for keys that are not characters */
unsigned char getScanCode();

/* Clears the scancode */
void clearScanCode();

#endif