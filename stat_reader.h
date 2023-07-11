#pragma once

#include <string_view>
#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"

namespace transport_catalogue {

namespace stat_reader {

void PrintBusInfo(TransportCatalogue& catalogue, std::string_view name, std::ostream& output);

void RequestToGetInfo(TransportCatalogue& catalogue, std::istream& input, std::ostream& output);

void PrintStopToBuses(TransportCatalogue& catalogue, std::string_view stopname, std::ostream& output);

}//namespace stat_reader
}//namespace transport_catalogue