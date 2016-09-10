
#ifndef _WIN_LINK_H
#define _WIN_LINK_H

#include <stdint.h>


//
// List entry - doubly linked list
//

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY  *ForwardLink;
    struct _LIST_ENTRY  *BackLink;
} LIST_ENTRY;



LIST_ENTRY *
InitializeListHead ( LIST_ENTRY * );

LIST_ENTRY *
InsertHeadList ( LIST_ENTRY *ListHead, LIST_ENTRY *Entry);

LIST_ENTRY *
InsertTailList ( LIST_ENTRY *ListHead, LIST_ENTRY  *Entry); 

LIST_ENTRY *
GetFirstNode ( LIST_ENTRY  *List);

LIST_ENTRY *
GetNextNode ( LIST_ENTRY *List, LIST_ENTRY *Node);

LIST_ENTRY *
GetPreviousNode ( LIST_ENTRY *List, LIST_ENTRY *Node);

bool
IsListEmpty ( LIST_ENTRY *ListHead);

bool
IsNull ( LIST_ENTRY  *List, LIST_ENTRY *Node);

bool
IsNodeAtEnd ( LIST_ENTRY *List, LIST_ENTRY *Node);

LIST_ENTRY *
SwapListEntries ( LIST_ENTRY *FirstEntry, LIST_ENTRY  *SecondEntry);

LIST_ENTRY *
RemoveEntryList ( LIST_ENTRY *Entry);

//
//  FIELD_OFFSET - returns the byte offset to a field within a structure
//

#define FIELD_OFFSET(TYPE,Field) ((UINTN)(&(((TYPE *) 0)->Field)))

//
//  CONTAINING_RECORD - returns a pointer to the structure
//      from one of it's elements.
//

#define CONTAINING_RECORD(Record, TYPE, Field)  \
    ((TYPE *) ( (uint8_t *)(Record) - (uint8_t *) &(((TYPE *) 0)->Field)))



#endif

