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

//My header file.
#include <robotdefinitions.h>

#define SPEED_MODE true

AnalogInputPin rightOpto(FEHIO::P2_0);
AnalogInputPin centerOpto(FEHIO::P2_1);
AnalogInputPin leftOpto(FEHIO::P2_2);

FEHMotor leftMotor(FEHMotor::Motor0, 12.0);
FEHMotor rightMotor(FEHMotor::Motor1, 12.0);
FEHMotor rollServo(FEHMotor::Motor2, 5.5);

DigitalEncoder leftEncoder(FEHIO::P0_2);
DigitalEncoder rightEncoder(FEHIO::P0_0);

AnalogInputPin cdsCell(FEHIO::P0_1);

//On the left. White wire in front.
FEHServo yawServo(FEHServo::Servo0);

DigitalInputPin left_switch(FEHIO::P0_7);
DigitalInputPin right_switch(FEHIO::P0_6);

DigitalOutputPin LEDS[8] = { DigitalOutputPin(FEHIO::P1_0),
                             DigitalOutputPin(FEHIO::P1_1),
                             DigitalOutputPin(FEHIO::P1_2),
                             DigitalOutputPin(FEHIO::P1_4),
                             DigitalOutputPin(FEHIO::P1_5),
                             DigitalOutputPin(FEHIO::P1_6),
                             DigitalOutputPin(FEHIO::P2_6),
                             DigitalOutputPin(FEHIO::P2_7)};

int main(void)
{
    //Go to the main menu after startup.
    yawServo.SetMin(SERVO_MIN);
    yawServo.SetMax(SERVO_MAX);
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
    char labels[NUM_MENU_BUTTONS][20] = {"RunCore", "CalRPS", "CalServ", "LEDTest", "CheckVitals", "Test"};
    //Draw a 3x2 array of icons with red text and blue borders.
    FEHIcon::DrawIconArray(buttons, 3, 2, 20, 20, 60, 60, labels, MIDNIGHTBLUE, TEXT_COLOR);
    //Coordinates for screen touches.
    float x,y;
    while (true) {
            //Wait for the user to touch the screen
            LCD.Touch(&x,&y);
            //Check if the final run button is pressed
            if(buttons[0].Pressed(x,y,0)) {
                run_final();
            } else if(buttons[1].Pressed(x,y,0)) { //Calibrate RPS
                RPS.InitializeTouchMenu();
                FEHIcon::DrawIconArray(buttons, 3, 2, 20, 20, 60, 60, labels, MIDNIGHTBLUE, TEXT_COLOR);
            } else if(buttons[2].Pressed(x,y,0)) { //Currently shows off the servo motor
                test_1();
            } else if(buttons[3].Pressed(x,y,0)) { //Should blink LEDs sequentially
                while(true) {
                    for(int i = 0; i < 8; i++) { //Couldn't find size of LEDS so just said 4.
                        Sleep(0.15);
                        LEDS[i].Toggle();
                    }
                }
            } else if(buttons[4].Pressed(x,y,0)) { //Prints out available inputs to make sure code is working.
                Sleep(1);
                LCD.SetBackgroundColor(WHITE);
                LCD.SetFontColor(BLACK);
                float i = 0;
                while(!LCD.Touch(&x,&y)) {
                    LCD.WriteRC("Left:", 0, 0);
                    LCD.WriteRC("Center:", 1, 0);
                    LCD.WriteRC("Right:", 2, 0);
                    LCD.WriteRC(leftOpto.Value(), 0, 6);
                    LCD.WriteRC(centerOpto.Value(), 1, 8);
                    LCD.WriteRC(rightOpto.Value(), 2, 7);
                    LCD.WriteRC("RPS X:", 3, 0);
                    LCD.WriteRC("RPS Y:", 4, 0);
                    LCD.WriteRC("RPS Heading:", 5, 0);
                    LCD.WriteRC(RPS.X(), 3, 7);
                    LCD.WriteRC(RPS.Y(), 4, 7);
                    LCD.WriteRC(RPS.Heading(), 5, 13);
                    LCD.WriteRC("CdS Cell:", 6, 0);
                    LCD.WriteRC(cdsCell.Value(), 6, 10);
                    LCD.WriteRC("Left Bump:", 7, 0);
                    LCD.WriteRC(left_switch.Value(), 7, 11);
                    LCD.WriteRC("Right Bump:", 8, 0);
                    LCD.WriteRC(right_switch.Value(), 8, 12);
                    LCD.WriteRC("Left Shaft:", 9, 0);
                    LCD.WriteRC(leftEncoder.Counts(), 9, 12);
                    LCD.WriteRC("Right Shaft:", 10, 0);
                    LCD.WriteRC(rightEncoder.Counts(), 10, 13);
                    LCD.WriteRC("Battery Charge:", 11, 0);
                    LCD.WriteRC(Battery.Voltage(), 11, 16);
                    LCD.WriteRC(TimeNow(), 12, 16);
                }
            } else if(buttons[5].Pressed(x,y,0)) { //Test running into walls to diagnose Wall glitch
                yawServo.SetDegree(SAT_ANGLE);
                Sleep(0.2);
                encoderForward(40, (int)(0.5*COUNTS_PER_INCH));
                //S Turn Left into wall by satellite dish
                accelForwardSin(30, 80, 45, 95, (int)(1.5*COUNTS_PER_INCH));
                accelForwardSin(45, 95, 30, 80, (int)(1.3*COUNTS_PER_INCH));
                //Turn left a little to avoid hitting the dish.
                //encoderForwardWall(40, 45, 20*COUNTS_PER_INCH, 0.8);
                LCD.WriteRC("Before Bump", 13, 0);
                encoderForwardWall(30, 30, 16*COUNTS_PER_INCH, 2.5);
                Sleep(0.3);
                encoderForward(-30, -30, 8*COUNTS_PER_INCH);
            }
    }
}

