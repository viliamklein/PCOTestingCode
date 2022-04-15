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
#include "Cpcodisp.h"

#include "PCO_errt.h"
// }

#define BUFNUM 4

int image_nr_from_timestamp(void* buf,int shift);
DWORD grab_single(CPco_grab_clhs* grabber,void* picbuf);
DWORD get_image(CPco_grab_clhs* grabber,char* filename,WORD Segment,DWORD ImageNr);
DWORD grab_count_single(CPco_grab_clhs* grabber,int ima_count,CPCODisp* Cdispwin,CCambuf* Cbuf);
DWORD grab_count_wait(CPco_grab_clhs* grabber,int count);

void get_number(char* number,int len);
void get_text(char* text,int len);
void get_hexnumber(int* num,int len);

std::string printErrorMessage(DWORD errorValue);


extern const char tb[3][3];
extern const char tmode[4][20];


extern int shift;
extern CPCODisp* Cdispwin;

extern CPco_Log mylog;

extern CPco_grab_clhs *threadGrab;
extern CPco_grab_clhs *grabber;
extern CCambuf Cbuf[BUFNUM];

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

struct camThreadSettings
{
    std::string logFileName;
    
    unsigned int tempReadTimeout;
    unsigned int initBinning;

};

struct mgrThreadLock
{
    std::mutex mm;
    std::condition_variable cond;
    bool mgrRunning;
};

#endif