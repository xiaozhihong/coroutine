#include <stdlib.h>
#include <unistd.h>

#include <list>
#include <iomanip>
#include <iostream>

#include "coroutine.h"

static void print_ctx_stack(CoroutineContext* ctx)
{
    std::cout << "ctx:" << "\"" << ctx->name_ << "\" " << std::hex << (void*)ctx << std::endl;
    std::cout << "stack:" << std::hex << (void*)ctx->stack_ << std::endl;
    if (ctx != NULL)
    {   
        uint64_t* p = (uint64_t*)ctx->stack_;
        for (int i = 0; i != 10; ++i)
        {   
            std::cout << std::dec << "[" << std::setw(2) << i << "](" << p << ") ";
            std::cout << std::hex << "0x" << *p << std::endl;
            p++;
        }   
    }   
}

void CoroutineEntry(void* args)
{
    std::cout << __func__ << " coroutine entry" << std::endl;
    CoroutineContext* ctx = (CoroutineContext*)args;
    ctx->routine_func_(ctx->args_);

    std::cout << __func__ << " coroutine done" << std::endl;

    Yield(ctx, false);

    delete ctx;

    return;
}

__thread CoroutineContext* g_schedule_ctx = NULL;
__thread CoroutineContext* g_cur_ctx = NULL;
__thread uint64_t g_swap_count = 0;

std::list<CoroutineContext*> g_ctx_list;

void schedule_thread(void* args)
{
    std::cout << __func__ << std::endl;
    while (true)
    {
        std::cout << __func__ << std::endl;
        //usleep(10000);
        sleep(1);

        if (! g_ctx_list.empty())
        {
            CoroutineContext* ctx = g_ctx_list.front();
            std::cout << __func__ << " # resume pending ctx" << std::endl;
            g_ctx_list.pop_front();
            Resume(ctx);
        }
    }
}

CoroutineContext* get_thread_schedule_ctx()
{
    if (g_schedule_ctx == NULL)
    {
        std::cout << __func__ << " schedule_thread:" << (void*)schedule_thread << std::endl;
        g_schedule_ctx = CoroutineCreate("schedule_thread", schedule_thread, NULL);
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
    extern void AsmSwapRegister(CoroutineContext*, CoroutineContext*) asm("AsmSwapRegister");
}

CoroutineContext* CoroutineCreate(const std::string& name, RoutineFunc routine_func, void* args)
{
    CoroutineContext* ctx = new CoroutineContext();

    ctx->routine_func_ = routine_func;
    ctx->args_ = args;
    ctx->stack_size_ = kDefaultStackSize;
    ctx->name_ = name;

    void* point_this = ctx->stack_;
    *((void**)point_this) = (void*)(ctx);

    void* point_rbp = ctx->stack_ + 1 * sizeof(void*);
    *((void**)point_rbp) = (void*)(ctx->stack_ + ctx->stack_size_);

    void* point_rsp = ctx->stack_ + 2 * sizeof(void*);
    *((void**)point_rsp) = (void*)(ctx->stack_ + ctx->stack_size_);

    void* point_rip = ctx->stack_ + 7 * sizeof(void*);
    *((void**)point_rip) = (void*)CoroutineEntry;

    std::cout << "stack:" << (void*)ctx->stack_ << "-" << (void*)(ctx->stack_ + ctx->stack_size_) << std::endl;
    //std::cout << __func__ << " # init ctx:" << ctx << std::endl;
    //print_ctx_stack(ctx);

    return ctx;
}

void Swap(CoroutineContext* pre, CoroutineContext* cur)
{
    if (g_swap_count != 0)
    {
        std::cout << __func__ << " # before store register" << std::endl;
        print_ctx_stack(pre);
        AsmStoreRegister(pre);
        std::cout << __func__ << " # after store register" << std::endl;
        print_ctx_stack(pre);
    }

    ++g_swap_count;

    g_cur_ctx = cur;
    std::cout << __func__ << " # before load register" << std::endl;
    print_ctx_stack(cur);
    AsmLoadRegister(cur);
    std::cout << __func__ << " # after load register" << std::endl;
    print_ctx_stack(cur);
}

void SwapDirect(CoroutineContext* pre, CoroutineContext* cur)
{
    g_cur_ctx = cur;
    AsmSwapRegister(pre, cur);
}

void Yield(CoroutineContext* ctx, const bool& pending)
{
    std::cout << __func__ << std::endl;
    if (pending)
    {
        g_ctx_list.push_back(ctx);
    }
    SwapDirect(ctx, get_thread_schedule_ctx());
}

void Resume(CoroutineContext* ctx)
{
    std::cout << __func__ << std::endl;
    if (g_swap_count == 0)
    {
        Swap(get_thread_schedule_ctx(), ctx);
    }
    else
    {
        SwapDirect(get_thread_schedule_ctx(), ctx);
    }
}
