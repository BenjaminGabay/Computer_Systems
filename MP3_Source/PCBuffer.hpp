/* 
    File: PCBuffer.hpp

    Author: Benjamin Gabay
            Department of Computer Science and Engineering
            Texas A&M University
    Date  : 10/27/2019

    Modified:
    PCBuffer header file which acts as a bounded buffer
    using semaphores to regulate data request flow.
*/

#ifndef _PCBUFFER_H_
#define _PCBUFFER_H_

#include <string>
#include <queue>
#include "semaphore.hpp"
#include "mutex.hpp"


class PCBuffer
{
    private:
        int n;
        Semaphore full;
        Semaphore empty;
        Mutex m;
        std::queue<std::string> buffer;

    public:
        PCBuffer(int num);
        ~PCBuffer();
        void Produce(std::string item);
        std::string Consume();
};

#endif