#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <LCDColors.h>
#include <FEHBuzzer.h>
#include <FEHMotor.h>
#include <FEHRPS.h>
#include <FEHBattery.h>
#include <FEHServo.h>
#include <FEHSD.h>
#include <math.h>

//Our own classes.
#include <robotdefinitions.h>

AnalogInputPin rightOpto(FEHIO::P2_0);
AnalogInputPin centerOpto(FEHIO::P2_1);
AnalogInputPin leftOpto(FEHIO::P2_2);

FEHMotor leftMotor(FEHMotor::Motor0, 12.0);
FEHMotor rightMotor(FEHMotor::Motor1, 12.0);
FEHMotor rollServo(FEHMotor::Motor2, 5.5);

DigitalEncoder leftEncoder(FEHIO::P3_7);
DigitalEncoder rightEncoder(FEHIO::P0_0);

AnalogInputPin cdsCell(FEHIO::P0_1);

//On the left. White wire in front.
FEHServo yawServo(FEHServo::Servo0);

int main(void)
{
    //hardware check
    //post();
    //Go to the main menu after startup.
    yawServo.SetMin(SERVO_MIN);
    yawServo.SetMax(SERVO_MAX);
    //Sleep(5.0);
    //rollServo.SetPercent(0);
    displayMenu();
    return 0;
}

//Power On Self Test
void post() {
    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);
    encoderForward(10, 3*COUNTS_PER_INCH);
    LCD.WriteLine("Left Counts: ");
    LCD.Write(leftEncoder.Counts());
    LCD.WriteLine("Right Counts: ");
    LCD.Write(leftEncoder.Counts());
    LCD.WriteLine("Battery Voltage: ");
    LCD.Write(Battery.Voltage());
    Sleep(2.0);
}

void displayMenu() {
    //Clear screen/draw background.
    LCD.Clear(DARKSLATEGRAY);
    LCD.SetFontColor(TEXT_COLOR);
    //Declare our menu buttons.
    FEHIcon::Icon buttons[NUM_MENU_BUTTONS];
    //Button labels
    char labels[NUM_MENU_BUTTONS][20] = {"Run", "CalRPS", "CalServ", "PrintRPS", "LineStr", "CheckRPS"};
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
                Sleep(1);
                LCD.SetBackgroundColor(WHITE);
                LCD.SetFontColor(BLACK);
                float i = 0;
                while(!LCD.Touch(&x,&y)) {
                    LCD.WriteAt("Left:", 0, 20);
                    LCD.WriteAt("Center:", 0, 60);
                    LCD.WriteAt("Right:", 0, 100);
                    LCD.WriteAt(leftOpto.Value(), 100, 20);
                    LCD.WriteAt(centerOpto.Value(), 100, 60);
                    LCD.WriteAt(rightOpto.Value(), 100, 100);
                    LCD.WriteAt(TimeNow(), 20, 140);
                }
            } else if(buttons[5].Pressed(x,y,0)) {
                encoderForward(20, 4*COUNTS_PER_INCH);
                turnRPS(0, 20);
                Sleep(0.5);
                RPSMoveTo(30.00, 12.00, 15);
            }

    }
    return;
}

void run_final() {
    waitForLight();
    moveStartToCore();
}

void motor_test() {
    LCD.WriteLine("Forward");
    encoderForward(30, 10*COUNTS_PER_INCH);
    LCD.WriteLine("Left Turn");
    encoderLeftTurn(30, COUNTS_PER_90_DEGREES);
    LCD.WriteLine("Forward");
    encoderForward(30, 10*COUNTS_PER_INCH);
    LCD.WriteLine("Right Turn");
    encoderRightTurn(30, 2*COUNTS_PER_90_DEGREES);
    LCD.WriteLine("Forward");
    encoderForward(30, 10*COUNTS_PER_INCH);
    LCD.WriteLine("Right Turn");
    encoderRightTurn(30, COUNTS_PER_90_DEGREES);
    LCD.WriteLine("Forward");
    encoderForward(30, 10*COUNTS_PER_INCH);
    LCD.WriteLine("Right Turn");
    encoderRightTurn(30, 2*COUNTS_PER_90_DEGREES);
    LCD.WriteLine("Stop");
    leftMotor.Stop();
    rightMotor.Stop();
    Sleep(2.);
    LCD.WriteLine("Beverly Hills Cop");
    play_music();
}

