#include "fileUtils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#include <syslog.h>
#endif

#define LENGTH_OF(x) (sizeof(x)/(sizeof(x[0])))

int getFileSize(FILE* fp){
    int oldPos = ftell(fp);
    fseek(fp,0,SEEK_END);
    int sizeLen = ftell(fp);
    fseek(fp, oldPos, SEEK_SET);
    return sizeLen;
}

bool fileExisted(const char* pFileName) {
	return access(pFileName,F_OK)==0;
}

char* readFileAsString(const char* pFileName) {
	FILE* fd = fopen(pFileName, "r");
	if (fd == NULL){
		logger(LEVEL_ERROR, "Cannot open file %s\n", pFileName);
		perror("Cannot open file");
		exit(-1);
	}
	int length = getFileSize(fd);
	char* result;
	if (length < 1){
		result = new char[1];
		result[0] = 0;
		return result;
	}
	result = new char[length+1];
	int n = fread(result, 1, length, fd);
//	printf("read length is %d\n", n);
	result[n+1] = 0;
	return result;
}

void writeStringToFile(const char* pFileName, const char* pData) {
	FILE* fd = fopen(pFileName, "wt");
	if (fd == NULL) {
		perror("Cannot open device data storage file to write");
		return;
	}

	fseek(fd, 0, SEEK_SET);
	fprintf(fd, "%s", pData);
	fclose(fd);
}

void logger(unsigned int level, const char* format, ...) {
	const char* levels[] = {"ERROR","INFO","DEBUG"};
	const char* levelName = levels[1];
	if (level < LENGTH_OF(levels)){
		levelName=levels[level];
	}

	time_t now;
	struct tm *timenow;
	char strtemp[255];

	time(&now);
	timenow = localtime(&now);

	char* pformat = new char[strlen(format)+260];
	sprintf(pformat,"[%02d:%02d:%02d %s]%s\n",timenow->tm_hour, timenow->tm_min, timenow->tm_sec,levelName,format);
	va_list params;
	va_start(params, format);
	vprintf(pformat, params);
#ifndef _WIN32
	int prio = LOG_DEBUG;
	switch (level){
    case LEVEL_ERROR:
        prio = LOG_ERR;
        break;
    case LEVEL_INFO:
        prio = LOG_INFO;
        break;
    default:
        break;
	}
	va_start(params, format);
	openlog("DeltaDoreDriver", LOG_PID|LOG_CONS, LOG_USER);
	vsyslog(prio, format, params);
	closelog();
#endif
	va_end(params);

}


