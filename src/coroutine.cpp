#include <sys/syscall.h>

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

    LogDebug << LOG_PREFIX << "stack_size=" << stack_size << ", adjust_stack_size=" << adjust_stack_size << std::endl;

    return adjust_stack_size;
}

extern "C"
{
    extern void AsmSwapRegister(CoroutineContext*, CoroutineContext*) asm("AsmSwapRegister");
    extern void AsmLoadRegister(CoroutineContext*) asm("AsmLoadRegister");
}

__thread bool g_init = false;
__thread CoroutineContext* g_ctx_stack[64] = { 0 };
__thread int g_ctx_stack_top = 0;
__thread CID g_cid = 0;
__thread uint64_t g_tid = 0;

uint64_t get_tid()
{
    if (g_tid == 0)
    {
        g_tid = (uint64_t)syscall(SYS_gettid);
    }

    return g_tid;
}

static void CoroutineEntry(void* args)
{
    CoroutineContext* ctx = (CoroutineContext*)args;
#if defined(DEBUG)
    LogDebug << LOG_PREFIX << " ---- coroutine start ----\n" << CtxToString(ctx) << std::endl;
#endif
    if (ctx->routine_func_)
    {
        ctx->routine_func_(ctx->args_);
    }

    CoroutineContext* caller_ctx = g_ctx_stack[g_ctx_stack_top - 2];
    --g_ctx_stack_top;
#if defined(DEBUG)
    LogDebug << LOG_PREFIX << " ---- coroutine stop ----\n" << CtxToString(ctx) << std::endl;
    LogDebug << LOG_PREFIX << " ---- caller coroutine ----\n" << CtxToString(caller_ctx) << std::endl;
#endif

    free(ctx->stack_);
    delete ctx;

    AsmLoadRegister(caller_ctx);
}

#define CUR_CTX g_ctx_stack[g_ctx_stack_top - 1]

CoroutineContext* get_cur_ctx()
{
    return CUR_CTX;
}

CID get_cid()
{
    return CUR_CTX->cid_;
}

CoroutineContext* CreateCoroutine(const std::string& name, RoutineFunc routine_func, void* args, const int& stack_size)
{
    if (! g_init)
    {
        g_init = true;
        LogDebug << LOG_PREFIX << "init main ctx" << std::endl;
        g_ctx_stack[0] = CreateCoroutine("main", NULL, NULL, 8192);
        ++g_ctx_stack_top;
    }

    CoroutineContext* ctx = new CoroutineContext();

    ctx->routine_func_ = routine_func;
    ctx->args_ = args;
    ctx->stack_size_ = AdjustStackSize(stack_size);
    ctx->stack_ = (uint8_t*)malloc(ctx->stack_size_);
    ctx->name_ = name;
    ctx->cid_ = get_tid() << 32 | g_cid++;

    LogDebug << "tid=" <<get_tid() << std::endl;

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
#if defined(DEBUG)
    LogDebug << LOG_PREFIX << CtxToString(pre) << std::endl;
    LogDebug << LOG_PREFIX << CtxToString(cur) << std::endl;
#endif
    AsmSwapRegister(pre, cur);
}

void Yield()
{
    CoroutineContext* cur_ctx = CUR_CTX;
    CoroutineContext* caller_ctx = g_ctx_stack[g_ctx_stack_top - 2];
#if defined(DEBUG)
    LogDebug << LOG_PREFIX << "caller:" << caller_ctx << ", g_ctx_stack_top:" << g_ctx_stack_top << std::endl;
#endif
    --g_ctx_stack_top;
    Swap(cur_ctx, caller_ctx);
}

void Resume(CoroutineContext* ctx)
{
    CoroutineContext* caller_ctx = g_ctx_stack[g_ctx_stack_top - 1];
#if defined(DEBUG)
    LogDebug << LOG_PREFIX << "caller:" << caller_ctx << ", g_ctx_stack_top:" << g_ctx_stack_top << std::endl;
#endif
    g_ctx_stack[g_ctx_stack_top] = ctx;
    ++g_ctx_stack_top;

    Swap(caller_ctx, ctx);
}
