#include <stdlib.h>

#include <iomanip>
#include <iostream>

#include "coroutine.h"

static void print_ctx_stack(CoroutineContext* ctx)
{
    std::cout << "ctx:" << ctx << std::endl;
    std::cout << "stack:" << std::endl;
    if (ctx != NULL)
    {   
        uint64_t* p = (uint64_t*)ctx->stack_;
        for (int i = 0; i != 10; ++i)
        {   
            std::cout << std::dec << "[" << std::setw(2) << i << "](" << p << ") ";
            std::cout << std::hex << "0x" << *p++ << std::endl;
        }   
    }   
}

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

CoroutineContext::CoroutineContext(RoutineFunc routine_func, void* args, const uint32_t& stack_size)
    : routine_func_(routine_func)
    , args_(args)
    , stack_size_(stack_size)
{
    stack_ = (uint8_t*)calloc(stack_size_, sizeof(uint8_t));

    void* point_rbp = stack_ + 1 * sizeof(void*);
    *((void**)point_rbp) = (void*)stack_;

    void* point_rsp = stack_ + 2 * sizeof(void*);
    *((void**)point_rsp) = (void*)point_rsp;

    void* point_rip = stack_ + 7 * sizeof(void*);
    *((void**)point_rip) = (void*)routine_func_;
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
    std::cout << __func__ << " # before " << std::endl;
    AsmLoadRegister(this);
    std::cout << __func__ << " # after " << std::endl;
}

CoroutineContext* CoroutineCreate(RoutineFunc routine_func, void* args)
{
    CoroutineContext* ctx = new CoroutineContext(routine_func, args, kDefaultStackSize);

    return ctx;
}

void Swap(CoroutineContext* pre, CoroutineContext* cur)
{
    std::cout << "pre ctx stack" << std::endl;
    print_ctx_stack(pre);
    std::cout << "cur ctx stack" << std::endl;
    print_ctx_stack(cur);
    pre->StoreRegister();
    g_cur_ctx = cur;
    cur->LoadRegister();
}

void Yield(CoroutineContext* ctx)
{
    std::cout << __func__ << std::endl;
    Swap(ctx, get_thread_schedule_ctx());
}

void Resume(CoroutineContext* ctx)
{
    std::cout << __func__ << std::endl;
    print_ctx_stack(ctx);

    Swap(get_thread_schedule_ctx(), ctx);
}
