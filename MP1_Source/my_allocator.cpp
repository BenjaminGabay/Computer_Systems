/* 
    File: my_allocator.cpp

    Author: Benjamin Gabay
            Department of Computer Science
            Texas A&M University
    Date  : 9/5/2019

    Modified: 

    This file contains the implementation of the class MyAllocator.

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cstdlib>
#include "my_allocator.hpp"
#include <assert.h>
#include <iostream>

/*--------------------------------------------------------------------------*/
/* NAME SPACES */ 
/*--------------------------------------------------------------------------*/

using namespace std;
/* I know, it's a bad habit, but this is a tiny program anyway... */

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR CLASS MyAllocator */
/*--------------------------------------------------------------------------*/

MyAllocator::MyAllocator(size_t _basic_block_size, size_t _size)
{
	startAddr = (char*) std::malloc(_size);
	blockSize = _basic_block_size;
	
	for(int i = 0; getFibNum(i) <= _size / blockSize; i++)
	{
		fLists.push_back(FreeList(getFibNum(i), blockSize));
	}
	fLists[fLists.size() - 1].Add(new(startAddr) SegmentHeader(getFibNum(fLists.size() - 1) * blockSize));
}

MyAllocator::~MyAllocator()
{
	std::free(startAddr);
}

Addr MyAllocator::Malloc(size_t _length)
{
	
	//Check if desired amount of memory is available and allocate it if so
	size_t numBlocks = ((_length + sizeof(SegmentHeader)) / blockSize);
	if((_length + sizeof(SegmentHeader)) / blockSize != 0)
		numBlocks += 1;
	int index = 0;
	while(getFibNum(index) < numBlocks)
	{
		index++;
	}
	numBlocks = getFibNum(index);
	if(numBlocks > fLists[fLists.size() - 1].numBlocks)
	{
		cout << "End Malloc(bad1)" << endl;
		return nullptr;
	}
	
	//Return memory segment of the correct size
	SegmentHeader* currSeg = nullptr;
	while(true)
	{
		//Checks if correct free list has a free segment
		if(fLists[index].head != nullptr)
		{
			currSeg = fLists[index].head;
			currSeg->CheckValid();
			if(!(fLists[index].Remove(currSeg)))
				cout << "ErrorR4" << endl;
			currSeg->is_free = false;
			return ((char*) currSeg) + sizeof(SegmentHeader);
		}
		//Splits next available segment
		else
		{
			int nIndex = index + 1;
			while(nIndex < fLists.size() && fLists[nIndex].head == nullptr)
			{
				nIndex++;
			}
			if(nIndex == fLists.size())
			{
				return nullptr;
			}
			
			currSeg = fLists[nIndex].head;			
			if(!(fLists[nIndex].Remove(currSeg)))
				cout << "ErrorR3" << endl;
			if(nIndex < 2)
			{
				currSeg->is_free = false;
				return ((char*) currSeg) + sizeof(SegmentHeader);
			}
			
			if(currSeg == nullptr)
				cout << "Error: currSeg is nullptr" << endl;
			
			SegmentHeader* rightSeg = fLists[nIndex].Split(currSeg, getFibNum(nIndex - 2) * blockSize);
			rightSeg->CheckValid();
			
			if(!(fLists[nIndex - 2].Add(currSeg)))
				cout << "ErrorA1" << endl;
			
			if(!(fLists[nIndex - 1].Add(rightSeg)))
				cout << "ErrorA2" << endl;
		}
	}
}

bool MyAllocator::Free(Addr _a)
{
	char* tempP = ((char*) _a) - sizeof(SegmentHeader);
	SegmentHeader* freeSH = (SegmentHeader*) tempP;
	freeSH->CheckValid();
	int numBlocks = freeSH->length / blockSize;
	//Adds feee segment back to the vector of free lists
	//If its buddy is also free then it combines them
	while(true)
	{
		freeSH->is_free = true;
		numBlocks = freeSH->length / blockSize;
		int index = 0;
		while(getFibNum(index) != numBlocks)
		{
			index++;
			if(getFibNum(index) > numBlocks)
			{
				cout << "Error1 in Free" << endl;
				return false;
			}	
		}
		
		if(index >= fLists.size())
		{
			cout << "Error2 in Free" << endl;
			return false;
		}
		else if(index == fLists.size() - 1)
		{
			if(!(fLists[index].Add(freeSH)))
				cout << "ErrorA3" << endl;
			return true;
		}
		else
		{		
			SegmentHeader* buddySH = nullptr;
			bool correctSize = false;
			if(freeSH->isLeft)
			{
				tempP = ((char*) freeSH) + freeSH->length;
				buddySH = (SegmentHeader*) tempP;
				buddySH->CheckValid();
				if((buddySH->length / blockSize) == getFibNum(index + 1))
				{
					correctSize = true;
				}
			}
			else
			{
				int numBlocksPrev = getFibNum(index - 1);
				tempP = ((char*) freeSH) - (numBlocksPrev * blockSize);
				buddySH = (SegmentHeader*) tempP;
				buddySH->CheckValid();
				if((buddySH->length / blockSize) == getFibNum(index - 1))
				{
					correctSize = true;
				}
			}
			if(buddySH->is_free && correctSize)
			{
				if(freeSH->isLeft)
				{
					if(!(fLists[index + 1].Remove(buddySH)))
						cout << "ErrorR1" << endl;
					freeSH->length += buddySH->length;
					freeSH->isLeft = freeSH->inheritance;
					freeSH->inheritance = buddySH->inheritance;
				}
				else
				{					
					if(!(fLists[index - 1].Remove(buddySH)))
						cout << "ErrorR2" << endl;
					buddySH->length += freeSH->length;
					buddySH->isLeft = buddySH->inheritance;
					buddySH->inheritance = freeSH->inheritance;
					freeSH = buddySH;
				}
			}
			else
			{
				if(!(fLists[index].Add(freeSH)))
					cout << "ErrorA4" << endl;
				return true;
			}
		}
	}
}

//Finds the fibonachi number based on the index given
int MyAllocator::getFibNum(int i)
{
	int n1 = 1;
	int n2 = 2;
	if(i < 0)
	{
		return -1;
	}
	else if(i == 0)
	{
		return n1;
	}
	else if(i == 1)
	{
		return n2;
	}
	else
	{
		int num = n1 + n2;
		for(int j = 2; j < i; ++j)
		{
			n1 = n2;
			n2 = num;
			num = n1 + n2;
		}
		return num;
	}	
}
