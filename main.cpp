#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
#include <iomanip>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "serialization.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void CreateAndSerializeBase(TransportCatalogue& database, MapRenderer& renderer, TransportRouter& router,
                            RequestHandler& request_handler, Serializer& serialiser) {
    ToMakeBaseAndSerialize(database, std::cin, renderer, router, serialiser, request_handler);
}

void DeserializeBaseAndGetAnswers(TransportCatalogue& database, MapRenderer& renderer, TransportRouter& router,
                                  RequestHandler& request_handler, Serializer& serialiser) {
    DeserializeBaseAndGetInfo(database, std::cin, std::cout, request_handler, router, serialiser);
}


int main(int argc, char* argv[]) {
    TransportCatalogue database;                                // Создаем транспортный каталог
    MapRenderer renderer;                                       // Создаем MapRenderer для прорисовки карты
    TransportRouter router(database);                           // Создаём Router для расчёта маршрутов
    RequestHandler request_handler(renderer, database, router); // Создаем RequestHandler класс обработчика запросов
    Serializer serialiser(database, renderer, router);          // Создаём Serializer для сериализации базы в файл
    
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        CreateAndSerializeBase(database, renderer, router, request_handler, serialiser);
        // make base here
    } else if (mode == "process_requests"sv) {
        DeserializeBaseAndGetAnswers(database, renderer, router, request_handler, serialiser);
        // process requests here
    } else {
        PrintUsage();
        return 1;
    }
}