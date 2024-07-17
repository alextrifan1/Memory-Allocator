# Memory-Allocator
## Simple memory allocator made in C

Includes implementations for malloc, free, calloc, and realloc functions. The allocator manages dynamic memory allocation with the help of a custom header structure.

**Features**:
+ Alignment: Memory headers are aligned to 16 bytes.
+ Custom Header: Each allocated block has a header that stores the size, allocation status, and a pointer to the next block.
+ Thread Safety: All memory operations are protected with a mutex to support concurrent access.(The current implementation has several areas that can be improved to make it more memory safe 😓)
+ Linked List Management: Free blocks are managed using a linked list to efficiently find and reuse free memory blocks.

## Memory Layout of C Programs
  ![](https://media.geeksforgeeks.org/wp-content/uploads/memoryLayoutC.jpg)

  
