
/*
This file is part of OpenSceneGraph cross-platform guide:
  https://github.com/OGStudio/openscenegraph-cross-platform-guide

Copyright (C) 2017 Opensource Game Studio

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef OPENSCENEGRAPH_CROSS_PLATFORM_GUIDE_MAIN_H
#define OPENSCENEGRAPH_CROSS_PLATFORM_GUIDE_MAIN_H





// #include "functions.h"

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

// BEGIN FEATURE VBO

#include <osg/Geometry>

// END   FEATURE VBO

// BEGIN FEATURE RENDERING_DEFAULT
#include <osgGA/TrackballManipulator>
// END   FEATURE RENDERING_DEFAULT

#include "../src/viewer.hpp"
#include "../src/CommonMini.hpp"
#include "../src/RoadManager.hpp"
#include "../src/VBOSetupVisitor.hpp"
#include "../src/playerbase.hpp"
#include <chrono>
#include "../src/WSClient.h"



static ScenarioPlayer *player = 0;

// BEGIN FEATURE PLUGINS_STATIC
// Initialize OSG plugins when OpenSceneGraph is built
// as a static library.
USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)
USE_SERIALIZER_WRAPPER_LIBRARY(osgSim)
// END   FEATURE PLUGINS_STATIC

// This class prints OpenSceneGraph notifications to console.
class OSGLogger : public osg::NotifyHandler
{
    public:
        OSGLogger() { }
        virtual ~OSGLogger() { }

        // Override NotifyHandler::notify() to receive OpenSceneGraph notifications.
        void notify(osg::NotifySeverity severity, const char *message)
        {
            // std::string finalMessage =
            //     printfString(
            //         "OSG/%s %s",
            //         logLevelToString(severity).c_str(),
            //         message);
            // platformLog(finalMessage.c_str());
        }
};

// END   FEATURE VBO

class Application
{
public:
    Application(int width, int height)
        : width_(width)
        , height_(height)
    {
        setupLogging();
        setupRendering();
    }
    ~Application()
    {
        tearDownLogging();
        tearDownRendering();
    }

    int initPlayer(std::string oscFileName);

    int init();

    void reset();

    void loadScene(const std::string &fileName);
    bool handleEvent(SDL_Event &e);
    void setupWindow(int width, int height);
    void frame();

    void playerFrame();

    bool isInited() { return viewer_ != nullptr; }


private:
    void setupLogging()
    {
        // Create custom logger.
        mLogger = new OSGLogger;
        // Provide the logger to OpenSceneGraph.
        osg::setNotifyHandler(mLogger);
        // Only accept notifications of Info level or higher
        // like warnings and errors.
        osg::setNotifyLevel(osg::INFO);
    }
    void tearDownLogging()
    {
        // Remove the logger from OpenSceneGraph.
        // This also destroys the logger: no need to deallocate it manually.
        osg::setNotifyHandler(0);
    }
    void setupRendering();
    void tearDownRendering();
private:
    OSGLogger *mLogger;
    viewer::Viewer     *viewer_;

    int        width_;
    int        height_;
};

#endif // OPENSCENEGRAPH_CROSS_PLATFORM_GUIDE_MAIN_H

static char                   **argv_ = 0;
static int                      argc_ = 0;
static std::vector<std::string> args_v;



static void ConvertArguments()
{
    argc_ = static_cast<int>(args_v.size());
    argv_ = reinterpret_cast<char **>(malloc(args_v.size() * sizeof(char *)));
    std::string argument_list;
    for (unsigned int i = 0; i < static_cast<unsigned int>(argc_); i++)
    {
        argv_[i] = reinterpret_cast<char *>(malloc((args_v[i].size() + 1) * sizeof(char)));
        StrCopy(argv_[i], args_v[i].c_str(), static_cast<unsigned int>(args_v[i].size()) + 1);
        argument_list += std::string(" ") + argv_[i];
    }
    printf("Player arguments: %s\n", argument_list.c_str());
}

static void AddArgument(const char *str, bool split = true)
{
    std::vector<std::string> args;
    if (split)
    {
        // split separate argument strings
        args = SplitString(std::string(str), ' ');
    }
    else
    {
        args.push_back(std::string(str));
    }

    for (size_t i = 0; i < args.size(); i++)
    {
        args_v.push_back(args[i]);
    }
}

void Application::setupRendering()
{

}


void Application::reset()
{
    if(player != nullptr)
    {
        delete player;
        player = nullptr;
    }
    else if(viewer_ != nullptr)
    {
        delete viewer_;
        viewer_ = nullptr;
    }
}

int Application::init()
{
    SE_Options &opt = SE_Env::Inst().GetOptions();
    opt.Reset();
    //std::string odrFilename = "res/xodr/e6mini.xodr";
    // if (!roadmanager::Position::LoadOpenDrive(odrFilename.c_str()))
    // {
    //     printf("Failed to load ODR %s\n", odrFilename.c_str());
    //     return -1;
    // }
    roadmanager::OpenDrive *odrManager = roadmanager::Position::GetOpenDrive();
    int argc = 0;
    char **argv = NULL;
    osg::ArgumentParser arguments(&argc, argv);
    viewer_ = new viewer::Viewer(odrManager, "", NULL, "", arguments, &opt);
    viewer_->osgViewer_->setUpViewerAsEmbeddedInWindow(0, 0, width_, height_);
    viewer_->AddEntityModel(viewer_->CreateEntityModel(
        "/res/models/car_white.osgb",
        osg::Vec4(0.5, 0.5, 0.5, 1.0),
        viewer::EntityModel::EntityType::VEHICLE,
        false,
        "ego",
        0,
        EntityScaleMode::BB_TO_MODEL));
    setupWindow(width_, height_);



    TestrunInfo testrunInfo = getTestrunInfo();
    for(int i = 0;i<testrunInfo.traffic_nObjs;i++)
    {
        TrafficInfo obj = testrunInfo.trafficObjs[i];
        std::string modelFilePath = "";
        if(obj.rcsClass == RCS_Car || obj.rcsClass == RCS_Unknown)
        {
            modelFilePath = "/res/models/car_blue.osgb";
        }else if(obj.rcsClass == RCS_Truck)
        {
            modelFilePath = "/res/models/truck_trailer.osgb";
        }

        OSCBoundingBox bb;
        bb.center_.x_ = 0;
        bb.center_.y_ = 0;
        bb.center_.z_ = -obj.offset/2;
        bb.dimensions_.length_ = obj.dimension[0];
        bb.dimensions_.width_ = obj.dimension[1];
        bb.dimensions_.height_ = obj.dimension[2];
        viewer_->AddEntityModel(viewer_->CreateEntityModel(
            modelFilePath,
            osg::Vec4(0.5, 0.5, 0.5, 1.0),
            viewer::EntityModel::EntityType::VEHICLE,
            false,
            "",
            &bb,
            EntityScaleMode::MODEL_TO_BB));
    }


    return 0;

}

int Application::initPlayer(std::string oscFileName)
{
    
    printf("oscFileName = %s\n", oscFileName.c_str());
    args_v.clear();   
    AddArgument("./this.program");
    if(oscFileName.empty())
    {
        printf("No osc file specified!\n");
        return -1;
    }

    AddArgument("--osc");
    AddArgument(oscFileName.c_str(), false);
    static char winArg[64];
    snprintf(winArg, sizeof(winArg), "--window %d %d %d %d", 0, 0, width_, height_);
    AddArgument(winArg, true);
    ConvertArguments();
    try
    {
        // Initialize the scenario engine and viewer
        player     = new ScenarioPlayer(argc_, argv_);
        printf("Player created\n");
        
        int retval = player->Init();
        if (retval == -1)
        {
            printf("Failed to initialize scenario player\n");
        }
        else if (retval == -2)
        {
            printf("Skipped initialize scenario player\n");
        }

        if (retval != 0)
        {
            printf("Failed to initialize scenario player with retval = %d\n", retval);
        }
        printf("player init retval = %d\n", retval);
        viewer_ = player->viewer_;
        setupWindow(width_, height_);
    }
    catch (const std::exception &e)
    {
        printf("An exception throwed: %s\n",e.what());
    }

    return 0;
    
}

void Application::tearDownRendering()   
{
    if(viewer_ != nullptr)
    {
        delete viewer_;
    }
    viewer_ = nullptr;  
}

void Application::loadScene(const std::string &fileName)
{

}

// BEGIN FEATURE INPUT_EMSCRIPTEN
bool Application::handleEvent(SDL_Event &e)
{
    if(viewer_ == nullptr)
    {
        return false;
    }

    osgViewer::GraphicsWindow *gw =
        dynamic_cast<osgViewer::GraphicsWindow *>(
            viewer_->osgViewer_->getCamera()->getGraphicsContext());
    if (!gw)
    {
        printf("OSGWeb. Application. Graphics window is not initialized\n");
        return false;
    }
    osgGA::EventQueue &queue = *(gw->getEventQueue());
    switch (e.type)
    {
        case SDL_MOUSEMOTION:
            queue.mouseMotion(e.motion.x, e.motion.y);
            return true;
        case SDL_MOUSEBUTTONDOWN:
            queue.mouseButtonPress(e.button.x, e.button.y, e.button.button);
            return true;
        case SDL_MOUSEBUTTONUP:
            queue.mouseButtonRelease(e.button.x, e.button.y, e.button.button);
            return true;
        case SDL_MOUSEWHEEL:
            queue.mouseScroll(e.wheel.y > 0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN);
            return true;

        default:
            break;
    }
    return false;
}
// END   FEATURE INPUT_EMSCRIPTEN


// BEGIN FEATURE RENDERING_EMBEDDED
void Application::setupWindow(int width, int height)
{
    if(viewer_ == nullptr)
    {
        return;
    }
    viewer_->osgViewer_->setUpViewerAsEmbeddedInWindow(0, 0, width, height);
}
// END   FEATURE RENDERING_EMBEDDED



static int frameCount = 0;

static std::chrono::steady_clock::time_point lastTime;


void Application::playerFrame()
{
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (frameCount == 0) {
        printf("This is the first frame\n");
        player->Frame(0.0);
        
    }else if(frameCount == 1){
        printf("This is the second frame\n");
        player->Frame(0.001);
    }else{
        std::chrono::duration<double> time_span = 
            std::chrono::duration_cast<std::chrono::duration<double>>(now - lastTime);
        double dt = time_span.count();
        //printf("player Frame before now seconds = %f\n", seconds.count());
        player->Frame(dt);
        // std::chrono::steady_clock::time_point newNow = std::chrono::steady_clock::now();
        // seconds = std::chrono::duration_cast<std::chrono::duration<double>>(newNow.time_since_epoch());
        //printf("player Frame after now seconds = %f\n", seconds.count());
        
    }
    lastTime = now;
    frameCount++;
}


// BEGIN FEATURE RENDERING_DEFAULT
void Application::frame()
{

    if(player != nullptr)
    {
        playerFrame();
        return;
    }

    if(viewer_ == nullptr)
    {
        return;
    }
    simStatus status  = getSimStatus();
    viewer_->entities_[0]->SetPosition(status.ego.x, status.ego.y, status.ego.z);
    // if(frameCount++%60==0)
    // {
    //     printf("ego z = %lf\n", status.ego.z);
    // }

    viewer_->entities_[0]->SetRotation(status.ego.heading, 0, 0);

    while(viewer_->entities_.size() > status.others_num + 1)
    {
        viewer_->RemoveCar(viewer_->entities_.size() - 1);
    }
    while(viewer_->entities_.size() < status.others_num + 1)
    {
        viewer_->AddEntityModel(viewer_->CreateEntityModel(
            "/res/models/car_blue.osgb",
            osg::Vec4(0.5, 0.5, 0.5, 1.0),
            viewer::EntityModel::EntityType::VEHICLE,
            false,
            "",
            0,
            EntityScaleMode::BB_TO_MODEL));
    }

    for(int i = 0;i<status.others_num; i++)
    {
        viewer_->entities_[i+1]->SetPosition(status.others[i].x, status.others[i].y, status.others[i].z);
        viewer_->entities_[i+1]->SetRotation(status.others[i].heading, 0, 0);
    }
    viewer_->osgViewer_->frame();
    return;

    // player->FrameWithTp(tp);
    // return;
    // viewer_->osgViewer_->frame();
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::duration<double> seconds = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch());
    // std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastTime);
    // double dt = time_span.count();
    // if(frameCount++ %20 == 0)
    // {
    //     std::chrono::duration<double> seconds = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch());
    //     printf("dt = %f\n now = %f seconds\n", dt, seconds.count());
    // }
    // lastTime = now;
    // return;

}
// END   FEATURE RENDERING_DEFAULT