//Grabs core, deposits core in correct bucket based on color, and hits final button
void flippy_thing(int color) {
    Sleep(0.2);
    switch(color) {
    //Red
    case 1: { //Limit scope of case to declare variable inside.
        //Pick up core
        yawServo.SetDegree(LEVER_ANGLE  - 5);
        Sleep(0.5);
        encoderForward(30, (int)(3.5*COUNTS_PER_INCH));
        accelForwardSin(40, 90, 20, 60, 10*COUNTS_PER_INCH - 15);
        encoderForward(47, 40, 10*COUNTS_PER_INCH);
        yawServo.SetDegree(TURN_ANGLE);
        accelForwardSin(40, 70, 7, 17, (int)(13.2*COUNTS_PER_INCH));
        rollServo.SetPercent(-100.0); //Moved back
        encoderForwardWall(-43, -40, 10*COUNTS_PER_INCH, 1.5);
        Sleep(0.3);
        rollServo.Stop();
        LEDS[7].Toggle();
        //Curve into final button
        leftMotor.SetPercent(58);
        rightMotor.SetPercent(70);
        //Sleep 0.75
        //If there are leds plugged in, this oscillates them while the robot makes
        //the final turn, else it just makes the last turn normally.
        double startTime = TimeNow();
            while(TimeNow() - startTime < 0.75) {
                int index = ((int)((TimeNow()-startTime)/0.05))%8;
                LEDS[index].Toggle();
            }
            leftMotor.SetPercent(70);
            rightMotor.SetPercent(60);
            startTime = TimeNow();
            while(TimeNow() - startTime < 1.2) {
                int index = ((int)((TimeNow()-startTime)/0.05))%8;
                LEDS[index].Toggle();
            }
        }
        encoderForward(-30, 4*COUNTS_PER_INCH);
        encoderForwardWall(40, 30, 8*COUNTS_PER_INCH, 2.0);
        break;
    //Blue
    case 2:
        //Pick up core
        yawServo.SetDegree(LEVER_ANGLE - 5);
        Sleep(0.5);
        //Move down ramp
        encoderForward(30, (int)(3.5*COUNTS_PER_INCH));
        accelForwardSin(40, 90, 20, 61, 10*COUNTS_PER_INCH - 15);
        encoderForward(47, 40, 12*COUNTS_PER_INCH);
        //Flip around
        accelLeftSin(20, 80, 214*COUNTS_PER_90_DEGREES/90);
        //Turn arm and move towards bucket
        yawServo.SetDegree(TURN_ANGLE);
        rollServo.SetPercent(-100.0);
        encoderForwardWall(-30, -30, 8*COUNTS_PER_90_DEGREES, 1.7);
        Sleep(0.1);
        rollServo.Stop();
        //After depositing core, move to final button
        encoderForward(30, 30, 4*COUNTS_PER_INCH);
        encoderLeftTurn(30, COUNTS_PER_90_DEGREES);
        encoderForwardWall(35, 35, 18*COUNTS_PER_INCH, 2.0);
        encoderForward(-20, 1*COUNTS_PER_INCH);
        encoderRightTurn(30, COUNTS_PER_90_DEGREES);
        encoderForwardWall(50, 50, 16*COUNTS_PER_INCH, 3.0);
        break;
    }
}

