robot: robot.o
	c++ -o robot robot.o avc.o
	
robot.o: robot.cpp
	c++ -c robot.cpp
	
avc.o: avc.cpp avc.h
	c++ -c avc.cpp
	
clean:
	rm *.o