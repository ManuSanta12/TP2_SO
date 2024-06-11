#include "keyboard.h"
#include "time.h"
#include <stdint.h>

unsigned char scanCode = 0;
static char retChar = 0;
static int shift = 0 ;
static int capsLock = 0;



static const char hexMapPressed[256] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',//backspace,
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', //enter
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
        0, 0, 0, ' ', 0, 0 ,0,0,0,0,0,0,0, 0, 0, 0,
        0, 0 ,0,0,0,0,0,0,0,0,0,0,0
};



/*
 * 29: left cntrl
 * 42: left shift
 * 54: right shift
 * 55: no tengo numpad
 * 56: left alt
 * 58: caps lock
 * 59 - 68: F1-F10
 * 69: numLock
 * 70: scrollLock
 */


void keyboard_handler(uint8_t keyPressed) {
    scanCode = keyPressed;

    //shift pressed
    if (scanCode == 0x2A || scanCode == 0x36){
        shift = 1;
    }
    //shift not pressed
    if (scanCode == 0xAA || scanCode == 0xB6) {
        shift = 0;
    }
    //capsLock
    if (scanCode == 0x3A) {
        capsLock = (capsLock+1)%2 ;
    }


}


char getCharFromKeyboard() {
    //soltar tecla
    if (scanCode > 0x80 || scanCode == 0x0F){
        retChar = 0;
    } else {
        retChar = hexMapPressed[scanCode];
    }
    //mayuscula
    if ( (retChar >= 'a' && retChar <= 'z') && (shift == 1 || capsLock == 1) ){
        return retChar - ('a'-'A');
    }

    return retChar;
}

void clearScanCode(){
    scanCode = 0;
}



unsigned char getScanCode(){
    return scanCode;
}