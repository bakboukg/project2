
/***************************************************************************
* nameservercpp  -  Program to serve of last name statistics
 *
* copyright : (C) 2009 by Jim Skon
*
* This program runs as a background server to a CGI program, providinging US Census
* Data on the frequency of names in response to requestes.  The rational for this 
* is to demonstrait how a background process can serve up information to a web program,
* saving the web program from the work of intiallizing large data structures for every
* call.
* 
* 
*
***************************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <map>
#include "fifo.h"
#include <cstdlib>
#include <string>
#include<vector>
#include<algorithm>
#include<iterator>
#include <sstream>      // std::istringstream

using namespace std;

struct book {
  int start; // position of first line
  int finish; // position of last line
  string title;
};

/* Fifo names */
string receive_fifo = "WordSent";
string send_fifo = "WordResult";

map <string, vector<int> > theMap;
/* Name data structure */

/*
 * Read the US Census data file "dist.all.last" and load it into an
 * in memory b+tree with name as the key.
 *
 * Return 1 if success, 0 if fail
 */
void createMap(ifstream& infile, map <string, vector <int> >& refs) {
  string word, line; //="Love will prevail";
  int position = 0;
  while (!infile.fail()) {
    getline(infile, line); // get the next line of code
    istringstream lineStream(line); // Create a string stream of the line
    while (lineStream >> word) { // get the next word
      refs[word].push_back(position); // push the word and the line position on the vector for this word
    }
    position = infile.tellg(); // get the position of the next line
  }
}

vector <int> indexSearch(string word, map<string, vector<int> > refs) {
  map<string, vector<int> >::iterator it; // iterator for find
  vector<int> blank; // return for no matches
  /* find the word and get the vector of references */
  /* First use find, so as to NOT create a new entry */
  it = refs.find(word);
  if (it == refs.end()) {
    return (blank);
  } else {
    return (refs[word]);
  }
}

string readParagraph(istream& is) {
  string line;
  string paragraph;
  int lineNum = 0;
  //scan for the next paragraph
  do {
    getline(is, line);
  }    while (line.length() == 0 && !is.eof());

  // return nothing if eof
  if (is.eof()) {
    return "";
  }
  // Get the next paragraph
  do {
    // Only put a newline after first line
    if (lineNum++ > 0) {
      paragraph += "\n";
    }
    paragraph += line;
    getline(is, line);
  }    while (line.length() > 0 && !is.eof());

  return paragraph;
}

bool inRange(book ex, int pos)
// checks if the position of the word is in the selected book
{
  if (pos <= ex.finish && pos >= ex.start) {
    return true;
  }
  return false;
}

void buildList(vector <book>& list, istream& in_stream)
// builds a list of books that includes title and range of every book
{
  string s;
  book temp;
  while (!in_stream.eof()) {
    s = readParagraph(in_stream);
    if (s.substr(0, 4) == "BOOK") // the start of the book
      {
	temp.title = s;
	temp.start = (in_stream.tellg() - s.length()); // starts with the title of the book
      }
    if (s == "THE END " || s == "THE END") {
      temp.finish = in_stream.tellg();
      list.push_back(temp);
    }

  }
}

string  displayResults(vector <int> v, istream& is2, vector <book> list, string word)
//displays the lines where a certain word exists.
{
  if (v.size() == 0) {
    cout << "The word does not exist in Shakespear's works." << endl;
  } else {
    cout << "The word exists " << v.size() << " times" << endl;
    string liney;
    for (int i = 0; i < v.size(); i++) {
      is2.seekg(v[i], is2.beg); // sets the position to where the word was found
      getline(is2, liney);
      for (int j = 0; j < list.size(); j++) {
	if (inRange(list[j], v[i])) {
  cout << list[j].title << " : ";
  break;
	}
      }
      return( liney.substr(0, liney.find(word)) + "\e[1m" +  word + "\e[0m" + liney.substr(liney.find(word) + word.length()) +"\n");
      // bold the word.
    }
  }
}

string displayOneResult(vector <int> v, istream& is2, vector <book> list, string word,  int i)
//displays the lines where a certain word exists.
{
  if (v.size() == 0) {
    return( "The word does not exist in Shakespear's works.");
  } else {
    string liney, tite=" ";
    // for (int i = 0; i < v.size(); i++) {
    is2.seekg(v[i], is2.beg); // sets the position to where the word was found
    getline(is2, liney);
            for (int j = 0; j < list.size(); j++) {
    if (inRange(list[j], v[i])) {
    	tite= list[j].title + " : ";
    	break;
    }
	    }

  
    return( tite+ liney.substr(0, liney.find(word)) + "<b>" + word + "</b>" + liney.substr(liney.find(word) + word.length())+"\n");
    // bold the word.

  }
}


/* Server main line,create name MAP, wait for and serve requests */
  int main() {
  ifstream is, is2, in_stream;
  string s;
  vector <book> list; // a list of book
  book temp;
  in_stream.open("Shakespeare.txt");
  if (is.fail()) {
    cout << "The file does not exist";
    return 0;
  } else {
    cout << "File opened successfully!" << endl;
  }
  buildList(list, in_stream);
  in_stream.close();
  // map <string, vector<int> > theMap;
  is.open("Shakespeare.txt");
  createMap(is, theMap);
  is.close();
  // create the FIFOs for communication
  is2.open("Shakespeare.txt");
  string searchObject, liney, outMessage;
  vector <int> v;
  Fifo recfifo(receive_fifo);
  Fifo sendfifo(send_fifo);
  for (int i=0; i<list.size(); i++)
    {
      cout << list[i].title << endl;
    }
  while (1)
    {
      string line; 
      /* Get a message from a client */
    recfifo.openread();
    searchObject = recfifo.recv();
	/* Parse the incoming message */
	/* Form:  $type*name  */
    v = indexSearch(searchObject, theMap);
    // string s= to_string (v.size());
	sendfifo.openwrite();

    for (int i=0; i<v.size(); i++)
      {
	outMessage=displayOneResult(v, is2, list, searchObject, i);
      cout << outMessage;
  	cout << " Results: " << outMessage << endl;
		sendfifo.send(outMessage);
	//	sendfifo.fifoclose();
	
      }
    //        sendfifo.openwrite();
	sendfifo.send("$END");
	sendfifo.fifoclose();	
	recfifo.fifoclose();
	v.clear();
    
    }
  

  }
