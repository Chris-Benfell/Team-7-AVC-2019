 #include "E101.h"
 #include <string>
 #include <cstring>
 #include <iostream>
 
#include "avc.hpp"
 
 int main() {
	 char server_addr[15];
	 strcpy(server_addr, GATEIP.c_str());
	 std::string Spls = "Please";
	 char pls[24];
	 strcpy(pls, Spls.c_str());
 if(connect_to_server(server_addr, GATEPORT) == 0) {
            send_to_server(pls);
            char msg[24];
            receive_from_server(msg);
            send_to_server(msg);
            quadrant = 2;
        }
	}
