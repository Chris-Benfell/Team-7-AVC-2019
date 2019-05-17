
LIB = -L/opt/vc/lib 
INC=-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
LIBSO =-lbcm_host -lvcos -lmmal -lmmal_core -lmmal_util -lrt -lbcm2835

libe101: start.o camera.o cameracontrol.o
	g++ -shared -o libe101.so start.o camera.o cameracontrol.o $(LIB) $(LIBSO)

start.o: start.cpp
	g++ -Wall  $(INC) -c -fpic start.cpp 	
camera.o: camera.cpp
	g++ -Wall $(INC) -c -fpic camera.cpp
cameracontrol.o: cameracontrol.cpp
	g++ -Wall $(INC) -c -fpic cameracontrol.cpp

	