//The main code for running the robot and completing the course.
void run_final() {
    LCD.SetBackgroundColor(WHITE);
    LCD.SetFontColor(WHITE);
    //Wait for start light to turn on.
    waitForLight();
    LEDS[0].Toggle();
    //Move forwards out of the start
    accelForwardSin(20, 60, (int)(5.5*COUNTS_PER_INCH));
    encoderForward(20, (int)(1.5*COUNTS_PER_INCH));
    Sleep(0.1);
    accelLeftSin(20, 40, COUNTS_PER_90_DEGREES);
    //Turn left and move to light
    encoderForward(20, (int)(2.2*COUNTS_PER_INCH));
    Sleep(0.3);
    displayColor();
    //Save color in a variable for when we drop off the core.
    int color = getColorCDS();
    if(color < 1 || color > 2) {
        color = 1;
    }
    LEDS[1].Toggle();
    yawServo.SetDegree(SAT_ANGLE);
    Sleep(0.2);
    encoderForward(40, (int)(0.5*COUNTS_PER_INCH));
    //S curve left into the wall near the satellite dish
    accelForwardSin(30, 80, 45, 95, (int)(1.5*COUNTS_PER_INCH));
    //Hit wall: This is where the wall glitch happens. Reason unknown
    LCD.WriteRC("Before Bump", 13, 0);
    encoderForwardWall(35, 35, 28*COUNTS_PER_INCH, 2.5);
    Sleep(0.3);
    LCD.WriteRC("After Bump", 13, 0);
    yawServo.SetDegree(SAT_ANGLE);
    Sleep(0.2);
    LCD.WriteRC("After Arm", 13, 0);
    encoderForward(-26, (int)(1.8*COUNTS_PER_INCH));
    LCD.WriteRC("After Back", 13, 0);
    encoderLeftTurn(30, COUNTS_PER_90_DEGREES);
    encoderForwardWall(-45, -45, 8*COUNTS_PER_INCH, 1.0);
    yawServo.SetDegree(SAT_ANGLE + 20);
    LEDS[2].Toggle();
    if(!SPEED_MODE) {
        encoderForward(20, (int)(1.5*COUNTS_PER_INCH));
        encoderRightTurn(30, 10*COUNTS_PER_90_DEGREES/9);
        //Line up against wall after turning dish
        bumpWall(40);
        yawServo.SetDegree(0);
        //Move into position to move up ramp
        encoderForward(-30, -30, (int)(7*COUNTS_PER_INCH));
        encoderLeftTurn(35, COUNTS_PER_90_DEGREES);
        //Move up ramp
        LCD.WriteRC("Going up ramp.", 13, 0);
        accelForwardSin(45, 93, 40, 90, (int)(17.75*COUNTS_PER_INCH));
        LEDS[3].Toggle();
        Sleep(0.1);
        //Back up to correct y position if needed.
        leftMotor.SetPercent(-15);
        rightMotor.SetPercent(-15);
        while(RPS.Y()> 48.5);
        leftMotor.Stop();
        rightMotor.Stop();
        yawServo.SetDegree(PARALLEL_ANGLE);
        //Turn right to line up with lever.
        accelRightSin(20, 50, 22*COUNTS_PER_90_DEGREES/18);
    } else {
        //Move arm up
        yawServo.SetDegree(SAT_ANGLE + 26);
        //Move past satellite dish
        encoderForward(40, 2*COUNTS_PER_INCH);
        //S curve up the ramp
        accelForwardSin(10, 20, 50, 80, 4*COUNTS_PER_INCH);
        encoderForward(50, (int)(1.0*COUNTS_PER_INCH));
        accelForwardSin(60, 90, 10, 20, 4.0*COUNTS_PER_INCH);
        encoderForward(60, (int)(11.0*COUNTS_PER_INCH));
        LEDS[3].Toggle();
        //Turn right to line up with  lever
        accelRightSin(20, 50, 135*COUNTS_PER_90_DEGREES/90); //This bothers me...
    }

    LCD.WriteRC("Going to lever.", 13, 0);

    Sleep(0.1);
    //Corrects left to make sure it hits the lever
    encoderForwardWall(-40, -40, 8*COUNTS_PER_INCH, 0.8);
    rightMotor.SetPercent(30);
    Sleep(0.1);
    rightMotor.Stop();
    encoderForwardWall(-35, -35, 8*COUNTS_PER_INCH, 0.5);
    //Move forwards slightly to get in position
    encoderForwardWall(20, 20, (int)(0.5*COUNTS_PER_INCH), 0.3);\
    //Hit lever and move forwards to flip it.
    yawServo.SetDegree(LEVER_ANGLE);
    encoderForward(30, 3*COUNTS_PER_INCH);
    yawServo.SetDegree(PARALLEL_ANGLE);
    LEDS[4].Toggle();

    //Pivot to Button
    accelForwardSin(10, 40, 55, 100, 7*COUNTS_PER_INCH);

    LCD.WriteRC("Going to Button", 13, 0);
    //Keep button pressed
    encoderForwardWall(40, 40, 24*COUNTS_PER_INCH, 6.0);
    encoderForward(-40, (int)(1.0*COUNTS_PER_INCH));
    LEDS[5].Toggle();

    //Pull Core

    //Line up with the wall near the button
    accelRightSin(20, 45, 80*COUNTS_PER_90_DEGREES/90);
    bumpWall(50);
    Sleep(0.25);
    LCD.WriteRC("Pulling Core.", 13, 0);
    //Turn claw into correct position
    yawServo.SetDegree(TURN_ANGLE);
    rollServo.SetPercent(50.0);
    //Back up into line
    encoderForward(-50, (int)(5.5*COUNTS_PER_INCH));
    rollServo.Stop();
    yawServo.SetDegree(0);
    //Move backwards indefinitely
    LCD.WriteRC("Finding the Line", 13, 0);
    leftMotor.SetPercent(-20);
    rightMotor.SetPercent(-20);
    //Wait for center line to appear under optosensor
    while(centerOpto.Value() > CENTER_OPTO_ORANGE + 0.8);
    //When the center line appears, move a little bit farther to line the claw up with the line.
    encoderForward(-25, (int)(1.55*COUNTS_PER_INCH));
    leftMotor.Stop();
    rightMotor.Stop();
    Sleep(0.2);
    //Turn roughly 45 degrees to line up exactly with the core.
    encoderRightTurn(30, (int)(40*COUNTS_PER_90_DEGREES/90));
    encoderForwardWall(-40, -40, 20*COUNTS_PER_INCH, 0.8);
    encoderForward(30,(int)(0.4*COUNTS_PER_INCH));
    Sleep(0.2);
    LEDS[6].Toggle();

    //If the robot is running the fast run, go into the fast run method.
    if(SPEED_MODE) {
        flippy_thing(color);
    }

    //Else continue on with the slow code.

    //Pick up core
    yawServo.SetDegree(LEVER_ANGLE);
    Sleep(0.3);
    //Curve down the ramp
    encoderForward(30, 4*COUNTS_PER_INCH);
    accelForwardSin(40, 90, 20, 65, 10*COUNTS_PER_INCH - 20);
    encoderForward(40, 14*COUNTS_PER_INCH);
    encoderLeftTurn(35, 2*COUNTS_PER_90_DEGREES/3);
    //Turn facing the left side of the course to prepare to drop the core off.
    leftTurnRPS(350, 15);


    switch(color) {
        //NONE, measure again
        case 0:

        break;
        //RED
        case 1:
            yawServo.SetDegree(TURN_ANGLE);
            //Move 7 inches towards red bucket.
            encoderForward(-20, (int)(6.4*COUNTS_PER_INCH));
            //Turn towards bucket
            encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
            //Move until it hits the bucket
            encoderForwardWall(-20, -20, 7*COUNTS_PER_INCH, 2.0);
            //Turn the arm until the core is dropped
            rollServo.SetPercent(-100.0);
            Sleep(1.8);
            rollServo.Stop();
            //Curve into finish
            leftMotor.SetPercent(50);
            rightMotor.SetPercent(75);
            Sleep(0.8);
            leftMotor.SetPercent(70);
            rightMotor.SetPercent(50);
            Sleep(0.7);
        break;
        //BLUE
        case 2:
            //Move arm into position to turn
            yawServo.SetDegree(TURN_ANGLE);
            //Move back a little bit to get in the correct x position.
            encoderForward(-30, (int)(1.5*COUNTS_PER_INCH));
            //Face towards bucket
            encoderLeftTurn(20, COUNTS_PER_90_DEGREES - 10);
            //Move back until the robot hits the bucket
            encoderForwardWall(-30, -30, 7*COUNTS_PER_INCH, 2.0);
            //Turn the arm until the core is dropped
            rollServo.SetPercent(-100.0);
            Sleep(1.8);
            rollServo.Stop();
            //Move forward and run into the wall
            encoderForward(20, 4*COUNTS_PER_INCH);
            encoderLeftTurn(30, COUNTS_PER_90_DEGREES);
            encoderForwardWall(40, 40, 10*COUNTS_PER_INCH, 1.6);
        break;
    }

    //Run into stop button from wall
    encoderForward(-30, 1*COUNTS_PER_INCH);
    encoderRightTurn(25, COUNTS_PER_90_DEGREES);
    encoderForward(50, 10*COUNTS_PER_INCH);
}

