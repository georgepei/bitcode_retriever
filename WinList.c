//
//  efilink.c
//
//  EFI linked list functions
//

#include <stdbool.h>
#include "WinList.h"

#define ATTRIBUTE_NO_INSTRUMENT __attribute__((no_instrument_function))

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
InitializeListHead (
   LIST_ENTRY                *ListHead
  )
{
  ListHead->ForwardLink = ListHead;
  ListHead->BackLink = ListHead;
  return ListHead;
}

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
InsertHeadList (
  LIST_ENTRY                *ListHead,
  LIST_ENTRY                *Entry
  )
{
  Entry->ForwardLink = ListHead->ForwardLink;
  Entry->BackLink = ListHead;
  Entry->ForwardLink->BackLink = Entry;
  ListHead->ForwardLink = Entry;
  return ListHead;
}

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
InsertTailList (
  LIST_ENTRY                *ListHead,
  LIST_ENTRY                *Entry
  )
{
  Entry->ForwardLink = ListHead;
  Entry->BackLink = ListHead->BackLink;
  Entry->BackLink->ForwardLink = Entry;
  ListHead->BackLink = Entry;
  return ListHead;
}

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
GetFirstNode (
  LIST_ENTRY          *List
  )
{
  return List->ForwardLink;
}

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
GetNextNode (
   LIST_ENTRY          *List,
   LIST_ENTRY          *Node
  )
{
  return Node->ForwardLink;
}

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
GetPreviousNode (
   LIST_ENTRY          *List,
   LIST_ENTRY          *Node
  )
{
  return Node->BackLink;
}

ATTRIBUTE_NO_INSTRUMENT
bool
IsListEmpty (
   LIST_ENTRY          *ListHead
  )
{
  return (bool)(ListHead->ForwardLink == ListHead);
}

ATTRIBUTE_NO_INSTRUMENT
bool
IsNull (
   LIST_ENTRY          *List,
   LIST_ENTRY          *Node
  )
{
  return (bool)(Node == List);
}

ATTRIBUTE_NO_INSTRUMENT
bool
IsNodeAtEnd (
  LIST_ENTRY          *List,
  LIST_ENTRY          *Node
  )
{
  return (bool)(!IsNull (List, Node) && List->BackLink == Node);
}

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
SwapListEntries (
  LIST_ENTRY                *FirstEntry,
  LIST_ENTRY                *SecondEntry
  )
{
  LIST_ENTRY                    *Ptr;

  if (FirstEntry == SecondEntry) {
    return SecondEntry;
  }

  //
  // Ptr is the node pointed to by FirstEntry->ForwardLink
  //
  Ptr = RemoveEntryList (FirstEntry);

  //
  // If FirstEntry immediately follows SecondEntry, FirstEntry will be placed
  // immediately in front of SecondEntry
  //
  if (Ptr->BackLink == SecondEntry) {
    return InsertTailList (SecondEntry, FirstEntry);
  }

  //
  // Ptr == SecondEntry means SecondEntry immediately follows FirstEntry,
  // then there are no further steps necessary
  //
  if (Ptr == InsertHeadList (SecondEntry, FirstEntry)) {
    return Ptr;
  }

  //
  // Move SecondEntry to the front of Ptr
  //
  RemoveEntryList (SecondEntry);
  InsertTailList (Ptr, SecondEntry);
  return SecondEntry;
}

ATTRIBUTE_NO_INSTRUMENT
LIST_ENTRY *
RemoveEntryList (
  LIST_ENTRY          *Entry
  )
{
  
  Entry->ForwardLink->BackLink = Entry->BackLink;
  Entry->BackLink->ForwardLink = Entry->ForwardLink;
  return Entry->ForwardLink;
}
