/* 
    File: semaphore.cpp

    Author: Benjamin Gabay
            Department of Computer Science and Engineering
            Texas A&M University
    Date  : 10/27/2019

    Modified:
    Semaphore class method definitions
*/

#include "semaphore.hpp"

Semaphore::Semaphore(int _val)
{
	value = _val;
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&c, NULL);
}

Semaphore::~Semaphore()
{
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&c);
}

int Semaphore::P()
{
	pthread_mutex_lock(&m);
	value--;
	if(value < 0)
	{
		pthread_cond_wait(&c, &m);
	}
	pthread_mutex_unlock(&m);
}

int Semaphore::V()
{
	pthread_mutex_lock(&m);
	value++;
	if(value <= 0)
	{
		pthread_cond_signal(&c);
	}
	pthread_mutex_unlock(&m);
}