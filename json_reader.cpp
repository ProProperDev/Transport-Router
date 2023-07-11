#include "json_reader.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace transport_catalogue {

// Верхний уровень обработки запроса    
void RequestProcess(std::istream& input,std::ostream& output, MapRenderer& renderer,
                    RequestHandler& request_handler, TransportRouter& router, Serializer& serialiser) {
    
    json::Document requests = json::Load(input);
    
    for (const auto& [request_type, request_body] : requests.GetRoot().AsDict()) {
        if (request_type == "base_requests") {
            request_handler.UpdateDataBase(request_body);

        } else if (request_type == "stat_requests") {
          request_handler.RequestToGetInfo(request_body, output);
            

        } else if (request_type == "render_settings") {
          SetRenderSettings(renderer, request_body);
        
        } else if (request_type == "routing_settings") {
          SetRouterSettings(router, request_body);
        
        } else if (request_type == "serialization_settings") {
            SetSerializationSettings(serialiser, request_body);
        }
    }
}
    
void ToMakeBaseAndSerialize(TransportCatalogue& catalogue, std::istream& input, MapRenderer& renderer,
                            TransportRouter& router, Serializer& serialiser, RequestHandler& request_handler) {
    json::Document requests = json::Load(input);
    for (const auto& [request_type, request_body] : requests.GetRoot().AsDict()) {
        if (request_type == "base_requests") {
            request_handler.UpdateDataBase(request_body);
        } else if (request_type == "render_settings"){
            SetRenderSettings(renderer, request_body);
        } else if (request_type == "routing_settings"){
            SetRouterSettings(router, request_body);
        } else if (request_type == "serialization_settings"){
            SetSerializationSettings(serialiser, request_body);
        }
    }
    serialiser.Serialize();
}

void DeserializeBaseAndGetInfo(TransportCatalogue& catalogue, std::istream& input, std::ostream& output,
                               RequestHandler& request_handler, TransportRouter& router, Serializer& serialiser) {
    json::Document requests = json::Load(input);
    for (const auto& [request_type, request_body] : requests.GetRoot().AsDict()) {
        if (request_type == "stat_requests") {
            serialiser.Deserialize();
            router.BuildAllRoutes();
            request_handler.RequestToGetInfo(request_body, output);
        } else if (request_type == "serialization_settings"){
            SetSerializationSettings(serialiser, request_body);
        }
    }
}
    
//Установка настроек роутера
void SetRouterSettings(TransportRouter& router, const json::Node& request_body) {
    json::Dict setting_dict = request_body.AsDict();
    TransportRouter::RouterSettings router_settings {
        setting_dict.at("bus_wait_time").AsInt(),
        setting_dict.at("bus_velocity").AsDouble()
    };
    router.SetSettings(router_settings);
}
    
//Установка настроек сериализации
void SetSerializationSettings(Serializer& serialiser, const json::Node& request_body) {
    json::Dict setting_dict = request_body.AsDict();
    serialiser.SetSettings(setting_dict.at("file").AsString());
}

// Установка настроек карты
void SetRenderSettings(MapRenderer& renderer, const json::Node& request_body) {
    json::Dict setting_dict = request_body.AsDict();
    std::vector<svg::Color> color_palette;
    for (const auto& color_node: setting_dict.at("color_palette").AsArray()) {
        color_palette.push_back(CreateColorByNode(color_node));
    }
    render_settings render_settings{
        setting_dict.at("width").AsDouble(),
        setting_dict.at("height").AsDouble(),
        setting_dict.at("padding").AsDouble(),
        setting_dict.at("line_width").AsDouble(),
        setting_dict.at("stop_radius").AsDouble(),
        setting_dict.at("bus_label_font_size").AsInt(),
        {setting_dict.at("bus_label_offset").AsArray()[0].AsDouble(), setting_dict.at("bus_label_offset").AsArray()[1].AsDouble()},
        setting_dict.at("stop_label_font_size").AsInt(),
        {setting_dict.at("stop_label_offset").AsArray()[0].AsDouble(), setting_dict.at("stop_label_offset").AsArray()[1].AsDouble()},
        CreateColorByNode(setting_dict.at("underlayer_color")),
        setting_dict.at("underlayer_width").AsDouble(),
        color_palette
    };
    
    renderer.SetSettings(render_settings);
}
    
