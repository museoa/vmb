/* File: $Id: types.h,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/****************************************************************************
 * 
 * This header file contains some type definitions needed.
 * 
 * "Word" (unsigned word) and "SWord" (signed word) represent MIPS
 * registers, and were introduced to allow a better debugging; by using them
 * appropriately, it was possible to detect where possibly incorrect
 * manipulation of register values and format were done.
 *
 ****************************************************************************/

// utility types

typedef unsigned long Word;

typedef signed long SWord;

typedef unsigned int Boolean;
