/* 
    File: free_list.cpp

    Author: <your name>
            Department of Computer Science
            Texas A&M University
    Date  : <date>

    Modified: 

    This file contains the implementation of the class FreeList.

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <iostream>
#include "free_list.hpp"

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
/* FUNCTIONS FOR CLASS SegmentHeader */
/*--------------------------------------------------------------------------*/

SegmentHeader::SegmentHeader(size_t _length, bool _isLeft, bool _inheritance, bool _is_free) {
  length = _length;
  is_free = _is_free;
  isLeft = _isLeft;
  inheritance = _inheritance;
  cookie = COOKIE_VALUE;
  // You may need to initialize more members here!
  next = nullptr;
  prev = nullptr;
}

SegmentHeader::~SegmentHeader() {
  // You may need to add code here.
  //next = nullptr;
  //prev = nullptr;
}


void SegmentHeader::CheckValid() {
  if (cookie != COOKIE_VALUE) {
    cout << "INVALID SEGMENT HEADER!!" << endl;
    assert(false);
    // You will need to check with the debugger to see how we got into this
    // predicament.
  }
}

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR CLASS FreeList */
/*--------------------------------------------------------------------------*/

FreeList::FreeList(int _numBlocks, size_t _blockSize) {
	head = nullptr;
	numBlocks = _numBlocks;
	blockSize = _blockSize;
}

FreeList::~FreeList() {
  // You may need to add code here.
  //delete head;
}

bool FreeList::Add(SegmentHeader * _segment) {
	if(_segment == nullptr)
	{
		//assert(false); // This implementation does nothing, other than abort.
		return false;
	}
	if((_segment->length) / blockSize != numBlocks)
	{
		return false;
	}	
	else
	{
		if(head == nullptr)
		{
			head = _segment;
		}
		else
		{
			_segment->next = head;
			head->prev = _segment;
			head = _segment;
		}
		return true;
	}
  //assert(false); // This implementation does nothing, other than abort.
}

bool FreeList::Remove(SegmentHeader * _segment)
{
	if(_segment == nullptr)
	{
		//assert(false); // This implementation does nothing, other than abort.
		return false;
	}

	if((_segment->length) / blockSize != numBlocks)
	{
		return false;
	}
	else
	{
		if(_segment == head)
		{
			head = head->next;
			_segment->next = nullptr;
			if(head != nullptr)
			{
				head->prev = nullptr;
			}			
			_segment->prev = nullptr;
		}
		else if(_segment->next == nullptr)
		{
			_segment->prev->next = nullptr;
			_segment->next = nullptr;
			_segment->prev = nullptr;
		}
		else
		{
			_segment->prev->next = _segment->next;
			_segment->next->prev = _segment->prev;
			_segment->prev = nullptr;
			_segment->next = nullptr;
		}	
		return true;
	}
}

SegmentHeader* FreeList::Split(SegmentHeader * cSeg, size_t _length)
{
	if(cSeg->length > _length)
	{
		SegmentHeader* nSeg = new(((char*) cSeg) + _length) SegmentHeader(cSeg->length - _length, false, cSeg->inheritance);
		nSeg->CheckValid();
		cSeg->length = _length;
		cSeg->inheritance = cSeg->isLeft;
		cSeg->isLeft = true;
		return nSeg;
	}
	return nullptr;
}