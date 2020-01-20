/* 
    File: semaphore.cpp

    Author: Benjamin Gabay
            Department of Computer Science and Engineering
            Texas A&M University
    Date  : 10/27/2019

    Modified:
    Mutex class method definitions
*/

#include "mutex.hpp"

Mutex::Mutex()
{
    pthread_mutex_init(&m, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m);
}

void Mutex::Lock()
{
    pthread_mutex_lock(&m);
}

void Mutex::Unlock()
{
    pthread_mutex_unlock(&m);
}