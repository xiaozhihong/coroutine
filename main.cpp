#include <iostream>

#include "coroutine.h"

using namespace std;

void routine(void* args)
{
    cout << __func__ << endl;
}

int main(int argc, char* argv[], char* env[])
{
    CoroutineContext* ctx = CoroutineCreate(routine);

    Resume(ctx);
    ctx->StoreRegister();
    ctx->LoadRegister();

    return 0;
}