void test_1() {
    yawServo.SetDegree(LEVER_ANGLE);
    Sleep(.5);
    yawServo.SetDegree(0.);
    Sleep(.5);
    yawServo.SetDegree(LEVER_ANGLE);
    Sleep(.5);
    yawServo.SetDegree(PARALLEL_ANGLE);
    Sleep(.5);
    yawServo.SetDegree(90.);
    LCD.WriteLine("WUZZUP");
}

void test_2() {
    Sleep(0.5);
    float x = 0, y = 0;
   while(!LCD.Touch(&x, &y)) {
       LCD.WriteRC("RPS X:", 1, 1);
       LCD.WriteRC(RPS.X(), 1, 8);
       LCD.WriteRC("RPS Y:", 2, 1);
       LCD.WriteRC(RPS.Y(), 2, 8);
   }
}

void moveStartToButton() {
    LCD.WriteLine("We have liftoff!");
    //Around wall
    encoderForward(30, 6*COUNTS_PER_INCH);
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, 7*COUNTS_PER_INCH);
    encoderLeftTurn(20, 1*COUNTS_PER_90_DEGREES);
    //Up ramp
    encoderForward(32, 30, 4*COUNTS_PER_INCH);
    encoderForward(42, 40, 4*COUNTS_PER_INCH);
    encoderForward(50, 48, 4*COUNTS_PER_INCH);
    encoderForward(40, 6*COUNTS_PER_INCH);
    encoderForward(30, 2*COUNTS_PER_INCH);
    //To button
    encoderRightTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, 5*COUNTS_PER_INCH);
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, 20*COUNTS_PER_INCH);
    //Back up
    encoderForward(-20, 7*COUNTS_PER_INCH);
    encoderRightTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(-20, 12*COUNTS_PER_INCH);
}

void moveStartToLever() {
    //Around wall
    encoderForward(20, 6*COUNTS_PER_INCH);
    yawServo.SetDegree(PARALLEL_ANGLE);
    displayColor();
    encoderForward(20, 1*COUNTS_PER_INCH);
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, 2*COUNTS_PER_INCH);
    displayColor();
    encoderForward(20, 1*COUNTS_PER_INCH);
    displayColor();
    encoderForward(20, 1*COUNTS_PER_INCH);
    displayColor();
    encoderForward(20, 3*COUNTS_PER_INCH);
    encoderLeftTurn(20, 1*COUNTS_PER_90_DEGREES);
    //Up ramp
    encoderForward(32, 30, 4*COUNTS_PER_INCH);
    encoderForward(42, 40, 4*COUNTS_PER_INCH);
    encoderForward(50, 48, 4*COUNTS_PER_INCH);
    encoderForward(40, 6*COUNTS_PER_INCH);
    encoderForward(30, 1*COUNTS_PER_INCH);
    //To lever
    encoderRightTurn(20, COUNTS_PER_90_DEGREES);
    //Backs up to lever, flips servo after 3 seconds, and moves forward.
    encoderForwardWall(-20, -20, 6*COUNTS_PER_INCH, 3.);
    yawServo.SetDegree(LEVER_ANGLE);
    encoderForward(20, 3*COUNTS_PER_INCH);
}

