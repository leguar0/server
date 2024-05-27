#include "Database.h"

void Database::createTables()
{
	const char* create_table_items_sql = "CREATE TABLE IF NOT EXISTS Items ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"name TEXT NOT NULL,"
		"level INTEGER NOT NULL,"
		"item_type TEXT NOT NULL,"
		"rank_type TEXT NOT NULL,"
		"rarity_type TEXT NOT NULL);";
	int rc = sqlite3_exec(db, create_table_items_sql, nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Error creating table: " << sqlite3_errmsg(db) << std::endl;
		throw std::runtime_error("Table creation error");
	}
	else 
	{ 
		std::cout << "Table created: Items" << std::endl;
	}
}

Database::Database()
{
	int rc = sqlite3_open(DATABASE_NAME, &db);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		throw std::runtime_error("Database open error");
	}
	
	std::cout << "Database active" << std::endl;

	createTables();
}

Database::~Database() 
{
	sqlite3_close(db);
}

sqlite3* Database::getHandle() const
{
	return db;
}

void Database::insertItem(item::Item item)
{
	const char* insert_data_item_sql = "INSERT INTO Items (name, level, item_type, rank_type, rarity_type) VALUES (?, ?, ?, ?, ?)";
	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, insert_data_item_sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
		throw std::runtime_error("Statement preparation error");
	}

	sqlite3_bind_text(stmt, 1, item.name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, item.level);
	sqlite3_bind_int(stmt, 3, item.type);
	sqlite3_bind_int(stmt, 4, item.rank);
	sqlite3_bind_int(stmt, 5, item.rarity);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		std::cerr << "Error inserting item: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_finalize(stmt);
		throw std::runtime_error("Item insertion error");
	}

	sqlite3_finalize(stmt);
	std::cout << "Item inserted successfully" << std::endl;
}

