#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <chrono>
#include <thread>
#include <asio.hpp>

#include "threadSafeQueue.h"

// #include <InfluxDBFactory.h>

#define PCO_ERRT_H_CREATE_OBJECT
// #define _MSC_VER 1100

// #include "tsPCO.h"
#include "pcoCamTS.h"
#include "networkingControl.h"

// #define TOML_HEADER_ONLY 0
#include <toml++/toml.h>

std::mutex frameStart, frameEnd;
std::condition_variable frameReady, frameCaptured;
std::atomic<bool> endGrabThread{false};

bool frameStartFlag = false;
bool frameEndFlag   = false;

std::string exec(const char* cmd);
// const void setEnvVars(void);

int main(int argc, char *argv[]){

    
    
    //============================================//
    // Parse config file
    //============================================//
	toml::table tbl;
    try
    {
        tbl = toml::parse_file(argv[1]);
    }
    catch (const toml::parse_error& err)
    {
        std::cerr << "Parsing failed:\n" << err << "\n";
        return 1;
    }

    //setup up PCO MGR THREAD with future/promise kill pattern
    std::promise<void> exitSignalPCO_MGR;
    std::future<void> futPCOMGR = exitSignalPCO_MGR.get_future();

    mgrThreadLock mgrLock;
    mgrLock.mgrRunning = true;
    std::unique_ptr<mgrThreadLock> lockPtr(&mgrLock);
    // std::thread pcoMGR( pcoMGRThread, std::move(lockPtr), std::move(futPCOMGR));

    // std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    std::cout << "Main thread waiting for mgr\n";
    std::unique_lock<std::mutex> lk{mgrLock.mm};
    while (!mgrLock.mgrRunning) mgrLock.cond.wait(lk);
    
    //==============================================//
    // Start PCO threads
    //==============================================//
    PCOcam cam0(0);
    PCOcam cam1(1);

    ThreadsafeQueue<std::string, 10> pcoCam1cmds;
    ThreadsafeQueue<std::string, 10> pcoCam0cmds;
    
    camThreadSettings cam1Settings;
    camThreadSettings cam0Settings;
    
    std::optional<int> iST = tbl["PCOtimeouts"]["imageSendingTimeout"].value<int>();
    std::optional<int> tRT = tbl["PCOtimeouts"]["tempReadTimeout"].value<int>();
    
    cam0Settings.tempReadTimeout = *tRT;
    cam0Settings.imageSendingTimeout = *iST;
    cam1Settings.tempReadTimeout = *tRT;
    cam1Settings.imageSendingTimeout = *iST;

    //setup up PCO cam 0 thread with future/promise kill pattern
    std::promise<void> exitSignalPCO0;
    std::future<void> futPCOThread0 = exitSignalPCO0.get_future();
    std::thread pcoThreadCam0(pcoControlThread, &cam0, cam0Settings, &pcoCam0cmds, std::move(futPCOThread0));

    //setup up PCO cam 0 thread with future/promise kill pattern
    std::promise<void> exitSignalPCO1;
    std::future<void> futPCOThread1 = exitSignalPCO1.get_future();
    std::thread pcoThreadCam1(pcoControlThread, &cam1, cam1Settings, &pcoCam1cmds, std::move(futPCOThread1));
    

    // for(int ii=0; ii<5; ii++){
    for(;;){
        std::string line;
        std::getline(std::cin, line);

        pcoCam1cmds.push(line);

        if(line == "x") break;
    }
    
    exitSignalPCO1.set_value();
    pcoThreadCam1.join();

    exitSignalPCO0.set_value();
    pcoThreadCam0.join();

    exitSignalPCO_MGR.set_value();

    std::cout << "Done\n";
    return 0;
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    std::cout << cmd << std::endl;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
