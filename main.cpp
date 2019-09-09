#include <iomanip>
#include <iostream>
#include <sstream>

#include "coroutine.h"

using namespace std;

struct UserParam
{
    UserParam()
        : str("")
        , i(0)
        , d(0.0)
    {
    }

    std::string print()
    {
        std::ostringstream os;

        os << "str:" << str
           << ", i:" << i
           << ", d:" <<d
           << endl;

        return os.str();
    }

    std::string str;
    int i;
    double d;
};

void routine(void* args)
{
    cout << __func__ << ", args:" << args << endl;

    UserParam* param = (UserParam*)args;
    cout << param->print() << endl;

    CoroutineContext* ctx = get_cur_ctx();
    Yield(ctx);
}

int main(int argc, char* argv[], char* env[])
{
    UserParam* param = new UserParam();
    CoroutineContext* ctx = CoroutineCreate(routine, param);

    cout << __func__ << ":" << "func addr:" << "\n"
         << "    main:" << (void*)main
         << "    routine:" << (void*)routine
         << endl;

    //ctx->StoreRegister();
    cout << __func__ << ":" << __LINE__ << endl;
    Resume(ctx);

    return 0;
}
