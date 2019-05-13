#include <iostream>
#include <time.h>
#include "LibE101/E101.h"
#include "avc.h"
using namespace std;

// Initialize AVC to run the specified quadrant
AVC::AVC(int q) {
    quadrant = q;
}

void AVC::openGate() {
    // set quadrant to 2 once gate is open
}

void AVC::followLine() {
    while (quadrant == 2) {
        take_picture();
        if (checkRed()) {
            quadrant = 4;
        } else {
            getBlackPx();
            calcError();

            // calculate error
            // get change in time
            // calculate kd
            // calculate adjustment
            // set motors
            // ???
        }
    }
}

void AVC::findDuck() {
    // look for a red duck, green duck and blue duck
    // set Quadrant to 5 when done
}

// Check for red spot in middle of camera indicating end of quadrant 3 and start of quadrant 4
bool AVC::checkRed() {
    int numRedPx = 0;
    for (int col = CAMERAWIDTH/4; col < CAMERAWIDTH*3/4; col++) {
        int red = get_pixel(SCANNEDROW, col, 0);
        int green = get_pixel(SCANNEDROW, col, 1);
        int blue = get_pixel(SCANNEDROW, col, 2);

        if ((2.0 * red)  / (green + blue) > 2.5) {
            numRedPx += 1;
        }
    }
    return 2.0 * numRedPx / CAMERAWIDTH > 0.7;
}

void AVC::getBlackPx() {
    double threshold = calcThreshold();
    for (int col = 0; col < CAMERAWIDTH; col++) {
        int whiteness = get_pixel(SCANNEDROW, col, 3);

        if (whiteness < threshold) {
            blackPx[col] = 1;
        } else {
            blackPx[col] = 0;
        }
    }
}

double AVC::calcThreshold() {
    int min = INT_MAX;
    int max = INT_MIN;
    for (int col = 0; col < CAMERAWIDTH; col++) {
        int whiteness = get_pixel(SCANNEDROW, col, 3);
        if (whiteness < min) {
            min = whiteness;
        }
        if (whiteness > max) {
            max = whiteness;
        }
    }
    return (min+max)/2.0;
}

void AVC::calcError() {
    lastError = error; // record last measurement
    for (int i = 0; i < CAMERAWIDTH; i++) {
        error += blackPx[i] * (i - MIDDLECOL);
    }
}

void AVC::setMotors(int left, int right) {
    vLeft = left;
    vRight = right;
    set_motors(LEFTMOTOR, vLeft);
    set_motors(RIGHTMOTOR, vRight);
    hardware_exchange();
}