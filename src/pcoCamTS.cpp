#include "pcoCamTS.h"

void pcoMGRThread(std::unique_ptr<mgrThreadLock> lock, std::future<void> exitSignal){

    char outputBuffer[256];
    std::stringstream mgrStream;
    std::string mgrStringData;
    bool start = true;

    const std::string mgrCommandString("/home/viliam/PCO/pco_camera/pco_clhs/bin/./pco_clhs_mgr -g0x00010000");
    // Open pipe to file
    FILE* pipe = popen(mgrCommandString.c_str(), "r");
    if (!pipe) throw std::runtime_error("MGR PIPE failed to open");;

    // read till end of process:
    while ((!feof(pipe)) && (exitSignal.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout)) {

        // use buffer to read and add to result
        if (fgets(outputBuffer, 256, pipe) != NULL)
            mgrStream << outputBuffer;
        
        // std::cout << "Here\n";

        mgrStringData = mgrStream.str();
        mgrStream.str(std::string());

        if(start){

            std::cout << mgrStringData;

            if(mgrStringData.find("pco_clhs_mgr is already started"))
                throw std::runtime_error("mgr already running");

            std::size_t dotPos = mgrStringData.find("....");
            if(dotPos != std::string::npos){

                start = false;
                std::cout << "MGR status output: \n\n" 
                          << mgrStringData.substr(0, dotPos) << std::endl;
                
                {
                    std::lock_guard<std::mutex> lk{lock->mm};
                    lock->mgrRunning = true;
                }

            }
        }
   }

   pclose(pipe);

}

int tempTimeoutTask(int delay){
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    return 1;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}


void pcoControlThread(PCOcam * camObj, 
                      camThreadSettings settings,
                      ThreadsafeQueue<std::string, 10> * cmdQue,
                      std::future<void> exitSignal){

    std::cout << "PCO cam thread\n";
    std::optional<std::string> cmdString;

    //================================//
    // Launch async status and temperature checking timeout
    //================================//
    auto tempTimeout = std::async(std::launch::async, tempTimeoutTask, settings.tempReadTimeout);
    
    //================================//
    // Main running loop that waits for exit signal to kill thread
    //================================//
    while (exitSignal.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout){

        switch (camObj->stateMachineState)
        {
        case PCOIDLE_STATE:
            if(camObj->stateChange){
                std::cout << "PCO state changed to PCOIDLE_STATE\n";
                camObj->stateChange = false;
                
                camObj->err = camObj->camera->PCO_SetRecordingState(false);
                if(camObj->err!=PCO_NOERROR) camObj->processErrVal();

                camObj->err = camObj->grabber->Stop_Acquire();
                if(camObj->err!=PCO_NOERROR) camObj->processErrVal();
            }

            break;
        
        case PCOINITREC_STATE:
            if(camObj->stateChange){
                std::cout << "PCO state changed to PCOINITREC_STATE\n";
                camObj->stateChange = false;

                camObj->err = camObj->camera->PCO_ArmCamera();
                if(camObj->err!=PCO_NOERROR) camObj->processErrVal();
                
                camObj->err = camObj->camera->PCO_SetRecordingState(true);
                if(camObj->err!=PCO_NOERROR) camObj->processErrVal();

                camObj->err = camObj->grabber->Start_Acquire();
                if(camObj->err!=PCO_NOERROR) camObj->processErrVal();
            }

            // grab frame. Do nothing. yet
            camObj->err = camObj->grabber->Wait_For_Next_Image(camObj->picBuf->at(0).picbuf, 500);

            break;
        default:
            break;
        }
        
        //==============================================//
        // Check if there's a command to process
        //==============================================//
        cmdString = cmdQue->pop();
        if(cmdString){

            std::string stringVal = cmdString.value();
            std::vector<std::string> vecStrings = split(stringVal, ' ');

            if(vecStrings[0].find("exposure") != std::string::npos){
                
                DWORD expCmdTime;
                std::cout << "exp command" << "\n";
                
                expCmdTime = std::atoi(vecStrings[1].c_str());
                camObj->cameraExposureSettings.dwFrameRateExposure = expCmdTime;
                camObj->cameraExposureSettings.wFrameRateMode = 0x0002; // prioritize exposure, not frame rate
                camObj->updateExposureSettings();

            }

            else if(vecStrings[0].find("framerate") != std::string::npos){
                
                DWORD frameRateCmdTime;
                std::cout << "framerate command" << "\n";
                
                frameRateCmdTime = std::atoi(vecStrings[1].c_str());
                camObj->cameraExposureSettings.dwFrameRate = frameRateCmdTime;
                camObj->cameraExposureSettings.wFrameRateMode = 0x0001; // prioritize frame rate, not exposure
                camObj->updateExposureSettings();

            }

            else if (vecStrings[0].find("state") != std::string::npos)
            {
                /* code */
                // std::cout << "state change set to: " << vecStrings[1] << "\n";
                unsigned int stateCmdVal = std::atoi(vecStrings[1].c_str());
                switch (stateCmdVal)
                {
                case PCOIDLE_STATE:
                    /* code */
                    camObj->stateChange = true;
                    camObj->stateMachineState = PCOIDLE_STATE;
                    break;
                    
                case PCOINITREC_STATE:
                    /* code */
                    camObj->stateChange = true;
                    camObj->stateMachineState = PCOINITREC_STATE;
                    break;
                
                default:
                    break;
                }

            }
            

            // std::cout << cmdString.value() << "\n";
        }
        
        //==============================================//
        // Check if it's time to read PCO temperature. 
        //==============================================//
        auto status = tempTimeout.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready){

            tempTimeout = std::async(std::launch::async, tempTimeoutTask, settings.tempReadTimeout);
            camObj->getTemperature();
            camObj->camera->PCO_GetHealthStatus(&camObj->warnings,
                                                &camObj->errors,
                                                &camObj->camStatusVal);
            {
                std::lock_guard<std::mutex> lck(camObj->curlWriteMut);
                camObj->curlWriteReady = true;
            }
        }
    }

    std::cout << "PCO thread Done" << std::endl;

}

