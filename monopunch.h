// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef monopunch_H_
#define monopunch_H_
#include "Arduino.h"
//add your includes for the project monopunch here


//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project monopunch here


#define FW_VERSION 0.9

void unrecognized(const char *command);
void displayVersion();
void displayStatus();
void displayHello();
void advanceLine();
void reverseLine();
void beginPunch();
void endPunch();
void readPunch();
void sendcode(unsigned short * code);
void drain();
void low_sendcode(unsigned short * code);
void reset_buffer();

//Do not add code below this line
#endif /* monopunch_H_ */
