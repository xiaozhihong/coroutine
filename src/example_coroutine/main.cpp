#include <iomanip>
#include <iostream>
#include <sstream>

#include "coroutine.h"
#include "log.h"

using namespace std;

struct UserParam
{
    UserParam(const std::string& str = "")
        : str_(str)
        , i_(0)
        , u64_(12388)
        , d_(3.141592353)
    {
    }

    std::string dump()
    {
        char buf[1024];
        int n = 0;
        n += snprintf(buf + n, sizeof(buf), "str=%s, i=%d, d=%.2f", str_.c_str(), i_, d_);

        buf[n] = '\0';

        return string(buf);
    }

    std::string str_;
    int i_;
    uint64_t u64_;
    double d_;
};

void TestYield(void* args)
{
    UserParam* param = (UserParam*)args;
    LogDebug << "param=" << param->dump() << endl;

    CoroutineContext* ctx = get_cur_ctx();

    int i = 0;

    for (int i = 0; i != 10; ++i)
    {
        LogDebug << dec << "(" << i << ") ---> yield" << endl;
        Yield(ctx);
        LogDebug << dec << "(" << i << ") <--- resume" << endl;
        LogDebug << "param=" << param->dump() << endl;
    }
}

int main(int argc, char* argv[], char* env[])
{
    for (int i = 0; i != 4; ++i)
    {
        ostringstream os;
        os << "routine_" << i;
        UserParam* param = new UserParam(os.str());
        CoroutineContext* ctx = CreateCoroutine("TestYield", TestYield, param, 1335);
        Resume(ctx);
    }

    LogDebug << __func__ << " # run schedule" << endl;

    schedule_thread(NULL);    

    LogDebug << __func__ << " # main exit" << endl;

    return 0;
}
