#ifndef PCOCAM_THAISPICE
#define PCOCAM_THAISPICE

#include <stdexcept>
#include <chrono>
#include <thread>
#include <vector>
#include <sstream>
// #include <fmt/format.h>
#include <future>
#include <curl/curl.h>
#include <optional>

#include "threadSafeQueue.h"
#include "tsPCO.h"
// extern "C"{

#include "VersionNo.h"
#include "Cpco_com.h"
#include "Cpco_grab_clhs.h"
#include "file12.h"
#include "Ccambuf.h"
#include "Cpcodisp.h"
#include "PCO_errt.h"

// }
#define PCOIDLE_STATE 0
#define PCOINITREC_STATE 1
#define PCORECNTRD_STATE 2


// struct frameBuffer {
//     WORD * picbuf;
//     int xx;
// };

class PCOcam
{
    public:

    // PCOcam();
    PCOcam(int camNumber);
    ~PCOcam();

    void getTemperature();
    void processErrVal();
    WORD updateExposureSettings();

    unsigned int stateMachineState = PCOIDLE_STATE;
    bool stateChange = false;
    bool recordingState = false;
    
    DWORD err = 0;
    std::string errMsg;

    CPco_com* camera;
    // CPco_Log pcoCamLog;
    CPco_grab_clhs *grabber;

    WORD camtype;
    DWORD serialnumber;

    signed short ccdtemp = 0;
    signed short camtemp = 0;
    signed short pstemp = 0;

    unsigned int warnings, errors, camStatusVal;

    WORD wRoiX0, wRoiY0, wRoiX1, wRoiY1;
    WORD binhorz,binvert;
    DWORD width,height,secs,nsecs;
    
    CCambuf CbufDefault[2];
    CCambuf CbufHalf[2];
    std::vector<frameBuffer > picBuf2048;
    std::vector<frameBuffer > picBuf1024;
    std::vector<frameBuffer > * picBuf;
    int numBufs = 100;
    
    DWORD exp_time = 1000;
    DWORD delay_time = 0;

    camExpSettings cameraExposureSettings;

    std::promise<void> exitSignalCurlThread;
    std::future<void> futCurl;
    std::thread curlTempWriterThread;
    // void curlInfluxWriter(int camNumber, std::future<void> exitSignal);
    void curlInfluxWriter(int camNumber);

    std::mutex curlWriteMut;
    std::condition_variable curlCond;
    bool curlWriteReady = false;

    private:

    int PicTimeOut = 500;
    SC2_Camera_Description_Response description;
    char infostr[100];

    //set camera timebase to us
    DWORD pixelrate;
    WORD exp_timebase = 1;
    WORD del_timebase = 1;

    WORD act_recstate, act_align;
    WORD triggermode;

};

void pcoControlThread(PCOcam * camObj, 
                      camThreadSettings settings,
                      ThreadsafeQueue<std::string, 10> * cmdQue,
                      std::future<void> exitSignal);
void pcoMGRThread(std::unique_ptr<mgrThreadLock> lock, std::future<void> exitSignal);

std::vector<std::string> split(const std::string &s, char delim);
#endif