#ifndef __IO_H__
#define __IO_H__

#include <vector>

#include "coroutine.h"

class Epoller;

Epoller* get_epoller();

void EpollWait(std::vector<CoroutineContext*>& actives);

#endif // __IO_H__
