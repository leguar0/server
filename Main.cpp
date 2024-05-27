#define _WIN32_WINNT 0x0601
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "Database.h"

using std::string;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

string getRankType(item::RankType rank)
{
    switch (rank) 
    {
    case item::I: return "I";
    case item::II: return "II";
    case item::III: return "III";
    case item::IV: return "IV";
    case item::V: return "V";
    case item::VI: return "VI";
    case item::VII: return "VII";
    case item::VIII: return "VIII";
    case item::IX: return "IX";
    case item::X: return "X";
    default: return "null";
    }
}

string getItemType(item::ItemType type)
{
    switch (type) 
    {
    case item::ARMOUR: return "ARMOUR";
    case item::RING: return "RING";
    default: return "null";
    }
}

string getRarityType(item::RarityType rarity)
{
    switch (rarity)
    {
    case item::COMMON: return "COMMON";
    case item::UNCOMMON: return "UNCOMMON";
    case item::SET: return "SET";
    case item::RARE: return "RARE";
    case item::LEGENDARY: return "LEGENDARY";
    case item::ANCIENT: return "ANCIENT";
    default: return "null";
    }
}

void handle_request(http::request<http::string_body>& request, tcp::socket& socket, Database* db) {
    if (request.target() == "/items") {
        http::response<http::string_body> response(http::status::ok, request.version());
        response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        response.set(http::field::content_type, "application/json");
        response.set(http::field::cache_control, "no-cache, no-store, must-revalidate");
        response.keep_alive(request.keep_alive());

        std::string body = "[";

        const char* select_data_sql = "SELECT * FROM Items";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db->getHandle(), select_data_sql, -1, &stmt, nullptr);

        bool first = true;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            int level = sqlite3_column_int(stmt, 2);
            int type = sqlite3_column_int(stmt, 3);
            int rank = sqlite3_column_int(stmt, 4);
            int rarity = sqlite3_column_int(stmt, 5);

            std::string name_str(reinterpret_cast<const char*>(name));

            if (!first) {
                body += ",";
            }
            else {
                first = false;
            }

            body += "{";
            body += "\"id\":" + std::to_string(id) + ",";
            body += "\"name\":\"" + name_str + "\",";
            body += "\"level\":" + std::to_string(level) + ",";
            body += "\"type\":\"" + getItemType(static_cast<item::ItemType>(type)) + "\",";
            body += "\"rank\":\"" + getRankType(static_cast<item::RankType>(rank)) + "\",";
            body += "\"rarity\":\"" + getRarityType(static_cast<item::RarityType>(rarity)) + "\"";
            body += "}";
        }


        body += "]";

        response.body() = body;
        response.prepare_payload();

        http::write(socket, response);
    }
    else if (request.target() == "/item" && request.method() == http::verb::get) {
        const char* select_data_sql = "SELECT * FROM Items";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db->getHandle(), select_data_sql, -1, &stmt, nullptr);

        std::string html_form = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Items</title>
        </head>
        <body>
        )";

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            std::string name_str;
            if (name) {
                name_str = reinterpret_cast<const char*>(name);
            }

            int level = sqlite3_column_int(stmt, 2);
            int type = sqlite3_column_int(stmt, 3);
            int rank = sqlite3_column_int(stmt, 4);
            int rarity = sqlite3_column_int(stmt, 5);

            html_form += R"(
            <p>)" + std::to_string(id) + R"(</p>
            <p>)" + name_str + R"(</p>
            <p>)" + std::to_string(level) + R"(</p>
            <p>)" + std::to_string(type) + R"(</p>
            <p>)" + std::to_string(rank) + R"(</p>
            <p>)" + std::to_string(rarity) + R"(</p>
            )";
        }

        sqlite3_finalize(stmt);

        html_form += R"(
        </body>
        </html>
        )";

        http::response<http::string_body> response(http::status::ok, request.version());
        response.set(http::field::content_type, "text/html");
        response.body() = html_form;
        response.prepare_payload();
        http::write(socket, response);
    }
    else if (request.target() == "/add_item" && request.method() == http::verb::get) {
        std::string html_form = R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>Add Item</title>
            </head>
            <body>
                <h1>Add New Item</h1>
                <form action="/add_item" method="post">
                    Name: <input type="text" name="name"><br>
                    Level: <input type="number" name="level"><br>
                    Type: <input type="number" name="type"><br>
                    Rank: <input type="number" name="rank"><br>
                    Rarity: <input type="number" name="rarity"><br>
                    <input type="submit" value="Add Item">
                </form>
            </body>
            </html>
        )";

        http::response<http::string_body> response(http::status::ok, request.version());
        response.set(http::field::content_type, "text/html");
        response.body() = html_form;
        response.prepare_payload();
        http::write(socket, response);
    }
    else if (request.target() == "/add_item" && request.method() == http::verb::post) {
        auto body = request.body();
        std::istringstream body_stream(body);
        std::string line;
        std::map<std::string, std::string> form_data;

        while (std::getline(body_stream, line, '&')) {
            auto equal_pos = line.find('=');
            if (equal_pos != std::string::npos) {
                auto key = line.substr(0, equal_pos);
                auto value = line.substr(equal_pos + 1);
                form_data[key] = value;
            }
        }

        item::Item item;
        item.id = -1;
        item.name = form_data["name"];
        item.level = std::stoi(form_data["level"]);
        item.type = static_cast<item::ItemType>(std::stoi(form_data["type"]));
        item.rank = static_cast<item::RankType>(std::stoi(form_data["rank"]));
        item.rarity = static_cast<item::RarityType>(std::stoi(form_data["rarity"]));
           
        db->insertItem(item);
        std::cout << "Item added" << std::endl;

        http::response<http::string_body> response(http::status::ok, request.version());
        response.set(http::field::content_type, "text/plain");
        response.body() = "Item added successfully";
        response.prepare_payload();
        http::write(socket, response);
    }
    else {
        http::response<http::string_body> response(http::status::not_found, request.version());
        response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        response.keep_alive(request.keep_alive());
        response.body() = "Resource not found";
        response.prepare_payload();
        http::write(socket, response);
    }
}

int main() {
    int _port = 12345;

    try {
        Database db;
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), _port));

        std::cout << "Server ON:port: " << _port << std::endl;

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            boost::beast::flat_buffer buffer;
            http::request<http::string_body> request;
            http::read(socket, buffer, request);

            handle_request(request, socket, &db);
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
