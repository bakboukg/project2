#include <iostream>
// Stuff for AJAX
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

//Stuff for pipes
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "fifo.h"
#include <string>

using namespace std;
using namespace cgicc; // Needed for AJAX functions.


//Indeed, myString needs to be a copy of the original string
std::string StringToUpper(std::string myString)
{
  const int length = myString.length();
  for(int i=0; i!=length ; ++i)
    {
      myString[i] = std::toupper(myString[i]);
    }
  return myString;
}

// fifo for communication
string receive_fifo = "WordResult";
string send_fifo = "WordSent";

int main() {
  Cgicc cgi;    // Ajax object
  char *cstr;
  // Create AJAX objects to recieve information from web page.
  //  form_iterator ts = cgi.getElement("type_select");
  form_iterator word = cgi.getElement("word");

  // create the FIFOs for communication
  Fifo recfifo(receive_fifo);
  Fifo sendfifo(send_fifo);

  // string type = "$LAST*";
  // if (**ts == LAST) {
  // type = "$LAST*";
  // } else if (**ts == MALE) {
  // type = "$MALE*";
  // } else if (**ts == FEMALE) {
  // type = "$FEMALE*";
  //  }

  // Call server to get results
  string stword = **word;
  // stname = StringToUpper(stname);
  string message =  stword;
  sendfifo.openwrite();
  sendfifo.send(message);

  /* Get a message from a server */
  recfifo.openread();
  cout << "Content-Type: text/plain\n\n";
  //  cout << "my life is falling apart";
  string results = " ";
  while (results.find("$END")==string::npos)
   {
        cout << "<p>"+ results; 
     results = recfifo.recv();
    }
  recfifo.fifoclose();
  sendfifo.fifoclose();


  return 0;
}
