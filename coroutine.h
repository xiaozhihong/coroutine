#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stdint.h>

typedef void(RoutineFunc)(void*);

struct CoroutineContext
{
    CoroutineContext(RoutineFunc routine_func, void* args, const int& stack_size);
    ~CoroutineContext();

    void AllocStack();

    void StoreRegister();
    void LoadRegister();

    uint8_t* stack_; // bottom of the stack
    void* args_;
    int stack_size_;
    RoutineFunc* routine_func_;
};

CoroutineContext* get_thread_schedule_ctx();
CoroutineContext* get_cur_ctx();

int CoroutineEntry();
CoroutineContext* CoroutineCreate(RoutineFunc routine_func, void* args);
void Yield(CoroutineContext* ctx);
void Resume(CoroutineContext* ctx);
void Swap(CoroutineContext* pre, CoroutineContext* cur);

#endif // __COROUTINE_H__