void moveStartToCore() {
    //Around wall
    encoderForward(20, 6*COUNTS_PER_INCH);
    encoderForward(20, (int)(1.5*COUNTS_PER_INCH));
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(20, (int)(1.5*COUNTS_PER_INCH));
    displayColor();
    encoderForward(20, (int)(1*COUNTS_PER_INCH));
    displayColor();
    int color = getColorCDS();
    encoderForward(20, (int)(0.5*COUNTS_PER_INCH));
    displayColor();
    encoderForward(20, 1*COUNTS_PER_INCH);
    encoderForward(20, (int)(3.4*COUNTS_PER_INCH));
    encoderLeftTurn(20, 1*COUNTS_PER_90_DEGREES);
    //Up ramp
    encoderForward(32, 30, 4*COUNTS_PER_INCH);
    encoderForward(46, 40, 4*COUNTS_PER_INCH);
    encoderForward(50, 48, 4*COUNTS_PER_INCH);
    encoderForward(40, 6*COUNTS_PER_INCH);
    encoderForward(30, 1*COUNTS_PER_INCH);
    //Run into core and pick it up
    //RPSMoveTo(22, 50, 35);
    //RPSPointTowards(8.9, 63.7, 35);
    encoderLeftTurn(20, 1*COUNTS_PER_90_DEGREES);
    encoderForward(-20, 4*COUNTS_PER_INCH);
    encoderLeftTurn(20, 3*COUNTS_PER_90_DEGREES/2 + 5);
    encoderForwardWall(-30,-30,15*COUNTS_PER_INCH, 4.5);
    encoderForward(20, (int)(0.5*COUNTS_PER_INCH));
    Sleep(0.5);
    yawServo.SetDegree(LEVER_ANGLE);
    Sleep(0.3);
    encoderForward(30, 9*COUNTS_PER_INCH);
    //yawServo.SetDegree(0);
    //follow_straight_line(-20);
    //Sleep(0.5);
    //yawServo.SetDegree(LEVER_ANGLE);
    //encoderForward(20, 8*COUNTS_PER_INCH);
    //encoderLeftTurn(20, COUNTS_PER_90_DEGREES/2);
    //Sleep(0.5);
    //Line up against wall, turn towards ramp
    //Down ramp
    RPSMoveTo(30, 16, 30);
    encoderForward(-20, 2*COUNTS_PER_INCH);
    encoderLeftTurn(30, COUNTS_PER_90_DEGREES);
    switch(color) {
        //NONE, measure again
        case 0:

        break;
        //RED
        case 1:
            yawServo.SetDegree(TURN_ANGLE);
            encoderForward(-20, 8*COUNTS_PER_INCH);
            encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
            encoderForwardWall(-20, -20, 7*COUNTS_PER_INCH, 3.5);
            rollServo.SetPercent(-50.0);
        break;
        //BLUE
        case 2:
            yawServo.SetDegree(TURN_ANGLE);
            encoderForward(-20, 4*COUNTS_PER_INCH);
            encoderLeftTurn(20, COUNTS_PER_90_DEGREES - 10);
            encoderForwardWall(-20, -20, 7*COUNTS_PER_INCH, 3.5);
            rollServo.SetPercent(-50.0);
        break;
    }
    Sleep(4.0);
    rollServo.SetPercent(0);
    yawServo.SetDegree(0);
    Sleep(0.5);
    encoderForward(20, 4*COUNTS_PER_INCH);
    encoderRightTurn(20, COUNTS_PER_90_DEGREES + 15);
    encoderForwardWall(30, 30, 34*COUNTS_PER_INCH, 4.0);
    encoderForward(-20, 2*COUNTS_PER_INCH);
    encoderRightTurn(20, COUNTS_PER_90_DEGREES);
    encoderForwardWall(20, 20, 6*COUNTS_PER_INCH, 2.0);
    LCD.WriteLine("WERE DONE BIYATTCH");
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

void follow_straight_line(int power) {
    rightMotor.SetPercent(power);
    leftMotor.SetPercent(power);
    int state = ON_LINE_FIRST;
    double startTime = TimeNow();
    while(true) {
        if(TimeNow() - startTime > 3.0) {
            break;
        }
        if(leftOpto.Value() < LEFT_OPTO_ORANGE_BG + 1.0)
            state = LINE_ON_RIGHT;
        if(rightOpto.Value() < RIGHT_OPTO_ORANGE_BG + 1.0)
            state = LINE_ON_LEFT;
        if(centerOpto.Value() < CENTER_OPTO_ORANGE_BG + 1.0) {
            if(leftOpto.Value() < LEFT_OPTO_ORANGE_BG + 1.0)
                state = LINE_ON_RIGHT;
            if(rightOpto.Value() < RIGHT_OPTO_ORANGE_BG + 1.0)
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
            rightMotor.SetPercent(power - 6);
            leftMotor.SetPercent(power);
        break;
        case LINE_ON_RIGHT:
            rightMotor.SetPercent(power);
            leftMotor.SetPercent(power - 6);
        break;
        case ON_LINE_FIRST:
            rightMotor.SetPercent(power);
            leftMotor.SetPercent(power - 2);
        break;
        case ON_LINE_SECOND:
            rightMotor.SetPercent(power - 2);
            leftMotor.SetPercent(power);
        break;
        }
    }
}

void follow_black_line(int power) {
    rightMotor.SetPercent(power);
    leftMotor.SetPercent(power);
    while(rightOpto.Value() > LEFT_OPTO_BLACK_BG + .5) {
     LCD.WriteLine(centerOpto.Value());
    }
    int state = ON_LINE_FIRST;
    double startTime = TimeNow();
    while(true) {
        if(TimeNow() - startTime > 3.0) {
            break;
        }
        if(leftOpto.Value() < LEFT_OPTO_BLACK_BG - 0.7)
            state = LINE_ON_RIGHT;
        if(rightOpto.Value() < RIGHT_OPTO_BLACK_BG - 0.7)
            state = LINE_ON_LEFT;
        if(centerOpto.Value() < CENTER_OPTO_BLACK_BG - 0.7) {
            if(leftOpto.Value() < LEFT_OPTO_BLACK_BG - 0.7)
                state = LINE_ON_RIGHT;
            if(rightOpto.Value() < RIGHT_OPTO_BLACK_BG - 0.7)
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
            rightMotor.SetPercent(power + 8);
            leftMotor.SetPercent(power);
        break;
        case LINE_ON_RIGHT:
            rightMotor.SetPercent(power);
            leftMotor.SetPercent(power + 8);
        break;
        case ON_LINE_FIRST:
            rightMotor.SetPercent(power);
            leftMotor.SetPercent(power + 2);
        break;
        case ON_LINE_SECOND:
            rightMotor.SetPercent(power + 2);
            leftMotor.SetPercent(power);
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
    float startTime = TimeNow();
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts && TimeNow() - startTime < TIME_UNTIL_STOP) {
    }

    rightMotor.Stop();
    leftMotor.Stop();
}

void encoderForward(int leftPercent, int rightPercent, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    rightMotor.SetPercent(rightPercent);
    leftMotor.SetPercent(leftPercent);
    float startTime = TimeNow();
    //Moves on, just in case.
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts && TimeNow() - startTime < TIME_UNTIL_STOP) {
    }

    rightMotor.Stop();
    leftMotor.Stop();
}

//This one is only used for the times when it hits a wall and stops, to make sure it can back up.
void encoderForwardWall(int leftPercent, int rightPercent, int counts, double backTime) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();
    rightMotor.SetPercent(rightPercent);
    leftMotor.SetPercent(leftPercent);
    float startTime = TimeNow();
    //Moves on after specified time
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2. < counts && TimeNow() - startTime < backTime) {
    }
    rightMotor.Stop();
    leftMotor.Stop();
}

