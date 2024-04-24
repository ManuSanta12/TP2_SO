#include <sys_calls.h>
#include <usr_stdlib.h>

int getHours(){
	return sys_getHours();
}

int getMinutes(){
	return sys_getMinutes();
}

int getSeconds(){
	return sys_getSeconds();
}

void display_time(){
	int hours, minutes, seconds;

	hours = getHours();
	minutes = getMinutes();
	seconds = getSeconds();
	
	printc('\n');
	printHex(hours);
	printc(':');
	printHex(minutes);
	printc(':');
	printHex(seconds);
}


