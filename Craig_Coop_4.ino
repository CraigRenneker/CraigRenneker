// Included libraries for devices:

#include <DS1307.h>                       //Realtime Clock
#include <Wire.h>                         // Basic I/O
#include "rgb_lcd.h"                      // LCD display
#include "Grove_Motor_Driver_TB6612FNG.h" //motor driver
#include <Dusk2Dawn.h>                    // Sunrise/Sunset calculation
#include <I2Cdev.h>                       // IC2 bus driver

// Define variables:
DS1307 clock;           // For Real-Time clock:
rgb_lcd lcd;            // For LCD Display
const int colorR = 255; // 0-255 range for Red, Green, Blue color
const int colorG = 0;
const int colorB = 0;
MotorDriver motor; // For Motor Controller
#define Coop_up 2
#define Coop_dn 3
#define Run_up 4
#define Run_dn 5

int MinSinceMid = 0;

// Setup lines:
void setup()
{
    pinMode(Coop_up, INPUT);
    pinMode(Coop_dn, INPUT);
    pinMode(Run_up, INPUT);
    pinMode(Run_dn, INPUT);

    // For Real-Time clock:
    Serial.begin(9600);
    clock.begin();
    clock.fillByYMD(2022, 9, 24); // Sept 18,2022
    clock.fillByHMS(18, 48, 30);  // 07 23 19 30
    clock.fillDayOfWeek(FRI);     // Friday
    clock.setTime();              // write time to the RTC chip
    // For LCD Display
    lcd.begin(16, 2);
    lcd.setRGB(colorR, colorG, colorB);
    //    delay(1000);
    // For Motor Controller
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();
    Serial.begin(9600);
    motor.init();
    // For Sunrise/Sunset
    Serial.begin(9600);
    Dusk2Dawn SalemTwp(42.4010266, -83.6229506, -4);
    int CoopSunrise = SalemTwp.sunrise(2022, 9, 17, false);
    int CoopSunset = SalemTwp.sunset(2022, 9, 17, false);

    Serial.println(CoopSunrise); // 435 on Sept 16, 2022
    Serial.println(CoopSunset);  // 1184 on Sept 16, 2022

    char time[5];
    Dusk2Dawn::min2str(time, CoopSunrise);
    Serial.println(time);

    char time2[] = "00:00";
    Dusk2Dawn::min2str(time2, CoopSunset);
    Serial.println(time2); // 16:53

    char time4[] = "00:00";
    bool response = Dusk2Dawn::min2str(time4, CoopSunrise);
    if (response == false)
    {
        Serial.println(time4); // "ERROR"
        Serial.println("Uh oh!");
    }
    motor.dcMotorStop(MOTOR_CHA); // Ensure motors are stopped on initiation
    motor.dcMotorStop(MOTOR_CHB);
    Serial.println("Motors Stopped in Setup");
}
// Looping section:
void loop()
{
    // Clear LDC display
    for (int n = 0; n < 20; n++) // 20 indicates symbols in line. For 2x16 LCD write - 16
    {
        lcd.setCursor(n, 0);
        lcd.print(" ");
        lcd.setCursor(n, 1);
        lcd.print(" ");
    }
    // Calculate Sunrise/Sunset on every loop
    printTime();
    Dusk2Dawn SalemTwp(42.4010266, -83.6229506, -4);
    int CoopSunrise = SalemTwp.sunrise(clock.year + 2000, clock.month, clock.dayOfMonth, false);
    int CoopSunset = SalemTwp.sunset(clock.year + 2000, clock.month, clock.dayOfMonth, false);

    clock.getTime();
    lcd.setCursor(0, 0);
    lcd.print("Min to Opn ");
    lcd.print(CoopSunrise - (clock.hour * 60 + clock.minute)); // Show min to Sunrise
    delay(1000);
    lcd.setCursor(0, 1);
    //  Serial.println(clock.hour*60+clock.minute);
    lcd.print("Min to Cls ");
    lcd.print(CoopSunset - (clock.hour * 60 + clock.minute)); // Show min to Sunset
    delay(1000);
    Serial.println(clock.hour * 60 + clock.minute - CoopSunrise);
    Serial.println(clock.hour * 60 + clock.minute - CoopSunset);
    Serial.println("  ");
    Serial.print(digitalRead(Coop_up));
    Serial.print(digitalRead(Coop_dn));
    Serial.print(digitalRead(Run_up));
    Serial.println(digitalRead(Run_dn));

    if (clock.hour * 60 + clock.minute - CoopSunrise == 0) // If at Sunrise, raise door
    {
        Serial.println("Open the Coop Door");
        Serial.println(digitalRead(Coop_up));
        motor.dcMotorRun(MOTOR_CHB, 255);   // Run motor up
        while (digitalRead(Coop_up) == LOW) // check to see if coop up switch is NOT closed
        {
            Serial.println("Coop Motor Running Up");
            delay(100);
        }
        motor.dcMotorStop(MOTOR_CHB); // Stop when up switch sets
        Serial.println("Coop door open");
        Serial.println(digitalRead(Coop_up));

        Serial.println("Open the Run Door");
        Serial.println(digitalRead(Run_up));
        motor.dcMotorRun(MOTOR_CHA, 255);  // Run motor up
        while (digitalRead(Run_up) == LOW) // check to see if coop up switch is NOT closed
        {
            Serial.println("Run Motor Running Up");
            delay(100);
        }
        motor.dcMotorStop(MOTOR_CHA); // Stop when up switch sets
        Serial.println("Run door open");
        Serial.println(digitalRead(Run_up));
    }

    if (clock.hour * 60 + clock.minute - CoopSunset == 0) // If at Sunset, lower doors
    {
        Serial.println("Close the Coop Door");
        Serial.println(digitalRead(Coop_dn));
        motor.dcMotorRun(MOTOR_CHB, -255);  // Run motor down
        while (digitalRead(Coop_dn) == LOW) // check to see if coop up switch is NOT closed
        {
            delay(100);
            Serial.println("Coop Motor Running Down");
        }
        motor.dcMotorStop(MOTOR_CHB); // Stop when up switch sets
        Serial.println("Coop Door Closed");
        Serial.println(digitalRead(Coop_dn));

        Serial.println("Close the Run Door");
        Serial.println(digitalRead(Run_dn));
        motor.dcMotorRun(MOTOR_CHA, -255); // Run motor up
        while (digitalRead(Run_dn) == LOW) // check to see if coop up switch is NOT closed
        {
            delay(100);
            Serial.println("Run Motor Running Down");
        }
        motor.dcMotorStop(MOTOR_CHA); // Stop when up switch sets
        Serial.println("Run Door Closed");
        Serial.println(digitalRead(Run_dn));
    }
}
/*Function: Display time on the serial monitor*/
void printTime() // Not sure what this does
{
    clock.getTime();
}
