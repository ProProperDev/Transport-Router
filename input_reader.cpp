#include <string>
#include <iostream>
#include <string_view>
#include <vector>
#include <algorithm>
#include <iterator>

#include "input_reader.h"

namespace transport_catalogue {

namespace input_reader {

//промежуточный map для хранения расстояний между остановками
std::unordered_map<std::pair<std::string, std::string>, int64_t, detail::DistanceHasher<std::string>> stop_to_stop_dist;

std::string ReadLine(std::istream& input) {
    std::string s;
    std::getline(input, s);
    return s;
}

int ReadLineWithNumber(std::istream& input) {
    int result;
    input >> result;
    ReadLine(input);
    return result;
}

//универсальная функциия выделения имени из запроса
std::string ParseName(std::string_view request) {
	int64_t space = request.find(' ');
	request.remove_prefix(space + 1);
	int64_t name_end = request.find(':');
	std::string name = std::string(request.substr(0,name_end));
	return name;
	}

//функция выделения из запроса координат остановки, 
//включающая в себя функцию записи во временный map расстояний до ближайших остановок
Coordinates ParseStopCoordinates(std::string_view request) {
	std::string stopname(ParseName(request));
	int64_t start = request.find(':');
	request.remove_prefix(start + 1);
	int64_t end_of_lat = request.find(',');
	double lat = std::stod(std::string(request.substr(0, end_of_lat)));
	request.remove_prefix(end_of_lat + 2);
	int64_t end_of_lng = request.find(',');
	double lng = std::stod(std::string(request.substr(0, end_of_lng)));
    
    if(end_of_lng!=request.npos) {
    	request.remove_prefix(end_of_lng + 2);
    	ParseNearStopDistances(stopname, request);
    }

    return {lat, lng};
}

void ParseNearStopDistances(std::string stopname, std::string_view request) {
    auto comma_pos = request[0];    
    while(comma_pos != request.npos) {
    	int64_t m_pos = request.find('m');
        int64_t dist = std::stoi(std::string(request.substr(0,m_pos)));
        request.remove_prefix(m_pos + 5);
        comma_pos = request.find(',');
        std::string dst_stop_name(request.substr(0,comma_pos));
        auto pair_of_stops = std::pair(stopname, dst_stop_name);
        stop_to_stop_dist.emplace(pair_of_stops, dist);//добавляем во временный map пару названий остановок и расстояние между ними
        if(comma_pos != request.npos) {
            request.remove_prefix(comma_pos + 2);
        }
    }
}

std::vector<std::string_view> ParseBusInfo(std::string_view request) {
	std::vector<std::string_view> result_route;
	int64_t start = request.find(':');
	request.remove_prefix(start + 2);
	//создаем универсальную лямбда-функцию для обработки маршрута из запроса
	auto processing_ = [&result_route](std::string_view request, char route_flag) {
		while(request.find(route_flag) != request.npos) {
			std::string_view stop_name(request.substr(0, request.find(route_flag)-1));
			result_route.push_back(stop_name);
			request.remove_prefix(request.find(route_flag)+2);
			if(request.find(route_flag) == request.npos) {
				result_route.push_back(request);
			} else {
				continue;
			}
		}
	};
	
	if(request.find('>') != request.npos) {
		processing_(request, '>');
	} else {
		processing_(request, '-');
		//добавляем остановки N-1, N-2 ... 0, тк кольцевой маршрут
		std::vector<std::string_view> rev = result_route;
		std::reverse(rev.begin(), rev.end());
		rev.erase(rev.begin());
		result_route.insert(result_route.end(),rev.begin(), rev.end());
	}
	return result_route;
}

Request TypeOfRequest(std::string_view request) {
	auto type_request = request.substr(0, request.find(' '));
	if(type_request == "Bus") {
		return Request::BUS;
	}

	if(type_request == "Stop") {
		return Request::STOP;
	}
	
	return Request::UNKNOWN;
}

void ProcessingRequest(std::istream& input, TransportCatalogue& catalogue) {
	int request_sum = ReadLineWithNumber(input);
	std::string request;
	std::vector<std::string> buses_to_add;
	for(int request_count=0; request_count < request_sum; ++request_count) {
		std::getline(input, request);

	 	Request type_of_request = TypeOfRequest(request);
	
	    if(type_of_request==Request::STOP) {
	    	catalogue.AddStop(ParseName(request), ParseStopCoordinates(request));
	    }
	
	    if(type_of_request==Request::BUS) {
            buses_to_add.emplace_back(std::move(request));//std::move
            continue;
	    
	    } else {
		    continue;
	    }
    }
    
    //после добавления всех остановок в каталог, записываем реальные расстояния между остановками из временного map,
    //и освобождаем его
    std::for_each(begin(stop_to_stop_dist), end(stop_to_stop_dist), [&catalogue](auto between_stops_dist) {
    	const auto& [depart_stop, arrive_stop] = between_stops_dist.first;
    	const auto& dist = between_stops_dist.second;
    	catalogue.SetStopToStopDistance(depart_stop, arrive_stop, dist);});
    stop_to_stop_dist.clear();
	 
	//добавляем все автобусы из запроса
	for(auto bus_to_add : buses_to_add) {
	    catalogue.AddBus(ParseName(bus_to_add), ParseBusInfo(bus_to_add));
	}
}

}//namespace input_reader
}//namespace transport_catalogue

