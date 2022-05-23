#include "networkingControl.h"
// #include "tsPCO.h"

// asio::awaitable<void> listener()
// {
//   auto executor = co_await this_coro::executor;
//   tcp::acceptor acceptor(executor, {tcp::v4(), 55555});
//   for (;;)
//   {
//     tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
//     co_spawn(executor, echo(std::move(socket)), detached);
//   }
// }

void asioRxCmd(std::future<void> exitSignal, 
			   ThreadsafeQueue<std::string, 10> *cmdQue,
			   networkThreadConfig netCfg){

	asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), netCfg.port));
	

	// asio::accep
	std::string msgData;


	for(;;){

		std::cout << "Waiting for connection\n";
		asio::ip::tcp::socket socket(io_context);
    	acceptor.accept(socket);

	//   std::size_t nn = asio::read_until(socket, asio::dynamic_string_buffer(msgData), "\n");
	
		std::cout << "Got connection\n";
		// std::size_t nn = asio::read(socket, asio::dynamic_string_buffer(msgData));
		std::size_t nn = asio::read_until(socket, asio::dynamic_string_buffer(msgData), "\n");
		// socket.close();

		std::cout << "Got: " << nn << " bytes\n";
		std::cout << msgData << std::endl;
		msgData.clear();
	}
	
	/*
	//===============================================//
	// ASIO sending
	//===============================================//
	asio::io_context io_context;
	asio::error_code error;

	std::stringstream gseport;
	gseport << netCfg.port;

	asio::ip::tcp::resolver ipres(io_context);
	auto endpoints = ipres.resolve(netCfg.GSEaddress, gseport.str());
	asio::ip::tcp::socket socket(io_context);
	asio::connect(socket, endpoints, error);

	std::string inputData, line;
	std::string endStr = "quit";
	// asio::dynamic_string_buffer ll(inputData);
	// ll.prepare(100);

	for(;;){
		std::size_t nn = 0;

		// nn = asio::read_until(socket, asio::dynamic_buffer(inputData), '\n');
		// // ll.consume
		// line = inputData.substr(0, nn);
		// std::cout << "Got this from server: " << line << "\n";
		// std::cout << "\n" << nn << " "  << "\n";

		try{
			
			asio::connect(socket, endpoints, error);
			std::cout << "\nConnected\n";

			nn = asio::read_until(socket, ll, '\n');

		}

		catch(const std::system_error& ee){

			std::cout << "got: " << ee.what() << "\n";
		}

		
		line = inputData.substr(0, nn);
		std::cout << "Got this from server: " << line << "\n";
		std::cout << "\n" << nn << " " << ll.size() << "\n";

		std::size_t ff = line.find(endStr);
		if (ff != std::string::npos) break;

		inputData.clear();
		line.clear();
		socket.close();

	}

	std::cout << "Here\n";
	// asio::read(socket, asio::buffer(vec));
	// asio::read_size_helper
	// asio::read(socket, asio::buffer(vec));
	// asio::read
	// std::string ss(vec.begin(), vec.end());


	
	io_context.stop();
	*/
}

asio::ip::tcp::socket openASIOSocket(asio::io_context & ioc,
									 networkThreadConfig netCfg){
	
	asio::error_code error;
	std::stringstream gseport;
	gseport << netCfg.port;

	asio::ip::tcp::resolver ipres(ioc);
	auto endpoints = ipres.resolve(netCfg.GSEaddress, gseport.str());
	asio::ip::tcp::socket socket(ioc);

	std::cout << "0x" << std::hex << std::this_thread::get_id() << " CONNECTING!\n";
    asio::connect(socket, endpoints, error);
	std::cout << "0x" << std::hex << std::this_thread::get_id() << " Done connecting!\n";

	// asio::async_connect()

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
	asio::io_context io_context(2);
	// asio::error_code error;

	// std::stringstream gseport;
	// gseport << configInfo.port;

	// asio::ip::tcp::resolver ipres(io_context);
	// auto endpoints = ipres.resolve(configInfo.GSEaddress, gseport.str());
	// asio::ip::tcp::socket socket(io_context);
	asio::ip::tcp::socket socket = openASIOSocket(io_context, configInfo);
	bool sockStatus = false;
	if(socket.is_open()) sockStatus = true;
	else std::cout << "not open\n";
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
