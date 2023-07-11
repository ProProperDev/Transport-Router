#pragma once

#include <string>
#include <iostream>
#include <string_view>
#include <vector>
#include <algorithm>
#include <iterator>

#include "transport_catalogue.h"
#include "geo.h"

namespace transport_catalogue {

namespace input_reader {

//универсальная структура для определения типа запроса как при обновлении базы,
//так и при получении информации от базы
enum class Request {
	STOP,
	BUS,
	UNKNOWN
};

std::string ReadLine(std::istream& input);

int ReadLineWithNumber(std::istream& input);

std::string ParseName(std::string_view request);

Coordinates ParseStopCoordinates(std::string_view request);

void ParseNearStopDistances(std::string stopname, std::string_view request);

std::vector<std::string_view> ParseBusInfo(std::string_view request);

Request TypeOfRequest(std::string_view request);

void ProcessingRequest(std::istream& input, TransportCatalogue& catalogue);

}//namespace input_reader
}//namespace transport_catalogue