#include <string>

// Declarations for all of the variables and functions in our AVC class
class AVC {
    private:
        // Constants
        const std::string GATEIP = "130.195.6.196"; // IP address of the gate. (Q1) Can't put into connect_to_server() for reasons?
        static const int GATEPORT = 1024; // Server port to connect to the gate (Q1)
        static const int CAMERAWIDTH = 320; // Takes a guess
        static const int CAMERAHEIGHT = 240; // Also pretty obvious
        static const int MIDDLEROW = 120; // Row number the program will use to follow the line (Q2 & Q3 & Q4)
        static const int MIDDLECOL = 160; // Column number used to weight distance from middle for error (Q2 & Q3 & Q4)
        static const int BACKROW = 220; // Row number used to detect line after line is lost from main row (Q3)
        static const int LEFTCOL = 20; // Column used to detect line to the left of the robot (Q3)
        static const int RIGHTCOL = 300; // Column used to detect line to the right of the robot (Q3)
        static const int LEFTMOTOR = 1; // Port number for left motor
        static const int RIGHTMOTOR = 5; // Port number for right motor
        static const int CAMERASERVO = 3; // Port number for camera servo (Q4)
        static const int STOP = 48; // Speed value used to stop motors
        static const int LEFTDEFAULT = 56; // Default speed for left motor to go forward
        static const int RIGHTDEFAULT = 38; // Default speed for right motor to go forward
        static const int DEBUG = false; // Set to true when you want to print stuff (use debug("hello");)

        // Variables
        int blackPx[CAMERAWIDTH]; // Array of ones and zeros to represent black and white picture (Q2 & Q3)
        int blackPxBack[CAMERAWIDTH]; // Array of black pixels for checking behind to detect a dead end (Q3)
        int blackPxLeft[CAMERAHEIGHT]; // Array of black pixels for checking vertically on left side of camera (Q3)
        int blackPxRight[CAMERAHEIGHT]; // Array of black pixels for checking vertically on right side of camera (Q3)
        int colorPx[CAMERAWIDTH]; // Array of color pixels to find the ducks (Q4)
        double vLeft = STOP; // Current left motor speed (Q2 & Q3 & Q4)
        double vRight = STOP; // Current right motor speed (Q2 & Q3 & Q4)
        double kp = 0.0009; // Proportional constant (Q2 & Q3 & Q4)
        double kd = 0.0007;  // Derivative constant (Q2 & Q3 & Q4)
        int error = 0; // Error value to calculate adjustment to motors speed (Q2 & Q3 & Q4)
        int errorBack = 0; // Error value closer to robot. Used to detect dead ends (Q3)
        int errorLeft = 0; // Error value on left side of camera. Used to detect right angle turns (Q3)
        int errorRight = 0; // Error value on right side of camera. Used to detect right angle turns (Q3)
        int errorPrev = 0; // Records previous calculated error to calculate de for de/dt (Q2 & Q3 & Q4)
        struct timespec timeStart; // Time measured to calculate dt for de/dt (Q2 & Q3 & Q4)
        struct timespec timeEnd; // Records previous measured time to calculate dt for de/dt (Q2 & Q3 & Q4)
        double adjustment = 0; // Amount to adjust motors (Q2 & Q3 & Q4)
        int direction = 1; // Keep track of direction when doing right turns (Q3)
        bool redDuck = false; // Whether the red duck has been found (Q4)
        bool greenDuck = false; // Whether the green duck has been found (Q4)
        bool blueDuck = false; // Whether the blue duck has been found (Q4)
        bool finish = false; // Whether the final yellow patch has been reached (Q4)

        // Quadrant 1 Methods
        // None cause its piss easy!! yay

        // Quadrant 2 and 3 Methods
        double propColor(std::string color); // Returns what proportion the image is of the given color
        void getBlackPx(); // Get an image and turn it into an array of 1s (black) and 0s (white)
        double calcThreshold(int row); // Calculate threshold for determining whether a pixel is black or white
        void calcError(); // Calculate error to required determine adjustment and record the time

        // Quadrant 4 Methods
        void getColorPx(std::string color); // Same as getBlackPx() but does it for red, green, blue, yellow
        bool findDuck(std::string color); // Find duck of the given color
        // Also uses propColor(), and calcError()

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
        void findDucks(); // Quadrant four method, finds and goes to ducks
        void dance(); // Dance to celebrate finishing
};