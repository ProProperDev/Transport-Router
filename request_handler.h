#pragma once

#include <string_view>
#include <set>
#include <vector>
#include <optional>

#include "transport_catalogue.h"
#include "domain.h"
#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace transport_catalogue {


class RequestHandler {
public:
    
    RequestHandler(MapRenderer& map_renderer, TransportCatalogue& db, TransportRouter& router);
    
    //Запрос на обновление базы
    void UpdateDataBase(json::Node request_body);
    
    //Запрос на печать данных из бд
    void RequestToGetInfo(json::Node request_body, std::ostream& output);

    //Запрос на получение маршрута между двумя остановками
    std::optional<TransportRouter::RouteItems> BuildRouteByStops(const std::string_view start_stop,
                                                               const std::string_view finish_stop) const;  

private:
    TransportCatalogue& db_;
    MapRenderer& map_renderer_;
    TransportRouter& router_;
};

} // namespace transport_catalogue
