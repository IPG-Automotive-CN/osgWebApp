
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <iostream>
#include "main.h"

#include <emscripten/html5.h>

#include "CommonMini.hpp"

#include "RoadManager.hpp"
#include "WSClient.h"


void beforeunload_callback(int eventType, const void* reserved, void* userData) {
    printf("***************beforeunload_callback\n");
    clearWebsocket();
}


// We use app global variable in loop() function.
Application *app = 0;
SDL_Window* window = NULL;
char *url = NULL;

int isHide = 0;

int loadOpenDrive()
{
    void *buf = getOpenDriveBuffer();
    size_t bufSize = getBufSize();

    pugi::xml_document doc;
    // First assume absolute path
    pugi::xml_parse_result result = doc.load_buffer(buf, bufSize);
    if (!result)
    {
        printf("load OpenDrive Buffer failed! Result: %s\n", result.description());
        //printf("%s at offset (character position): %d", result.description(), result.offset);
        return -1;
    }

    if(!roadmanager::Position::LoadOpenDrive(buf, bufSize))
    {
        printf("Failed to load ODR from buffer\n");
        return -2;
    }else{
        printf("Loaded ODR from buffer\n");
        return 0;
    }
}



void stopMainLoopX() {
    printf("Stopping main loop\n");
    

    if(app)
    {
        app->reset();
    }

    // 取消主循环
    emscripten_cancel_main_loop();

    clearWebsocket();

    // 销毁 SDL 窗口
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    // 退出 SDL
    SDL_Quit();

    printf("Exited SDL\n");


}

// Stand alone function that is called by Emscripten to run the app.
void loop()
{   
    if(!app)
        return;

    if(url != NULL)
    {
        if(isNeedReset())
        {
            app->reset();
            loadOpenDrive();
            setNeedReset(0);
        }
            
        if(isTestRunMsgReady())
        {
            app->init();
            setTestRunMsgReady(0);
        }
    }


    if(!app->isInited())
    {
        return;
    }
    
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        app->handleEvent(e);
    }
    app->frame();
}




extern "C" {

    #include <stdlib.h>
    #include <time.h>
    const char *xoscPaths[] = {
        "/res/xosc/cut-in.xosc",
        "/res/xosc/lane_change.xosc",
        "/res/xosc/straight_500m.xosc"
    };


    EMSCRIPTEN_KEEPALIVE void stopMainLoop() {
        stopMainLoopX();
        //emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_V, stopMainLoopX);
    }

}


void onAtExit()
{
    printf("onAtExit\n");
}

int main(int argc, char *argv[])
{   
    char *oscPath = NULL; 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--url") == 0 && i + 1 < argc) {
            url = argv[i + 1];
        }

        if (strcmp(argv[i], "--osc") == 0) {
            oscPath = argv[i + 1];
        }
    }
//git config --global user.email "hao.li@ipg-automotive.com"
//git config --global user.name "IPG-Automotive-CN"

    if(url != NULL)
    {
        printf("websocket url = %s\n", url);
        createWebsocket(url);
    }
    oscPath = "/res/xosc/cut-in.xosc";
    if(oscPath != NULL)
    {
        printf("osc file path = %s\n", oscPath);
    }

    
    
    //SE_Env::Inst().AddPath("res/xosc");

    printf("OSGWeb. Starting...XXXYYY\n");
    
    int width = 1280;
    int height = 720;
    // Make sure SDL is working.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("OSGWeb. Could not init SDL: '%s'\n", SDL_GetError());
        return -1;
    }

    // Clean SDL up at exit.
    // atexit(SDL_Quit);
    atexit(onAtExit);
    // Configure rendering context.
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // Create rendering window.
    window =
        SDL_CreateWindow(
            "OSGWeb",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_OPENGL);
    if (!window)
    {
        printf("OSGWeb. Could not create window: '%s'\n", SDL_GetError());
        return -2;
    }
    SDL_GL_CreateContext(window);
    // Create application.
    app = new Application(width, height);

    if(oscPath != NULL && strcmp(oscPath, "") != 0)
    {
        printf("app initPlayer %s\n",oscPath);
        app->initPlayer(oscPath);
    }
    emscripten_set_main_loop(loop, -1, 0);

    printf("main execute finished!\n");
    //emscripten_set_beforeunload_callback(NULL, beforeunload_callback);

    return 0;
}
