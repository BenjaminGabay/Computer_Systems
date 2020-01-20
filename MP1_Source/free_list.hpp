/* 
    File: free_list.hpp

    Author: <your name>
            Department of Computer Science and Engineering
            Texas A&M University
    Date  : <date>

    Modified:

*/

#ifndef _free_list_hpp_                   // include file only once
#define _free_list_hpp_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cstdlib>
#include <cassert>

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

class SegmentHeader {

private:

  static const unsigned COOKIE_VALUE = 0xBAAB00;
  unsigned int cookie; /* To check whether this is a genuine header! */
  
  
public:

	size_t length;
	bool is_free;
	bool isLeft;
	bool inheritance;
	
	// You will need additional data here!
	SegmentHeader* next;
	SegmentHeader* prev;

  SegmentHeader(size_t _length, bool _isLeft = true, bool _inheritance = true, bool _is_free = true);
  
  ~SegmentHeader();
  /* We probably won't need the destructor. */

  void CheckValid();
  /* Check if the cookie is valid. */
};

/*--------------------------------------------------------------------------*/
/* FORWARDS */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS  FreeList */
/*--------------------------------------------------------------------------*/

class FreeList {

 private:

public:

	/* Here you add whatever private members you need...*/
  SegmentHeader* head;
  int numBlocks;
  size_t blockSize;

  FreeList(int _numBlocks, size_t _blockSize); 
  /* This function initializes a new free-list. */

  ~FreeList(); 
  /* We probably don't need a destructor. */ 

  bool Remove(SegmentHeader * _segment); 
  /* Remove the given segment from the given free list. 
     Returns true if the function succeeds.
  */ 

  bool Add(SegmentHeader * _segment); 
  /* Add the segment to the given free list. */
  
  SegmentHeader* Split(SegmentHeader * cSeg, size_t _length);
  /* Split segment into one of size _length and a return a new segment
	 with the remaining length
  */
};

#endif 
