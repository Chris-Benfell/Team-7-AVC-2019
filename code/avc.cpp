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
            receive_from_server(msg); // Get password back
            send_to_server(msg); // Send password back to server
            setMotors("really fast");
            sleep1(6000);
            quadrant = 2; // Next quadrant
        }
    }
}

// Follow a line. Quadrant 2 and 3 Code
void AVC::followLine() {
    //open_screen_stream();
    while (quadrant == 2 || quadrant == 3) {
        // Start measuring time taken to run
        clock_gettime(CLOCK_MONOTONIC, &timeStart);

        // Get new picture
        take_picture();
        //update_screen();

        // Check if picture contains significant red indicating beginning of quadrant 3 or 4
        if (propColor("red") > 0.6) { // Significantly red
            quadrant++;
            if (quadrant == 3) { // Go over red patch if beginning of quadrant three
                setMotors("really fast");
                debug(to_string(direction));
                sleep1(2000);
            } else {
				setMotors("forward");
				sleep1(2000);
				setMotors("stop");	
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
                if (quadrant == 3 && direction - 1 > 0 && errorLeft > 400 && errorLeft < 1000 && errorLeft != 0) { // Check for a line on the left side (Q3)
                    // Turn 90 degrees left
                    sleep1(500);
                    setMotors("90 left");

                    // Update direction
                    direction--;

					
                    debug("Turn left");
                    debug(to_string(direction));
                    sleep1(2700);

                } else if (quadrant == 3 && direction + 1 < 4 && errorRight > 700 && errorRight < 1400 && errorRight != 0) { // Check for a line on the right side (Q3)
                    // Turn 90 degrees right
                    sleep1(500);
                    setMotors("90 right");

                    // Update direction
                    direction++;

                    debug("Turn right");
                    debug(to_string(direction));
                    sleep1(2700);

                } else if (error != 0 && error > -9000 && error < 9000) { // Check if going straight on the line

					// Measure current time to calculate dt
					clock_gettime(CLOCK_MONOTONIC, &timeEnd);

					// Calculate change in time since loop began
					double dt = ((timeEnd.tv_sec - timeStart.tv_sec) * 1000000000 + (timeEnd.tv_nsec - timeStart.tv_nsec))/10000000.0;
					
                    // Calculate motor adjustment
                    adjustment = (kp * error) + (kd * (error - errorPrev) / dt);

                    // Apply adjustment to motors
                    setMotors("turn");

                } else { // error is equal to 0

                    // Go straight
                    setMotors("forward");
                }
            } else { // Line lost

                // Check if line missing on sides but exists closer to the robot
                if (quadrant == 3 && (find(begin(blackPxBack), end(blackPxBack), 1) != end(blackPxBack)) && (find(begin(blackPxLeft), end(blackPxLeft), 1) == end(blackPxLeft)) && (find(begin(blackPxRight), end(blackPxRight), 1) == end(blackPxRight))) { // Line not found (Q3)

                    // Turn around 180 degrees
                    setMotors("180");

                    // Update direction
                    if (direction == 0 || direction == 1) {
						direction += 2;
					} else {
						direction -= 2;
					}

                    debug("Doing 180 turn");
                    debug(to_string(direction));
                    sleep1(3500);

                } else {
                    // Reverse until line is found
                    setMotors("reverse");
                    sleep1(1000);
                }
            }
        }
    }
    //close_screen_stream();
}

// Turn around and look for ducks on paper cylinders. Quadrant 4 Code
void AVC::findDucks() {
    if (quadrant == 4) {

        // Tilt camera
        set_motors(CAMERASERVO, 55);
        hardware_exchange();

        // Find a red duck
        while (!redDuck) {
            redDuck = findDuck("red");
        }
		debug("Found red duck");
        // Find a green duck
        while (!greenDuck) {
            greenDuck = findDuck("green");
        }
		debug("Found green duck");
        // Find a blue duck
        while (!blueDuck) {
            blueDuck = findDuck("blue");
        }
		debug("Found blue duck");
        // Find the yellow patch (The finish which doesn't have a duck :( )
        while (!finish) {
            finish = findDuck("yellow");
        }
		debug("Found yellow patch");
        // Finish was reached exit function
        quadrant = 5;
    }
}

