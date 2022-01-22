/**
 * @file list.c
 * @author Justen Di Ruscio (3624673)
 * @brief Definitions for doubly linked list and functions to modify it.
 * Doesn't use pool allocator but instead allocates each node individually
 * @version 0.1
 * @date 2021-02-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <jd/list.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List list_constructEmpty(const size_t dataSize) {
  List result = {.head = (void *)NULL,
                 .tail = (void *)NULL,
                 .length = 0,
                 .dataSize = dataSize,
                 .elementDeleter = NULL};
  return result;
}

OptionalList list_copyConstruct(const List *const other) {
  const char fooName[] = "list_copyConstruct";
  List newList = list_constructEmpty(0);
  OptionalList result = {.list = newList, .valid = false};

  // Argument Validity Check
  errno = 0;
  if (other == (List *)NULL) {
    fprintf(stderr, "field 'other' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return result;
  }

  // Copy Construct
  // list pointers
  newList.dataSize = other->dataSize;
  newList.head = other->head;
  newList.tail = other->tail;
  // nodes
  ListNode *node = other->head;
  while (node != (ListNode *)NULL) {
    const bool pushed = list_pushBack(&newList, node->data);
    if (!pushed) {
      fprintf(stderr, "Unable to push back element in %s\n", fooName);
      return result; // errno set by list_pushBack
    }
    node = node->next;
  }

  result.list = newList;
  result.valid = true;
  return result;
}

void list_freeNodes(List *const list) {
  // Return if there's nothing to free
  if (list == (List *)NULL) {
    return;
  }
  if (list->head == (ListNode *)NULL) {
    return;
  }

  // Free Nodes
  const ListNode *node = list->head;
  while (node != (ListNode *)NULL) {
    const ListNode *const next = node->next;
    list_freeNode(list, node);
    node = next;
  }
  list->head = (ListNode *)NULL;
  list->tail = (ListNode *)NULL;
  list->length = 0;
}

ListNode *list_nodeAt(const List *const list, const size_t index) {
  const char fooName[] = "list_nodeAt";

  // Argument Validity Check
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return (ListNode *)NULL;
  }
  if (index >= list->length) {
    fprintf(stderr,
            "field 'index' (%zu) is too large to index the provided list of "
            "length %zu in %s\n",
            index, list->length, fooName);
    errno = EOVERFLOW;
    return (ListNode *)NULL;
  }

  // Access node at index
  ListNode *node = list->head;
  if (node == (ListNode *)NULL) {
    fprintf(stderr, "Provided list to %s is empty. Cannot access node %zu\n",
            fooName, index);
    errno = EFAULT;
  }
  for (size_t i = 0; i < index; ++i) {
    node = node->next;
    if (node == (ListNode *)NULL) {
      fprintf(
          stderr,
          "list nodes aren't linked by 'next'. Failure to iterate list in %s\n",
          fooName);
      errno = EFAULT;
      break;
    }
  }
  return node;
}

void *list_elementAt(const List *const list, const size_t index) {
  const char fooName[] = "list_elementAt";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return (ListNode *)NULL;
  }
  if (index >= list->length) {
    fprintf(stderr,
            "field 'index' (%zu) is too large to index the provided list of "
            "length %zu in %s\n",
            index, list->length, fooName);
    errno = EOVERFLOW;
    return (ListNode *)NULL;
  }

  // Access element at index
  ListNode *node = list_nodeAt(list, index);
  if (node == (ListNode *)NULL) {
    fprintf(stderr,
            "failure accessing node in linked list at index %zu in %s\n", index,
            fooName);
    return (void *)NULL; // errno set by list_nodeAt
  }
  return node->data;
}

bool list_pushFront(List *const list, const void *const element) {
  const char fooName[] = "list_pushFront";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (element == (void *)NULL) {
    fprintf(stderr, "field 'element' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Push Front
  // allocate new node
  ListNode *newNode = list_newNode(list->dataSize, element);
  if (newNode == (ListNode *)NULL) {
    fprintf(stderr, "Unable to create new node for element in %s\n", fooName);
    return false; // errno set by list_newNode
  }
  // assign to front
  if (list->head != NULL) {
    list->head->prev = newNode;
  } else {
    list->tail = newNode;
  }
  newNode->next = list->head;
  list->head = newNode;
  ++list->length;
  return true;
}

bool list_popFront(List *const list) {
  const char fooName[] = "list_popFront";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Pop Front
  const ListNode *const head = list->head;
  // no nodes; list empty
  if (head == (ListNode *)NULL) {
    errno = EINVAL;
    return false;
  }
  // single node; popping will make empty
  if (head->next == (ListNode *)NULL) {
    list->head = (ListNode *)NULL;
    list->tail = (ListNode *)NULL;
  }
  // multiple nodes in list
  else {
    list->head = head->next;
    list->head->prev = (ListNode *)NULL;
  }

  list_freeNode(list, head);
  --list->length;
  return true;
}

bool list_pushNodeBack(List *const list, ListNode *const node) {
  const char fooName[] = "list_pushNodeBack";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (node == (void *)NULL) {
    fprintf(stderr, "field 'node' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Push Back
  // assign to back
  if (list->tail != (ListNode *)NULL) {
    list->tail->next = node;
  } else {
    list->head = node;
  }
  node->prev = list->tail;
  list->tail = node;
  ++list->length;
  return true;
}

bool list_pushBack(List *const list, const void *const element) {
  const char fooName[] = "list_pushBack";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (element == (void *)NULL) {
    fprintf(stderr, "field 'element' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Push Back
  // allocate new node
  ListNode *newNode = list_newNode(list->dataSize, element);
  if (newNode == (ListNode *)NULL) {
    fprintf(stderr, "Unable to create new node for element in %s\n", fooName);
    return false; // errno set by list_newNode
  }
  return list_pushNodeBack(list, newNode);
}

bool list_popBack(List *const list) {
  const char fooName[] = "list_popBack";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Pop Back
  const ListNode *const tail = list->tail;
  // no nodes; list empty
  if (tail == (ListNode *)NULL) {
    errno = EINVAL;
    return false;
  }
  // single node; popping will make empty
  if (tail->prev == (ListNode *)NULL) {
    list->head = (ListNode *)NULL;
    list->tail = (ListNode *)NULL;
  }
  // multiple nodes in list
  else {
    list->tail = tail->prev;
    list->tail->next = (ListNode *)NULL;
  }

  list_freeNode(list, tail);
  --list->length;
  return true;
}

bool list_insertAt(List *const list, const size_t index,
                   const void *const element) {
  const char fooName[] = "list_insertAt";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (index >= list->length) {
    fprintf(stderr,
            "field 'index' (%zu) is too large to index provided list of length "
            "%zu in %s\n",
            index, list->length, fooName);
    errno = EOVERFLOW;
    return false;
  }
  if (element == (void *)NULL) {
    fprintf(stderr, "field 'element' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Insert
  // push onto front
  if (index == 0) {
    const bool pushed = list_pushFront(list, element);
    if (!pushed) {
      fprintf(stderr, "Failure pushing element into list at index 0 in %s\n",
              fooName);
    }
    return pushed; // errno set by list_pushFront
  }
  // push onto back
  else if (index == list->length - 1) {
    const bool pushed = list_pushBack(list, element);
    if (!pushed) {
      fprintf(stderr, "Failure pushing element into list at index %zu in %s\n",
              list->length - 1, fooName);
    }
    return pushed; // errno set by list_pushBack
  }
  // push somewhere in between
  else {
    // construct new node
    ListNode *const new = list_newNode(list->dataSize, element);
    if (new == (ListNode *)NULL) {
      fprintf(stderr, "Error constructing new node in %s\n", fooName);
      return false; // errno set by list_newNode
    }
    // find location to insert new node
    ListNode *const node = list_nodeAt(list, index);
    if (node == (ListNode *)NULL) {
      fprintf(stderr, "Failure accessing node at index %zu in %s\n", index,
              fooName);
      return false; // errno set by list_nodeAt
    }
    // insert node
    new->next = node;
    new->prev = node->prev;
    node->prev->next = new;
    node->prev = new;
    return true;
  }
}

bool list_eraseAt(List *const list, const size_t index) {
  const char fooName[] = "list_eraseAt";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (index >= list->length) {
    fprintf(stderr,
            "field 'index' (%zu) is too large to index provided list of length "
            "%zu in %s\n",
            index, list->length, fooName);
    errno = EOVERFLOW;
    return false;
  }
  if (list->head == (ListNode *)NULL) {
    fprintf(stderr, "cannot erase from an empty list in %s\n", fooName);
    errno = EPERM;
    return false;
  }

  // Erase node at index
  // erase from front
  if (index == 0) {
    const bool erased = list_popFront(list);
    if (!erased) {
      fprintf(stderr, "Failure erasing element from list at index 0 in %s\n",
              fooName);
    }
    return erased; // errno set by list_popFront
  }
  // erase from back
  else if (index == list->length - 1) {
    const bool erased = list_popBack(list);
    if (!erased) {
      fprintf(stderr, "Failure erasing element from list at index %zu in %s\n",
              list->length - 1, fooName);
    }
    return erased; // errno set by list_popBack
  }
  // erase somewhere in between
  else {
    // find location to erase node
    ListNode *const node = list_nodeAt(list, index);
    if (node == (ListNode *)NULL) {
      fprintf(stderr, "Failure accessing node at index %zu in %s\n", index,
              fooName);
      return false; // errno set by list_nodeAt
    }
    // erase node
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list_freeNode(list, node);
    --list->length;
    return true;
  }
}

bool list_eraseNode(List *const list, ListNode *const node) {
  const char fooName[] = "list_eraseNode";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (node == (ListNode *)NULL) {
    fprintf(stderr, "argument 'node' of %s must point to a valid address\n",
            fooName);
    errno = EOVERFLOW;
    return false;
  }
  if (list->head == (ListNode *)NULL) {
    fprintf(stderr, "cannot erase from an empty list in %s\n", fooName);
    errno = EPERM;
    return false;
  }

  // Erase node
  // erase from front
  if (node == list->head) {
    const bool erased = list_popFront(list);
    if (!erased) {
      fprintf(stderr, "Failure erasing first node from list in %s\n", fooName);
    }
    return erased; // errno set by list_popFront
  }
  // erase from back
  else if (node == list->tail) {
    const bool erased = list_popBack(list);
    if (!erased) {
      fprintf(stderr, "Failure erasing last node from list in %s\n", fooName);
    }
    return erased; // errno set by list_popBack
  }
  // erase somewhere in between
  else {
    // erase node
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list_freeNode(list, node);
    --list->length;
    return true;
  }
}

ListNode *list_newNode(const size_t dataSize, const void *const element) {
  const char fooName[] = "list_newNode";

  // Argument Validity Check
  errno = 0;
  if (dataSize == 0 && element == (void *)NULL) {
    fprintf(stderr,
            "field 'dataSize' in %s must be greater than 0 when an element is "
            "provided\n",
            fooName);
    errno = EPERM;
    return (ListNode *)NULL;
  }

  // Allocate new node
  void *const listNodeBuffer = malloc(sizeof(ListNode) + dataSize);
  if (listNodeBuffer == (void *)NULL) {
    fprintf(stderr, "failure allocating memory for new ListNode in %s\n",
            fooName);
    return (ListNode *)NULL; // errno set by malloc
  }
  ListNode *newNode = (ListNode *)listNodeBuffer;
  newNode->data = (char *)listNodeBuffer + sizeof(ListNode);
  newNode->next = (ListNode *)NULL;
  newNode->prev = (ListNode *)NULL;

  // Copy element contents to new node
  if (element != (void *)NULL) {
    memcpy(newNode->data, element, dataSize);
  }
  return newNode;
}

void list_freeNode(const List *const list, const ListNode *const node) {
  if (list == (List *)NULL) {
    return;
  }
  if (list->elementDeleter != NULL) {
    list->elementDeleter(*(void **)node->data);
  }
  free((ListNode *)node);
}

bool list_containsNode(const List *const list, const ListNode *const node) {
  const char fooName[] = "list_containsNode";
  bool contains = false;

  // Argument Validity Checks
  if (list == (List *)NULL) {
    fprintf(stderr, "argument 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (node == (ListNode *)NULL) {
    fprintf(stderr, "argument 'node' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }

  // Find Node
  const ListNode *current = list->head;
  while (current != (ListNode *)NULL) {
    if (current == node) {
      contains = true;
      break;
    }
    current = current->next;
  }
  return contains;
}

bool list_removeNode(List *const list, ListNode *const node) {
  const char fooName[] = "list_removeNode";

  // Argument Validity Checks
  errno = 0;
  if (list == (List *)NULL) {
    fprintf(stderr, "field 'list' of %s must point to a valid address\n",
            fooName);
    errno = EPERM;
    return false;
  }
  if (node == (ListNode *)NULL) {
    fprintf(stderr, "argument 'node' of %s must point to a valid address\n",
            fooName);
    errno = EOVERFLOW;
    return false;
  }
  if (list->head == (ListNode *)NULL) {
    fprintf(stderr, "cannot remove from an empty list in %s\n", fooName);
    errno = EPERM;
    return false;
  }

  // Remove node
  // remove from front
  if (node == list->head) {
    list->head = node->next;
    if (list->head != NULL) {
      list->head->prev = NULL;
    }
  }
  // remove from back
  else if (node == list->tail) {
    list->tail = node->prev;
    if (list->tail != NULL) {
      list->tail->next = NULL;
    }
  }
  // remove somewhere in between
  else {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }
  node->next = NULL;
  node->prev = NULL;
  --list->length;
  return true;
}
