#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <time.h>
#include <cmath>
#include "E101.h"
#include "avc.hpp"
using namespace std;

// Initialize AVC
AVC::AVC(int q) {
    quadrant = q;
    init(0);
    hardware_exchange();
}

// Send a message to open the gate
void AVC::openGate() {
    if (quadrant == 1) {
        // Convert IP address to a char[15]
		char address[15];
		strcpy(address, GATEIP.c_str());

		// Convert message to open gate to a char[24]
		std::string sPls = "Please";
		char pls[24];
		strcpy(pls, sPls.c_str());

		// Attempt to connect to server
		if(connect_to_server(address, GATEPORT) == 0) { // Connected
            send_to_server(pls); // Send request to open gate
            char msg[24]; // Prepare variable to store message
            receive_from_server(msg); // Gey password back
            send_to_server(msg); // Send password back to server
            quadrant = 2; // Next quadrant
        }
    }
}

// Follow a line. Quadrant 2 and 3 Code
void AVC::followLine() {
    open_screen_stream();
    while (quadrant == 2 || quadrant == 3) {
        take_picture();
        clock_gettime(CLOCK_MONOTONIC, &timeStart);
        update_screen();

        // Check if picture contains significant red indicating beginning of quadrant 3 or 4
        if (checkRed()) { // Significantly red
            quadrant++;
            if (quadrant == 3) { // Go over red patch if beginning of quadrant three
                setMotors("forward");
                sleep1(1000);
            }
            debug(to_string(quadrant));
        } else { // Line following code

            // Turn image into an array of 1s (black) and 0s (white)
            getBlackPx();

            // Check if blackPx contains 1s to indicate line detected. No 1s indicates line has been lost
            if (find(begin(blackPx), end(blackPx), 1) != end(blackPx)) { // Found line

                // Calculate the error value
                calcError();

                // Check error values for in front of robot, to left, and to right of robot
                if (quadrant == 3 && errorLeft > -200 && errorLeft < 300) { // Check for a line on the left side (Q3)
                    // Turn 90 degrees left
                    setMotors("90 left");
                    sleep1(1000);

                } else if (quadrant == 3 && errorRight > -300 && errorLeft < 200) { // Check for a line on the right side (Q3)
                    // Turn 90 degrees right
                    setMotors("90 right");
                    sleep1(1000);

                } else if (error != 0) { // Check if going straight on the line

					// measure current time to measure dt later on
					clock_gettime(CLOCK_MONOTONIC, &timeEnd);

					// Calculate time elapsed between last adjustment made
					double elapsed = ((timeEnd.tv_sec - timeStart.tv_sec) * 1000000000 + (timeEnd.tv_nsec - timeStart.tv_nsec))/10000000.0;
					
                    // Calculate motor adjustment
                    adjustment = (kp * error) + (kd * (error - errorPrev) / elapsed);

                    // Set motors to turn based on adjustment
                    setMotors("turn");

                } else { // error is equal to 0

                    // Go straight
                    setMotors("forward");
                }
            } else { // Line lost

                // Check for line on the sides
                if (quadrant == 3 && find(begin(blackPx), end(blackPx), 1) != end(blackPx) && find(begin(blackPx), end(blackPx), 1) != end(blackPx)) { // Line not found (Q3)

                    // Won't work

                    // Turn around 180 degrees
                    setMotors("180");
                    sleep1(2000);

                } else {
                    // Reverse until line is found
                    setMotors("reverse");
                    sleep1(1000);
                }
            }
        }
    }
    close_screen_stream();
}

// Turn around and look for ducks on paper cylinders. Quadrant 4 Code
void AVC::findDuck() {
    while (quadrant == 4) {
        // Tilt camera
        // look for a red duck, green duck, blue duck and a yellow patch
        // set quadrant to 5 when done
        quadrant = 5;
    }
    // Stop the robot
    stoph();
}

// Check for red spot in middle of camera indicating end of quadrant 3 and start of quadrant 4
bool AVC::checkRed() {
    // Record the number of red pixels
    int numRedPx = 0;

    // Loop through all pixels in the image
    for (int col = CAMERAWIDTH/4; col < CAMERAWIDTH*3/4; col++) {

        // Get RGB values of the pixel
        int red = get_pixel(MIDDLEROW, col, 0);
        int green = get_pixel(MIDDLEROW, col, 1);
        int blue = get_pixel(MIDDLEROW, col, 2);

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
        int whiteness = get_pixel(MIDDLEROW, col, 3);

        // Check if black or white
        if (whiteness < threshold && threshold < 150) { // Is black
            // Set pixel as black in array
            blackPx[col] = 1;
        } else { // Is white
            // Set pixel as white in the array
            blackPx[col] = 0;
        }
    }

    if (quadrant == 3) { // Only if on quadrant 3

        // Loop through all rows of pixels in the image
        for (int row = 0; row < CAMERAHEIGHT; row++) {

            // Get pixels whiteness for left and right side of camera
            int whiteLeft = get_pixel(row, LEFTCOL, 3);
            int whiteRight = get_pixel(row, RIGHTCOL, 3);

            // Check if black or white for right side of camera
            if (whiteLeft < threshold && threshold < 150) { // Is black
                // Set pixel as black in the left array
                blackPxLeft[row] = 1;
            } else { // Is white
                // Set pixel as white in the left array
                blackPxLeft[row] = 0;
            }

            // Check if black or white for right side of camera
            if (whiteRight < threshold && threshold < 150) { // Is black
                // Set pixel as black in the right array
                blackPxRight[row] = 1;
            } else { // Is white
                // Set pixel as white in the right array
                blackPxRight[row] = 0;
            }
        }
    }
}

// Calculate threshold to determine if a pixel is black or white
double AVC::calcThreshold() {
    // Set maximum and minimum possible values
    int min = 255;
    int max = 0;

    // Loop through all pixels in the image
    for (int col = 0; col < CAMERAWIDTH; col++) {

        // Get pixels whiteness robot.cpp avc
        int whiteness = get_pixel(MIDDLEROW, col, 3);

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
    // Calculate average of the min and max. This is threshold.
    return (min+max)/2.0;
}

// Calculate the error value and record the time
void AVC::calcError() {
    // Record last error measurement
    errorPrev = error;

    // Reset error values
    error = 0;
    errorLeft = 0;
    errorRight = 0;

    // Loop through array of black pixels
    for (int i = 0; i < CAMERAWIDTH; i++) {

        // Weight the pixels distance from the middle column
        error += blackPx[i] * (i - MIDDLECOL);
    }

    if (quadrant == 3) { // Only if on quadrant 3

        // Loop through array of black pixels for left and right of camera
        for (int i = 0; i < CAMERAHEIGHT; i++) {

            // Weight the pixels distance from the middle row
            errorLeft += blackPxLeft[i] * (i - MIDDLEROW);
            errorRight += blackPxRight[i] * (i - MIDDLEROW);
        }
    }
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

    // Set left motors speed
    set_motors(LEFTMOTOR, round(vLeft));

    // Set right motors speed
    set_motors(RIGHTMOTOR, round(vRight));
    
    hardware_exchange();
}

// debug function Run this to print out messages instead of cout<<""<<endl;
// DEBUG constant in avc.hpp must be set to true
void AVC::debug(string string) {
    if (DEBUG) {
        cout<<string<<endl;
    }
}