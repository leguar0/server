#ifndef DATABASE_H
#define DATABASE_H
#define DATABASE_NAME "database.db"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "sqlite/sqlite3.h"
#include "Item.cpp"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class Database
{
private:
	sqlite3* db;

	void createTables();
public:
	Database();
	~Database();

	sqlite3* getHandle() const;
	void insertItem(item::Item item);
};

#endif 