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

    // std::setenv("")
    std::string res;
    // std::string sourceCmd = ". /opt/siso/setup-siso-env.sh";
    // res = exec(sourceCmd.c_str());
    // // res = exec("ls -la");
    // std::cout << res << "\n";
    // std::cout << "Done with source command\n";

    // const char* ldPathString = std::getenv("PATH");
    // std::cout << "LD_LIBRARY_PATH: " << ldPathString << "\n\n\n";

    // std::cin >> res;
    
    unsigned int warn, err, info;
    
    // std::cout << "Here\n";
    // // setEnvVars();

    // // std::cout << "LD_LIBRARY_PATH = " << getenv("LD_LIBRARY_PATH"); 
    // // std::string test(getenv("LD_LIBRARY_PATH"));
    // char ldPathString[] = "LD_LIBRARY_PATH=/opt/SiliconSoftware/Runtime5.7.0/lib64:/opt/SiliconSoftware/Runtime5.7.0/genicam/bin/Linux64_x64:/opt/SiliconSoftware/Runtime5.7.0/lib";
    // // char sisoPathString[] = "SISODIR5=/opt/SiliconSoftware/Runtime5.7.0";
    // putenv(ldPathString);
    // // putenv(sisoPathString);
    // exec("source /opt/SiliconSoftware/Runtime5.7.0/setup-siso-env.sh");

    // std::string test2(getenv("LD_LIBRARY_PATH"));
    // std::cout << test2 << "\n";
    // std::cout << getenv("SISODIR5") << std::endl;
    // std::cout.flush();
    // std::cout << "2nd here\n\n";
    // // for(int xx=0; xx<100; xx++) std::cout << "\n";
    // std::cout.flush();
    
    
    // setenv("SISODIR5", "/opt/SiliconSoftware/Runtime5.7.0/", true);
    // const char* path = std::getenv("PATH");
    // std::string pathString(path);
    // pathString = "/opt/SiliconSoftware/Runtime5.7.0/bin:" + pathString;
    // setenv("PATH", pathString.c_str(), true);

    // setenv("LD_LIBRARY_PATH", "/opt/SiliconSoftware/Runtime5.7.0/genicam/bin/Linux64_x64:/opt/SiliconSoftware/Runtime5.7.0/lib:/opt/SiliconSoftware/Runtime5.7.0/lib64", true);
    // setenv("GENICAM_ROOT", "/opt/SiliconSoftware/Runtime5.7.0/genicam", true);
    // setenv("GENICAM_CACHE_V3_0", "//opt/SiliconSoftware/Runtime5.7.0/genicam/cache", true);
    // setenv("GENICAM_LOG_CONFIG_V3_0", "/opt/SiliconSoftware/Runtime5.7.0/genicam/log/config/SisoLogging.properties", true);
    
    WORD camtype = 0;
    DWORD serialnumber = 0;

    CPco_com * cam1 = new CPco_com_clhs();
    err = cam1->Open_Cam(0);
    std::string errMsg = printErrorMessage(err);
    std::cout << "\n\nError in cam0: " << errMsg << "\n";

    err=cam1->PCO_GetCameraType(&camtype, &serialnumber);
    std::cout << "CamType: " << camtype << "\nSerialNumber: " << serialnumber << std::endl;
    std::cout << std::flush;
    CPco_grab_clhs * grab1 = new CPco_grab_clhs((CPco_com_clhs*) cam1);
    err = grab1->Open_Grabber(0);

    // err = grabber->Open_Grabber(1);
    CPco_com * cam2 = new CPco_com_clhs();
    err = cam2->Open_Cam(1);
    errMsg = printErrorMessage(err);
    std::cout << "Error in cam1: " << errMsg << "\n";

    err=cam2->PCO_GetCameraType(&camtype, &serialnumber);
    std::cout << "CamType: " << camtype << "\nSerialNumber: " << serialnumber << std::endl;
    CPco_grab_clhs * grab2 = new CPco_grab_clhs((CPco_com_clhs*) cam2);
    err = grab2->Open_Grabber(1);


    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    grab1->Close_Grabber();
    cam1->Close_Cam();

    grab2->Close_Grabber();
    cam2->Close_Cam();

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

