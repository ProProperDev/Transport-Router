#include <iostream>
#include <iomanip>
#include <tuple>
#include <string>

#include "stat_reader.h"

namespace transport_catalogue {

namespace stat_reader {

void RequestToGetInfo(TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
    int request_num = input_reader::ReadLineWithNumber(input);
    std::string request;
    for(int request_ = 0; request_ < request_num; ++request_) {
        std::getline(input, request);
        if(input_reader::TypeOfRequest(request) == input_reader::Request::BUS) {
            PrintBusInfo(catalogue, request.substr(4, request.npos), output);
        }
        if(input_reader::TypeOfRequest(request) == input_reader::Request::STOP) {
            PrintStopToBuses(catalogue, request.substr(5, request.npos), output);
        } else {
            return;
        }
    }
}

void PrintBusInfo(TransportCatalogue& catalogue, std::string_view name, std::ostream& output) {
    if (!(catalogue.FindBus(name))) {
        output << "Bus " << name << ": not found" << std::endl;
    } else {
        detail::InfoAboutBus bus_info = catalogue.GetBusInfo(name);
        output << "Bus " << name << ": "
                         << bus_info.amount_of_stops <<" stops on route, "
                         << bus_info.uniq_stops << " unique stops, "
                         << std::setprecision(6) << bus_info.real_route_length << " route length, "
                         << bus_info.curvature << " curvature"
                         << std::endl;
    }

}

void PrintStopToBuses(TransportCatalogue& catalogue, std::string_view stopname, std::ostream& output) {
    if(!(catalogue.FindStop(stopname))) {
        output << "Stop " << stopname << ": not found" << std::endl;
        return;
    }
    
    if(catalogue.GetStopInfo(stopname).empty()) {
        output << "Stop " << stopname << ": no buses" << std::endl;
        return;
    } else {
        output << "Stop " << stopname << ": buses";
        for(const auto& bus_name : catalogue.GetStopInfo(stopname)) {
            output << " " << bus_name;
        }
        output << std::endl;
        return;
    }
}

}//namespace stat_reader
}//namespace transport_catalogue