svg::Color CreateColorByNode(const json::Node& color_node) {
    svg::Color color;
    if (color_node.IsString()) {
        color = color_node.AsString();
    } else if (color_node.AsArray().size() == 3) {
        color = svg::Rgb(color_node.AsArray()[0].AsDouble(),
                         color_node.AsArray()[1].AsDouble(),
                         color_node.AsArray()[2].AsDouble());
    } else if (color_node.AsArray().size() == 4) {
        color = svg::Rgba(color_node.AsArray()[0].AsDouble(),
                          color_node.AsArray()[1].AsDouble(),
                          color_node.AsArray()[2].AsDouble(),
                          color_node.AsArray()[3].AsDouble());
    }
    return color;
}
    
// Возвращает готовую остановку на добавление
domain::Stop ParseStopInfo(std::map<std::string, json::Node> to_set_info) {
    auto raw_info = to_set_info;
    std::string stopname = raw_info["name"].AsString();
    double latitude = raw_info["latitude"].AsDouble();
    double longitude = raw_info["longitude"].AsDouble();
    return {stopname, {latitude, longitude}};
}
        
// Парсинг информации с расстояниями до ближайших остановок
std::map<std::string, int>  ParseToNearStopDist(std::map<std::string, json::Node> to_set_info) {
//Выделяем из запроса только словарь с расстояниями до ближайших остановок
    auto to_stop_dist = to_set_info["road_distances"].AsDict();
    std::map<std::string, int> result_map;
    for(auto info : to_stop_dist) {
        result_map[info.first] = info.second.AsInt();
    }
    return result_map;
}
        
//Возвращает готовый маршрут на добавление
domain::Bus ParseBusInfo(std::map<std::string, json::Node> to_set_info, TransportCatalogue& catalogue) {
    auto raw_info = to_set_info;
    std::string busname = raw_info["name"].AsString();
    std::vector<domain::Stop*> route = ParseRoute(raw_info["stops"].AsArray(), raw_info["is_roundtrip"].AsBool(), catalogue);
    return {busname, route, raw_info["is_roundtrip"].AsBool()};
}

// Парсинг остановок маршрута     
std::vector<domain::Stop*> ParseRoute(std::vector<json::Node> raw_route_info, bool is_roundtrip, TransportCatalogue& catalogue) {
    std::vector<domain::Stop*> result_route;
            
    if(is_roundtrip) {
        std::transform(begin(raw_route_info), end(raw_route_info), std::back_inserter(result_route),
                              [&catalogue](const auto& stop){return catalogue.FindStop(stop.AsString());});
        return result_route;
    } else {
        std::transform(begin(raw_route_info), end(raw_route_info), std::back_inserter(result_route),
                              [&catalogue](const auto& stop){return catalogue.FindStop(stop.AsString());});
        std::vector<domain::Stop*> rev = result_route;
        std::reverse(rev.begin(), rev.end());
        rev.erase(rev.begin());
        result_route.insert(result_route.end(),rev.begin(), rev.end());
        return result_route;
    }
}

//Обработка запроса на обновление бд
void ToUpdateDataBase(json::Node request_body, TransportCatalogue& db_) {
    std::vector<json::Node> buses;
    std::unordered_map<std::pair<std::string, std::string>, int64_t, detail::DistanceHasher<std::string>> stop_to_stop_dist;
        
    for (auto to_set_info_ : request_body.AsArray()) {
           //Обработка запроса на добавление остановки
            auto to_set_info = to_set_info_.AsDict();
            if (to_set_info["type"].AsString() == "Stop") {
                domain::Stop new_stop = std::move(ParseStopInfo(to_set_info));
                db_.AddStop(new_stop);
                auto near_stops_dist = ParseToNearStopDist(to_set_info);
                std::for_each(begin(near_stops_dist), end(near_stops_dist), [&new_stop, &stop_to_stop_dist](auto& to_near_stop)
                              {auto pair_of_stops = std::pair(new_stop.name, to_near_stop.first);
                               stop_to_stop_dist.emplace(pair_of_stops, to_near_stop.second);});
           }
            
            if(to_set_info["type"].AsString() == "Bus") {
                buses.push_back(to_set_info);
            }
        }
    //Добавляем расстояния между остановками
    std::for_each(begin(stop_to_stop_dist), end(stop_to_stop_dist), [&db_](auto between_stops_dist) {
                  const auto& [depart_stop, arrive_stop] = between_stops_dist.first;
                  const auto& dist = between_stops_dist.second;
                  db_.SetStopToStopDistance(depart_stop, arrive_stop, dist);});
        
        //Обработка запроса на добавление автобуса
    for (auto to_set_info : buses) {
        domain::Bus new_bus = std::move(ParseBusInfo(to_set_info.AsDict(), db_));
        db_.AddBus(new_bus);
    }    
}
    
