#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <list>
#include <iomanip>
#include <iostream>

#include "coroutine.h"
#include "log.h"

static std::string CtxToString(CoroutineContext* ctx)
{
    char buf[1024];
    int n = 0;

    n += snprintf(buf + n, sizeof(buf), "ctx=%p, name=%s, stack=%p-%p\n", ctx, ctx->name_.c_str(), ctx->stack_ + ctx->stack_size_, ctx->stack_);

    if (ctx != NULL)
    {   
        uint64_t* p = (uint64_t*)ctx->stack_;
        for (int i = 0; i != 8; ++i)
        {   
            n += snprintf(buf + n, sizeof(buf), "  [%02d]  %p  %p\n", i, p, (void*)*p++);
        }   
    }   

    buf[n] = '\0';

    return std::string(buf);
}

static int AdjustStackSize(const int& stack_size)
{
    int adjust_stack_size = stack_size >> 4;
    adjust_stack_size |= 0x1000;
    adjust_stack_size <<= 4;

    std::cout << LOG_PREFIX << "stack_size=" << stack_size << ", adjust_stack_size=" << adjust_stack_size << std::endl;

    return adjust_stack_size;
}

extern "C"
{
    extern void AsmSwapRegister(CoroutineContext*, CoroutineContext*) asm("AsmSwapRegister");
    extern void AsmLoadRegister(CoroutineContext*) asm("AsmLoadRegister");
}

static void CoroutineEntry(void* args)
{
    CoroutineContext* ctx = (CoroutineContext*)args;

    if (ctx->routine_func_)
    {
        ctx->routine_func_(ctx->args_);
    }

    delete ctx;
    AsmLoadRegister(get_main_ctx());
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
            if (ctx != NULL)
            {
                Resume(ctx);
            }
        }
        else
        {
            usleep(10*1000);
        }
    }
}

CoroutineContext* get_main_ctx()
{
    if (g_schedule_ctx == NULL)
    {
        g_schedule_ctx = CreateCoroutine("main", NULL, NULL);
    }

    return g_schedule_ctx;
}

CoroutineContext* get_cur_ctx()
{
    return g_cur_ctx;
}

CoroutineContext* CreateCoroutine(const std::string& name, RoutineFunc routine_func, void* args, const int& stack_size)
{
    CoroutineContext* ctx = new CoroutineContext();

    ctx->routine_func_ = routine_func;
    ctx->args_ = args;
    ctx->stack_size_ = AdjustStackSize(stack_size);
    ctx->stack_ = (uint8_t*)malloc(ctx->stack_size_);
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

    return ctx;
}

void Swap(CoroutineContext* pre, CoroutineContext* cur)
{
    g_cur_ctx = cur;
#if defined(DEBUG)
    std::cout << LOG_PREFIX << CtxToString(pre) << std::endl;
    std::cout << LOG_PREFIX << CtxToString(cur) << std::endl;
#endif
    AsmSwapRegister(pre, cur);
}

void Yield(CoroutineContext* ctx)
{
    g_ctx_list.push_back(ctx);
    Swap(ctx, get_main_ctx());
}

void Resume(CoroutineContext* ctx)
{
    Swap(get_main_ctx(), ctx);
}