PCOcam::~PCOcam(){

    grabber->Close_Grabber();
    camera->Close_Cam();

    exitSignalCurlThread.set_value();
    curlTempWriterThread.join();

    std::cout << "Closed all CAM related things\n";
    std::cout.flush();
    
}

void PCOcam::processErrVal(){

    std::string errMsg = printErrorMessage(err);
    std::cout << errMsg << "\n";

}

// void PCOcam::curlInfluxWriter(int camNumber, std::future<void> exitSignal){
void PCOcam::curlInfluxWriter(int camNumber){

    
    CURL *curl;
    CURLcode res;

    // need to make these things read from the settings file. 
    std::string urldb = "http://localhost:8086/api/v2/write?org=thaispice&bucket=vn300Testing&precision=ns";
    std::string authHeader = "Authorization: Token FzANVq9O0CVYN4iHQivNgchUsZhM6HbomP0HuXHKuv5Xp11Xcyb5pEIuZbXnpOqSGfqEc03eel_cS9euGBTPxw==";
    std::string contHeader = "Content-Type: text/plain; charset=utf-8";
    std::string acceptHeader = "Accept: application/json";
    struct curl_slist* headers = NULL;

    // data strings for line protocol writing
    // std::string measTags = "PCOcam1,testdata=true ";
    std::stringstream line;
    std::string fullDataStr;
    line << "PCOCam" << camNumber << ",testdata=true";
    std::string measTags = line.str();
    line.str(std::string());

    //================================//
    // Main running loop that waits for exit signal to kill thread
    //================================//
    while (futCurl.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout){

        //================================//
        // Wait on cond variable that is set in the main PCO thread. 
        // should be set repeatedly at some given interval
        //================================//
        std::unique_lock<std::mutex> lck(curlWriteMut);
        if(curlCond.wait_for(lck, std::chrono::microseconds(1), [this]{return curlWriteReady;})){
            // std::cout << "Curl writing data\n";
            curlWriteReady = false;

            //================================//
            // Make influxdb line protocol data for temperature and status
            //================================//
            auto now = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now());
            line << measTags
                 << " ccdtemp=" << ccdtemp/10.0
                 << ",camtemp=" << camtemp
                 << ",ptemp=" << pstemp
                 << ",warningsVal=" << warnings
                 << ",errorsVal=" << errors
                 << ",camStatusVal=" << camStatusVal
                 << " " << now.time_since_epoch().count() << "\n";
                
            fullDataStr = line.str();
            line.str("");
            line.clear();

            //================================//
            // use libCurl to write to influxdb on flight comp
            //================================//
            curl = curl_easy_init();
            if(curl){

                headers = curl_slist_append(headers, authHeader.c_str());
                headers = curl_slist_append(headers, contHeader.c_str());
                headers = curl_slist_append(headers, acceptHeader.c_str());

                curl_easy_setopt(curl, CURLOPT_URL, urldb.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fullDataStr.c_str());

                /* Perform the request, res will get the return code */ 
                res = curl_easy_perform(curl);

                /* Check for errors */ 
                if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));

                /* always cleanup */ 
                curl_easy_cleanup(curl);
                headers = NULL;

            }

        }

    }

    curl_global_cleanup();

    std::cout << "PCO Curl Thread Done\n";

}

