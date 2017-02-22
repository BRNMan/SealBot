#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <LCDColors.h>
#include <FEHBuzzer.h>
#include <FEHMotor.h>
#include <FEHRPS.h>

//Our own classes.
//#include "mainmenu.h"
#include <robotdefinitions.h>

AnalogInputPin rightOpto(FEHIO::P0_1);
AnalogInputPin centerOpto(FEHIO::P0_2);
AnalogInputPin leftOpto(FEHIO::P0_3);

FEHMotor leftMotor(FEHMotor::Motor0, 12.0);
FEHMotor rightMotor(FEHMotor::Motor1, 12.0);

DigitalEncoder leftEncoder(FEHIO::P1_1);
DigitalEncoder rightEncoder(FEHIO::P1_0);


int main(void)
{
    //Go to the main menu after startup.
    displayMenu();
    return 0;
}

void displayMenu() {
    //Clear screen/draw background.
    LCD.Clear(DARKSLATEGRAY);
    LCD.SetFontColor(TEXT_COLOR);
    //Declare our menu buttons.
    FEHIcon::Icon buttons[NUM_MENU_BUTTONS];
    //Button labels
    char labels[NUM_MENU_BUTTONS][20] = {"Run", "Calibrate", "TestTurn", "TestRun", "Test3", "Idk"};
    //Draw a 3x2 array of icons with red text and blue borders.
    FEHIcon::DrawIconArray(buttons, 3, 2, 20, 20, 60, 60, labels, MIDNIGHTBLUE, TEXT_COLOR);
    //Coordinates for screen touches.
    float x,y;
    while (true) {
            LCD.Touch(&x,&y);
            if(buttons[0].Pressed(x,y,0)) {
                run_final();
            } else if(buttons[1].Pressed(x,y,0)) {
                RPS.InitializeTouchMenu();
                FEHIcon::DrawIconArray(buttons, 3, 2, 20, 20, 60, 60, labels, MIDNIGHTBLUE, TEXT_COLOR);
            } else if(buttons[2].Pressed(x,y,0)) {
                test_1();
            } else if(buttons[3].Pressed(x,y,0)) {
                test_2();
            } else if(buttons[4].Pressed(x,y,0)) {
                LCD.WriteLine("Forward");
                encoderForward(20, 10*COUNTS_PER_INCH);
                LCD.WriteLine("Left Turn");
                encoderLeftTurn(20, 10*COUNTS_PER_INCH);
                LCD.WriteLine("Right Turn");
                encoderRightTurn(20, 10*COUNTS_PER_INCH);
                LCD.WriteLine("Stop");
                leftMotor.Stop();
                rightMotor.Stop();
                Sleep(2.);
                LCD.WriteLine("Beverly Hills Cop");
                play_music();
            }

    }
    return;
}

void run_final() {
    LCD.WriteLine("For all the marbles.");
}

void test_1() {
    LCD.WriteLine("Hello World!");
    //Test
    encoderForward(BASE_MOTOR_POWER, 4*COUNTS_PER_INCH);
    encoderRightTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, 20*COUNTS_PER_INCH);

}

void test_2() {
    moveStartToButton();
}


void moveStartToButton() {
    encoderForward(20, 6*COUNTS_PER_INCH);
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, 10*COUNTS_PER_INCH);
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, 4*COUNTS_PER_INCH);
    encoderForward(30, 3*COUNTS_PER_INCH);
    encoderForward(40, 3*COUNTS_PER_INCH);
    encoderForward(30, 3*COUNTS_PER_INCH);
    encoderForward(20, 3*COUNTS_PER_INCH);
}

void measure_optosensors() {
    while(true) {
        LCD.WriteAt("Right opto:  ", 10, 10);
        LCD.WriteAt(rightOpto.Value(), 140, 10);
        LCD.WriteAt("Center opto: ", 10, 30);
        LCD.WriteAt(centerOpto.Value(), 140, 30);
        LCD.WriteAt("Left opto:   ", 10, 50);
        LCD.WriteAt(leftOpto.Value(), 140, 50);
    }
}

void measure_optosensors1() {
    LCD.WriteAt("Right opto:  ", 10, 10);
    LCD.WriteAt(rightOpto.Value(), 140, 10);
    LCD.WriteAt("Center opto: ", 10, 30);
    LCD.WriteAt(centerOpto.Value(), 140, 30);
    LCD.WriteAt("Left opto:   ", 10, 50);
    LCD.WriteAt(leftOpto.Value(), 140, 50);
}

