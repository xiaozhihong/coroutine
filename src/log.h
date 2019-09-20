#ifndef __LOG_H__
#define __LOG_H__

#define LOG_PREFIX __FILE__<<"#"<<__func__<<":"<<__LINE__<<" # "
#define LogErr std::cout<<LOG_PREFIX
#define LogInfo std::cout<<LOG_PREFIX
#define LogDebug std::cout<<LOG_PREFIX

#endif // __LOG_H__
