/* 
    File: simpleclient.cpp

    Author: U. Chanchlani
            Department of Computer Science
            Texas A&M University
    Date  : 2019/10/04

    Testing Suite for MP2 in CSCE313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    
    /* 
     * One of the three test macro needs to be defined
     * You can define it either here or in compiler directives
     */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <sstream>

#include "mutex.hpp"
#include "mutex_guard.hpp"
#include "semaphore.hpp"

using namespace std;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

#ifdef MUTEX_TEST
Mutex* m;
#elif MUTEX_GUARD_TEST
Mutex* m;
#else
Semaphore* s;
#endif

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

struct ThreadData {
  long threadId;
  unsigned long iters;
  unsigned long sleepTime;
};

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string generateThreadStats(ThreadData *tData, int iter) {
  stringstream ss;
  ss << "Thread " << tData->threadId << ", iter: " << iter << endl;
  return ss.str();
}

void* thread_function(void *threadData) {
  ThreadData* tData = (ThreadData*) threadData;
  for(int i = 0; i < tData->iters; i++) {
    {

#ifdef MUTEX_TEST
      m->Lock();
#elif MUTEX_GUARD_TEST
      auto mg = MutexGuard(*m);
#else
      s->P();
#endif
    
      cout << generateThreadStats(tData, i);
      usleep(tData->sleepTime); // takes some time in critical section

#ifdef MUTEX_TEST
      m->Unlock();
#elif MUTEX_GUARD_TEST
#else
      s->V();
#endif
    }
    
    usleep(tData->sleepTime); // takes some time in non critical section
  }
  return NULL;
}

void run_test_threads(int numThreads, int numIters, int sleepTime) {
  pthread_t* threads = new pthread_t[numThreads];
  ThreadData* threadData = new ThreadData[numThreads];
  for(int i = 0; i < numThreads; i++) {
    threadData[i].threadId = i;
    threadData[i].iters = numIters;
    threadData[i].sleepTime = sleepTime;
    int rc = pthread_create(threads + i, NULL, thread_function, (void *)(threadData + i));
    if (rc < 0) {
      cerr << "Thread unable to start... exitting" << endl;
      exit(1);
    }
  }
  for(int i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }
  cout << "Didn't stop here" << endl;
  delete[] threads;
  delete[] threadData;
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char* argv[]) {
#ifdef MUTEX_TEST
    m = new Mutex();
#elif MUTEX_GUARD_TEST
    m = new Mutex();
#else
    s = new Semaphore(5); // Semaphore value (configurable)
#endif
  run_test_threads(10, // Number of threads to spin off
      10, // Number of iterations each thread will take
      500000 // Wait time in usecs. This value is 0.5 sec
    );
#ifdef MUTEX_TEST
    delete m;
#elif MUTEX_GUARD_TEST
    delete m;
#else
    delete s;
#endif
  return 0;
}