//TODO: Accelerates forward amount specified by counts at maximum power of power.
void encoderAccelForward(int power, int counts);
void encoderAccelForward(int power, int counts) {
    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts) {

    }
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

    //Turns the wheels a certain distance modified by a wheel offset.
    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts - RTURN_OFFSET*counts/COUNTS_PER_90_DEGREES);
    rightMotor.Stop();
    leftMotor.Stop();
}

//CCW Turn
void leftTurnRPS(float finalHeading, float power) {
    float startHeading = RPS.Heading();
    if(finalHeading < 1.0) {
        finalHeading = 1.0;
    }
    leftMotor.SetPercent(-power);
    rightMotor.SetPercent(power);
    if(finalHeading < startHeading) {
        while(RPS.Heading() < finalHeading || RPS.Heading() > startHeading - 2.);
    } else {
        while(RPS.Heading() < finalHeading);
    }
    rightMotor.SetPercent(0);
    leftMotor.SetPercent(0);
}

//Clockwise turn
void rightTurnRPS(float finalHeading, float power) {
    float startHeading = RPS.Heading();
    if(finalHeading < 1.0) {
        finalHeading = 359.9;
    }
    leftMotor.SetPercent(power);
    rightMotor.SetPercent(-power);
    if(finalHeading > startHeading) {
        while(RPS.Heading() > finalHeading || RPS.Heading() < startHeading + 2.);
    } else {
        while(RPS.Heading() > finalHeading);
    }
    rightMotor.SetPercent(0);
    leftMotor.SetPercent(0);
}

void turnRPS(float finalHeading, float power) {
    LCD.WriteRC(RPS.Heading(), 1, 1); //225
    LCD.WriteRC(finalHeading, 2, 1);  //90
    bool over180 = false;
    float divAngle = 180. + RPS.Heading();
    if(divAngle > 359.9) {
        divAngle -= 360.;
        over180 = true;
    }
    if(over180) {
        if(finalHeading < RPS.Heading() && finalHeading > divAngle) {
            rightTurnRPS(finalHeading, power);
        } else {
            leftTurnRPS(finalHeading, power);
        }
    } else {
        if(finalHeading > RPS.Heading() && finalHeading < divAngle) {
            leftTurnRPS(finalHeading, power);
        } else {
            rightTurnRPS(finalHeading, power);
        }
    }
}

