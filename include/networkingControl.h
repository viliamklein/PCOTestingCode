#ifndef NETWORKING_CONTROL_PCO
#define NETWORKING_CONTROL_PCO

#include <thread> // std::this_thread::sleep_for
#include <asio.hpp>

#include "ImageMessages.pb.h"
#include "threadSafeQueue.h"
#include "pcoCamTS.h"
// #include "tsPCO.h"



void PCOImagesNetworkingThread(std::future<void> exitSignal,
        ThreadsafeQueue<std::pair<PCOCamControlValues, std::vector<unsigned char>>, IMGQUEMAXLEN> *imgQue,
        networkThreadConfig configInfo);

asio::ip::tcp::socket openASIOSocket(asio::io_context & ioc, networkThreadConfig netCfg);

#endif
