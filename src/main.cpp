
#include <iostream>
#include <SDL.h>
#include <vector>
#include <map>
#include <string>
#include "GamepadController.h"
#include <Poco/Net/HTTPRequest.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>
#include <sstream>
#include <build/AsyncTaskHandler.h>

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec) {
	os << "[";
	for (auto&& val : vec) {
		os << val << ", ";
	}
	os << "]" << '\n';
	return os;
}

std::ostream& operator<<(std::ostream& os, const GamepadAction& ac) {
	os << "New gamepad action: ";
	if (ac.type == GamepadActionType::GAMEPAD_AXIS) {
		os << "axis ";
	} else if (ac.type == GamepadActionType::GAMEPAD_BUTTON) {
		os << "button ";
	} else {
		os << "unimplemented";
	}
	os << "type:" << ac.num;
	os << " state:" << ac.val;
	os << '\n';
	return os;
}

std::string ofPostRequest(std::string url, std::string body) {
	try
	{
		Poco::URI uri(url);
		Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

		std::string path(uri.getPathAndQuery());
		if (path.empty()) path = "/";

		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1);
		req.setContentType("application/json");
		req.set("X-Api-Key", "3B9B5D370FA04B84827C4AC9EC6B4D2C");

		req.setContentLength(body.length());

		std::ostream& os = session.sendRequest(req);
		os << body;
		Poco::Net::HTTPResponse res;
		std::cout << res.getStatus() << " " << res.getReason() << std::endl;

		std::istream &is = session.receiveResponse(res);
		std::stringstream ss;
		Poco::StreamCopier::copyStream(is, ss);

		return ss.str();
	}
	catch (Poco::Exception &ex)
	{
		std::cerr << ex.displayText() << std::endl;
		return "";
	}
	return "";
}

//--------------------------------------------------------------
void setup(float x, float y, float feed) {

	std::string body = "{ \"command\" : \"G1 X" + std::to_string(x) + "Y" + std::to_string(y) + "F" + std::to_string(feed) + "\" }";

	ofPostRequest("http://192.168.1.43:5000/api/printer/command", body);
}
int main(int argc, char** argv){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	SDL_Window* window = SDL_CreateWindow("XBOX Controller", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 900, 0);
	if (!window)  {
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	bool running = true;
	std::cout << "Init" << '\n';
	std::cout << "Found controllers" << GamepadController::getAllControllers() <<'\n';
	std::unique_ptr<GamepadController> controller;
	if (GamepadController::attachedControllers()) {
		controller = std::make_unique<GamepadController>(-1);
		controller->notifier = [](const GamepadAction& action) {
			std::cout << action;
		};
	}

	long ticks = SDL_GetTicks();
	AsyncTaskHandler handler;
	std::string body = "{ \"command\" : \"G91\" }";
	std::cout << ofPostRequest("http://192.168.1.43:5000/api/printer/command", body) << "\n";
	while (running) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
			if (controller) {
				controller->processEvent(event);
			}
		}
		if (controller && (SDL_GetTicks() - ticks) > 100) {
			float x = SDL_JoystickGetAxis(controller->getJoystick(), 0);
			float y = SDL_JoystickGetAxis(controller->getJoystick(), 1);
			double scale = 5.f / 22000.f;
			if (std::abs(x) > 10000) {
				x -= 10000 * (x > 0 ? 1 : -1);
				x = (x * scale) + (x > 0 ? 1 : -1);
			} else {
				x = 0;
			}
			if (std::abs(y) > 10000) {
				y -= 10000 * (x > 0 ? 2 : -2);
				y = (-y * scale) + (y > 0 ? -2 : 2);
			} else {
				y = 0;
			}
			ticks = SDL_GetTicks();
			if (x != 0 || y != 0) {
				handler.addTask([x, y]()
				{
					setup(x, y, ((x + y)/2.0f) * 3000);
				}, [](){});
			}
		}
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyWindow(window);
	return 0;
}
