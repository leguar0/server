#define _WIN32_WINNT 0x0601
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "sqlite/sqlite3.h"

using std::string;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

void handle_request(http::request<http::string_body>& request, tcp::socket& socket, sqlite3* db) {
    if (request.target() == "/endpoint") {
        http::response<http::string_body> response(http::status::ok, request.version());
        response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        response.set(http::field::content_type, "application/json");
        response.set(http::field::cache_control, "no-cache, no-store, must-revalidate");
        response.keep_alive(request.keep_alive());

        std::string body = "[";

        const char* select_data_sql = "SELECT * FROM Items";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, select_data_sql, -1, &stmt, nullptr);

        bool first = true;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);

            std::string name_str(reinterpret_cast<const char*>(name));

            if (!first) {
                body += ",";
            }
            else {
                first = false;
            }

            body += "{";
            body += "\"id\":" + std::to_string(id) + ",";
            body += "\"name\":\"" + name_str + "\"";
            body += "}";
        }


        body += "]";

        response.body() = body;
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
    sqlite3* db;
    int rc;
    rc = sqlite3_open("database.db", &db);

    if (rc == SQLITE_OK)
        std::cout << "DATABASE: OK" << std::endl;

    const char* create_table_sql = "CREATE TABLE IF NOT EXISTS Items ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "name TEXT NOT NULL);";
    rc = sqlite3_exec(db, create_table_sql, nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK)
    {
        std::cout << "TABLE CREATED" << std::endl;
        const char* insert_data_sql = "INSERT INTO Items (name) VALUES ('TEST')";

        rc = sqlite3_exec(db, insert_data_sql, nullptr, nullptr, nullptr);
        if (rc == SQLITE_OK)
            std::cout << "OK" << std::endl;
    }

    try {
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), _port));

        std::cout << "Server ON:port: " << _port << std::endl;

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            boost::beast::flat_buffer buffer;
            http::request<http::string_body> request;
            http::read(socket, buffer, request);

            handle_request(request, socket, db);
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
