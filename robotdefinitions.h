#ifndef ROBOTDEFINITIONS_H
#define ROBOTDEFINITIONS_H

//Menu defines
#define NUM_MENU_BUTTONS 6

#define TEXT_COLOR 0x4f2f2f

//Testing speed
#define BASE_MOTOR_POWER 20

#define TIME_UNTIL_STOP 7

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
#define COUNTS_PER_INCH 28
#define COUNTS_PER_90_DEGREES 113
//Left wheel turns more, so subtract this value from the counts for a right turn
#define RTURN_OFFSET 30

//CDS Cell readings for different lights with no filter.
#define CDS_RED .3
#define CDS_BLUE .8
#define CDS_NONE 3.0

//Base optosensor readings for each relevant surface.
#define LEFT_OPTO_ORANGE 1.5
#define CENTER_OPTO_ORANGE 1.
#define RIGHT_OPTO_ORANGE 1.3
#define LEFT_OPTO_ORANGE_BG 3.0
#define CENTER_OPTO_ORANGE_BG 3.0
#define RIGHT_OPTO_ORANGE_BG 3.0

#define LEFT_OPTO_BLACK 2.3
#define CENTER_OPTO_BLACK 2.1
#define RIGHT_OPTO_BLACK 2.3
#define LEFT_OPTO_BLACK_BG 1.2
#define CENTER_OPTO_BLACK_BG .8
#define RIGHT_OPTO_BLACK_BG 1.1

//Yaw Servo Defines
#define SERVO_MAX 2355
#define SERVO_MIN 500
#define PARALLEL_ANGLE 60.
#define LEVER_ANGLE 45.
#define TURN_ANGLE 87.

void post();
void displayMenu();
void run_final();
void test_1();
void test_2();
void motor_test();
void play_music();
void measure_optosensors();
void follow_straight_line(int power);
void follow_black_line(int power);
void follow_curve();

//Specific Navigation
void moveStartToButton();
void moveStartToLever();
void moveStartToCore();

//General navigation
void encoderForward(int percent, int counts);
void encoderForward(int leftPercent, int rightPercent, int counts);

void encoderLeftTurn(int motorPower, int counts);
void encoderRightTurn(int motorPower, int counts);
void encoderForwardWall(int leftPercent, int rightPercent, int counts, double backTime);

void leftTurnRPS(float finalheading, float power);
void rightTurnRPS(float finalheading, float power);
void turnRPS(float finalHeading, float power);
void RPSMoveTo(float x, float y, float power);
void RPSPointTowards(float x, float y, int power);
void RPSPointAway(float x, float y, int power);
void checkRPSHeading(float heading);

void waitForLight();
void displayColor();
int getColorCDS();

void showOff();

#endif // ROBOTDEFINITIONS_H
