#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
namespace domain {

struct InfoAboutBus {
	std::string_view busname;
	size_t amount_of_stops;
    int uniq_stops;
    double real_route_length;
    double curvature;
};

struct Stop {
    Stop() = default;
	Stop(std::string name, Coordinates geo_pos)
	:name(std::move(name))
	, stop_geo_pos(geo_pos) {		
	}
    
    std::string name;
	Coordinates stop_geo_pos;
};

struct Bus {
    Bus() = default;
	Bus(std::string name, std::vector<Stop*> route, bool is_roundtrip)
	:name(std::move(name))
	, bus_route(std::move(route))
	, is_roundtrip(is_roundtrip) {
	}

	std::string name;
	std::vector<Stop*> bus_route;
	bool is_roundtrip;
};
	
} //namespace domain