#include "networkingControl.h"
// #include "tsPCO.h"


asio::ip::tcp::socket openASIOSocket(asio::io_context & ioc,
									 networkThreadConfig netCfg){
	
	asio::error_code error;
	std::stringstream gseport;
	gseport << netCfg.port;

	asio::ip::tcp::resolver ipres(ioc);
	auto endpoints = ipres.resolve(netCfg.GSEaddress, gseport.str());
	asio::ip::tcp::socket socket(ioc);
    asio::connect(socket, endpoints, error);

	return socket;
}

void PCOImagesNetworkingThread(std::future<void> exitSignal,
        ThreadsafeQueue<std::pair<PCOCamControlValues, std::vector<unsigned char>>, IMGQUEMAXLEN> *imgQue,
		networkThreadConfig configInfo){

	// std::optional<ImageMessages::ASIimage> imgData;
    std::optional<std::pair<PCOCamControlValues, std::vector<unsigned char>>> imgData;
    ImageMessagesPCO::PCOImage frameToSend;

	int counter = 0;
	std::string sendBuffer = "";

	//===============================================//
	// ASIO sending
	//===============================================//
	asio::io_context io_context;
	// asio::error_code error;

	// std::stringstream gseport;
	// gseport << configInfo.port;

	// asio::ip::tcp::resolver ipres(io_context);
	// auto endpoints = ipres.resolve(configInfo.GSEaddress, gseport.str());
	// asio::ip::tcp::socket socket(io_context);
	asio::ip::tcp::socket socket = openASIOSocket(io_context, configInfo);
	bool sockStatus = true;
    // asio::connect(socket, endpoints, error);

	asio::socket_base::send_buffer_size option(0x8000);
	socket.set_option(option);


	//===============================================//
	// Waiting to send loop
	//===============================================//

	while (exitSignal.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout)
    {
		imgData = imgQue->pop();
		if (imgData) {
			std::cout << "got frame" << std::endl;
			
            frameToSend.set_header(0xAAAAAAAA);
            frameToSend.set_id(0);
			// frameToSend.set_bitdepth(16);
            // if(imgData->first.imgType == ASI_IMG_RAW16) frameToSend.set_bitdepth(16);
            // else frameToSend.set_bitdepth(8);
            
            frameToSend.set_width(imgData->first.width);
            frameToSend.set_height(imgData->first.height);
            frameToSend.set_messagebytelength(30+imgData->first.imgSize);
            std::string strData(imgData->second.begin(), imgData->second.end());
            frameToSend.set_imagedata(strData);

			frameToSend.AppendToString(&sendBuffer);

			if (socket.is_open()){
				
				socket.send(asio::buffer(sendBuffer));

				std::cout << "SendBuffer Size: " << sendBuffer.size() << std::endl;
			}
			else{
				std::cout << "Socket fail\n";
				sockStatus = false;
			}

			sendBuffer = "";
			std::cout << "Got frame. Que size: " << (int) imgQue->size() << " Frame num: " << (int) counter << "\n";
			counter++;
		}

		if(!sockStatus){
			socket.shutdown(asio::ip::tcp::socket::shutdown_both);
			socket = openASIOSocket(io_context, configInfo);
		}
	}

	io_context.stop();
}
