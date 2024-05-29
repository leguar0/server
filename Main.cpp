#define _WIN32_WINNT 0x0601
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "Database.h"
#include <cctype>

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
    case item::HELMET: return "HELMET";
    case item::NECKLACE: return "NECKLACE";
    case item::GLOVES: return "GLOVES";
    case item::RING: return "RING";
    case item::BRACERS: return "BRACERS";
    case item::SHIELD: return "SHIELD";
    case item::BOOTS: return "BOOTS";
    case item::PANTS: return "PANTS";
    case item::BELT: return "BELT";
    case item::ARMOUR: return "ARMOUR";
    case item::CAPE: return "CAPE";
    case item::STAFF: return "STAFF";
    default: return "null";
    }
}

string getStatType(item::StatType stat)
{
    switch (stat)
    {
    case item::STRENGTH: return "STRENGTH";
    case item::DEXTERITY: return "DEXTERITY";
    case item::POWER: return "POWER";
    case item::KNOWLEDGE: return "KNOWLEDGE";
    case item::HEALTH: return "HEALTH";
    case item::MANA: return "MANA";
    case item::STAMINA: return "STAMINA";
    case item::SLASHING: return "SLASHING";
    case item::CRUSHING: return "CRUSHING";
    case item::PIERCING: return "PIERCING";
    case item::COLD: return "COLD";
    case item::FIRE: return "FIRE";
    case item::ENERGY: return "ENERGY";
    case item::MENTAL: return "MENTAL";
    default:
        break;
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
    if (request.target() == "/requirements") {
        http::response<http::string_body> response(http::status::ok, request.version());
        response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        response.set(http::field::content_type, "application/json");
        response.set(http::field::cache_control, "no-cache, no-store, must-revalidate");
        response.keep_alive(request.keep_alive());

        std::string body = "[";

        const char* select_data_sql = "SELECT * FROM requirements";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db->getHandle(), select_data_sql, -1, &stmt, nullptr);

        bool first = true;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int req_strength = sqlite3_column_int(stmt, 1);
            int req_dexterity = sqlite3_column_int(stmt, 2);
            int req_power = sqlite3_column_int(stmt, 3);
            int req_knowledge = sqlite3_column_int(stmt, 4);

            if (!first) {
                body += ",";
            }
            else {
                first = false;
            }

            body += "{";
            body += "\"id\":" + std::to_string(id) + ",";
            body += "\"req_strength\":" + std::to_string(req_strength) + ",";
            body += "\"req_dexterity\":\"" + std::to_string(req_dexterity) + "\",";
            body += "\"req_power\":\"" + std::to_string(req_power) + "\",";
            body += "\"req_knowledge\":\"" + std::to_string(req_knowledge) + "\"";
            body += "}";
        }


        body += "]";

        response.body() = body;
        response.prepare_payload();

        http::write(socket, response);
    }
    else if (request.target() == "/item" && request.method() == http::verb::get) {
        const char* select_data_sql = "SELECT * FROM items";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db->getHandle(), select_data_sql, -1, &stmt, nullptr);

        std::string html_form = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Items</title>
            <style>
                body {
                    background-color: black;
                    line-height: 0.9;
                    font-size: 14px;
                }
                p {
                    color: #bab0a3;
                }
                .ancient {
                    font-size: 16px;
                    color: #dca779;
                    font-weight: bold;
                }
                .rare {
                    font-size: 16px;
                    color: #42c8f5;
                    font-weight: bold;
                }
                .legendary {
                    font-size: 16px;
                    color: #fcba03;
                    font-weight: bold;
                }
            </style>
        </head>
        <body>
        )";

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            item::Item item;
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            if (name) {
                item.name = reinterpret_cast<const char*>(name);
            }

            item.level = sqlite3_column_int(stmt, 2);
            item.type = static_cast<item::ItemType>(sqlite3_column_int(stmt, 3));
            item.rank = static_cast<item::RankType>(sqlite3_column_int(stmt, 4));
            item.rarity = static_cast<item::RarityType>(sqlite3_column_int(stmt, 5));

            map<item::StatType, int> stats;
            stats.emplace(item::STRENGTH,sqlite3_column_int(stmt, 6));
            stats.emplace(item::DEXTERITY,sqlite3_column_int(stmt, 7));
            stats.emplace(item::POWER,sqlite3_column_int(stmt, 8));
            stats.emplace(item::KNOWLEDGE,sqlite3_column_int(stmt, 9));
            stats.emplace(item::HEALTH,sqlite3_column_int(stmt, 10));
            stats.emplace(item::MANA,sqlite3_column_int(stmt, 11));
            stats.emplace(item::STAMINA,sqlite3_column_int(stmt, 12));
            stats.emplace(item::SLASHING,sqlite3_column_int(stmt, 13));
            stats.emplace(item::CRUSHING,sqlite3_column_int(stmt, 14));
            stats.emplace(item::PIERCING,sqlite3_column_int(stmt, 15));
            stats.emplace(item::FIRE,sqlite3_column_int(stmt, 16));
            stats.emplace(item::COLD,sqlite3_column_int(stmt, 17));
            stats.emplace(item::ENERGY,sqlite3_column_int(stmt, 18));
            stats.emplace(item::MENTAL,sqlite3_column_int(stmt, 19));
            item.stats = stats;

            item::RarityType st = static_cast<item::RarityType>(item.rarity);
            string str = getRarityType(st);
            
            for (int i = 0; i < str.length(); i++)
            {
                str[i] = std::tolower(str[i]);
            }

            html_form += R"(
            <p><span class=')" + str+ R"('>)" + item.name + R"(</span> [)" + getRankType(item.rank) + R"(]</p>
            <p> Item level: )" + std::to_string(item.level) + R"(</p>
            <p> Item type: )" + getItemType(static_cast<item::ItemType>(item.type)) + R"(</p>
            )";


            for (const auto& stat : item.stats)
            {
                if (stat.second > 0)
                {
                    html_form += R"(<p>)" + getStatType(stat.first) + R"(: )" + std::to_string(item.stats[stat.first]) + R"(</p>)";
                }
            }
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
                    <h4>Requirements</h4>
                    strength: <input type="number" name="req_strength"><br>
                    dexterity: <input type="number" name="req_dexterity"><br>
                    power: <input type="number" name="req_power"><br>
                    knowledge: <input type="number" name="req_knowledge"><br>
                    <h4>Stats</h4>
                    strength: <input type="number" name="strength"><br>
                    dexterity: <input type="number" name="dexterity"><br>
                    power: <input type="number" name="power"><br>
                    knowledge: <input type="number" name="knowledge"><br>
                    health: <input type="number" name="health"><br>
                    mana: <input type="number" name="mana"><br>
                    stamina: <input type="number" name="stamina"><br>
                    slashing: <input type="number" name="slashing"><br>
                    crushing: <input type="number" name="crushing"><br>
                    piercing: <input type="number" name="piercing"><br>
                    fire: <input type="number" name="fire"><br>
                    cold: <input type="number" name="cold"><br>
                    energy: <input type="number" name="energy"><br>
                    mental: <input type="number" name="mental"><br>
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
           
        map<item::StatType, int> req;
        req.emplace(item::StatType::STRENGTH, std::stoi(form_data["req_strength"]));
        req.emplace(item::StatType::DEXTERITY, std::stoi(form_data["req_dexterity"]));
        req.emplace(item::StatType::POWER, std::stoi(form_data["req_power"]));
        req.emplace(item::StatType::KNOWLEDGE, std::stoi(form_data["req_knowledge"]));

        map<item::StatType, int> stats;
        stats.emplace(item::StatType::STRENGTH, std::stoi(form_data["strength"]));
        stats.emplace(item::StatType::DEXTERITY, std::stoi(form_data["dexterity"]));
        stats.emplace(item::StatType::POWER, std::stoi(form_data["power"]));
        stats.emplace(item::StatType::KNOWLEDGE, std::stoi(form_data["knowledge"]));
        stats.emplace(item::StatType::HEALTH, std::stoi(form_data["health"]));
        stats.emplace(item::StatType::MANA, std::stoi(form_data["mana"]));
        stats.emplace(item::StatType::STAMINA, std::stoi(form_data["stamina"]));
        stats.emplace(item::StatType::SLASHING, std::stoi(form_data["slashing"]));
        stats.emplace(item::StatType::CRUSHING, std::stoi(form_data["crushing"]));
        stats.emplace(item::StatType::PIERCING, std::stoi(form_data["piercing"]));
        stats.emplace(item::StatType::FIRE, std::stoi(form_data["fire"]));
        stats.emplace(item::StatType::COLD, std::stoi(form_data["cold"]));
        stats.emplace(item::StatType::ENERGY, std::stoi(form_data["energy"]));
        stats.emplace(item::StatType::MENTAL, std::stoi(form_data["mental"]));
        
        item.stats = stats;

        int id = db->insertRequirements(req);
        db->insertItem(item, id);
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
