/**
 * @file main.cpp
 * @author Theodoros Temourtzidis (thodoristem@gmail.com)
 * @brief An Arduino firmware for a DIY H-SEQ-Shifter & Ebrake
 * @version 1.0
 * @date 2024-01-30
 */
#include <AnalogInput.h>
#include <Wire.h>
#include <AS5600.h>
#include <joystick.h>

// H-Shifter Settings
#define SHIFTER_MOD_SWITCH_PIN 9
#define SHIFTER_SERIAL_PLUS 8
#define SHIFTER_SERIAL_MINUS 7

// Gear gate limits
const short SHIFTER_GEAR_LEFT_LIMITS[] = {0, 100};
const short SHIFTER_GEAR_MIDDLE_LIMITS[] = {400, 600};
const short SHIFTER_GEAR_RIGHT_LIMITS[] = {900, 1023};
const short SHIFTER_GEAR_TOP_LIMIT = 800;
const short SHIFTER_GEAR_BOTTOM_LIMIT = 200;

AnalogInput *shifterInput[2] = {
    new AnalogInput(A3, 0), // Shifter X
    new AnalogInput(A0, 0)  // Shifter Y
};

AnalogInput *eBrake = new AnalogInput(A1, 10, 0, 900); // E-Brake

AS5600 as5600; // Position Sensor

// Joystick configuration
Joystick_ joystick(
    JOYSTICK_DEFAULT_REPORT_ID, // USB ID
    JOYSTICK_TYPE_JOYSTICK,     // TYPE
    14,                         // Button count
    0,                          // Hat Switch
    false,                      // X
    false,                      // Y
    true,                       // Z
    false,                      // RotationX
    false,                      // RotationY
    false,                      // RotationZ
    false,                      // Rudder
    false,                      // Throttle
    false,                      // Accelarator
    false,                      // Brake
    false                       // Steering
);

void pressButton(short key, bool mod)
{
    if (mod)
    {
        joystick.releaseButton(key);
        joystick.pressButton(key + 6);
    }
    else
    {
        joystick.releaseButton(key + 6);
        joystick.pressButton(key);
    }
}

bool modifyPressed = false;

void shifterLoop()
{
    short x = shifterInput[0]->getValue();
    short y = shifterInput[1]->getValue();

    // Serial.print("X:");
    // Serial.print(x);
    // Serial.print(", Y:");
    // Serial.println(y);

    byte isModOn = !digitalRead(SHIFTER_MOD_SWITCH_PIN);

    if (y > SHIFTER_GEAR_TOP_LIMIT)
    {
        if (x > SHIFTER_GEAR_LEFT_LIMITS[0] && x < SHIFTER_GEAR_LEFT_LIMITS[1])
        {
            pressButton(0, modifyPressed);
        }
        else if (x > SHIFTER_GEAR_MIDDLE_LIMITS[0] && x < SHIFTER_GEAR_MIDDLE_LIMITS[1])
        {
            pressButton(2, modifyPressed);
        }
        else if (x > SHIFTER_GEAR_RIGHT_LIMITS[0] && x < SHIFTER_GEAR_RIGHT_LIMITS[1])
        {
            pressButton(4, modifyPressed);
        }
    }
    else if (y < SHIFTER_GEAR_BOTTOM_LIMIT)
    {
        if (x > SHIFTER_GEAR_LEFT_LIMITS[0] && x < SHIFTER_GEAR_LEFT_LIMITS[1])
        {
            pressButton(1, modifyPressed);
        }
        else if (x > SHIFTER_GEAR_MIDDLE_LIMITS[0] && x < SHIFTER_GEAR_MIDDLE_LIMITS[1])
        {
            pressButton(3, modifyPressed);
        }
        else if (x > SHIFTER_GEAR_RIGHT_LIMITS[0] && x < SHIFTER_GEAR_RIGHT_LIMITS[1])
        {
            pressButton(5, modifyPressed);
        }
    }
    else
    {
        for (short i = 0; i <= 11; i++)
        {
            joystick.releaseButton(i);
        }

        modifyPressed = (isModOn == LOW);
    }
}

void setup()
{
    // Serial.begin(115200);

    eBrake->begin();
    shifterInput[0]->begin();
    shifterInput[1]->begin();

    Wire.begin();

    as5600.begin();

    delay(3000);

    if (as5600.isConnected())
    {
        as5600.setZPosition(300);
        as5600.setMPosition(2000);
        as5600.setOutputMode(0);
    }

    delay(1000);

    joystick.begin();

    pinMode(SHIFTER_MOD_SWITCH_PIN, INPUT_PULLUP);
    pinMode(SHIFTER_SERIAL_PLUS, INPUT_PULLUP);
    pinMode(SHIFTER_SERIAL_MINUS, INPUT_PULLUP);
}

void loop()
{
    // E-Brake
    joystick.setZAxis(eBrake->getValue());

    // Seq-Shifter buttons
    joystick.setButton(12, (digitalRead(SHIFTER_SERIAL_PLUS) == LOW));
    joystick.setButton(13, (digitalRead(SHIFTER_SERIAL_MINUS) == LOW));

    // H-Shifter
    shifterLoop();
}