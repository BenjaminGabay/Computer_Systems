/* 
    File: mutex_guard.cpp

    Author: Benjamin Gabay
    Date  : 10/8/2019
*/

#include "mutex_guard.hpp"

MutexGuard::MutexGuard(Mutex & m)
{
    this->m = &m;
    m.Lock();
}

MutexGuard::~MutexGuard()
{
    m->Unlock();
}