// Return what proportion the image is of the given color
// This is literally just my Ruby code but generalised
double AVC::propColor(string color) {
    // Record the number of correct pixels
    int numPx = 0;

    // Loop through all pixels in the image
    for (int col = CAMERAWIDTH/4; col < CAMERAWIDTH*3/4; col++) {

        // Get RGB values of the pixels
        int red = get_pixel(MIDDLEROW, col, 0);
        int green = get_pixel(MIDDLEROW, col, 1);
        int blue = get_pixel(MIDDLEROW, col, 2);

        // Compare ratios of RGB to determine if correct color
        if (color == "red" && (2.0 * red)  / (green + blue) > 2.3) { // Is red
            // Record red pixel
            numPx += 1;
        } else if (color == "green" && (2.0 * green)  / (red + blue) > 1.7) { // Is green
            // Record green pixel
            numPx += 1;
        } else if (color == "blue" && (2.0 * blue)  / (red + green) > 1.7) { // Is blue
            // Record blue pixel
            numPx += 1;
 //       } else if (color == "yellow" &&  (red + green) / (2.0 * blue) > 1.4) { // Is yellow
            // Record yellow pixel
 //           numPx += 1;
        } else {
			debug("red " + to_string(1.0 * red / blue));
			debug("green " + to_string(1.0 * green / blue));
		}
    }
    // Return proportion of image that is correct colour. e.g. 0.7 indicates an image which
    // is 70% that colour which is likely to indicate that it is mostly that colour
    return 2.0 * numPx / CAMERAWIDTH;
}

