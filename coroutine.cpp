#include <stdlib.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>

#include "coroutine.h"

static void print_ctx_stack(CoroutineContext* ctx)
{
    std::cout << "ctx:" << std::hex << (void*)ctx << std::endl;
    std::cout << "stack:" << std::hex << (void*)ctx->stack_ << std::endl;
    if (ctx != NULL)
    {   
        uint64_t* p = (uint64_t*)ctx->stack_;
        for (int i = 0; i != 10; ++i)
        {   
            //if (i == 1 || i == 2 || i == 7)
            {
                std::cout << std::dec << "[" << std::setw(2) << i << "](" << p << ") ";
                std::cout << std::hex << "0x" << *p << std::endl;
            }

            p++;
        }   
    }   
}

__thread CoroutineContext* g_schedule_ctx = NULL;
__thread CoroutineContext* g_cur_ctx = NULL;

void schedule_thread(void* args)
{
    std::cout << __func__ << std::endl;
    while (true)
    {
        std::cout << __func__ << std::endl;
        sleep(1);
    }
}

CoroutineContext* get_thread_schedule_ctx()
{
    if (g_schedule_ctx == NULL)
    {
        std::cout << __func__ << " schedule_thread:" << (void*)schedule_thread << std::endl;
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

CoroutineContext* CoroutineCreate(RoutineFunc routine_func, void* args)
{
    CoroutineContext* ctx = new CoroutineContext();

    ctx->routine_func_ = routine_func;
    ctx->args_ = args;
    ctx->stack_size_ = kDefaultStackSize;

    void* point_rbp = ctx->stack_ + 1 * sizeof(void*);
    *((void**)point_rbp) = (void*)ctx->stack_;

    void* point_rsp = ctx->stack_ + 2 * sizeof(void*);
    *((void**)point_rsp) = (void*)ctx->stack_;

    void* point_rip = ctx->stack_ + 7 * sizeof(void*);
    *((void**)point_rip) = (void*)ctx->routine_func_;

    //std::cout << __func__ << " # init ctx:" << ctx << std::endl;
    //print_ctx_stack(ctx);

    return ctx;
}

void Swap(CoroutineContext* pre, CoroutineContext* cur, bool store)
{
    if (store)
    {
        std::cout << __func__ << " # before store register" << std::endl;
        print_ctx_stack(pre);
        AsmStoreRegister(pre);
        std::cout << __func__ << " # after store register" << std::endl;
        print_ctx_stack(pre);
    }

    g_cur_ctx = cur;
    std::cout << __func__ << " # before load register" << std::endl;
    print_ctx_stack(cur);
    AsmLoadRegister(cur);
    std::cout << __func__ << " # after load register" << std::endl;
    print_ctx_stack(cur);
}

void Yield(CoroutineContext* ctx)
{
    std::cout << __func__ << std::endl;
    Swap(ctx, get_thread_schedule_ctx(), true);
}

void Resume(CoroutineContext* ctx)
{
    std::cout << __func__ << std::endl;
    Swap(get_thread_schedule_ctx(), ctx, false);
}
