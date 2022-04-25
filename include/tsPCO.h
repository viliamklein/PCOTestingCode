#ifndef TSPCOHEADER
#define TSPCOHEADER

#include <string>
#include <iostream>
#include <mutex>
#include <condition_variable>

// extern "C"{
#include "VersionNo.h"
#include "Cpco_com.h"
#include "Cpco_grab_clhs.h"
#include "file12.h"

#include "Ccambuf.h"
// #include "Cpcodisp.h"

#include "PCO_errt.h"
// }


// std::string printErrorMessage(DWORD errorValue);
extern const char tmode[4][20];


struct frameBuffer {
    WORD * picbuf;
    int xx;
};

struct  camExpSettings
{
    WORD wFrameRateStatus;
    DWORD dwFrameRate;
    DWORD dwFrameRateExposure;
    WORD wFrameRateMode;

    DWORD dwDelay;
    DWORD dwExposure;
    WORD wTimeBaseDelay;
    WORD wTimeBaseExposure;
};

struct PCOCamControlValues
{
    camExpSettings expSettings;
    int width, height;
    long imgSize;
    long sensorTemp;

    std::chrono::system_clock::time_point timeOfExp;
};

struct camThreadSettings
{
    std::string logFileName;
    
    unsigned int tempReadTimeout;
    unsigned int imageSendingTimeout;
    unsigned int initBinning;

};

struct mgrThreadLock
{
    std::mutex mm;
    std::condition_variable cond;
    bool mgrRunning;
};

#endif