//An optional test to make sure the motors are driving forward and straight.
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

//A test specificalle made to show off the arm.
//Moves the arm to different angles, rotates it, and rotates it back.
void test_1() {
    yawServo.SetDegree(LEVER_ANGLE);
    Sleep(.5);
    yawServo.SetDegree(0.);
    Sleep(.5);
    yawServo.SetDegree(SAT_ANGLE);
    Sleep(.5);
    yawServo.SetDegree(TURN_ANGLE);
    Sleep(.5);
    rollServo.SetPercent(-100.0); //Moved back
    Sleep(1.0);
    rollServo.Stop();
    Sleep(1.0);
    yawServo.SetDegree(0.);
    Sleep(1.5);
    yawServo.SetDegree(TURN_ANGLE);
    Sleep(.5);
    rollServo.SetPercent(100.0); //Moved back
    Sleep(1.0);
    rollServo.Stop();
    yawServo.SetDegree(0.);
    Sleep(1.5);
    LCD.WriteLine("WUZZUP");
}

//Placeholder - currently runs performance test 4
void test_2() {
    waitForLight();
    moveStartToSat();
}


//PT1 (Performance Test 1)
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

//PT2
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

//PT3
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
    RPSMoveTo(22, 50, 25);
    RPSPointAway(8., 64., 10);
    Sleep(0.5);
    follow_straight_line(-20);
    yawServo.SetDegree(LEVER_ANGLE);
    Sleep(0.3);
    encoderForward(30, 9*COUNTS_PER_INCH);
    Sleep(0.5);
    encoderForward(20, (int)(0.5*COUNTS_PER_INCH));
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
}

