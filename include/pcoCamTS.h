#ifndef PCOCAM_THAISPICE
#define PCOCAM_THAISPICE

#include <stdexcept>
#include <chrono>
#include <thread>
#include <vector>
#include <sstream>

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
    
    DWORD err = 0;
    std::string errMsg;

    CPco_com* camera;
    // CPco_Log pcoCamLog;
    CPco_grab_clhs *grabber;

    WORD camtype;
    DWORD serialnumber;

    short ccdtemp = 0;
    short camtemp = 0;
    short pstemp = 0;

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

#endif