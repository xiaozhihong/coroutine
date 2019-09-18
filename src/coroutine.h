#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stdint.h>

#include <string>

typedef void(RoutineFunc)(void*);

const int kDefaultStackSize = 16*1024;

struct CoroutineContext
{
    uint8_t* stack_; // bottom of the stack
    void* args_;
    uint32_t stack_size_;
    RoutineFunc* routine_func_;
    std::string name_;
};

void schedule_thread(void* args);

CoroutineContext* get_main_ctx();
CoroutineContext* get_cur_ctx();

CoroutineContext* CreateCoroutine(const std::string& name, RoutineFunc routine_func, void* args = NULL, const int& stack_size = kDefaultStackSize);
void Yield(CoroutineContext* ctx);
void Resume(CoroutineContext* ctx);
void Swap(CoroutineContext* pre, CoroutineContext* cur);

#endif // __COROUTINE_H__
