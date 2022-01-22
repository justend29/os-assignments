/**
 * @file queue.h
 * @author Justen Di Ruscio
 * @brief Declarations for functions related to queue data structure using linked
 * list
 * @version 0.1
 * @date 2021-03-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once

#include <jd/list.h>

typedef struct Queue {
  List elements;
} Queue;

Queue q_constructEmpty(const size_t dataSize);
void q_freeElements(Queue* const q);

bool q_enqueue(Queue* const q, const void *const element);
bool q_dequeue(Queue* const q);

void* q_front(const Queue* const q);
void* q_back(const Queue* const q);
size_t q_length(const Queue* const q);
