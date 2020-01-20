/* 
    File: PCBuffer.cpp

    Author: Benjamin Gabay
            Department of Computer Science and Engineering
            Texas A&M University
    Date  : 10/27/2019

    Modified:
    PCBuffer class method definitions.
*/

#include "PCBuffer.hpp"

PCBuffer::PCBuffer(int num) : full(0), empty(num)
{
    n = num;
}

PCBuffer::~PCBuffer(){}

void PCBuffer::Produce(std::string item)
{
    empty.P();
    m.Lock();
    buffer.push(item);
    m.Unlock();
    full.V();
}

std::string PCBuffer::Consume()
{
    full.P();
    m.Lock();
    std::string str = buffer.front();
    buffer.pop();
    m.Unlock();
    empty.V();
    return str;
}