void GetJsonAnswerForBus(json::Builder& builder, std::map<std::string, json::Node> info,  TransportCatalogue& db_) {
    if(db_.FindBus(info["name"].AsString())) {
        auto res = db_.GetBusInfo(info["name"].AsString());
        builder.StartDict()
            .Key("curvature").Value(res.curvature)
            .Key("request_id").Value(info["id"].AsInt())
            .Key("route_length").Value(res.real_route_length)
            .Key("stop_count").Value(static_cast<int>(res.amount_of_stops))
            .Key("unique_stop_count").Value(res.uniq_stops)
            .EndDict();
    } else {
        builder.StartDict()
            .Key("request_id").Value(info["id"].AsInt())
            .Key("error_message").Value("not found")
            .EndDict();
    }
}

void GetJsonAnswerForStop(json::Builder& builder, std::map<std::string, json::Node> info, TransportCatalogue& db_) {
    if(db_.FindStop(info["name"].AsString())) {
        builder.StartDict();      
        auto buses_ = db_.GetStopInfo(info["name"].AsString());
        std::vector<json::Node> buses;
        std::transform(begin(buses_), end(buses_), std::back_inserter(buses),
                       [](auto bus) {return static_cast<std::string>(bus);});
        builder.Key("buses").Value(buses)
               .Key("request_id").Value(info["id"].AsInt())
               .EndDict();
    } else {
        builder.StartDict()
               .Key("request_id").Value(info["id"].AsInt())
               .Key("error_message").Value("not found")
               .EndDict();
    }
}

void GetJsonAnswerForMap(json::Builder& builder, std::map<std::string, json::Node> info,
                         TransportCatalogue& db_, MapRenderer& map_renderer_) {
    std::ostringstream out;
    svg::Document doc =  map_renderer_.RenderMap(db_);
    doc.Render(out);
    std::string svg_string = out.str();
    builder.StartDict()
           .Key("request_id").Value(info["id"].AsInt())
           .Key("map").Value(svg_string)
           .EndDict();
}
    
void GetJsonAnswerForRoute(json::Builder& builder, std::map<std::string, json::Node> info,
                         TransportRouter& router, TransportCatalogue& db_) {
    std::optional<TransportRouter::RouteItems> items = router.BuildRouteByStops(db_.FindStop(info["from"].AsString()),
                                                                              db_.FindStop(info["to"].AsString()));

    if (items) {
        builder.StartDict()
            .Key("request_id").Value(info["id"].AsInt())
            .Key("total_time").Value(items.value().total_time)
            .Key("items")
            .StartArray();
        for (const auto& item : items.value().items) {
            InsertItem(builder, item);
        }
        builder.EndArray();
        builder.EndDict();
    } else {
        builder.StartDict()
            .Key("request_id").Value(info["id"].AsInt())
            .Key("error_message").Value("not found")
            .EndDict();
    }
}

void InsertItem(json::Builder& builder,const TransportRouter::Item& item) {
    if (item.type == TransportRouter::ItemType::WAIT) {
        builder.StartDict()
                .Key("type").Value("Wait")
                .Key("stop_name").Value(std::string(item.name))
                .Key("time").Value(item.time)
            .EndDict();
    } else if (item.type == TransportRouter::ItemType::BUS) {
        builder.StartDict()
                .Key("type").Value("Bus")
                .Key("bus").Value(std::string(item.name))
                .Key("span_count").Value(item.span_count)
                .Key("time").Value(item.time)
            .EndDict();
    }
}

//Обработка запроса на получение инфо о маршрутах и остановках из бд
void RequestToGetInfo(json::Node request_body, std::ostream& output,
                      TransportCatalogue& db_, MapRenderer& map_renderer_,TransportRouter& router_) {
    router_.BuildAllRoutes();
    json::Builder builder;
    builder.StartArray();
    auto raw_query = request_body.AsArray();
    for(auto info_about_ : raw_query) {
        auto info_about = info_about_.AsDict();
        // Формирование ответа на запрос Bus в JSON
        if(info_about["type"].AsString()=="Bus") {
            GetJsonAnswerForBus(builder, info_about, db_);
        }
        // Формирование ответа на запрос Stop в JSON
        if(info_about["type"].AsString()=="Stop") {
            GetJsonAnswerForStop(builder, info_about, db_);
        }
        // Формирование ответа на запрос Map в JSON
        if(info_about["type"].AsString()=="Map") {
            GetJsonAnswerForMap(builder, info_about, db_, map_renderer_);
        }
        //Формирование ответа на запрос Route в JSON
         if(info_about["type"].AsString()=="Route") {
            GetJsonAnswerForRoute(builder, info_about, router_, db_);
        }
    }

    builder.EndArray();
    json::Document res(builder.Build());
    json::Print(res, output);
}
    
} //namespace transport_catalogue