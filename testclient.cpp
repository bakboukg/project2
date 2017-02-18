#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include "fifo.h"

using namespace std;
string receive_fifo = "WordResult";
string send_fifo = "WordSent";

int main() {
  string message, name;
  string reply=" ";

  	// create the FIFOs for communication
	Fifo recfifo(receive_fifo);
	Fifo sendfifo(send_fifo);

  while (1) {
	cout << "Enter a word to search:";
	cin >>  name;
	//	transform(name.begin(), name.end(), name.begin(), ::toupper);

	

	message = name;
	cout << "Send:" << message << endl;
	sendfifo.openwrite();
	sendfifo.send(message);
	recfifo.openread();
	//	cout << "end check: " << reply.find("$END") << endl;

	/* Get a message from a server */
        while (reply.find("$END")>reply.length())
	  {
	    
	reply = recfifo.recv();
	//	cout << "end check: " << reply.find("$END") << endl;
	cout << "Server sent: " << reply << endl;
  }
	
	recfifo.fifoclose();
	sendfifo.fifoclose();

  }

}
