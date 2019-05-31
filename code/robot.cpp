#include <iostream>
#include <time.h>
#include "E101.h"
#include "avc.hpp"
using namespace std;

// Initialize AVC class, hardware, etc..
// Run code to open gate (Q1) (IP: 130.195.6.196, Port: 1024)
// Then run line following code (Q2 & Q3)
// once line following is done (Found red spot)
// Tilt camera and look for red/green/blue duck/cylinder (Q4)
int main() {
    // Initialize robot
    AVC robot(3);

    // Run functions for each quadrant
    robot.openGate();
    robot.followLine();
    robot.findDucks();
    robot.dance();

    // Stop the robot
    stoph();
}
