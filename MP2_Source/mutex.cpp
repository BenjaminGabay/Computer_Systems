/* 
    File: mutex.cpp

    Author: Benjamin Gabay
    Date  : 10/8/2019
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