/*
 * Stepper Motor Demonstration 1
 * stepper-test.ino
 * Demonstrates 28YBJ-48 Unipolar Stepper with ULN2003 Driver
 * User Arduino Stepper Library
 */

#include <Stepper.h>

//Define Constants

// Number of steps per internal motor revolution
const float STEPS_PER_REV = 32;

// Amount of Gear Reduction
const float GEAR_RED = 64;

// Number of steps per geared output rotation
const float STEPS_PER_OUT_REV = STEPS_PER_REV * GEAR_RED;

// Define Variables

// Number of Steps Required
int StepsRequired;

// Create Instance of Stepper Class
// Specify Pins used for motor coils
// The pins used are 8, 6, 5, 4
// Connected to the ULN2003 Motor Driver In1, In2, In3, In4 - TODO check if this is the case
// Pinns entered in sequence 1-3-2-4 for proper step sequencing
Stepper steppermotor(STEPS_PER_REV, 4, 5, 6, 8);

void setup()
{
    Serial.begin(115200);
    // Nothing (Stepper Library sets pins as outputs)
}

void loop()
{
    // // Slow - 4-step CW sequence to observe lights on driver board
    // Serial.println(STEPS_PER_OUT_REV);
    // Serial.println("low - 4-step CW sequence to observe lights on driver board");
    // steppermotor.setSpeed(1);
    // StepsRequired = STEPS_PER_OUT_REV/2;
    // steppermotor.step(StepsRequired);
    // delay(2000);
    // Stepper STOP
    Serial.println("Stepper STOP");
    delay(2000);

    // Rotate CW 1.2 turn slowly
    Serial.println("Rotate CW 1.2 turn slowly");
    StepsRequired = STEPS_PER_OUT_REV/2;
    steppermotor.setSpeed(100);
    steppermotor.step(StepsRequired);
    delay(1000);

    //Rotate CCW 1/2 turn quickly
    Serial.println("Rotate CCW 1/2 turn quickly");
    StepsRequired = - STEPS_PER_OUT_REV/2;
    steppermotor.setSpeed(100);
    steppermotor.step(StepsRequired);
    delay(2000);

}