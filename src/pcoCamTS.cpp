#include "pcoCamTS.h"

PCOcam::~PCOcam(){

    grabber->Close_Grabber();
    camera->Close_Cam();
    
}

void PCOcam::processErrVal(){

    std::string errMsg = printErrorMessage(err);
    std::cout << errMsg << "\n";

}

PCOcam::PCOcam(int camNumber){

    using namespace std::chrono_literals;

    DWORD logLvL = 0x0000F0FF;
    pcoCamLog.set_logbits(logLvL);

    // Make new cam object
    camera = new CPco_com_clhs();
    if(camera==NULL)
    {
        // printf("ERROR: Cannot create camera object\n");
        pcoCamLog.writelog(1, "ERROR: Cannot create camera object");
        throw std::runtime_error("Camera is NULL");
    }
    if(logLvL>0) camera->SetLog(&pcoCamLog);

    // Opening cam object
    std::cout << "Opening cam\n\n";
    // err=camera->Open_Cam(camNumber);

    // CPco_com_clhs cam();
    CPco_com * cam = new CPco_com_clhs();
    err = cam->GetConnectionStatus();
    if(err!=PCO_NOERROR)
    {
        // printf("ERROR: 0x%x in Open_Cam\n",err);
        errMsg = printErrorMessage(err);
        delete camera;
        throw std::runtime_error(errMsg);
    }

    cam1 = new CPco_com_clhs();
    cam2 = new CPco_com_clhs();
    if(logLvL>0) cam1->SetLog(&pcoCamLog);
    if(logLvL>0) cam2->SetLog(&pcoCamLog);

    err=cam1->Open_Cam(1);
    err=cam1->PCO_GetCameraType(&camtype, &serialnumber);
    std::this_thread::sleep_for(100ms);
    grabCam1 = new CPco_grab_clhs((CPco_com_clhs*) cam1);
    err = grabCam1->Open_Grabber(1);
    std::this_thread::sleep_for(100ms);

    err = cam2->Open_Cam(0);

    if(err!=PCO_NOERROR)
    {
        // printf("ERROR: 0x%x in Open_Cam\n",err);
        errMsg = printErrorMessage(err);
        delete camera;
        throw std::runtime_error(errMsg);
    }
    std::this_thread::sleep_for(500ms);

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
    std::this_thread::sleep_for(500ms);

    // get cam grabber
    
    std::cout << "Opening grabber\n\n";
    grabber = new CPco_grab_clhs((CPco_com_clhs*) camera);
    if(logLvL>0) grabber->SetLog(&pcoCamLog);

    err = grabber->Open_Grabber(0);
    if(err!=PCO_NOERROR)
    {
        // printf("ERROR: 0x%x in Open_Grabber",err);
        errMsg = printErrorMessage(err);
        delete grabber;
        camera->Close_Cam();
        delete camera;
        
        throw std::runtime_error(errMsg);
    }
    std::this_thread::sleep_for(500ms);

    // set some camera and grabber params
    err=grabber->Set_Grabber_Timeout(PicTimeOut);
    // if(err!=PCO_NOERROR) printf("error 0x%x in Set_Grabber_Timeout",err);
    if(err!=PCO_NOERROR) processErrVal();

    err=camera->PCO_GetCameraDescriptor(&description);
    // if(err!=PCO_NOERROR) printf("PCO_GetCameraDescriptor() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();
    std::this_thread::sleep_for(500ms);

    err=camera->PCO_GetInfo(1,infostr,sizeof(infostr));
    // if(err!=PCO_NOERROR) printf("PCO_GetInfo() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();
    else
    {
        // printf("Camera Name is: %s\n",infostr);
        // printf("Camera Typ is : 0x%04x\n",camtype);
        // printf("Camera Serial : %d\n",serialnumber);

        std::this_thread::sleep_for(500ms);
        std::cout << "Camera Name is: " << (std::string) infostr << "\n";
        std::cout << "Camera Typ is: " << camtype << "\n";
        std::cout << "Camera Serial: " << serialnumber << "\n";
    }

    err=camera->PCO_SetCameraToCurrentTime();
    // if(err!=PCO_NOERROR) printf("PCO_SetCameraToCurrentTime() Error 0x%x\n",err);
    if(err!=PCO_NOERROR) processErrVal();

    //set RecordingState to STOP
    err=camera->PCO_SetRecordingState(0);
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

    for(int ii=0; ii<10; ii++) {
        picbuffers.push_back(new frameBuffer);
        picbuffers[ii]->picbuf = (WORD*)malloc(width*height*sizeof(WORD));
    }

    //
}

void PCOcam::getTemperature(){
    
    err = camera->PCO_GetTemperature(&ccdtemp,&camtemp,&pstemp);
    
    if(err!=PCO_NOERROR){
        printf("ERROR: 0x%x in Open_Grabber",err);
        errMsg = printErrorMessage(err);
        
        throw std::runtime_error(errMsg);
    }

}