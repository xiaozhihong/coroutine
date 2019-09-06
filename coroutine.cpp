#include <stdlib.h>

#include <iostream>

#include "coroutine.h"

const int kDefaultStackSize = 16*1024;
__thread CoroutineContext* g_schedule_ctx = NULL;
__thread CoroutineContext* g_cur_ctx = NULL;

void schedule_thread(void* args)
{
    std::cout << __func__ << std::endl;
}

CoroutineContext* get_thread_schedule_ctx()
{
    if (g_schedule_ctx == NULL)
    {
        g_schedule_ctx = CoroutineCreate(schedule_thread, NULL);
        g_schedule_ctx->AllocStack();
    }

    return g_schedule_ctx;
}

CoroutineContext* get_cur_ctx()
{
    return g_cur_ctx;
}

extern "C"
{
    extern void AsmStoreRegister(CoroutineContext*) asm("AsmStoreRegister");
    extern void AsmLoadRegister(CoroutineContext*) asm("AsmLoadRegister");
}

CoroutineContext::CoroutineContext(RoutineFunc routine_func, void* args, const int& stack_size)
    : routine_func_(routine_func)
    , args_(args)
    , stack_size_(stack_size)
    , stack_(NULL)
{
}

void CoroutineContext::AllocStack()
{
    std::cout << __func__ << " # alloc " << stack_size_ << " bytes stack " << std::endl;
    stack_ = (uint8_t*)calloc(stack_size_, sizeof(uint8_t));
}

void CoroutineContext::StoreRegister()
{
    uint64_t u = 0;
    std::cout << __func__ << " # before &u = " << (void*)&u << std::endl;
    AsmStoreRegister(this);
    std::cout << __func__ << " # after &u = " << (void*)&u << std::endl;
}

void CoroutineContext::LoadRegister()
{
    AsmLoadRegister(this);
}

CoroutineContext* CoroutineCreate(RoutineFunc routine_func, void* args)
{
    CoroutineContext* ctx = new CoroutineContext(routine_func, args, kDefaultStackSize);

    return ctx;
}

void Swap(CoroutineContext* pre, CoroutineContext* cur)
{
    pre->StoreRegister();
    g_cur_ctx = cur;
    cur->LoadRegister();
}

void Yield(CoroutineContext* ctx)
{
    Swap(get_thread_schedule_ctx(), ctx);
}

void Resume(CoroutineContext* ctx)
{
    if (ctx->stack_ == NULL)
    {
        g_cur_ctx = ctx;
        ctx->AllocStack();
        ctx->routine_func_(ctx->args_);
    }
    else
    {
        Swap(ctx, get_thread_schedule_ctx());
    }
}
