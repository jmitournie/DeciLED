/**
 * Conditionally enables or disables debug printing based on the value of the DEBUG macro.
 * If DEBUG is set to 1, the DEBUG_PRINT and DEBUG_PRINTLN macros will output the provided
 * arguments to the Serial port. If DEBUG is set to 0, the macros will do nothing.
 * This allows for easy toggling of debug output without having to remove the print statements.
 * 
 * Call DEBUG_SETUP() in the setup function, to enable (or not) the serial mode.
 * Do not call it, if you use Serial.begin in your code
 * 
 * DEBUG_METRIC log a metric with the Teleport format, ex : ">temperature:24.2"
 * -> can be display in VSCode with Teleport pluggin
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_SETUP()   Serial.begin(115200)
    #define DEBUG_METRIC(metricName, value) Serial.println(">" + String(metricName) + ":" + String(value))
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_SETUP()
    #define DEBUG_METRIC(metricName, value)
#endif // DEBUG

#endif // _DEBUG_H_
