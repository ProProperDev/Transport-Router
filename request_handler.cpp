#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */


namespace transport_catalogue {

RequestHandler::RequestHandler( MapRenderer& map_renderer, TransportCatalogue& db, TransportRouter& router)
: db_(db)
, map_renderer_(map_renderer)
, router_(router) {
}
//Перенаправляем разбор запроса в json_reader  
void RequestHandler::UpdateDataBase(json::Node request_body) {
    transport_catalogue::ToUpdateDataBase(request_body, db_);
}
//Перенаправляем разбор запроса в json_reader 
void RequestHandler::RequestToGetInfo(json::Node request_body, std::ostream& output) {
    transport_catalogue::RequestToGetInfo(request_body, output, db_, map_renderer_, router_);
}

std::optional<TransportRouter::RouteItems> RequestHandler::BuildRouteByStops(const std::string_view start_stop, const std::string_view finish_stop) const {
    return router_.BuildRouteByStops(db_.FindStop(start_stop), db_.FindStop(finish_stop));
}

} // namespace transport_catalogue
        