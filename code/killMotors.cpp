#include "E101.h"
#include <iostream>

int main() {
	init(0);
	hardware_exchange();
	set_motors(1,48);
	set_motors(5,48);
	set_motors(3,60);
	hardware_exchange();
}
