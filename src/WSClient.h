
#ifdef __cplusplus
extern "C"
{
#endif

#include "DataDefine.h"



int createWebsocket(const char *url);
void clearWebsocket();

int isNeedReset();
void setNeedReset(int val);
int isTestRunMsgReady();
void setTestRunMsgReady(int val);


void *getOpenDriveBuffer();
size_t getBufSize();


TestrunInfo getTestrunInfo();
simStatus getSimStatus();

#ifdef __cplusplus
} /* extern "C" */
#endif

