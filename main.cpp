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
    //cout << param->print() << endl;
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
        cout << "<--- resum" << endl;
    }

    cout << __func__ << endl;
}

int main(int argc, char* argv[], char* env[])
{
    UserParam* param = new UserParam();
    CoroutineContext* ctx = CoroutineCreate("routine", routine, param);

    cout << __func__ << ":" << "func addr:" << "\n"
         << "    main:" << (void*)main
         << "    routine:" << (void*)routine
         << endl;

    int i = 0;
    cout << __func__ << ":" << __LINE__ << endl;
    cout << __func__ << "&i:" << (void*)&i << endl;
    Resume(ctx);

    return 0;
}
