#include "E101.h"
#include <iostream>

class Test {
	public:
		double getSpeed(double x);
		
	};

int main() {
	Test test;
	init(0);
	for (int i = 0; i < 4; i++) {
		int oddEven = i%2;
		int oddEven2 = (i+1)%2;
		set_motors(oddEven,(unsigned char)test.getSpeed(0.75));
		set_motors(oddEven2,(unsigned char)test.getSpeed(-0.75));
		hardware_exchange();
		sleep1(5000);
		}
}

//converts the random values for speed so we can use -1 to 1
//i.e. -1 gives you max speed anti-clockwise, 1 gives you max speed clockwise
double Test::getSpeed(double x) {
	return ((18*x)+48);
	}
