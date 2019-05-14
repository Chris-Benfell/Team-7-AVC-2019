#include <iostream>
#include <time.h>
#include "E101.h"
#include "avc.h"
using namespace std;

// Initialize AVC class, hardware, etc..
// Run code to open gate (Q1) (IP: 130.195.6.196, Port: 1024)
// Then run line following code (Q2 & Q3)
// once line following is done (Found red spot)
// Tilt camera and look for red/green/blue duck/cylinder (Q4)
int main() {
    AVC robot(0);

    if (robot.quadrant == 1) {
        robot.openGate();
    } else if (robot.quadrant == 2) {
        robot.followLine();
    } else if (robot.quadrant == 4) {
        robot.findDuck();
    } else {
        robot.openGate();
        robot.followLine();
        robot.findDuck();
    }
}