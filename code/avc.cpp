#include <iostream>
#include <algorithm>
#include <time.h>
#include "E101.h"
#include "avc.h"
using namespace std;

// Initialize AVC
AVC::AVC(int q) {
    quadrant = q;
    init(0);
    hardware_exchange();
}

void AVC::openGate() {
    quadrant = 1;
    // send message??
    while (quadrant == 1) {
        // wait for message??
        // set quadrant to 2 once gate is open
    }
}

void AVC::followLine() {
    while (quadrant == 2) {
        take_picture();

        // Check if picture contains significant red indicating beginning of quadrant 4
        if (checkRed()) { // Significantly red
            quadrant = 4;
        } else { // Line following code

            // Turn image into an array of 1s (black) and 0s (white)
            getBlackPx();

            // Check if blackPx contains 1s to indicate line detected. No 1s indicates line has been lost
            if (find(begin(blackPx), end(blackPx), 1) != end(blackPx)) { // Found line

                // Calculate the error value
                calcError();

                // Check if error is equal to 0
                if (error != 0) { // Not 0

                    // Calculate motor adjustment
                    adjustment = (kp * error) + (kd * (error - errorPrev) / (time.tv_sec - timePrev.tv_sec));

                    // Set motors
                    setMotors("turn");

                } else { // error is equal to 0

                    // Go straight
                    setMotors("forward");
                }
            } else { // Line lost

                // Reverse until line is found
                setMotors("reverse");
            }
        }
    }
}

void AVC::findDuck() {
    while (quadrant == 4) {
        // look for a red duck, green duck and blue duck
        // set Quadrant to 5 when done

    }
    stoph(); // I think this is important?
}

// Check for red spot in middle of camera indicating end of quadrant 3 and start of quadrant 4
bool AVC::checkRed() {
    // Record the number of green pixels
    int numRedPx = 0;

    // Loop through all pixels in the image
    for (int col = CAMERAWIDTH/4; col < CAMERAWIDTH*3/4; col++) {

        // Get RGB values of the pixel
        int red = get_pixel(SCANNEDROW, col, 0);
        int green = get_pixel(SCANNEDROW, col, 1);
        int blue = get_pixel(SCANNEDROW, col, 2);

        // Compare ratio of R:G+B to threshold to determine if red
        if ((2.0 * red)  / (green + blue) > 2.5) { // Is red
            // Record red pixel
            numRedPx += 1;
        }
    }
    // Return proportion of image that is red. 0.6 indicates a 60%
    // red image which is likely to indicate a red spot
    return 2.0 * numRedPx / CAMERAWIDTH > 0.6;
}

// Get array of black pixels (1s and 0s)
void AVC::getBlackPx() {
    // Get threshold to determine whether a pixels is black or white
    double threshold = calcThreshold();

    // Loop through all pixels in the image
    for (int col = 0; col < CAMERAWIDTH; col++) {

        // Get pixels whiteness
        int whiteness = get_pixel(SCANNEDROW, col, 3);

        // Check if black or white
        if (whiteness < threshold) { // Is black
            // Set pixel as black in array
            blackPx[col] = 1;
        } else { // Is white
            // Set pixel as white in the array
            blackPx[col] = 0;
        }
    }
}

// Calculate threshold to determine if a pixel is black or white
double AVC::calcThreshold() {
    // Set maximum and minimum possible values
    int min = INT_MAX;
    int max = INT_MIN;

    // Loop through all pixels in the image
    for (int col = 0; col < CAMERAWIDTH; col++) {

        // Get pixels whiteness
        int whiteness = get_pixel(SCANNEDROW, col, 3);

        // Check if lower the current min
        if (whiteness < min) { // Is lower
            // Set as new min
            min = whiteness;
        }
        // Check if higher than current max
        if (whiteness > max) { // is higher
            // Set as new max
            max = whiteness;
        }
    }
    // Return average of the min and max. This is threshold.
    return (min+max)/2.0;
}

// Calculate the error value and record the time
void AVC::calcError() {
    errorPrev = error; // Record last error measurement
    timePrev = time; // Record last time measurement

    // Loop through array of black pixels
    for (int i = 0; i < CAMERAWIDTH; i++) {

        // Weight the pixels distance from the middle
        error += blackPx[i] * (i - MIDDLECOL);
    }

    // measure current time to measure dt later on
    clock_gettime(CLOCK_MONOTONIC, &time);
}

// Set the speed of each motor to the given value
void AVC::setMotors(string direction) {

    // Set appropriate speed values
    if (direction == "forward") {
        vLeft = LEFTDEFAULT;
        vRight = RIGHTDEFAULT;
    } else if (direction == "reverse") {
        vLeft = RIGHTDEFAULT;
        vRight = LEFTDEFAULT;
    } else if (direction == "turn") {
        vLeft = LEFTDEFAULT + adjustment;
        vRight = RIGHTDEFAULT + adjustment;
    }

    // Set Motors speed
    set_motors(LEFTMOTOR, vLeft);
    set_motors(RIGHTMOTOR, vRight);

    // update hardware
    hardware_exchange();
}

// debug function Run this to print out messages instead of cout<<""<<endl;
void AVC::debug(string string) {
    if (DEBUG == true) {
        cout<<string<<endl;
    }
}