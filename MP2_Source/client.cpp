/* 
    File: client.cpp

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2019/09/23

    Simple client main program for MP2 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>

#include "reqchannel.hpp"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    const int MAX_MESSAGE = 255;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

std::string int2string(int number) {
  std::stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

void print_time_diff(const std::string _label, 
                     const struct timeval & _tp1, 
                     const struct timeval & _tp2) {
  /* Prints to stdout the difference, in seconds and museconds, between
     two timevals. 
  */

  std::cout << _label;
  long sec = _tp2.tv_sec - _tp1.tv_sec;
  long musec = _tp2.tv_usec - _tp1.tv_usec;
  if (musec < 0) {
    musec += 1000000;
    sec--;
  }
  std::cout << " [sec = " << sec << ", musec = " << musec << "]" << std::endl;

}

std::string generate_data() {
  // Generate the data to be returned to the client.
  return int2string(rand() % 100);
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
	
	pid_t pid = fork();
	
	if(pid == 0)
		execvp("./dataserver", nullptr);
	else
	{
		std::cout << "CLIENT STARTED:" << std::endl;

		std::cout << "Establishing control channel... " << std::flush;
		RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
		std::cout << "done." << std::endl;

		/* -- Start sending a sequence of requests */
		std::string reply1 = chan.send_request("hello");
		std::cout << "Reply to request 'hello' is '" << reply1 << "'" << std::endl;

		std::string reply2 = chan.send_request("data Joe Smith");
		std::cout << "Reply to request 'data Joe Smith' is '" << reply2 << "'" << std::endl;

		std::string reply3 = chan.send_request("data Jane Smith");
		std::cout << "Reply to request 'data Jane Smith' is '" << reply3 << "'" << std::endl;

    std::ofstream outFile;
    outFile.open("timeData.csv");
    struct timeval tp_start1, tp_end1, tp_start2, tp_end2;
    long sec, musec;
		for(int i = 1; i <= 100; i += 11)
    {
      outFile << i << ",";
      gettimeofday(&tp_start1, NULL);
      for(int j = 0; j < i; j++)
      {
			  std::string request_string("data TestPerson" + int2string(i));
			  std::string reply_string = chan.send_request(request_string);
			  std::cout << "reply to request " << i << ":" << reply_string << std::endl;;
		  }
      gettimeofday(&tp_end1, NULL);
      sec = tp_end1.tv_sec - tp_start1.tv_sec;
      musec = tp_end1.tv_usec - tp_start1.tv_usec;
      musec += sec*1000000;
      outFile << musec << ",";
      gettimeofday(&tp_start2, NULL);
      for(int j = 0; j < i; j++)
      {
	      generate_data();
		  }
      gettimeofday(&tp_end2, NULL);
      sec = tp_end2.tv_sec - tp_start2.tv_sec;
      musec = tp_end2.tv_usec - tp_start2.tv_usec;
      musec += sec*1000000;
      outFile << musec << "\n";   
    }
    outFile.close();
 
		std::string reply4 = chan.send_request("quit");
		std::cout << "Reply to request 'quit' is '" << reply4 << std::endl;
		
		usleep(1000000);
	}  
}
