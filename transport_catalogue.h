#pragma once

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
#include <optional>
#include <set>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

namespace detail {

template<typename T>
struct DistanceHasher {
	size_t operator()(const std::pair<T, T> pair_of_elements) const {
		return hasher(pair_of_elements.first) ^ hasher(pair_of_elements.second);
    }
    std::hash<T> hasher;
};

}//namespace detail

class TransportCatalogue {
public:
	
	void AddStop(domain::Stop);
    
    void SetStopToStopDistance(const std::string_view& departure_stop,
    	                       const std::string_view& arrive_stop,
    	                       int64_t distance);
    
    int64_t GetStopToStopDistance(domain::Stop* departure_stop, domain::Stop* arrive_stop) const;

	domain::Stop* FindStop(std::string_view stopname) const;

	void AddBus(domain::Bus);

	domain::Bus* FindBus(const std::string_view& busname) const;

	domain::InfoAboutBus GetBusInfo(std::string_view busname);
    
    std::set<std::string_view> GetStopInfo(std::string_view stopname);
    
    const std::deque<domain::Bus>& GetAllBuses() const;
    
    const std::unordered_map<std::string_view, domain::Stop*>& GetAllStops() const;
    
    const std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, double, detail::DistanceHasher<domain::Stop*>>&
    GetDistancesMap() const;
    
    const std::unordered_map<std::string_view, domain::Bus*>& GetAllBusnameToBusPtr() const;
    
private:
    double CalculateTeoryRouteLength(const std::vector<domain::Stop*>& bus_route);
    double CalculateRealRouteLength(const std::vector<domain::Stop*>& bus_route);
		
	std::deque<domain::Stop> all_stops;
	std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop;
	std::deque<domain::Bus> all_buses;
	std::unordered_map<std::string_view, domain::Bus*> busname_to_bus;
	std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, double, detail::DistanceHasher<domain::Stop*>> stop_to_stop_distance;
    std::unordered_map<domain::Stop*, std::set<std::string_view>> stop_to_buses;
};

}//namespace transport_catalogue




