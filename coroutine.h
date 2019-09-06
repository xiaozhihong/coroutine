#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stdint.h>

typedef void*(RoutineFunc)(void*);

struct CoroutineContext
{
    CoroutineContext(RoutineFunc routine_func, const int& stack_size);
    ~CoroutineContext();

    void StoreRegister();
    void LoadRegister();

    uint8_t* stack_; // bottom of the stack
    RoutineFunc* routine_func_;
};

int CoroutineEntry();
int CoroutineCreate(RoutineFunc routine_func);
int Yiled(CoroutineContext* ctx);
int Resume(CoroutineContext* ctx);
int Swap(CoroutineContext* pre, CoroutineContext* cur);

#endif // __COROUTINE_H__
