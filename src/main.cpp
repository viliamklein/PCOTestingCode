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

#include "tsPCO.h"
#include "pcoCamTS.h"

std::mutex frameStart, frameEnd;
std::condition_variable frameReady, frameCaptured;
std::atomic<bool> endGrabThread{false};

bool frameStartFlag = false;
bool frameEndFlag   = false;

std::string exec(const char* cmd);
// const void setEnvVars(void);

int main(int argc, char *argv[]){

    
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
    // PCOcam cam0(0);
    PCOcam cam1(0);

    ThreadsafeQueue<std::string, 10> pcoCam1cmds;
    
    camThreadSettings cam1Settings;
    cam1Settings.tempReadTimeout = 5000;
    cam1Settings.imageSendingTimeout = 200;
    //setup up PCO thread with future/promise kill pattern
    std::promise<void> exitSignalPCO1;
    std::future<void> futPCOThread1 = exitSignalPCO1.get_future();
    std::thread pcoThreadCam1(pcoControlThread, &cam1, cam1Settings, &pcoCam1cmds, std::move(futPCOThread1));

    // cam1.camera->PCO_SetTimestampMode

    // std::this_thread::sleep_for(std::chrono::milliseconds(26000));

    // for(int ii=0; ii<5; ii++){
    for(;;){
        std::string line;
        std::getline(std::cin, line);

        pcoCam1cmds.push(line);

        if(line == "x") break;
    }

    //===============================================//
	// ASIO sending
	//===============================================//
    /*
	asio::io_context io_context;
	asio::error_code error;

	std::stringstream gseport;
	gseport << 9998;

	asio::ip::tcp::resolver ipres(io_context);
	auto endpoints = ipres.resolve("10.40.0.69", gseport.str());
	asio::ip::tcp::socket socket(io_context);
    asio::connect(socket, endpoints, error);

	asio::socket_base::send_buffer_size option(0x8000);
	socket.set_option(option);

    socket.send(asio::buffer("testing PCO Connection"));

	io_context.stop();
    */

    // cam0.camera->PCO_SetRecordingState(1);
    // cam0.err = cam0.grabber->Start_Acquire();

    // cam1.camera->PCO_SetRecordingState(1);
    // cam1.err = cam1.grabber->Start_Acquire();

    // if(cam0.err!=PCO_NOERROR) cam0.processErrVal();

    // cam1.picBuf1024[0].picbuf;
    // cam0.(*picBuf)[0].picbuf;
    // cam0.picBuf->at(0).picbuf;

    // for(int ii = 0; ii<1000; ii++){
    //     cam0.err = cam0.grabber->Wait_For_Next_Image(cam0.picBuf->at(0).picbuf, 500);
    //     cam1.err = cam1.grabber->Wait_For_Next_Image(cam1.picBuf->at(0).picbuf, 500);
    //     std::cout << ii << "\r";
    //     std::cout.flush();
    // }
    // std::cout << "\n" << std::endl;
    
    // cam0.err = cam0.grabber->Stop_Acquire();
    // cam0.camera->PCO_SetRecordingState(0);
    
    // cam1.err = cam1.grabber->Stop_Acquire();
    // cam1.camera->PCO_SetRecordingState(0);
    
    exitSignalPCO1.set_value();
    pcoThreadCam1.join();

    exitSignalPCO_MGR.set_value();
    // pcoMGR.join();

    std::cout << "Done\n";

    // cameras->Close_Cam();
    // err = cam.Open_Cam(0);

    // CPco_com_clhs cam2;
    // err = cam2.Open_Cam(01);

    
    // res = exec("env");
    // std::cout << "\n" << res << "\n";


    // setenv("")
    // std::string res;
    // const char* ldres = std::getenv("SISODIR5");
    // std::cout << "LD_LIBRARY_PATH: " << ldres << 
    // "cameras->Close_Cam();n\n\n";

    // std::cin >> res;

    // try{
    //     PCOcam cam1;
    //     cam1.getTemperature();

    //     std::cout << "CCD temp: " <<  cam1.ccdtemp << "\n";
    //     std::cout << "Cam temp: " <<  cam1.camtemp << "\n";
    //     std::cout << "PS temp: " <<  cam1.pstemp << "\n";

    // }
    // catch(const std::exception& err){
    //     std::cout << err.what();
    //     std::cout << std::endl;
    //     return -1;
    // }
    // return 0;

    // CPco_com* cc1 = new CPco_com_clhs();
    // CPco_com* cc2 = new CPco_com_clhs();
    // CPco_clhs_cam * cc1 = new CPco_clhs_cam();
    // err = cc1->Open(0);
    
    // CPco_com* cc1 = new CPco_com_clhs();
    // err = cc1->Open_Cam(1);

    // if(err!=PCO_NOERROR)
    // {
    //     // printf("ERROR: 0x%x in Open_Cam\n",err);
    //     std::string errMsg = printErrorMessage(err);
    //     // delete camera;
    //     cc1->Close_Cam();
    //     cc1->Close_Cam();
    //     throw std::runtime_error(errMsg);
    // }
    

    // err = cc1->Open_Cam(1);

    // // DWORD err = 0;
    // // err = cc1->Open_Cam(0);
    // // err = cc1->Open_Cam(1);
    // std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // CPco_grab_clhs* grabCam1;
    // grabCam1 = new CPco_grab_clhs((CPco_com_clhs*) cc1);
    // err = grabCam1->Open_Grabber(1);
    
    // // err = cc2->Open_Cam(1);
    // // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // if(err!=PCO_NOERROR)
    // {
    //     // printf("ERROR: 0x%x in Open_Cam\n",err);
    //     std::string errMsg = printErrorMessage(err);
    //     // delete camera;
    //     cc1->Close_Cam();
    //     cc1->Close_Cam();
    //     throw std::runtime_error(errMsg);
    // }
    
    // // return 0;
    
    // PCOcam cam1(0);
    // {
    //     using namespace std::chrono_literals;

    //     std::cout << "\n\n Setting recording state" << std::endl;
    //     cam1.err = cam1.camera->PCO_SetRecordingState(1);
    //     if(cam1.err != PCO_NOERROR) cam1.processErrVal();
    //     std::this_thread::sleep_for(500ms);
    // }

    // std::cout << "grab frames\n";


    // for(int ii=0; ii<700; ii++){ 

    //     std::cout << "Frame " << ii << " start " << std::flush;
    //     // cam1.err = cam1.grabber->Get_Image(0,0, &cam1.CbufDefault[0]);
    //     // cam1.err = cam1.grabber->Get_Image(0,0, &cam1.CbufDefault[0]);
    //     cam1.err = cam1.grabber->Get_Image(0,0, cam1.picbuffers[ii % 2]->picbuf);
    //     if(cam1.err!=PCO_NOERROR) {
    //         cam1.processErrVal();
    //         break;
    //     }
    //     std::cout << "end\n";

    //     cam1.err = cam1.camera->PCO_GetHealthStatus(&warn, &err, &info);
    //     std::cout << warn << ", " << err << ", " << info << "\n";
    //     warn = 0;
    //     err = 0;
    //     info = 0;

    //     // if(ii == 800){
            
    //     //     using namespace std::chrono_literals;
    //     //     cam1.err=cam1.camera->PCO_SetDelayExposure(cam1.delay_time, (DWORD) 100000);
    //     //     if(cam1.err!=PCO_NOERROR){
    //     //         cam1.processErrVal();
    //     //         break;
    //     //     }
    //     //     std::this_thread::sleep_for(100ms);
    //     // }

    // }

    
    // cam1.err = cam1.camera->PCO_SetRecordingState(0);
    // if(cam1.err != PCO_NOERROR) cam1.processErrVal();

    return 0;
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
