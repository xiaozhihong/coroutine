#include <stdlib.h>

#include "coroutine.h"

const int kDefaultStackSize = 16*1024;
CoroutineContext* g_schedule_ctx = NULL;

extern "C"
{
    extern void AsmStoreRegister(CoroutineContext*) asm("AsmStoreRegister");
    extern void AsmLoadRegister(CoroutineContext*) asm("AsmLoadRegister");
}

CoroutineContext::CoroutineContext(RoutineFunc routine_func, const int& stack_size)
    : routine_func_(routine_func)
    , stack_(new uint8_t(stack_size))
{
}

void CoroutineContext::StoreRegister()
{
    AsmStoreRegister(this);
}

void CoroutineContext::LoadRegister()
{
    AsmLoadRegister(this);
}

CoroutineContext* CoroutineCreate(RoutineFunc routine_func)
{
    CoroutineContext* ctx = new CoroutineContext(routine_func, kDefaultStackSize);

    return ctx;
}

int Swap(CoroutineContext* pre, CoroutineContext* cur)
{
    pre->StoreRegister();
    cur->LoadRegister();

    return 0;
}

int Yield(CoroutineContext* ctx)
{
    return Swap(g_schedule_ctx, ctx);
}

int Resume(CoroutineContext* ctx)
{
    return Swap(ctx, g_schedule_ctx);
}
