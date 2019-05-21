#include <string>

// Declarations for all of the variables and functions in our avc class
class AVC {
    private:
        // Constants
        const std::string GATEIP = "130.195.6.196"; // IP address of the gate. (Q1) Can't put into connect_to_server() for reasons?
        static const int GATEPORT = 1024; // Server port to connect to the gate (Q1)
        static const int CAMERAWIDTH = 320; // Takes a guess
        static const int CAMERAHEIGHT = 240; // Also pretty obvious
        static const int SCANNEDROW = 120; // Row number the program will use to follow the line (Q2 & Q3)
        static const int LEFTCOL = 300; // Column used to detect line to the left of the robot (Q3)
        static const int RIGHTCOL = 20; // Column used to detect line to the right of the robot (Q3)
        static const int MIDDLECOL = 160; // Column number used to weight distance from middle for error (Q2 & Q3)
        static const int LEFTMOTOR = 1; // Port number for left motor
        static const int RIGHTMOTOR = 5; // Port number for right motor
        static const int STOP = 48; // Speed value used to stop motors
        static const int LEFTDEFAULT = 54; // Default speed for left motor to go forward
        static const int RIGHTDEFAULT = 40; // Default speed for right motor to go forward
        static const int DEBUG = true; // Set to true when you want to print stuff (use debug("hello");)

        // Variables
        int blackPx[CAMERAWIDTH]; // array of ones and zeros to represent black and white picture (Q2 & Q3)
        int blackPxLeft[CAMERAHEIGHT]; // Array of black pixels for checking vertically on left side of camera (Q3)
        int blackPxRight[CAMERAHEIGHT]; // Array of black pixels for checking vertically on right side of camera (Q3)
        double vLeft = STOP; // Current left motor speed
        double vRight = STOP; // Current right motor speed
        double kp = 0.001; // Proportional constant
        double kd = 0.0008;  // Derivative constant
        int error = 0; // Error value to calculate adjustment to motors speed (Q2 & Q3 & Q4)
        int errorLeft = 0; // Error value on left side of camera. Used to detect right angle turns (Q3)
        int errorRight = 0; // Error value on right side of camera. Used to detect right angle turns (Q3)
        int errorPrev = 0; // Records previous calculated error to calculate de for de/dt (Q2 & Q3)
        struct timespec timeStart; // Time measured to calculate dt for de/dt (Q2 & Q3 & Q4)
        struct timespec timeEnd; // Records previous measured time to calculate dt for de/dt (Q2 & Q3 & Q4)
        double adjustment = 0; // Amount to adjust motors (Q2 & Q3 & Q4)

        // Quadrant 1 Methods
        // None cause its piss easy

        // Quadrant 2 and 3 Methods
        bool checkRed(); // Check for the red spot indicating the end of quadrant three
        void getBlackPx(); // Get an image and turn it into an array of 1s (black) and 0s (white)
        double calcThreshold(); // Calculate threshold for determining whether a pixel is black or white
        void calcError(); // Calculate error to required determine adjustment and record the time

        // Quadrant 4 Methods
        // Fuck

        // Other
        void setMotors(std::string direction); // Set motors speed based on direction (forward, reverse turn)
        void debug(std::string string); // Print out messages. Set DEBUG = false to turn off all messages

    public:
        // Variables
        int quadrant; // quadrant number. 1 = Open Gate, 2 = Follow Line, 3 = Line Maze, 4 = Find Duck

        // Methods
        explicit AVC(int quadrant); // Constructor Method
        void openGate(); // Quadrant one method, opens the network gate
        void followLine(); // Quadrant two and three method, follows the line
        void findDuck(); // Quadrant four method, finds and goes to ducks
};
