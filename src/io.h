#ifndef __IO_H__
#define __IO_H__

#include "coroutine.h"

class Epoller;

Epoller* get_epoller();

void EventLoop();
int Read(const int& fd, uint8_t* data, const size_t& size);
int Write(const int& fd, const uint8_t* data, const size_t& size);
int ReadGivenSize(const int& fd, uint8_t* data, const size_t& size);
int WriteGivenSize(const int& fd, const uint8_t* data, const size_t& size);

#endif // __IO_H__
