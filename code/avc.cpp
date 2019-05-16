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

// Send a message to open the gate
void AVC::openGate() {
    // Connect to server (Use constants GATEIP and GATEPORT)
    // send message
    // receive message
    // set quadrant to 2 once gate is open
    quadrant = 2;
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

                // Check error values for in front of robot, to left, and to right of robot
                if (errorLeft < 5) { // Check for a line on the left side
                    // Turn 90 degrees left
                    setMotors("90 left");
                } else if (errorRight < 5) { // Check for a line on the right side
                    // Turn 90 degrees right
                    setMotors("90 right");
                } else if (error != 0) { // Check if going straight on the line

                    // Calculate motor adjustment
                    adjustment = (kp * error) + (kd * (error - errorPrev) / (time.tv_sec - timePrev.tv_sec));

                    // Set motors
                    setMotors("turn");

                } else { // error is equal to 0

                    // Go straight
                    setMotors("forward");
                }
            } else { // Line lost

                // Check for line on the sides
                if (find(begin(blackPx), end(blackPx), 1) != end(blackPx) && find(begin(blackPx), end(blackPx), 1) != end(blackPx)) { // Line not found
                    // Turn around 180 degrees
                    setMotors("180");
                } else {
                    // Reverse until line is found
                    setMotors("reverse");
                }
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
    // Record the number of red pixels
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

    // Loop through all columns of pixels in the image
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

    // Loop through all rows of pixels in the image
    for (int row = 0; row < CAMERAHEIGHT; row++) {
        // Get pixels whiteness for left and right side of camera
        int whiteLeft = get_pixel(row, LEFTCOL, 3);
        int whiteRight = get_pixel(row, RIGHTCOL, 3);

        // Check if black or white  for right side of camera
        if (whiteLeft < threshold) { // Is black
            // Set pixel as black in array
            blackPxLeft[row] = 1;
        } else { // Is white
            // Set pixel as white in the array
            blackPxLeft[row] = 0;
        }

        // Check if black or white for right side of camera
        if (whiteRight < threshold) { // Is black
            // Set pixel as black in array
            blackPxRight[row] = 1;
        } else { // Is white
            // Set pixel as white in the array
            blackPxRight[row] = 0;
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

        // Weight the pixels distance from the middle column
        error += blackPx[i] * (i - MIDDLECOL);
    }

    // Loop through array of black pixels for left and right of camera
    for (int i = 0; i < CAMERAHEIGHT; i++) {

        // Weight the pixels distance from the middle row
        errorLeft += blackPxLeft[i] * (i - SCANNEDROW);
        errorRight += blackPxRight[i] * (i - SCANNEDROW);
    }

    // measure current time to measure dt later on
    clock_gettime(CLOCK_MONOTONIC, &time);
}

// Set the speed of each motor to the given value
void AVC::setMotors(string direction) {

    // Set appropriate speed values
    if (direction == "forward") { // Move forward
        vLeft = LEFTDEFAULT;
        vRight = RIGHTDEFAULT;
    } else if (direction == "reverse") { // Reverse
        vLeft = RIGHTDEFAULT;
        vRight = LEFTDEFAULT;
    } else if (direction == "turn") { // Turn based on the adjustment value
        vLeft = LEFTDEFAULT + adjustment;
        vRight = RIGHTDEFAULT + adjustment;
    } else if (direction == "90 right") { // 90 degree right turn
        vLeft = LEFTDEFAULT;
        vRight = STOP;
    } else if (direction == "90 left") { // 90 degree left turn
        vLeft = STOP;
        vRight = RIGHTDEFAULT;
    } else if (direction == "180") { // 180 degree turn (Left)
        vLeft = LEFTDEFAULT;
        vRight = LEFTDEFAULT;
    }

    // Set Motors speed
    set_motors(LEFTMOTOR, vLeft);
    set_motors(RIGHTMOTOR, vRight);

    // update hardware
    hardware_exchange();
}

// debug function Run this to print out messages instead of cout<<""<<endl;
// DEBUG constant in avc.h must be set to true
void AVC::debug(string string) {
    if (DEBUG == true) {
        cout<<string<<endl;
    }
}