// Get array of black pixels (1s and 0s)
void AVC::getBlackPx() {
    // Get threshold to determine whether a pixels is black or white
    double threshold = calcThreshold(MIDDLEROW);
    double backThreshold = calcThreshold(BACKROW);

    // Loop through all columns of pixels in the image
    for (int col = 0; col < CAMERAWIDTH; col++) {

        // Get pixels whiteness
        int whiteness = get_pixel(MIDDLEROW, col, 3);
        int whiteBack = get_pixel(BACKROW, col, 3);

        // Check if black or white
        if (whiteness < threshold && threshold < 130) { // Is black
            // Set pixel as black
            blackPx[col] = 1;
        } else { // Is white
            // Set pixel as white
            blackPx[col] = 0;
        }

        // Check if black or white in front of main row in quadrant 3
        if (quadrant == 3 && whiteBack < backThreshold && backThreshold < 130) { // Is black
            // Set pixel as black in the front array
            blackPxBack[col] = 1;
        } else { // Is white
            // Set pixel as white in the front array
            blackPxBack[col] = 0;
        }
    }

    if (quadrant == 3) { // Only if on quadrant 3

        // Loop through all rows of pixels in the image
        for (int row = 0; row < CAMERAHEIGHT; row++) {

            // Get pixels whiteness for left and right side of camera
            int whiteLeft = get_pixel(row, LEFTCOL, 3);
            int whiteRight = get_pixel(row, RIGHTCOL, 3);

            // Check if black or white for right side of camera
            if (whiteLeft < backThreshold && backThreshold < 130) { // Is black
                // Set pixel as black in the left array
                blackPxLeft[row] = 1;
            } else { // Is white
                // Set pixel as white in the left array
                blackPxLeft[row] = 0;
            }

            // Check if black or white for right side of camera
            if (whiteRight < backThreshold && backThreshold < 130) { // Is black
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
double AVC::calcThreshold(int row) {
    // Set maximum and minimum possible values
    int min = 255;
    int max = 0;

    // Loop through all pixels in the image
    for (int col = 0; col < CAMERAWIDTH; col++) {

        // Get pixels whiteness robot.cpp avc
        int whiteness = get_pixel(row, col, 3);

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
    errorBack = 0;
    errorLeft = 0;
    errorRight = 0;

    if (quadrant == 2 || quadrant == 3) { // On both quadrant 2 and 3
        // Loop through array of black pixels
        for (int i = 0; i < CAMERAWIDTH; i++) {

            // Weight the pixels distance from the middle column
            error += blackPx[i] * (i - MIDDLECOL);

            if (quadrant == 3) { // Only if on quadrant 3

                // Weight the pixels distance from the middle column
                errorBack += blackPxBack[i] * (i - MIDDLECOL);
            }
        }
    } else if (quadrant == 4) { // Only if on quadrant 4
        // Loop through array of colored pixels
        for (int i = 0; i < CAMERAWIDTH; i++) {

            // Weight the pixels distance from the middle column
            error += colorPx[i] * (i - MIDDLECOL);
        }
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

// Find a duck of a given color
bool AVC::findDuck(string color) {
    // Start measuring time taken to run
    clock_gettime(CLOCK_MONOTONIC, &timeStart);

    // Get new picture
    take_picture();

    // Turn picture into an array of pixels of the right color (1) and not (0)
    getColorPx(color);

    // Check if there are pixels of the right color in image
    if (find(begin(colorPx), end(colorPx), 1) != end(colorPx)) { // Found pixels

        // Check if color enough to have reached a duck
        if (propColor(color) > 0.9) { // Reached duck
            return true;
        } else { // Has not reached duck

            // Calculate the error value
            calcError();

            // Check if duck is roughly in center of image
            if (error > -300 && error < 300) {
                setMotors("forward");
            } else { // Not in center

                // Measure current time to calculate dt
                clock_gettime(CLOCK_MONOTONIC, &timeEnd);

                // Calculate change in time since loop began
                double dt = ((timeEnd.tv_sec - timeStart.tv_sec) * 1000000000 + (timeEnd.tv_nsec - timeStart.tv_nsec))/10000000.0;

                // Calculate motor adjustment
                adjustment = (kp * error) + (kd * (error - errorPrev) / dt);

                // Apply adjustment to motors
                setMotors("turn");
            }
        }
    } else { // Didn't find any, or enough of the right color

        // Turn to find it
        if (color == "green" || color == "yellow") {
            setMotors("rotate left");
        } else {
            setMotors("rotate right");
        }
    }
    return false;
}

void AVC::getColorPx(string color) {

    // Loop through all columns of pixels in the image
    for (int col = 0; col < CAMERAWIDTH; col++) {

        // Get RGB values of the pixels
        int red = get_pixel(MIDDLEROW, col, 0);
        int green = get_pixel(MIDDLEROW, col, 1);
        int blue = get_pixel(MIDDLEROW, col, 2);

        // Compare ratio of RGB to determine the pixels color
        if (color == "red" && (2.0 * red)  / (green + blue) > 2.3) { // Is red
            // Set pixel as red
            colorPx[col] = 1;
        } else if (color == "green" && (2.0 * green)  / (red + blue) > 1.7) { // Is green
            // Set pixel as green
            colorPx[col] = 1;
        } else if (color == "blue" && (2.0 * blue)  / (red + green) > 1.7) { // Is blue
            // Set pixel as blue
            colorPx[col] = 1;
        } else if (color == "yellow" && (red + green) / (2.0 * blue)  > 1.4) { // Is yellow
            // Set pixel as yellow
            colorPx[col] = 1;
        } else {
            // Pixel is not the defined color
            colorPx[col] = 0;
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
        vLeft = LEFTDEFAULT + 2;
        vRight = STOP;
    } else if (direction == "90 left") { // 90 degree left turn
        vLeft = STOP;
        vRight = RIGHTDEFAULT - 2;
    } else if (direction == "180") { // 180 degree turn (right)
        vLeft = LEFTDEFAULT;
        vRight = LEFTDEFAULT + 2;
    } else if (direction == "rotate right") { // Rotate slowly right
        vLeft = LEFTDEFAULT - 2;
        vRight = LEFTDEFAULT;
    } else if (direction == "rotate left") { // Rotate slowly left
        vLeft = RIGHTDEFAULT+2;
        vRight = RIGHTDEFAULT;
    } else if (direction == "really fast") { // Go really fast
        vLeft = 63;
        vRight = 30;
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

// Dance to celebrate finishing
void AVC::dance() {
    if (finish) {
        setMotors("reverse");
        sleep1(2000);
        setMotors("180");
        sleep1(5000);
        setMotors("stop");
    }
}
