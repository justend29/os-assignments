/**
 * @file queue.c
 * @author Justen Di Ruscio
 * @brief Definitions for functions related to queue data structure using linked
 * list
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "jd/list.h"
#include <jd/queue.h>

Queue q_constructEmpty(const size_t dataSize) {
  Queue q;
  q.elements = list_constructEmpty(dataSize);
  return q;
}

void q_freeElements(Queue *const q) { list_freeNodes(&q->elements); }

bool q_enqueue(Queue *const q, const void *const element) {
  return list_pushBack(&q->elements, element);
}

bool q_dequeue(Queue *const q) { return list_popFront(&q->elements); }

void *q_front(const Queue *const q) { return list_elementAt(&q->elements, 0); }

void *q_back(const Queue *const q) {
  return list_elementAt(&q->elements, q_length(q) - 1);
}

size_t q_length(const Queue *const q) { return q->elements.length; }
