#ifndef ROBOTDEFINITIONS_H
#define ROBOTDEFINITIONS_H

//Menu defines
#define NUM_MENU_BUTTONS 6

#define TEXT_COLOR 0x4f2f2f

//Testing speed
#define BASE_MOTOR_POWER 20

//Optosensor readings
#define LEFT_STRAIGHT .935
#define CENTER_STRAIGHT .585
#define RIGHT_STRAIGHT .735
#define LEFT_BG 1.6
#define CENTER_BG 1.33
#define RIGHT_BG 1.58
#define LEFT_CURVE 2.33
#define CENTER_CURVE 2.13
#define RIGHT_CURVE 2.2
#define LEFT_BGC .35
#define CENTER_BGC .198
#define RIGHT_BGC .221

//Optosensor line states
enum LineStates {
    LINE_ON_RIGHT,
    ON_LINE_FIRST,
    LINE_ON_LEFT,
    ON_LINE_SECOND,
    NOT_ON_LINE
};

//IGWAN encoder counts
#define COUNTS_PER_INCH 41
#define COUNTS_PER_90_DEGREES 200


void displayMenu();
void run_final();
void test_1();
void test_2();
void play_music();
void measure_optosensors();
void follow_straight_line();
void follow_curve();

void moveStartToButton();

void encoderForward(int percent, int counts);

void encoderLeftTurn(int motorPower, int counts);
void encoderRightTurn(int motorPower, int counts);

void leftTurnRPS(float finalheading, float power);
void rightTurnRPS(float finalheading, float power);

#endif // ROBOTDEFINITIONS_H
