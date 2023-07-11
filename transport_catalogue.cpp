#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <algorithm>
#include <tuple>
#include <numeric>
#include <map>
#include <iostream>
#include <iterator>

#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(domain::Stop new_stop) {
	domain::Stop* stop_ptr = &all_stops.emplace_back(std::move(new_stop));//std::move
	std::string_view stopname_(stop_ptr->name);
	stopname_to_stop.insert({stopname_, stop_ptr});
}

void TransportCatalogue::SetStopToStopDistance(const std::string_view& departure_stop,
	                                           const std::string_view& arrive_stop,
	                                           int64_t distance) {
	//создаём пару указателей на остановки из известных std::string_view остановок
	auto pair_of_stops_ptr = std::pair(stopname_to_stop[departure_stop], stopname_to_stop[arrive_stop]);
	stop_to_stop_distance[pair_of_stops_ptr] = distance;
}
    
int64_t TransportCatalogue::GetStopToStopDistance(domain::Stop* departure_stop,  domain::Stop* arrive_stop) const {
    if(stop_to_stop_distance.count(std::pair(departure_stop, arrive_stop))!=0){
        return stop_to_stop_distance.at({departure_stop, arrive_stop});
    } else {
        return stop_to_stop_distance.at({arrive_stop, departure_stop});
    }
}
    
const std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, double, detail::DistanceHasher<domain::Stop*>>&
TransportCatalogue::GetDistancesMap() const {
    return stop_to_stop_distance;
}


domain::Stop* TransportCatalogue::FindStop(std::string_view stopname) const {
	if(stopname_to_stop.count(stopname) == 0) {
		return nullptr;
	}
	return stopname_to_stop.at(stopname);
}

void TransportCatalogue::AddBus(domain::Bus new_bus) {
	domain::Bus* bus_ptr = &all_buses.emplace_back(std::move(new_bus));//std::move
	std::string_view busname_(bus_ptr->name);
	busname_to_bus.insert({busname_, bus_ptr});
    std::for_each(begin(bus_ptr->bus_route), end(bus_ptr->bus_route),
        	    [this, &busname_](const auto& stop_ptr) {stop_to_buses[stop_ptr].insert(busname_);});
}

domain::Bus* TransportCatalogue::FindBus(const std::string_view& busname) const {
	if(busname_to_bus.count(busname) == 0) {
		return nullptr;
	}
	return busname_to_bus.at(busname);
}

double TransportCatalogue::CalculateTeoryRouteLength(const std::vector<domain::Stop*>& bus_route) {
	double teory_route_length = std::transform_reduce(std::next(begin(bus_route)), end(bus_route),
	    begin(bus_route), 0.0, std::plus{},
				[this](const auto& stop_ptr_lhs, const auto& stop_ptr_rhs) {
					return ComputeDistance(stop_ptr_lhs->stop_geo_pos, stop_ptr_rhs->stop_geo_pos);});
	return teory_route_length;
}
    
const std::deque<domain::Bus>& TransportCatalogue::GetAllBuses() const{
	return all_buses;
}

const std::unordered_map<std::string_view, domain::Bus*>& TransportCatalogue::GetAllBusnameToBusPtr() const {
	return busname_to_bus;
}
    
const std::unordered_map<std::string_view, domain::Stop*>& TransportCatalogue::GetAllStops() const {
    return stopname_to_stop;
}

double TransportCatalogue::CalculateRealRouteLength(const std::vector<domain::Stop*>& bus_route) {
	double real_route_length = std::transform_reduce(std::next(begin(bus_route)), end(bus_route),
	                                                 begin(bus_route), 0.0, std::plus{},
				                                        [this](const auto& stop_ptr_rhs, const auto& stop_ptr_lhs) {
					                                     if(stop_to_stop_distance.count({stop_ptr_lhs, stop_ptr_rhs})) {
						                                     return stop_to_stop_distance[{stop_ptr_lhs, stop_ptr_rhs}];
					                                     } else {
						                                     return stop_to_stop_distance[{stop_ptr_rhs, stop_ptr_lhs}];};});
	return real_route_length;
}

domain::InfoAboutBus TransportCatalogue::GetBusInfo(std::string_view busname) {
	if(!FindBus(busname)) {
		std::cerr<<"error";
	}
	domain::Bus* found_bus = FindBus(busname);
	std::vector<domain::Stop*> copy_route = found_bus->bus_route;//копируем маршрут
	std::sort(begin(copy_route), end(copy_route));
	auto iter_uniq_stops = std::unique(begin(copy_route), end(copy_route));
	int uniq_stops = std::distance(begin(copy_route), iter_uniq_stops);

	//считаем теоретическую длину маршрута (складывая кратчайшее расстояние по поверхности земли между 2-мя координатами(остановками))
	double teory_route_length = CalculateTeoryRouteLength(found_bus->bus_route);

	//считаем реальную длину маршрута, складывая известные расстояния между остановками маршрута
	double real_route_length = CalculateRealRouteLength(found_bus->bus_route);

	auto curvature = real_route_length/teory_route_length;
	return {busname, found_bus->bus_route.size(), uniq_stops, real_route_length, curvature};
}

std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stopname) {
	auto stop_ptr = FindStop(stopname);
	return stop_to_buses[stop_ptr];
}

}//namespace transport_catalogue