//=================================================//
// INIT camera thread function
//=================================================//
/*
int cameraInit(void *data)
{   
    char cc;
    int ch;
    frameStartFlag = false;

    DWORD err;
    CPco_com* camera;

    int x;
    int help=0;
    int board=0;
    char infostr[100];
    char number[20];

    int ima_count=100;
    int loop_count=1;
    int PicTimeOut=10000; //10 seconds

    WORD act_recstate,act_align;
    DWORD exp_time,delay_time,pixelrate;
    WORD exp_timebase,del_timebase;
    DWORD width,height,secs,nsecs;
    WORD triggermode;
    WORD binhorz,binvert;   
    WORD wRoiX0, wRoiY0, wRoiX1, wRoiY1;
    SC2_Camera_Description_Response description;
    double freq;
    SHORT ccdtemp,camtemp,pstemp;
    WORD camtype;
    DWORD serialnumber;
    int loglevel=0x0000F0FF;

    int bwmin,bwmax;


    mylog.set_logbits(loglevel);
    printf("Logging set to 0x%08x\n",mylog.get_logbits());
    camera= new CPco_com_clhs();
    if(camera==NULL)
    {
        printf("ERROR: Cannot create camera object\n");
        return -1;
    }

    if(loglevel>0) camera->SetLog(&mylog);

    printf("Try to open Camera\n");
    err=camera->Open_Cam(board);
    if(err!=PCO_NOERROR)
    {
        printf("ERROR: 0x%x in Open_Cam\n",err);
        printErrorMessage(err);
        delete camera;
        return -1;
    }

    err=camera->PCO_GetCameraType(&camtype,&serialnumber);
    if(err!=PCO_NOERROR)
    {
        printf("ERROR: 0x%x in PCO_GetCameraType\n",err);
        camera->Close_Cam();
        delete camera;
        return -1;
    }

    //the clhs_grabber class supports all PCO clhs cameras

    printf("Grabber is CPco_grab_clhs\n");
    grabber=new CPco_grab_clhs((CPco_com_clhs*)camera);
    // threadGrab = grabber;
    
    if(loglevel>0)
    grabber->SetLog(&mylog);

    printf("Try to open Grabber\n");
    err=grabber->Open_Grabber(board);
    if(err!=PCO_NOERROR)
    {
        printf("ERROR: 0x%x in Open_Grabber",err);
        delete grabber;
        camera->Close_Cam();
        delete camera;
        // return NULL;
    }

    err=grabber->Set_Grabber_Timeout(PicTimeOut);
    if(err!=PCO_NOERROR) printf("error 0x%x in Set_Grabber_Timeout",err);

    err=camera->PCO_GetCameraDescriptor(&description);
    if(err!=PCO_NOERROR) printf("PCO_GetCameraDescriptor() Error 0x%x\n",err);

    err=camera->PCO_GetInfo(1,infostr,sizeof(infostr));
    if(err!=PCO_NOERROR) printf("PCO_GetInfo() Error 0x%x\n",err);
    else
    {
        printf("Camera Name is: %s\n",infostr);
        printf("Camera Typ is : 0x%04x\n",camtype);
        printf("Camera Serial : %d\n",serialnumber);
    }

    err=camera->PCO_SetCameraToCurrentTime();
    if(err!=PCO_NOERROR) printf("PCO_SetCameraToCurrentTime() Error 0x%x\n",err);

    err=camera->PCO_GetTemperature(&ccdtemp,&camtemp,&pstemp);
    if(err!=PCO_NOERROR) printf("PCO_GetTemperature() Error 0x%x\n",err);
    else
    {
        printf("current temperatures\n");
        printf("Camera:      %d°C\n",camtemp);
        if(ccdtemp != (SHORT)(-32768)) printf("Sensor:      %d°C\n",ccdtemp);
        if(pstemp != (SHORT)(-32768)) printf("PowerSupply: %d°C\n",pstemp);
    }

    //set RecordingState to STOP
    err=camera->PCO_SetRecordingState(0);
    if(err!=PCO_NOERROR) printf("PCO_SetRecordingState() Error 0x%x\n",err);

    //start from a known state
    err=camera->PCO_ResetSettingsToDefault();
    if(err!=PCO_NOERROR) printf("PCO_ResetSettingsToDefault() Error 0x%x\n",err);

    err=camera->PCO_SetTimestampMode(2);
    if(err!=PCO_NOERROR) printf("PCO_SetTimestampMode() Error 0x%x\n",err);

    //set camera timebase to us
    exp_time    =50000;
    delay_time  =0;
    exp_timebase=1;
    del_timebase=1;

    err=camera->PCO_SetTimebase(del_timebase,exp_timebase);
    if(err!=PCO_NOERROR) printf("PCO_SetTimebase() Error 0x%x\n",err);

    err=camera->PCO_SetDelayExposure(delay_time,exp_time);
    if(err!=PCO_NOERROR) printf("PCO_SetDelayExposure() Error 0x%x\n",err);

    if(description.wNumADCsDESC>1)
    {
        err=camera->PCO_SetADCOperation(2);
        if(err!=PCO_NOERROR) printf("PCO_SetADCOperation() Error 0x%x\n",err);
    }

    err=camera->PCO_GetPixelRate(&pixelrate);
    if(err!=PCO_NOERROR) printf("PCO_GetPixelrate() Error 0x%x\n",err);
    else
    {
        printf("actual PixelRate: %d\n",pixelrate);
        printf("possible PixelRates:\n");
    }
    for(x=0;x<4;x++)
    {
        if(description.dwPixelRateDESC[x]!=0)
        {
            printf("%d: %d\n",x,description.dwPixelRateDESC[x]);
        }
    }

    err=camera->PCO_SetBitAlignment(BIT_ALIGNMENT_LSB);
    if(err!=PCO_NOERROR) printf("PCO_SetBitAlignment() Error 0x%x\n",err);

    //prepare Camera for recording
    err=camera->PCO_ArmCamera();
    if(err!=PCO_NOERROR) printf("PCO_ArmCamera() Error 0x%x\n",err);

    err=camera->PCO_GetBitAlignment(&act_align);
    if(err!=PCO_NOERROR) printf("PCO_GetBitAlignment() Error 0x%x\n",err);

    shift=0;
    if(act_align!=BIT_ALIGNMENT_LSB)
    {
        shift=16-description.wDynResDESC;
        printf("BitAlignment MSB shift %d\n",shift);
    }

    err=camera->PCO_GetTriggerMode(&triggermode);
    if(err!=PCO_NOERROR) printf("PCO_GetGetTriggermode() Error 0x%x\n",err);
    else printf("actual Triggermode: %d %s\n",triggermode,tmode[triggermode]);

    err=camera->PCO_GetBinning(&binhorz,&binvert);
    if(err!=PCO_NOERROR) printf("PCO_GetBinning() Error 0x%x\n",err);
    else printf("actual Binning: %dx%d\n",binhorz,binvert);

    err=camera->PCO_GetROI(&wRoiX0, &wRoiY0, &wRoiX1, &wRoiY1);
    if(err!=PCO_NOERROR) printf("PCO_GetROI() Error 0x%x\n",err);
    else printf("actual ROI: %d-%d, %d-%d\n",wRoiX0,wRoiX1,wRoiY0,wRoiY1);

    err=camera->PCO_GetActualSize(&width,&height);
    if(err!=PCO_NOERROR)
    {
        printf("PCO_GetActualSize() Error 0x%x\n",err);
        printf("Actual Resolution %d x %d\n",width,height);
    }

    err=grabber->PostArm();
    if(err!=PCO_NOERROR) printf("grabber->PostArm() Error 0x%x\n",err);
    err=camera->PCO_SetRecordingState(1);
    if(err!=PCO_NOERROR) printf("PCO_SetRecordingState() Error 0x%x\n",err);

    for(int ii=0;ii<BUFNUM;ii++)
    {
        Cbuf[ii].Allocate(width,height,description.wDynResDESC,0,IN_BW);
        printf("Cbuf[%d] allocated width %d,height %d\n",ii,Cbuf[ii].Get_actwidth(),Cbuf[ii].Get_actheight());
    }

    Cdispwin= new CPCODisp;
    printf("CPCODisp created\n");

    sprintf(infostr,"pco.camera size %dx%d",width,height);
    if(Cdispwin->initialize((char*)infostr)!=PCO_NOERROR)
    {
        delete Cdispwin;
        Cdispwin=NULL;
    }

    if(Cdispwin)
    {
        printf("CPCODisp Set_Actual_pic(&Cbuf[0]\n");
        Cdispwin->Set_Actual_pic(&Cbuf[0]);
        bwmin=200;
        bwmax=10000;
        Cdispwin->SetConvert(bwmin,bwmax);
        Cdispwin->convert();
    }

    fflush(stdout); 
    cc=' ';

    SHORT CCDtemp, cameraTemp, powerTemp;
    std::string instr;

    camExpSettings pcoExpSettings, *settingsPtr;
    pcoExpSettings.wTimeBaseDelay = 1;
    pcoExpSettings.wTimeBaseExposure = 1;

    WORD wFrameRateStatus;
    DWORD dwFrameRate, dwFrameRateExposure;

    settingsPtr = &pcoExpSettings;
    delay_time = 0;
    //=================================================//
    // input loop
    //=================================================//
    while(cc != 'x')
    {

        std::cout << std::endl << std::endl << "f to get frame" << std::endl;
        std::cout << "v to grab 100 frames" << std::endl;
        std::cout << "e to set exp" << std::endl;
        std::cout << "x to quit" << std::endl << std::endl;

        for(int xx = 0; (xx < 2) && ((ch = getchar()) != EOF) && (ch != '\n'); xx++ )
        cc = (char)ch;

        if( cc == 'f')
        {
            {
                std::lock_guard<std::mutex> lk(frameStart);
                frameStartFlag = true;
                frameEndFlag = false;
            }
            frameReady.notify_one();

            std::unique_lock<std::mutex> lk(frameEnd);
            frameCaptured.wait(lk, []{return frameEndFlag;});


            Cdispwin->Set_Actual_pic(&Cbuf[0]);
            Cdispwin->convert();

        }

        if( cc == 'v')
        {
            using Duration = std::chrono::nanoseconds;
            std::vector<double> startTime;
            std::vector<double> frameTime;
            std::pair<int, int> bounds;
            int loopCount;

            std::cout << std::endl << "loop count: ";
            std::getline(std::cin, instr);
            std::stringstream(instr) >> loopCount;

            grabber->Get_actual_size(&width,&height,NULL);

            std::vector<CCambuf> allPics;
            CCambuf temp[0];
            temp[0].Allocate(width,height,description.wDynResDESC,0,IN_BW);

            // std::cout << "allocating for all pictures" << std::endl;
            // for(int ii = 0; ii<loopCount; ii++)
            // {
            //     allPics.push_back(temp);
            // }

            auto btotal = std::chrono::high_resolution_clock::now();
            
            std::vector<frameBuffer *> picbuffers;

            for(int ii=0; ii<loopCount; ii++) {
                picbuffers.push_back(new frameBuffer);
                picbuffers[ii]->picbuf = (WORD*)malloc(width*height*sizeof(WORD));
            }
            
            std::cout << "Starting Frame Capture" << std::endl;
            for(int ii = 0; ii<loopCount; ii++)
            {
                auto begin = std::chrono::high_resolution_clock::now();
                // err=grabber->Acquire_Image(Cbuf->Get_actadr());
                // err=grabber->Get_Image(0,0,picbuf);
                err=grabber->Get_Image(0,0,picbuffers[ii]->picbuf);
                auto end = std::chrono::high_resolution_clock::now();

                // times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin));
                frameTime.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count());
                startTime.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end-btotal).count());

                if(err!=PCO_NOERROR) printErrorMessage(err);
            }
            std::cout << loopCount << " Frames Captured" << std::endl;
            
            double max = *max_element(frameTime.begin(), frameTime.end());
            double min = *min_element(frameTime.begin(), frameTime.end());

            double sum = std::accumulate(std::begin(frameTime), std::end(frameTime), 0.0);
            double mm =  sum / frameTime.size();

            double accum = 0.0;
            std::for_each (std::begin(frameTime), std::end(frameTime), [&](const double dd) {
                accum += (dd - mm) * (dd - mm);
            });
            double stdev = sqrt(accum / (frameTime.size()-1));

            std::cout << std::scientific;
            std::cout << std::setprecision(9);
            std::cout << std::setw(10) << "Min: "  << std::setw(20) << min << std::endl;
            std::cout << std::setw(10) << "Max: "  << std::setw(20) << max << std::endl;
            std::cout << std::setw(10) << "Mean: " << std::setw(20) << mm << std::endl;
            std::cout << std::setw(10) << "STD: "  << std::setw(20) << stdev << std::endl;

            std::ofstream stats ("timing.txt");
            // for(auto xx : frameTime) stats << std::fixed << std::setprecision(1) << xx << std::endl;
            for(int ii = 0; ii < frameTime.size(); ii++)
            {
                stats << std::fixed << std::setprecision(1) << startTime[ii] << "," << frameTime[ii] << std::endl;
            }
            stats.close();

            std::ostringstream fstream;
            std::string filenameTIF;
            std::string filenameb16;
            char *cstr;

            for(int ii = 0; ii<loopCount; ii++){
                fstream  << "out/frame" << std::setfill('0') << std::setw(5) << ii << ".tif";
                filenameTIF = fstream.str();
                fstream.str("");
                fstream  << "out/frame" << std::setfill('0') << std::setw(5) << ii << ".b16";
                filenameb16 = fstream.str();
                fstream.str("");
                // std::vector<char> cstr(filename.c_str(), filename.c_str() + filename.size() + 1);1
                
                cstr = &filenameTIF[0];
                store_tif(cstr, width, height, 0, picbuffers[ii]->picbuf);
                store_b16(&filenameb16[0], width, height, 0, picbuffers[ii]->picbuf);
            }

            picbuffers.clear();

            Cdispwin->Set_Actual_pic(&Cbuf[0]);
            // Cdispwin->Set_Actual_pic(picbuffers[0]->picbuf);
            Cdispwin->convert();

        }

        if( cc == 'e')
        {
            
            std::cout << std::endl << "enter exposure time in us: ";
            std::getline(std::cin, instr);
            std::stringstream(instr) >> exp_time;
            std::cout << "setting exp to " << exp_time << " us" << std::endl;

            //set RecordingState to STOP
            err=camera->PCO_SetRecordingState(0);
            if(err!=PCO_NOERROR) printErrorMessage(err);

            std::cout << "Get frame rate info" << std::endl;
            err=camera->PCO_GetFrameRate(&wFrameRateStatus,
                                         &dwFrameRate,
                                         &dwFrameRateExposure);
            if(err!=PCO_NOERROR) printErrorMessage(err);

            // err=camera->PCO_SetTimebase(del_timebase,exp_timebase);
            // if(err!=PCO_NOERROR) printf("PCO_SetTimebase() Error 0x%x\n",err);

            std::cout << "Set frame rate info" << std::endl;
            dwFrameRate = 10000;
            dwFrameRateExposure = exp_time;
            err = camera->PCO_SetFrameRate(&wFrameRateStatus, 0x0, &dwFrameRate, &dwFrameRateExposure);
            if(err!=PCO_NOERROR) printErrorMessage(err);

            std::cout << "Set exp rate info" << std::endl;
            err=camera->PCO_SetDelayExposure(delay_time,exp_time);
            if(err!=PCO_NOERROR) printErrorMessage(err);

            //prepare Camera for recording
            err=camera->PCO_ArmCamera();
            if(err!=PCO_NOERROR) printErrorMessage(err);

            err=grabber->PostArm();
            if(err!=PCO_NOERROR) printErrorMessage(err);
            err=camera->PCO_SetRecordingState(1);
            if(err!=PCO_NOERROR) printErrorMessage(err);

        }

        camera->PCO_GetTemperature(&CCDtemp, &cameraTemp, &powerTemp);

        std::cout << std::endl;
        std::cout << "CCD Temp: "    << std::setprecision(3) << CCDtemp/10 << std::endl;
        std::cout << "Camera Temp: " << std::setprecision(3) << cameraTemp << std::endl;
        std::cout << "Power Temp: "  << std::setprecision(3) << powerTemp << std::endl;

    }

    endGrabThread = true;

    for(int i=0;i<BUFNUM;i++) Cbuf[i].FreeBuffer();

    grabber->Close_Grabber();
    delete grabber;

    camera->Close_Cam();
    delete camera;

    delete Cdispwin;
    Cdispwin=NULL;

    std::cout << "finishing init cam thread" << std::endl;

    return NULL;
}
*/