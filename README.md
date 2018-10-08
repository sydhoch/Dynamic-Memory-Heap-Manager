# Heap-Manager
Description: Implementing a basic heap manager in C.

The "Makefile" contains information on compiling the program and running the tests that were
given to me by my professor.

The "dmm.h" file was provided to me by my professors so they could set certain variables 
equal to numbers they specified in the file. 

The "dmm.c" creates a struct called metadata_t which is used to keep track of memory 
available to the user in our "freelist". The freelist is a sorted list that maintains all 
of the blocks which are not in use.
This file contains the functions "dmalloc", "dfree", "dmalloc_init", and "print_freelist".

	dmalloc_init:
		This function creates our freelist with one block initially which is our untouched
		memory block. When dmalloc is called, this block will be carved out and separated into
		multiple blocks. These blocks(metadata structs) which will point to each other and then be known as the freelist.
	dmalloc:
		This function is responsible for allocating memory to the user by returning a pointer
		to the memory. It takes memory from the freelist and slices off the amount it needs
		from a block, returns that to the user and returns the rest of the block back to 
		the freelist.
	dfree:
		This function takes a pointer that is given to the program and frees the space and
		puts it back into our "freelist." If the memory given to us is contiguous with a 
		block already in our freelist, we coalesce these blocks together.
	print_freelist:
		This functions prints out our freelist after many dmallocs and dfrees were called by
		the test cases, or by the user. 

