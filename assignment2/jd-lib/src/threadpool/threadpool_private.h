#pragma once

#include <pthread.h>
#include <stdbool.h>

#include <jd/threadpool.h>

bool tp_spawnThread(ThreadPool* const tp);

void *workerFunction(void* tp);
