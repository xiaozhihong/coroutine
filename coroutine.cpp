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
    CoroutineContext* ctx = (CoroutineContext*)args;

    std::cout << __func__ << " coroutine entry, ctx:" << ctx << ", args:" << ctx->args_ << std::endl;

    ctx->routine_func_(ctx->args_);

    std::cout << __func__ << " coroutine done, ctx:" << ctx << std::endl;

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
        usleep(100*1000);
        //usleep(1);
        //sleep(1);

        if (! g_ctx_list.empty())
        {
            CoroutineContext* ctx = g_ctx_list.front();
            g_ctx_list.pop_front();
            std::cout << __func__ << " # resume pending ctx:" << (void*)ctx << std::endl;
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

    std::cout << "stack:" << (void*)ctx->stack_ << "-" << (void*)(ctx->stack_ + ctx->stack_size_) << ", args:" << args << std::endl;
    //std::cout << __func__ << " # init ctx:" << ctx << std::endl;
    //print_ctx_stack(ctx);

    std::cout << __func__ << " # push ctx:" << (void*)ctx << std::endl;
    g_ctx_list.push_back(ctx);

    return ctx;
}

void Swap(CoroutineContext* pre, CoroutineContext* cur)
{
    g_cur_ctx = cur;
    AsmSwapRegister(pre, cur);
}

void Yield(CoroutineContext* ctx, const bool& pending)
{
    std::cout << __func__ << std::endl;
    if (pending)
    {
        std::cout << __func__ << " # push ctx:" << (void*)ctx << std::endl;
        g_ctx_list.push_back(ctx);
    }
    Swap(ctx, get_thread_schedule_ctx());
}

void Resume(CoroutineContext* ctx)
{
    std::cout << __func__ << std::endl;
    if (g_swap_count == 0)
    {
        auto iter = g_ctx_list.begin();
        while (iter != g_ctx_list.end())
        {
            if (*iter == ctx)
            {
                iter = g_ctx_list.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
        g_cur_ctx = ctx;
        AsmLoadRegister(ctx);
    }
    else
    {
        Swap(get_thread_schedule_ctx(), ctx);
    }
}