//PT4
void moveStartToSat() {
    encoderForwardWall(30, 30, 6*COUNTS_PER_INCH, 3.0);
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    encoderForwardWall(40, 40, 60*COUNTS_PER_INCH, 5.5);
    encoderForward(-20, 2*COUNTS_PER_INCH);
    encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
    yawServo.SetDegree(SAT_ANGLE);
    encoderForwardWall(-20, -20, 8*COUNTS_PER_INCH, 3.0);
    encoderForward(20, (int)(1.5*COUNTS_PER_INCH));
    encoderRightTurn(20, COUNTS_PER_90_DEGREES);
    encoderForward(-20, 3*COUNTS_PER_INCH);
    Sleep(1.0);
    leftTurnRPS(90.0, 15);
    encoderForward(20, 4*COUNTS_PER_INCH);
    if(RPS.SatellitePercent() < 0) {
        encoderRightTurn(20, COUNTS_PER_90_DEGREES);
        encoderForwardWall(30, 30, 8*COUNTS_PER_INCH, 3.0);
        encoderForward(-20, 2*COUNTS_PER_INCH);
        encoderLeftTurn(20, COUNTS_PER_90_DEGREES);
        encoderForwardWall(-20, -20, 8*COUNTS_PER_INCH, 3.0);
        encoderForward(20, (int)(1.5*COUNTS_PER_INCH));
        encoderRightTurn(20, COUNTS_PER_90_DEGREES);
        Sleep(1.0);
        leftTurnRPS(90.0, 15);
        encoderForward(20, 4*COUNTS_PER_INCH);
    }
    //Up and down the ramp
    RPSMoveTo(22, 50, 40);
    encoderForward(-30, -30, 5*COUNTS_PER_INCH);
    RPSMoveTo(30, 16, 30);
    turnRPS(175, 15);
    encoderForwardWall(30, 30, 60*COUNTS_PER_INCH, 3.5);
    encoderForward(-20, (int)(1.5*COUNTS_PER_INCH));
    encoderRightTurn(20, COUNTS_PER_90_DEGREES);
    encoderForwardWall(40, 40, 12*COUNTS_PER_INCH, 1.5);
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

//Orange line following
//Left optosensor is from robots point of view. This method is optimized to drive backwards into the target.
void follow_straight_line(int power) {
    rightMotor.SetPercent(power);
    leftMotor.SetPercent(power);
    int state = NOT_ON_LINE;
    int turnPower = 8;
    double startTime = TimeNow();
    while(true) {
        if(TimeNow() - startTime > 4.2) {
            break;
        }
        LCD.SetBackgroundColor(BLACK);
        LCD.SetFontColor(WHITE);
        LCD.WriteRC("LeftOpto:", 3, 0);
        LCD.WriteRC("MidOpto:", 4, 0);
        LCD.WriteRC("RightOpto:", 5, 0);
        LCD.WriteRC(leftOpto.Value(), 3, 9);
        LCD.WriteRC(centerOpto.Value(), 4, 8);
        LCD.WriteRC(rightOpto.Value(), 5, 10);
        LCD.WriteRC(TimeNow(), 6, 0);
        LCD.WriteRC(state, 7, 0);
        if(leftOpto.Value() < LEFT_OPTO_ORANGE_BG - 1.0)
            state = LINE_ON_LEFT;
        if(rightOpto.Value() < RIGHT_OPTO_ORANGE_BG - 1.0)
            state = LINE_ON_RIGHT;
        if(centerOpto.Value() < CENTER_OPTO_ORANGE_BG - 1.0) {
            if(leftOpto.Value() < LEFT_OPTO_ORANGE_BG - 1.0)
                state = ON_LINE_SECOND;
            if(rightOpto.Value() < RIGHT_OPTO_ORANGE_BG - 1.0)
                state = ON_LINE_FIRST;
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
            leftMotor.SetPercent(0);

        break;
        case LINE_ON_RIGHT:
            rightMotor.SetPercent(0);
            leftMotor.SetPercent(power - 6);
        break;
        case ON_LINE_FIRST:
            rightMotor.SetPercent(power + 2);
            leftMotor.SetPercent(power - 2);
        break;
        case ON_LINE_SECOND:
            rightMotor.SetPercent(power - 2);
            leftMotor.SetPercent(power + 2);
        break;
        default:
        break;
        }
    }
}

//Code from line following exploration just in case
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

//Moves both motors forward by a specified power and distance.
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

//Moves both motors forward by a specified and distance, and power for each .
void encoderForward(int leftPercent, int rightPercent, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    rightMotor.SetPercent(rightPercent);
    leftMotor.SetPercent(leftPercent);
    float startTime = TimeNow();
    //Moves on after a few seconds, just in case.
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts && TimeNow() - startTime < TIME_UNTIL_STOP) {
    }

    rightMotor.Stop();
    leftMotor.Stop();
}