void checkRPSHeading(float heading) {
    turnRPS(heading, 5);
}

void RPSMoveTo(float x, float y, float power) {
    RPSPointTowards(x, y, 20);
    leftMotor.SetPercent(power);
    rightMotor.SetPercent(power);
    while(abs(x - RPS.X()) > 1 && abs(y - RPS.Y()) > 1);
    leftMotor.SetPercent(0);
    rightMotor.SetPercent(0);
    LCD.WriteRC(sqrt((x-RPS.X())*(x-RPS.X()) + (y-RPS.Y())*(y-RPS.Y())), 10, 4);
}

void RPSPointTowards(float x, float y, int power) {
    float curX = RPS.X();
    float curY = RPS.Y();
    float theta = (180.0/3.14159)*atan2((double)(y - curY), (double)(x - curX));
    if(theta < 0) {
        theta += 360;
    }
    turnRPS(theta, power);

}

void RPSPointAway(float x, float y, int power) {
    float curX = RPS.X();
    float curY = RPS.Y();
    float theta = (180.0/3.14159)*atan2((double)(y - curY), (double)(x - curX));
    if(theta < 0) {
        theta += 360;
    }
    if(theta > 180) {
        theta -= 180;
    } else {
        theta += 180;
    }
    turnRPS(theta, power);

}

void waitForLight() {
    while(cdsCell.Value() > 1 + CDS_RED);
}

void displayColor() {
    if(cdsCell.Value() < .4 + CDS_RED) {
        LCD.SetBackgroundColor(RED);
        LCD.SetFontColor(BLUE);
        LCD.WriteLine("ITS RED! REEEEEDDDDDDDDD!!!!");
    } else {
        if(cdsCell.Value() < .5 + CDS_BLUE) {
           LCD.SetBackgroundColor(BLUE);
           LCD.SetFontColor(RED);
           LCD.WriteLine("It's blue.");
        } else {
           LCD.SetBackgroundColor(BLACK);
           LCD.SetFontColor(WHITE);
           LCD.WriteLine("No Color");
        }
    }
}

//1 for red, 2 for blue. 0 For none.
int getColorCDS() {
    if(cdsCell.Value() < .4 + CDS_RED) {
        return 1;
    } else if(cdsCell.Value() < .5 + CDS_BLUE) {
       return 2;
    } else {
       return 0;
    }
}

void play_music() {
    Buzzer.Tone(FEHBuzzer::A4, 200);
    Buzzer.Tone(FEHBuzzer::C5, 200);
    Buzzer.Tone(FEHBuzzer::A4, 150);
    Buzzer.Tone(FEHBuzzer::A4, 50);
    Buzzer.Tone(FEHBuzzer::D5, 100);
    Buzzer.Tone(FEHBuzzer::G4, 100);
    Buzzer.Tone(FEHBuzzer::F4, 100);
    Buzzer.Tone(FEHBuzzer::A4, 200);
    Buzzer.Tone(FEHBuzzer::E5, 200);
    Buzzer.Tone(FEHBuzzer::A4, 150);
    Buzzer.Tone(FEHBuzzer::A4, 50);
    Buzzer.Tone(FEHBuzzer::F5, 100);
    Buzzer.Tone(FEHBuzzer::E5, 100);
    Buzzer.Tone(FEHBuzzer::C4, 100);
    Buzzer.Tone(FEHBuzzer::A4, 100);
    Buzzer.Tone(FEHBuzzer::C5, 100);
    Buzzer.Tone(FEHBuzzer::A5, 100);
    Buzzer.Tone(FEHBuzzer::G4, 150);
    Buzzer.Tone(FEHBuzzer::G4, 50);
    Buzzer.Tone(FEHBuzzer::E4, 100);
    Buzzer.Tone(FEHBuzzer::B4, 100);
    Buzzer.Tone(FEHBuzzer::A4, 200);
}

void showOff() {
    encoderForward(20, 4*COUNTS_PER_INCH);
    leftMotor.SetPercent(-25.);
    rightMotor.SetPercent(25.);
}
