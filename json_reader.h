#pragma once
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include <iostream>
#include <iomanip>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

namespace transport_catalogue {


class RequestHandler;

void RequestProcess(std::istream& input, std::ostream& output, MapRenderer& renderer, RequestHandler& request_handler,
                    TransportRouter& router, Serializer& serializer);
    
void ToMakeBaseAndSerialize(TransportCatalogue& catalogue, std::istream& input,MapRenderer& renderer,
                            TransportRouter& router,Serializer& serialiser, RequestHandler& request_handler);
    
void DeserializeBaseAndGetInfo(TransportCatalogue& catalogue, std::istream& input, std::ostream& output,
                               RequestHandler& request_handler,TransportRouter& router,Serializer& serialiser);

domain::Stop ParseStopInfo(std::map<std::string, json::Node> to_set_info);
    
std::map<std::string, int>  ParseToNearStopDist(std::map<std::string, json::Node> to_set_info);
    
domain::Bus ParseBusInfo(std::map<std::string, json::Node> to_set_info, TransportCatalogue& catalogue);
    
std::vector<domain::Stop*> ParseRoute(std::vector<json::Node> raw_route_info, bool is_roundtrip, TransportCatalogue& catalogue);
    
void SetRenderSettings(MapRenderer& renderer, const json::Node& request_body);
    
void SetRouterSettings(TransportRouter& router, const json::Node& request_body);
    
void SetSerializationSettings(Serializer& serialiser, const json::Node& request_body);
    
svg::Color CreateColorByNode(const json::Node& color_node);

void ToUpdateDataBase(json::Node request_body, TransportCatalogue& db_);

void RequestToGetInfo(json::Node request_body, std::ostream& output, TransportCatalogue& db_,
                      MapRenderer& map_renderer_, TransportRouter& router_);
    
void GetJsonAnswerForMap(json::Builder& builder, std::map<std::string, json::Node> info,
                         TransportCatalogue& db_, MapRenderer& map_renderer_);
    
void GetJsonAnswerForStop(json::Builder& builder, std::map<std::string, json::Node> info, TransportCatalogue& db_);
    
void GetJsonAnswerForBus(json::Builder& builder, std::map<std::string, json::Node> info,  TransportCatalogue& db_);
    
void GetJsonAnswerForRoute(json::Builder& builder, std::map<std::string, json::Node> info,
                           TransportRouter& router, TransportCatalogue& db_);
    
void InsertItem(json::Builder& builder,const TransportRouter::Item& item);
    
} // namespace transport_catalogue