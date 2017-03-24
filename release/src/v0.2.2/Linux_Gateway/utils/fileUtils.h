#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#define LEVEL_ERROR 0
#define LEVEL_INFO 1
#define LEVEL_DEBUG 2

extern bool fileExisted(const char* pFileName);

extern char* readFileAsString(const char* pFileName);

extern void writeStringToFile(const char* pFileName, const char* pData);

extern void logger(unsigned int level, const char* format, ...);

#undef __FILE_UTILS_H__
#endif
