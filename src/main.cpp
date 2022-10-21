// COMP-10184 – Mohawk College 
// Project #1 Alarm System 
// Stage 5 is complete
//
// This program is designed to use the motion sensor in the lab kit to simulate an alarm system
// when started the sensor will continuously look for motion if it detected a 10 second timer will start and the built in LED in the WeMos will flash 4 times a second
//after 10 seconds it will no longer be able to be disarmed and the LED will be solid will stay that way untill reset
//if during this 10 second period the pushbutton connected to D6 is pressed, the alarm will be disarmed the LED will turn off and no motion will trigger the alarm countdown
//when the pushbutton is pressed the second time the alarm will be armed and will start looking for motion again
// 
// @author  Sadeva Chanuka Karunanayaka 
// @id   000776370 
// 
// I created this work and I have not shared it with anyone else. 
// 

#include <Arduino.h>

// digital input pin definitions
#define PIN_PIR D5
#define PIN_BUTTON D6
//set constants for led interval and button delay
#define LED_INTERVAL 250
#define BUTTON_DELAY  1

//set booleans for whether the alarm has been tripped if its armed or if the button has been pressed
bool bAlarmTripped = false;
bool bAlarmSystemArmed = true;
bool bButtonPressed = false;

// *********************************************************** 
// when this method is calle the motion sensor connected to PIN_PIR (D5) will be read and if it returns true then motion has been sensed set alarm tripped to true indicating that the alarm is set off
//
// 
void checkAlarmSystem()
{

  bool bPIR;

  // read PIR sensor
  bPIR = digitalRead(PIN_PIR);

  if (bPIR)
  {

    bAlarmTripped = true;
  }
}
// *********************************************************** 
// activated the alarm systems countdown after it is armed and detects motion
//will start a 10 second countdown and flash the built in LED 4 times a second
//if the pushbutton is pressed during the countdown the countdown will end early and the alarm tripped global bool will be set to false indicating
//that the alarm system was deactivated
//if the pushbutton isnt pressed during the 10 second countdown then the method will end without changing the alarmTripped bool meening the alarm system wasnt deactivated
//the main loop method will check the bAlarmTripped variable after this method to decide if the alarm was successfully deactivated or not
// 
void activateAlarm()
{
  bool bCountDownActive = true;
 // Serial.println("ALARM SYSTEM TRIPPED STARTING COUNTDOWN");
  //setup start and end time millis as well as an LED state bool to flip every interval
  unsigned long lStartMillis = millis();
  unsigned long lIntervalMillis = millis();
  long lEndTime = 10000;
  bool bLEDState = false;
  digitalWrite(LED_BUILTIN, bLEDState);

  while (bCountDownActive)
  {
    //gets current millis subtracts by interval millis and checks if its equal or greather than the led interval constant
    unsigned long lCurrentMillis = millis();
    if (lCurrentMillis - lIntervalMillis >= LED_INTERVAL)//if true its time to switch the led 
    {
      lIntervalMillis = lCurrentMillis;//sets the interval millis to the current millis for comparison in the next interval
      // flip the led state boolean
      if (bLEDState)//flips the led state bool to the opposite
      {
        bLEDState = false;
      }
      else
      {
        bLEDState = true;
      }

      //writes the led to the led boolean that was flipped
      digitalWrite(LED_BUILTIN, bLEDState);
    }
    //if the current millis- the millis when the loop started is equal to or greater than the end time ms then the countdown is over sets the countDownActive bool to false
    if (lCurrentMillis - lStartMillis >= lEndTime)
    {
      bCountDownActive = false;
    }
    //gets the button input from pin_button 
    int iButton = digitalRead(PIN_BUTTON);

    if (iButton == 0 && !bButtonPressed)//if the button is 0 and the button has not already been pressed then the alarm was deactivated successfully
    {
      //sets countdownactive bool to false to end the loop
      bCountDownActive = false;
      //sets alarm tripped to false to indicate the alarm was deactivated
      bAlarmTripped = false;
      //sets buttonpressed to true to keep track of if the button is currently being pressed down or not(prevents double button inputs)
      bButtonPressed = true;
    }
    else if (iButton == 1 && bButtonPressed)//if the button was unpressed and the buttonPressed bool is true make it false
    {
      bButtonPressed = false;
      Serial.println("BUTTON UNPRESSED");
    }
    delay(BUTTON_DELAY);
  }
}
// *********************************************************** 
// rearm alarm system after it has been deactivated  waits for a button press and once it happens the system will start looking for motion events again
//
void reArmAlarmSystem()
{

  bool bSystemReArmed = false;
  while (!bSystemReArmed)//while the system has NOT been armed check if the button has been pressed
  {
    //check button
    int iButton = digitalRead(PIN_BUTTON);
    //if button is pressed and bButtonPress is false then the button has been pressed 
    if (iButton == 0 && !bButtonPressed)
    {
      //Rearm the system by seting the bAlarmSystemArmed to true set bButtonPressed to true to prevent repeat inputs and set bSystemReArmed to true to exit the loop
      //Serial.println("REARMING SYSTEM");
      bButtonPressed = true;
      bSystemReArmed = true;
      bAlarmSystemArmed = true;
    }
    //if button is unpressed and the previous state is pressed (bButtonPressed being true) then set bButtonPressed to false to allow a new button press
    else if (iButton == 1 && bButtonPressed)
    {
      bButtonPressed = false;
      //Serial.println("BUTTON UNPRESSED");
    }
    delay(BUTTON_DELAY);
  }
  delay(200);
}

// *************************************************************
void setup()
{
  // configure the USB serial monitor
  Serial.begin(115200);

  // configure the LED output
  pinMode(LED_BUILTIN, OUTPUT);

  // PIR sensor is an INPUT
  pinMode(PIN_PIR, INPUT);

  // Button is an INPUT
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, HIGH);
}

// *************************************************************
void loop()
{
  // check if the button has been unpressed to prevent double button presses
  int iButton = digitalRead(PIN_BUTTON);
  //if button is unpressed and the previous state bool bButtonPressed was true then set it to false
  if (iButton == 1 && bButtonPressed)
  {
    bButtonPressed = false;
   // Serial.println("BUTTON UNPRESSED");
  }
  //if the alarm system is armed keep checking the motion sensor
  if (bAlarmSystemArmed)
  {
    checkAlarmSystem();//checks the motion sensor
    //if motion is detected bAlarmTripped will be true
    if (bAlarmTripped)
    {
      //activate alarm countdown
      activateAlarm();
      //if the countdown finished and the button to disable the alarm was not presseed then bAlarmTripped will still be true
      if (bAlarmTripped)
      {
        //alarm failed to disable
        //Serial.println("Alarm failed to be disabled");
        //sets the led to stay indefinetely untill the WeMos is reset
        digitalWrite(LED_BUILTIN, LOW);

        while (true)
        {
          yield();
        }
      }
      else//else the button was pressed during the countdown making bAlarmTripped = false
      {
        //sets alarm system armed bool to false to stop checking the motion sensor on loop
        //Serial.println("Alarm deactivated");
        bAlarmSystemArmed = false;
        digitalWrite(LED_BUILTIN, HIGH);//turn the led off
      }
    }
  }
  //else the bAlarmSystemArmed is false and the system is currently disabled
  else
  {
    //run reArmAlarmSystem to continuously wait for a button press to reactivate the alarm

    reArmAlarmSystem();
  }
}
