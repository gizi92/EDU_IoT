/*
 * SendLGAirConditionerDemo.cpp
 *
 *  Sending LG air conditioner IR codes controlled by Serial input
 *  Based on he old IRremote source from https://github.com/chaeplin
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2022 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */
#include <Arduino.h>

/*
 * LG2 has different header timing and a shorter bit time
 * Known LG remote controls, which uses LG2 protocol are:
 * AKB75215403
 * AKB74955603
 * AKB73757604:
 */
//#define USE_LG2_PROTOCOL // Try it if you do not have success with the default LG protocol
//#define NUMBER_OF_COMMANDS_BETWEEN_PRINT_OF_MENU 5

/*
 * Define macros for input and output pin etc.
 */
//#include "PinDefinitionsAndMore.h"
//#define IR_RECEIVE_PIN      2 // To be compatible with interrupt example, pin 2 is chosen here.
#define IR_SEND_PIN         7
//#define TONE_PIN            4
//#define APPLICATION_PIN     5
//#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
//#define _IR_TIMING_TEST_PIN 3

#include <IRremote.hpp>
#include "ac_LG.hpp"
//#define INFO // Deactivate this to save program space and suppress info output from the LG-AC driver.
//#define DEBUG // Activate this for more output from the LG-AC driver.



#define SIZE_OF_RECEIVE_BUFFER 10
char sRequestString[SIZE_OF_RECEIVE_BUFFER];

Aircondition_LG MyLG_Aircondition;

void setup() {
    Serial.begin(115200);
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    /*
     * The IR library setup. That's all!
     */
#if defined(IR_SEND_PIN)
    IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
#else
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin
#endif

    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
    Serial.println();
    MyLG_Aircondition.setType(LG_IS_WALL_TYPE);
//    MyLG_Aircondition.printMenu(&Serial);
//
//    delay(1000);

// test
     MyLG_Aircondition.sendCommandAndParameter(LG_COMMAND_ON, 1);
     delay(5000);
     MyLG_Aircondition.sendCommandAndParameter(LG_COMMAND_OFF, 2);
     delay(5000);

}

void loop() {
     Serial.println("[UNO] Sending LG ON");
     MyLG_Aircondition.sendIRCommand(LG_COMMAND_ON);
     delay(3000);
     Serial.println("[UNO] Sending LG OFF");
     MyLG_Aircondition.sendIRCommand(LG_COMMAND_OFF);
     delay(3000);
}