//Accelerates forward with velocity in the form of a sine wave.
void accelForwardSin(float minPercent, float maxPercent, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();
    leftMotor.SetPercent(minPercent);
    rightMotor.SetPercent(maxPercent);
    float startTime = TimeNow();
    while ((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts && TimeNow() - startTime < TIME_UNTIL_STOP) {
        //set motor velocities according to a function based on their elapsed counts
        float motorPercent = (maxPercent - minPercent) * sin(((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0) * 3.14159/((float)counts)) + minPercent;
        leftMotor.SetPercent(motorPercent);
        rightMotor.SetPercent(motorPercent);
    }
    rightMotor.Stop();
    leftMotor.Stop();
}

//Accelerates with velocity in a sine wave, with independent powers for each wheel.
void accelForwardSin(float minPercentLeft, float maxPercentLeft, float minPercentRight, float maxPercentRight, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();
    leftMotor.SetPercent(minPercentLeft);
    rightMotor.SetPercent(maxPercentRight);
    float startTime = TimeNow();
    while ((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts && TimeNow() - startTime < TIME_UNTIL_STOP) {
        leftMotor.SetPercent((maxPercentLeft - minPercentLeft) * sin(((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0) * 3.14159/((float)counts)) + minPercentLeft);
        rightMotor.SetPercent((maxPercentRight - minPercentRight) * sin(((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0) * 3.14159/((float)counts)) + minPercentRight);
    }
    rightMotor.Stop();
    leftMotor.Stop();
}

//Accelerates to the maximum percent in sine wave form, stays at that speed for highCounts counts, and then decellerates down to minPercent.
void accelForwardStay(float minPercentLeft, float maxPercentLeft, float minPercentRight, float maxPercentRight, int counts, int highCounts) {
    int highStart = (counts - highCounts)/2;
    int highEnd = counts - highStart;
    leftMotor.SetPercent(-minPercentLeft);
    rightMotor.SetPercent(minPercentRight);
    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts) {
        int elapsedCounts = (leftEncoder.Counts()+rightEncoder.Counts())/2;
        //Stay at the maximum for highCounts number of counts.
        if(elapsedCounts > highStart && elapsedCounts < highEnd) {
            leftMotor.SetPercent(maxPercentLeft);
            rightMotor.SetPercent(maxPercentRight);
        } else if(elapsedCounts < highStart) { //If starting, sine accelerate
            float motorPercentLeft = (maxPercentLeft - minPercentLeft) * sin((float)elapsedCounts * 3.14159/((float)highStart*2)) + minPercentLeft;
            float motorPercentRight = (maxPercentRight - minPercentRight) * sin((float)elapsedCounts * 3.14159/((float)highStart*2)) + minPercentRight;
            leftMotor.SetPercent(motorPercentLeft);
            rightMotor.SetPercent(motorPercentRight);
        } else { //If ending, sine decelerate
            float motorPercentLeft = (maxPercentLeft - minPercentLeft) * sin(((float)(elapsedCounts - highCounts)) * 3.14159/((float)highStart*2)) + minPercentLeft;
            float motorPercentRight = (maxPercentRight - minPercentRight) * sin(((float)(elapsedCounts - highCounts)) * 3.14159/((float)highStart*2)) + minPercentRight;
            leftMotor.SetPercent(motorPercentLeft);
            rightMotor.SetPercent(motorPercentRight);
        }
    }
    rightMotor.Stop();
    leftMotor.Stop();
}

//A left turn using sine wave acceleration
void accelLeftSin(int minPercent, int maxPercent, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();
    leftMotor.SetPercent(-minPercent);
    rightMotor.SetPercent(minPercent);
    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts) {
        float motorPercent = (maxPercent - minPercent) * sin(((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0) * 3.14159/((float)counts)) + minPercent;
        leftMotor.SetPercent(-motorPercent);
        rightMotor.SetPercent(motorPercent);
    }
    rightMotor.Stop();
    leftMotor.Stop();
}

//A right turn using sine wave acceleration
void accelRightSin(int minPercent, int maxPercent, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();
    leftMotor.SetPercent(minPercent);
    rightMotor.SetPercent(-minPercent);
    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts) {
        float motorPercent = (maxPercent - minPercent) * sin(((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0) * 3.14159/((float)counts)) + minPercent;
        leftMotor.SetPercent(motorPercent);
        rightMotor.SetPercent(-motorPercent);
    }
    rightMotor.Stop();
    leftMotor.Stop();
}

//Bump into the wall at a specified motor power, and then return.
void bumpWall(int percent) {
    double startTime = TimeNow();
    //Wait for both bump switches to be pressed or half a second to elapse.
    while((left_switch.Value() || right_switch.Value()) && TimeNow() - startTime < 0.5) {
        //If only one pressed is pressed, pivot so that both are pressed.
        if(left_switch.Value()) {
            leftMotor.SetPercent(percent);
        } else {
            leftMotor.SetPercent(-15);
        }
        if(right_switch.Value()) {
            rightMotor.SetPercent(percent);
        } else {
            rightMotor.SetPercent(-15);
        }
    }
    leftMotor.Stop();
    rightMotor.Stop();
}

//This one is only used for the times when it hits a wall and stops, to make sure it can back up.
void encoderForwardWall(int leftPercent, int rightPercent, int counts, double backTime) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();
    rightMotor.SetPercent(rightPercent);
    leftMotor.SetPercent(leftPercent);
    float startTime = TimeNow();
    //Moves on after specified time
    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2. < counts && (TimeNow() - startTime) < backTime) {
        LCD.SetBackgroundColor(WHITE);
        LCD.SetFontColor(BLACK);
        LCD.WriteRC(leftEncoder.Counts(), 0, 0);
        LCD.WriteRC(rightEncoder.Counts(), 1, 0);
        LCD.WriteRC(TimeNow() - startTime, 2, 0);
    }
    rightMotor.Stop();
    leftMotor.Stop();
}

//Basic stationary left turn using shaft encoding.
void encoderLeftTurn(int motorPower, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    leftMotor.SetPercent(-motorPower);
    rightMotor.SetPercent(motorPower);

    while((leftEncoder.Counts()+rightEncoder.Counts())/2 < counts);
    rightMotor.Stop();
    leftMotor.Stop();
}

//Basic stationary right turn using shaft encoding.
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

//CCW Turn using RPS
void leftTurnRPS(float finalHeading, float power) {
    float startHeading = RPS.Heading();
    //Check if the finalHeading is near zero to avoid edge cases.
    if(finalHeading < 1.0) {
        finalHeading = 1.0;
    }
    leftMotor.SetPercent(-power);
    rightMotor.SetPercent(power);
    //Make sure robot goes past heading even if it has to pass zero degrees
    if(finalHeading < startHeading) {
        while(RPS.Heading() < finalHeading || RPS.Heading() > startHeading - 2.);
    } else {
        while(RPS.Heading() < finalHeading);
    }
    rightMotor.SetPercent(0);
    leftMotor.SetPercent(0);
}

//Clockwise turn using RPS
void rightTurnRPS(float finalHeading, float power) {
    float startHeading = RPS.Heading();
    //Check if the finalHeading is near zero to avoid edge cases.
    if(finalHeading < 1.0) {
        finalHeading = 359.9;
    }
    leftMotor.SetPercent(power);
    rightMotor.SetPercent(-power);
    //Make sure robot goes past heading even if it has to pass zero degrees
    if(finalHeading > startHeading) {
        while(RPS.Heading() > finalHeading || RPS.Heading() < startHeading + 2.);
    } else {
        while(RPS.Heading() > finalHeading);
    }
    rightMotor.SetPercent(0);
    leftMotor.SetPercent(0);
}

//Selects optimal direction to turn and turns to the specified heading.
void turnRPS(float finalHeading, float power) {
    LCD.WriteRC(RPS.Heading(), 1, 1);
    LCD.WriteRC(finalHeading, 2, 1);
    bool over180 = false;
    //Check if the robot is over 180 degrees, if so, make sure it turns the corret direction.
    float divAngle = 180. + RPS.Heading();
    if(divAngle > 359.9) {
        divAngle -= 360.;
        over180 = true;
    }
    //Turn correct direction
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

//Turn to a specified point and then move towards it at the specified power.
void RPSMoveTo(float x, float y, float power) {
    RPSPointTowards(x, y, 20);
    leftMotor.SetPercent(power);
    rightMotor.SetPercent(power);
    while(abs(x - RPS.X()) > 1 && abs(y - RPS.Y()) > 1);
    leftMotor.SetPercent(0);
    rightMotor.SetPercent(0);
    LCD.WriteRC(sqrt((x-RPS.X())*(x-RPS.X()) + (y-RPS.Y())*(y-RPS.Y())), 10, 4);
}

//Point towards a specified point using trig.
void RPSPointTowards(float x, float y, int power) {
    float curX = RPS.X();
    float curY = RPS.Y();
    float theta = (180.0/3.14159)*atan2((double)(y - curY), (double)(x - curX));
    if(theta < 0) {
        theta += 360;
    }
    turnRPS(theta, power);

}

//Point away from a specified point using trig.
void RPSPointAway(float x, float y, int power) {
    float curX = RPS.X();
    float curY = RPS.Y();
    float theta = (180.0/3.14159)*atan2((double)(y - curY), (double)(x - curX));
    if(theta < 0) {
        theta += 360;
    } else {
        theta += 180;
    }
    turnRPS(theta, power);
}

//Wait for the CdS cell to turn on.
void waitForLight() {
    while(cdsCell.Value() > 1 + CDS_RED);
}

//Display the color read from the optosensor on the screen.
void displayColor() {
    //Check if the CdS cell reads under the max red value.
    if(cdsCell.Value() < 1.0 + CDS_RED) {
        LCD.Clear(RED);
        LCD.SetBackgroundColor(RED);
        LCD.SetFontColor(BLUE);
        LCD.WriteRC("ITS RED! REEEEEDDDDDDDDD!!!!",0,1);
    } else { //Defaults to blue, but still prints what it reads on the screen.
        if(cdsCell.Value() < 1.0 + CDS_BLUE) {
            LCD.Clear(BLUE);
           LCD.SetBackgroundColor(BLUE);
           LCD.SetFontColor(RED);
           LCD.WriteRC("It's blue.", 0, 1);
        } else {
           LCD.SetBackgroundColor(BLACK);
           LCD.SetFontColor(WHITE);
           LCD.WriteRC("No Color", 0, 1);
        }
    }
    LCD.WriteRC(cdsCell.Value(), 4, 1);
}

//1 for red, 2 for blue. 0 For none.
int getColorCDS() {
    //returns red if under max red value, else it defaults to blue.
    if(cdsCell.Value() < 1.0 + CDS_RED) {
        return 1;
    } else if(cdsCell.Value() < 1.0 + CDS_BLUE) {
       return 2;
    } else {
       return 2;
    }
}

//Plays the beverly hills cop theme. Just in case.
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