WORD PCOcam::updateExposureSettings(void){

    camera->PCO_SetFrameRate(&cameraExposureSettings.wFrameRateStatus,
                             cameraExposureSettings.wFrameRateMode,
                             &cameraExposureSettings.dwFrameRate,
                             &cameraExposureSettings.dwFrameRateExposure);
    
    if(recordingState==false){
        err = camera->PCO_ArmCamera();
        if(err!=PCO_NOERROR) processErrVal();
    }

    return cameraExposureSettings.wFrameRateStatus;
}

PCOcam::PCOcam(int camNumber){

    futCurl = exitSignalCurlThread.get_future();
    curlTempWriterThread = std::thread(&PCOcam::curlInfluxWriter, this, camNumber);
    
    cameraExposureSettings.dwFrameRate = 100000; // 100,00 mHz, should be 100 Hz
    cameraExposureSettings.dwFrameRateExposure = 1000000; // 1ms, should be 1E6 ns.
    cameraExposureSettings.wFrameRateMode = 0x0002;

    // DWORD logLvL = 0x0000F0FF;
    // DWORD logLvL = 0;
    std::stringstream ss;
    ss << "PCOLogCam_" << camNumber;
    std::string logname(ss.str());
    std::cout << ss.str() << std::endl;
    // std::string logname("pcoLogCam" + camNumber);
    // pcoCamLog = CPco_Log(logname.c_str());
    // pcoCamLog.set_logbits(logLvL);

    // Make new cam object
    camera = new CPco_com_clhs();
    if(camera==NULL)
    {
        // printf("ERROR: Cannot create camera object\n");
        // pcoCamLog.writelog(1, "ERROR: Cannot create camera object");
        throw std::runtime_error("Camera is NULL");
    }
    // if(logLvL>0) camera->SetLog(&pcoCamLog);

    // Opening cam object
    err = camera->Open_Cam(camNumber);
    if(err!=PCO_NOERROR)
    {
        // printf("ERROR: 0x%x in Open_Cam\n",err);
        errMsg = printErrorMessage(err);
        throw std::runtime_error(errMsg);
    }

    // Get camera type
    err=camera->PCO_GetCameraType(&camtype, &serialnumber);
    if(err!=PCO_NOERROR)
    {
        // printf("ERROR: 0x%x in PCO_GetCameraType\n",err);
        errMsg = printErrorMessage(err);
        camera->Close_Cam();
        delete camera;
        throw std::runtime_error(errMsg);
    }

    // Get cam grabber
    grabber = new CPco_grab_clhs((CPco_com_clhs*) camera);
    // if(logLvL>0) grabber->SetLog(&pcoCamLog);

    err = grabber->Open_Grabber(camNumber);
    if(err!=PCO_NOERROR)
    {
        // printf("ERROR: 0x%x in Open_Grabber",err);
        errMsg = printErrorMessage(err);
        delete grabber;
        camera->Close_Cam();
        delete camera;
        
        throw std::runtime_error(errMsg);
    }

    // set some camera and grabber params
    err=grabber->Set_Grabber_Timeout(PicTimeOut);
    // if(err!=PCO_NOERROR) printf("error 0x%x in Set_Grabber_Timeout",err);
    if(err!=PCO_NOERROR) processErrVal();

    err=camera->PCO_GetCameraDescriptor(&description);
    // if(err!=PCO_NOERROR) printf("PCO_GetCameraDescriptor() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();
    // std::this_thread::sleep_for(500ms);

    err=camera->PCO_GetInfo(1,infostr,sizeof(infostr));
    // if(err!=PCO_NOERROR) printf("PCO_GetInfo() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();
    else
    {
        // printf("Camera Name is: %s\n",infostr);
        // printf("Camera Typ is : 0x%04x\n",camtype);
        // printf("Camera Serial : %d\n",serialnumber);

        // std::this_thread::sleep_for(500ms);
        std::cout << "Camera Name is: " << (std::string) infostr << "\n";
        std::cout << "Camera Typ is: " << camtype << "\n";
        std::cout << "Camera Serial: " << serialnumber << "\n";
    }

    err=camera->PCO_SetCameraToCurrentTime();
    // if(err!=PCO_NOERROR) printf("PCO_SetCameraToCurrentTime() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();

    //set RecordingState to STOP
    err=camera->PCO_SetRecordingState(recordingState);
    // if(err!=PCO_NOERROR) printf("PCO_SetRecordingState() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();

    //start from a known state
    err=camera->PCO_ResetSettingsToDefault();
    // if(err!=PCO_NOERROR) printf("PCO_ResetSettingsToDefault() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();

    err=camera->PCO_SetTimestampMode(2);
    // if(err!=PCO_NOERROR) printf("PCO_SetTimestampMode() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();


    err=camera->PCO_SetTimebase(del_timebase,exp_timebase);
    // if(err!=PCO_NOERROR) printf("PCO_SetTimebase() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();

    err=camera->PCO_SetDelayExposure(delay_time,exp_time);
    // if(err!=PCO_NOERROR) printf("PCO_SetDelayExposure() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();

    if(description.wNumADCsDESC>1)
    {
        err=camera->PCO_SetADCOperation(2);
        // if(err!=PCO_NOERROR) printf("PCO_SetADCOperation() Error 0x%x\n",err);
        if(err!=PCO_NOERROR) processErrVal();
    }

    err=camera->PCO_GetPixelRate(&pixelrate);
    // if(err!=PCO_NOERROR) printf("PCO_GetPixelrate() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();
    else
    {
        // printf("actual PixelRate: %d\n",pixelrate);
        // printf("possible PixelRates:\n");

        std::cout << "actual PixelRate: " << pixelrate << "\n";
    }
    for(int x=0; x<4; x++)
    {
        if(description.dwPixelRateDESC[x]!=0)
        {
            // printf("%d: %d\n",x,description.dwPixelRateDESC[x]);
            std::cout << "possible PixelRate: " << description.dwPixelRateDESC[x] << "\n";
        }
    }

    err=camera->PCO_SetBitAlignment(BIT_ALIGNMENT_LSB);
    if(err!=PCO_NOERROR) processErrVal();
    
    //prepare Camera for recording
    err=camera->PCO_ArmCamera();
    if(err!=PCO_NOERROR) processErrVal();
    
    err=camera->PCO_GetBitAlignment(&act_align);
    if(err!=PCO_NOERROR) processErrVal();

    shift=0;
    if(act_align!=BIT_ALIGNMENT_LSB)
    {
        shift=16-description.wDynResDESC;
        printf("BitAlignment MSB shift %d\n",shift);
    }

    err=camera->PCO_GetTriggerMode(&triggermode);
    if(err!=PCO_NOERROR) processErrVal();
    else printf("actual Triggermode: %d %s\n",triggermode,tmode[triggermode]);

    err=camera->PCO_GetBinning(&binhorz,&binvert);
    if(err!=PCO_NOERROR) processErrVal();
    else printf("actual Binning: %dx%d\n",binhorz,binvert);

    err=camera->PCO_GetROI(&wRoiX0, &wRoiY0, &wRoiX1, &wRoiY1);
    if(err!=PCO_NOERROR) processErrVal();
    else printf("actual ROI: %d-%d, %d-%d\n",wRoiX0,wRoiX1,wRoiY0,wRoiY1);

    err=camera->PCO_GetActualSize(&width,&height);
    if(err!=PCO_NOERROR)
    {
        printf("PCO_GetActualSize() Error 0x%x\n",err);
        printf("Actual Resolution %d x %d\n",width,height);
        processErrVal();

    }

    err=grabber->PostArm();
    if(err!=PCO_NOERROR) printf("grabber->PostArm() Error 0x%x\n",err);

    // for(int ii=0;ii<2;ii++)
    // {
    //     CbufDefault[ii].Allocate(width,height,description.wDynResDESC,0,IN_BW);
    //     CbufHalf[ii].Allocate(width/2,height/2,description.wDynResDESC,0,IN_BW);
    //     printf("Cbuf[%d] allocated width %d,height %d\n",ii,CbufDefault[ii].Get_actwidth(),CbufDefault[ii].Get_actheight());
    // }

    picBuf1024.resize(numBufs, frameBuffer());
    picBuf2048.resize(numBufs, frameBuffer());
    for(int ii=0; ii<numBufs; ii++) {
    //     frameBuffer imageFrame;
    //     picBuf2048.push_back(imageFrame);
        picBuf2048[ii].picbuf = (WORD*)malloc(2048*2048*sizeof(WORD));
        
    //     picBuf1024.push_back(imageFrame);
        picBuf1024[ii].picbuf = (WORD*)malloc(1024*1024*sizeof(WORD));
    }

    if(width == 2048) picBuf = &picBuf2048;
    else if(width == 1024) picBuf = &picBuf1024;
    
}

void PCOcam::getTemperature(){
    
    err = camera->PCO_GetTemperature(&ccdtemp,&camtemp,&pstemp);
    
    if(err!=PCO_NOERROR){
        printf("ERROR: 0x%x in Open_Grabber",err);
        errMsg = printErrorMessage(err);
        
        throw std::runtime_error(errMsg);
    }

}