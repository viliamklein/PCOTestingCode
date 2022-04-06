#ifndef PCOCAM_SIMPLE
#define PCOCAM_SIMPLE

#include <stdexcept>
#include <chrono>
#include <thread>
#include <vector>

#include "tsPCO.h"

#include "VersionNo.h"
#include "Cpco_com.h"
#include "Cpco_grab_clhs.h"
#include "file12.h"
#include "Ccambuf.h"
#include "Cpcodisp.h"
#include "PCO_errt.h"


struct pcoGenSettings
{
    PCO_General strGeneral;
    PCO_CameraType strCamType;
    PCO_Sensor strSensor;
    PCO_Description strDescription;
    PCO_Timing strTiming;
    PCO_Storage strStorage;
    PCO_Recording strRecording;

};


#endif
