#pragma once
/**
 * @file list.h
 * @author Justen Di Ruscio (3624673)
 * @brief Symbols for doubly linked list and functions to modify it.
 * Doesn't use pool allocator but instead allocates each node individually
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdbool.h>  // bool, true, false
#include <stddef.h>   // size_t

typedef struct ListNode {
  void *data;
  struct ListNode *next;
  struct ListNode *prev;
} ListNode;

typedef struct List {
  ListNode *head;
  ListNode *tail;
  size_t length;
  size_t dataSize;
  void (*elementDeleter)(void*);
} List;

typedef struct OptionalList {
  List list;
  bool valid;
} OptionalList;

List list_constructEmpty(const size_t dataSize);
OptionalList list_copyConstruct(const List *const other);
void list_freeNodes(List *const list);

ListNode *list_newNode(const size_t dataSize, const void *const element);
void list_freeNode(const List* const list, const ListNode* const);

bool list_containsNode(const List* const list, const ListNode* const node);
ListNode *list_nodeAt(const List* const list, const size_t index);
void *list_elementAt(const List* const list, const size_t index);

bool list_pushFront(List *const list, const void *const element);
bool list_popFront(List *const list);
bool list_pushBack(List *const list, const void *const element);
bool list_pushNodeBack(List* const list, ListNode* const node);
bool list_popBack(List *const list);
bool list_insertAt(List *const list, const size_t index,
                 const void *const element);

bool list_eraseAt(List *const list, const size_t index);
bool list_eraseNode(List *const list, ListNode *const node);
bool list_removeNode(List *const list, ListNode *const node);
