#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stdint.h>

typedef void(RoutineFunc)(void*);

const int kDefaultStackSize = 16*1024;

struct CoroutineContext
{
    uint8_t stack_[kDefaultStackSize]; // bottom of the stack
    void* args_;
    uint32_t stack_size_;
    RoutineFunc* routine_func_;
};

CoroutineContext* get_thread_schedule_ctx();
CoroutineContext* get_cur_ctx();

int CoroutineEntry();
CoroutineContext* CoroutineCreate(RoutineFunc routine_func, void* args);
void Yield(CoroutineContext* ctx);
void Resume(CoroutineContext* ctx);
void Swap(CoroutineContext* pre, CoroutineContext* cur, bool store);

#endif // __COROUTINE_H__
