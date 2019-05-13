// Declarations for all of the variables and functions in our avc class
class AVC {
    private:
        // Variables
        static const int CAMERAWIDTH = 320;
        static const int CAMERAHEIGHT = 240;
        static const int SCANNEDROW = 120; // Row number the program will use to follow the line
        static const int MIDDLECOL = 160; // Column number used to weight distance from middle for error
        static const int LEFTMOTOR = 1; // Port number for left motor
        static const int RIGHTMOTOR = 2; // port number for right motor
        static const int STOP = 48; // value used to stop motors
        int quadrant; // 1 = Open Gate, 2 = Follow Line (Q2, Q3), 4 = Find Duck
        int blackPx[CAMERAWIDTH]; // array of ones and zeros to represent black and white picture
        int vLeft = STOP;
        int vRight = STOP;
        double kp = 0;
        double kd = 0;
        int lastError = 0;
        int error = 0;
        int adjustment;


        // Methods
        bool checkRed();
        void getBlackPx();
        double calcThreshold();
        void calcError();
        void setMotors(int left, int right);
    public:
        AVC(int quadrant);
        void openGate();
        void followLine();
        void findDuck();
};