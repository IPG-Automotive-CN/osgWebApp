#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <emscripten/websocket.h>
#include "WSClient.h"


#define EMSTRCBK     em_str_callback_func
#define EMONLOADCBK  em_async_wget_onload_func



EMSCRIPTEN_WEBSOCKET_T ws = 0;

// TrajPoint latestPoint;
TestrunInfo testrunInfo;


simStatus status;

int needReset = 0;
int isNeedReset()
{
    return needReset;
}

void setNeedReset(int val)
{
    needReset = val;
}

int testRunMsgReady = 0;
int isTestRunMsgReady()
{
    return testRunMsgReady;
}

void setTestRunMsgReady(int val)
{
    testRunMsgReady = val;
}


int tsMsgReady = 0;
static void *buffer = NULL;
static size_t bufferLength = 0;



void *getOpenDriveBuffer()
{
    return buffer;
}

size_t getBufSize()
{
    return bufferLength;
}

TestrunInfo getTestrunInfo()
{
    return testrunInfo;
}

simStatus getSimStatus()
{
    return status;
}

static int frameCount = 0;
int parseStatusMsg(uint8_t *data, uint32_t numBytes)
{
    // Calculate the size of the fixed part of simStatus
    size_t fixed_part_len = sizeof(double) + sizeof(position) + sizeof(uint64_t);

    // Check if the message is long enough
    if (numBytes < fixed_part_len) {
        printf("The message is too short to be a valid simStatus, numBytes : %d\n", numBytes);
        return -1;
    }

    // Copy the fixed part of the message to a simStatus variable
    memcpy(&status, data, fixed_part_len);

    // Check if the message is long enough to contain the others array
    if (numBytes < fixed_part_len + sizeof(position) * status.others_num) {
        printf("The message is too short to contain the others array");
        return -2;
    }

    if(status.others != NULL)
    {
        free(status.others);
    }

    status.others = (position*)malloc(sizeof(position) * status.others_num);
    memcpy(status.others, (unsigned char*)data + fixed_part_len, sizeof(position) * status.others_num);

    // Now you can access the fields of status
    // double time = status.time;
    // position ego = status.ego;
    // uint32_t others_num = status.others_num;
    // position* others = status.others;

    // if(frameCount++ % 60 == 0)
    // {
    //     printf("received data numbytes = %d\n", numBytes);
    //     for(int i = 0;i<3;i++)
    //     {
    //         printf("x = %lf y = %lf  heading = %lf\n", others[i].x, others[i].y, others[i].heading);
    //     }
    // }
    return 0;
}

//TODO: clear websocket
void clearWebsocket()
{
    if (ws > 0) {
        emscripten_websocket_close(ws, 0, NULL);
        ws = 0;
    }
}

EM_BOOL onOpen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
    printf("WebSocket connection opened!\n");
    return 0;
}

EM_BOOL onClose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
    printf("WebSocket connection closed!\n");
    return 0;
}

EM_BOOL onError(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
    printf("WebSocket error!\n");
    return 0;
}



#define INITIAL_BUFFER_SIZE 4096

EM_BOOL onMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData) {
    static size_t bufferSize = 0;

    //printf("onMessage numBytes = %d\n", e->numBytes);
    static int prefix = 0;
    if (e->isText) {
        // printf(e->data);
        printf("onMessage text = %s\n", e->data);
        const char *text = (const char*)e->data;

        if (strcmp(text, "start of file") == 0) 
        {
            prefix = 1;
            // Allocate the initial buffer
            bufferSize = INITIAL_BUFFER_SIZE;
            buffer = malloc(bufferSize);
            bufferLength = 0;
            
        } else if (strcmp(text, "end of file") == 0) 
        {
            prefix = 0;
            setNeedReset(1);
            printf("Received file with %d bytes\n", bufferLength);

        }
        else if (strcmp(text, "start of init") == 0)
        {
            prefix = 2;
            printf("start of init\n");
        }else if (strcmp(text, "end of init") == 0)
        {
            prefix = 0;
            printf("end of init\n");
            setTestRunMsgReady(1);
        }
    } else {
        if(prefix == 2)
        {
            memcpy(&testrunInfo.traffic_nObjs, e->data, sizeof(uint64_t));
            int dynLen = sizeof(TrafficInfo) * testrunInfo.traffic_nObjs;
            testrunInfo.trafficObjs = (TrafficInfo*)malloc(dynLen);
            memcpy(testrunInfo.trafficObjs, (unsigned char*)e->data + sizeof(uint64_t), dynLen);
        }else if(prefix == 1)
        {
            printf("Received %d bytes (file buffer)\n", e->numBytes);
            // Ensure the buffer is large enough
            if (bufferLength + e->numBytes > bufferSize) {
                bufferSize *= 2;
                buffer = (unsigned char*)realloc(buffer, bufferSize);
            }

            // Add the received data to the buffer
            memcpy(buffer + bufferLength, e->data, e->numBytes);
            bufferLength += e->numBytes;
        }else
        {
            parseStatusMsg(e->data, e->numBytes);
        }
    }
    return 0;
}





int createWebsocket(const char *url)
{
    if (!emscripten_websocket_is_supported()) {
        printf("WebSockets are not supported in this browser\n");
        return 1;
    }

    EmscriptenWebSocketCreateAttributes attr;
    emscripten_websocket_init_create_attributes(&attr);

    attr.url = url;
    attr.protocols = NULL;
    attr.createOnMainThread = 1;

    ws = emscripten_websocket_new(&attr);
    if (ws <= 0) {
        printf("WebSocket creation failed!\n");
        return 1;
    }

    emscripten_websocket_set_onopen_callback(ws, NULL, onOpen);
    emscripten_websocket_set_onclose_callback(ws, NULL, onClose);
    emscripten_websocket_set_onerror_callback(ws, NULL, onError);
    emscripten_websocket_set_onmessage_callback(ws, NULL, onMessage);

    return 0;
}











    