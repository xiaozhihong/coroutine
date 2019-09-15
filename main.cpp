#include <iomanip>
#include <iostream>
#include <sstream>

#include "coroutine.h"

using namespace std;

struct UserParam
{
    UserParam(const std::string& s = "")
        : str(s)
        , i(0)
        , u64(12388)
        , d(3.141592353)
    {
    }

    std::string print()
    {
        std::ostringstream os;
        os << "str:" << str
           << ", i:" << i
           << ", d:" << d
           << ", dd:" << (d+1.045)
           << ", ddd:" << (d*d+1.045)
           << endl;

        return os.str();
    }

    std::string str;
    int i;
    uint64_t u64;
    // FIXME: double会崩溃
    double d;
};

void UserCoroutineTestYeildAndResume(void* args)
{
    cout << (double)1 << endl;
    cout << __func__ << ", args:" << args << endl;

    UserParam* param = (UserParam*)args;
    cout << "param print:" << param->print() << endl;

    char ch[8];
    ch[0] = 'x';
    ch[1] = 'i';
    ch[2] = 'a';
    ch[3] = 'o';
    ch[4] = 'z';
    ch[5] = 'h';
    ch[6] = 'i';
    ch[7] = '\0';

    cout << (void*)ch << ":" << ch << endl;

    CoroutineContext* ctx = get_cur_ctx();

    int i = 0;

    for (int i = 0; i != 5; ++i)
    {
        cout << dec << "(" << i << ") ---> yield by user" << endl;
        Yield(ctx);
        cout << "param print:" << param->print() << endl;
        cout << "<--- resum" << endl;
    }

    cout << __func__ << endl;
}

int main(int argc, char* argv[], char* env[])
{
    for (int i = 0; i != 2; ++i)
    {
        ostringstream os;
        os << "routine_" << i;
        UserParam* param = new UserParam(os.str());
        CoroutineContext* ctx = CoroutineCreate("UserCoroutineTestYeildAndResume", UserCoroutineTestYeildAndResume, param);
        Resume(ctx);
    }

    cout << __func__ << " # run schedule" << endl;

    schedule_thread(NULL);    

    cout << __func__ << " # main exit" << endl;

    return 0;
}
