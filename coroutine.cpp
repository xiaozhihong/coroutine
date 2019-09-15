#include <assert.h>
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
        for (int i = 0; i != 12; ++i)
        {   
            std::cout << std::dec << "[" << std::setw(2) << i << "](" << p << ") ";
            std::cout << std::hex << "0x" << *p << std::endl;
            p++;
        }   
    }   
}

extern "C"
{
    extern void AsmSwapRegister(CoroutineContext*, CoroutineContext*) asm("AsmSwapRegister");
    extern void AsmInitRegister(CoroutineContext*) asm("AsmInitRegister");
    extern void AsmLoadRegister(CoroutineContext*) asm("AsmLoadRegister");
}

void CoroutineEntry(void* args)
{
    CoroutineContext* ctx = (CoroutineContext*)args;

    std::cout << __func__ << " coroutine entry, ctx:" << ctx << ", args:" << ctx->args_ << std::endl;

    if (ctx->routine_func_)
    {
        ctx->routine_func_(ctx->args_);
    }

    delete ctx;
    std::cout << __func__ << " coroutine done, ctx:" << ctx << std::endl;
    AsmLoadRegister(get_thread_schedule_ctx());
}

__thread CoroutineContext* g_schedule_ctx = NULL;
__thread CoroutineContext* g_cur_ctx = NULL;

std::list<CoroutineContext*> g_ctx_list;

void CoEventLoop();
void schedule_thread(void* args)
{
    while (true)
    {
        if (! g_ctx_list.empty())
        {
            CoroutineContext* ctx = g_ctx_list.front();
            g_ctx_list.pop_front();
            //std::cout << __func__ << " # resume pending ctx:" << (void*)ctx << std::endl;
            if (ctx != NULL)
            {
                Resume(ctx);
            }
        }
        else
        {
            std::cout << __func__ << " # usleep " << std::endl;
            usleep(100000);
        }
    }
}

CoroutineContext* get_thread_schedule_ctx()
{
    if (g_schedule_ctx == NULL)
    {
        g_schedule_ctx = CoroutineCreate("main", NULL, NULL);
    }

    return g_schedule_ctx;
}

CoroutineContext* get_cur_ctx()
{
    return g_cur_ctx;
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

    void* stack_bottom = ctx->stack_ + ctx->stack_size_;
    void* stack_bottom_align = (void*)(((uint64_t)stack_bottom) & 0xFFFFFFFFFFFFFF00);

    void* point_rbp = ctx->stack_ + 1 * sizeof(void*);
    *((void**)point_rbp) = (void*)(stack_bottom_align);

    void* point_rsp = ctx->stack_ + 2 * sizeof(void*);
    *((void**)point_rsp) = (void*)(stack_bottom_align);

    void* point_rip = ctx->stack_ + 4 * sizeof(void*);
    *((void**)point_rip) = (void*)CoroutineEntry;

    AsmInitRegister(ctx);

    return ctx;
}

void Swap(CoroutineContext* pre, CoroutineContext* cur)
{
    g_cur_ctx = cur;
    AsmSwapRegister(pre, cur);
}

void Yield(CoroutineContext* ctx)
{
    g_ctx_list.push_back(ctx);
#if 1
    Swap(ctx, get_thread_schedule_ctx());
#else
    g_cur_ctx = get_thread_schedule_ctx();
    AsmSwapRegister(ctx, g_cur_ctx);
#endif
}

void Resume(CoroutineContext* ctx)
{
#if 1
    Swap(get_thread_schedule_ctx(), ctx);
#else
    g_cur_ctx = ctx;
    AsmSwapRegister(get_thread_schedule_ctx(), g_cur_ctx);
#endif
}