void follow_straight_line() {
    rightMotor.SetPercent(BASE_MOTOR_POWER);
    leftMotor.SetPercent(BASE_MOTOR_POWER);
    int state = ON_LINE_FIRST;
    while(true) {
        if(leftOpto.Value() > LEFT_BG - .5)
            state = LINE_ON_RIGHT;
        if(rightOpto.Value() > RIGHT_BG -.5)
            state = LINE_ON_LEFT;
        if(centerOpto.Value() > CENTER_BG - .5) {
            if(leftOpto.Value() > LEFT_BG - .5)
                state = LINE_ON_RIGHT;
            if(rightOpto.Value() > RIGHT_BG -.5)
                state = LINE_ON_LEFT;
        } else {
            if(state == LINE_ON_RIGHT) {
                state = ON_LINE_SECOND;
            } else if(state == LINE_ON_LEFT) {
                state = ON_LINE_FIRST;
            }
        }
        switch(state) {
        case LINE_ON_LEFT:
            rightMotor.SetPercent(BASE_MOTOR_POWER + 8);
            leftMotor.SetPercent(BASE_MOTOR_POWER);
        break;
        case LINE_ON_RIGHT:
            rightMotor.SetPercent(BASE_MOTOR_POWER);
            leftMotor.SetPercent(BASE_MOTOR_POWER + 8);
        break;
        case ON_LINE_FIRST:
            rightMotor.SetPercent(BASE_MOTOR_POWER);
            leftMotor.SetPercent(BASE_MOTOR_POWER + 2);
        break;
        case ON_LINE_SECOND:
            rightMotor.SetPercent(BASE_MOTOR_POWER + 2);
            leftMotor.SetPercent(BASE_MOTOR_POWER);
        break;
        }
    }
}

void follow_curve() {
    rightMotor.SetPercent(BASE_MOTOR_POWER);
    leftMotor.SetPercent(BASE_MOTOR_POWER);
    int state = ON_LINE_FIRST;
    while(true) {
        if(centerOpto.Value() < CENTER_BGC + 1) {
            if(leftOpto.Value() < LEFT_BGC + 1)
                state = LINE_ON_RIGHT;
            if(rightOpto.Value() < RIGHT_BGC + 1)
                state = LINE_ON_LEFT;
        } else {
            if(state == LINE_ON_RIGHT) {
                state = ON_LINE_SECOND;
            } else if(state == LINE_ON_LEFT) {
                state = ON_LINE_FIRST;
            }
        }
        switch(state) {
        case LINE_ON_LEFT:
            rightMotor.SetPercent(BASE_MOTOR_POWER + 10);
            leftMotor.SetPercent(0);
        break;
        case LINE_ON_RIGHT:
            rightMotor.SetPercent(0);
            leftMotor.SetPercent(BASE_MOTOR_POWER + 10);
        break;
        case ON_LINE_FIRST:
            rightMotor.SetPercent(BASE_MOTOR_POWER);
            leftMotor.SetPercent(BASE_MOTOR_POWER);
        break;
        case ON_LINE_SECOND:
            rightMotor.SetPercent(BASE_MOTOR_POWER);
            leftMotor.SetPercent(BASE_MOTOR_POWER);
        break;
        }
    }
}

void encoderForward(int percent, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    rightMotor.SetPercent(percent);
    leftMotor.SetPercent(percent);

    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2. < counts);

    rightMotor.Stop();
    leftMotor.Stop();
}

void encoderLeftTurn(int motorPower, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    leftMotor.SetPercent(-motorPower);
    rightMotor.SetPercent(motorPower);

    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts);
    rightMotor.Stop();
    leftMotor.Stop();
}

void encoderRightTurn(int motorPower, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    leftMotor.SetPercent(motorPower);
    rightMotor.SetPercent(-motorPower);

    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts);
    rightMotor.Stop();
    leftMotor.Stop();
}

void leftTurnRPS(float finalheading, float power) {
    while(RPS.Heading() < finalheading) {
        leftMotor.SetPercent(-power);
        rightMotor.SetPercent(power);
    }
    leftMotor.SetPercent(0);
    rightMotor.SetPercent(0);
}

void rightTurnRPS(float finalheading, float power) {
    while(RPS.Heading() < finalheading) {
        rightMotor.SetPercent(-power);
        leftMotor.SetPercent(power);
    }
    rightMotor.SetPercent(0);
    leftMotor.SetPercent(0);
}

void play_music() {
    Buzzer.Tone(FEHBuzzer::A3, 200);
    Buzzer.Tone(FEHBuzzer::C4, 200);
    Buzzer.Tone(FEHBuzzer::A3, 150);
    Buzzer.Tone(FEHBuzzer::A3, 50);
    Buzzer.Tone(FEHBuzzer::D4, 100);
    Buzzer.Tone(FEHBuzzer::G3, 100);
    Buzzer.Tone(FEHBuzzer::F3, 100);
    Buzzer.Tone(FEHBuzzer::A3, 200);
    Buzzer.Tone(FEHBuzzer::E4, 200);
    Buzzer.Tone(FEHBuzzer::A3, 150);
    Buzzer.Tone(FEHBuzzer::A3, 50);
    Buzzer.Tone(FEHBuzzer::F4, 100);
    Buzzer.Tone(FEHBuzzer::E4, 100);
    Buzzer.Tone(FEHBuzzer::C4, 100);
    Buzzer.Tone(FEHBuzzer::A3, 100);
    Buzzer.Tone(FEHBuzzer::C4, 100);
    Buzzer.Tone(FEHBuzzer::A4, 100);
    Buzzer.Tone(FEHBuzzer::G3, 150);
    Buzzer.Tone(FEHBuzzer::G3, 50);
    Buzzer.Tone(FEHBuzzer::E3, 100);
    Buzzer.Tone(FEHBuzzer::B3, 100);
    Buzzer.Tone(FEHBuzzer::A3, 200);

}
