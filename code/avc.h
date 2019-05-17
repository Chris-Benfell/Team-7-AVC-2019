using namespace std;

// Declarations for all of the variables and functions in our avc class
class AVC {
    private:
        // Constants
        const string GATEIP = "130.195.6.196"; // IP address of the gate
        static const int GATEPORT = 1024; // Server port to connect to the gate
        static const int CAMERAWIDTH = 320; // Takes a guess
        static const int CAMERAHEIGHT = 240; // Also pretty obvious
        static const int SCANNEDROW = 120; // Row number the program will use to follow the line
        static const int LEFTCOL = 260; // Column used to detect line to the left of the robot
        static const int RIGHTCOL = 60; // Column used to detect line to the left of the robot
        static const int MIDDLECOL = 160; // Column number used to weight distance from middle for error
        static const int LEFTMOTOR = 1; // Port number for left motor
        static const int RIGHTMOTOR = 5; // Port number for right motor
        static const int STOP = 48; // Speed value used to stop motors
        static const int LEFTDEFAULT = 53; // Default speed for left motor to go forward
        static const int RIGHTDEFAULT = 43; // Default speed for right motor to go forward
        static const int DEBUG = false; // Set to true when you want to print stuff (use debug("hello");)

        // Variables
        int blackPx[CAMERAWIDTH]; // array of ones and zeros to represent black and white picture
        int blackPxLeft[CAMERAHEIGHT]; // Array of black pixels for checking vertically on left side of camera
        int blackPxRight[CAMERAHEIGHT]; // Array of black pixels for checking vertically on right side of camera
        int vLeft = STOP; // Current left motor speed
        int vRight = STOP; // Current right motor speed
        double kp = 1; // Proportional constant
        double kd = 0.5;  // Derivative constant
        int error = 0; // Error value to calculate adjustment to motors speed
        int errorLeft = 0; // Error value on left side of camera. Used to detect right angle turns
        int errorRight = 0; // Error value on right side of camera. Used to detect right angle turns
        int errorPrev = 0; // Records previous calculated error to calculate de for de/dt
        struct timespec time; // Time measured to calculate dt for de/dt
        struct timespec timePrev; // Records previous measured time to calculate dt for de/dt
        double adjustment = 0; // Amount to adjust motors

        // Quadrant 1 Methods

        // Quadrant 2 and 3 Methods
        bool checkRed(); // Check for the red spot indicating the end of quadrant three
        void getBlackPx(); // Get an image and turn it into an array of 1s (black) and 0s (white)
        double calcThreshold(); // Calculate threshold for determining whether a pixel is black or white
        void calcError(); // Calculate error to required determine adjustment and record the time

        // Quadrant 4 Methods

        // Other
        void setMotors(string direction); // Set motors speed based on direction (forward, reverse turn)
        void debug(string string);

    public:
        // Variables
        int quadrant; // quadrant number. 1 = Open Gate, 2 = Follow Line (Q2, Q3), 4 = Find Duck

        // Methods
        explicit AVC(int quadrant); // Constructor Method
        void openGate(); // Quadrant one method, opens the network gate
        void followLine(); // Quadrant two and three method, follows the line
        void findDuck(); // Quadrant four method, finds and goes to ducks
};