#include "Database.h"
#include <cstdint>

void Database::createTables()
{
	const char* create_table_requirements_sql = "CREATE TABLE IF NOT EXISTS requirements("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"req_strength INTEGER NOT NULL,"
		"req_dexterity INTEGER NOT NULL,"
		"req_power INTEGER NOT NULL,"
		"req_knowledge INTEGER NOT NULL);";

	int rc = sqlite3_exec(db, create_table_requirements_sql, nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Error creating table: " << sqlite3_errmsg(db) << std::endl;
		throw std::runtime_error("Table creation error");
	}
	else
	{
		std::cout << "Table created: requirements" << std::endl;
	}

	const char* create_table_items_sql = "CREATE TABLE IF NOT EXISTS items ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"name TEXT NOT NULL,"
		"level INTEGER NOT NULL,"
		"item_type TEXT NOT NULL,"
		"rank_type TEXT NOT NULL,"
		"rarity_type TEXT NOT NULL,"
		"strength INTEGER NOT NULL,"
		"dexterity INTEGER NOT NULL,"
		"power INTEGER NOT NULL,"
		"knowledge INTEGER NOT NULL,"
		"health INTEGER NOT NULL,"
		"mana INTEGER NOT NULL,"
		"stamina INTEGER NOT NULL,"
		"slashing INTEGER NOT NULL,"
		"crushing INTEGER NOT NULL,"
		"piercing INTEGER NOT NULL,"
		"fire INTEGER NOT NULL," 
		"cold INTEGER NOT NULL," 
		"energy INTEGER NOT NULL,"
		"mental INTEGER NOT NULL,"
		"id_req INTEGER NOT NULL,"
		"FOREIGN KEY (id_req) REFERENCES requirements(id));";
	rc = sqlite3_exec(db, create_table_items_sql, nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Error creating table: " << sqlite3_errmsg(db) << std::endl;
		throw std::runtime_error("Table creation error");
	}
	else 
	{ 
		std::cout << "Table created: items" << std::endl;
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

void Database::insertItem(item::Item item, int req)
{
	const char* insert_data_item_sql = "INSERT INTO items (name, level, item_type, rank_type, rarity_type, strength, dexterity, power, knowledge, health, mana, stamina, slashing, crushing, piercing, fire, cold, energy, mental, id_req) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
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

	int i = 6;
	for (const auto& stat : item.stats)
	{
		sqlite3_bind_int(stmt, i, stat.second);
		i++;
	}

	sqlite3_bind_int(stmt, i, req);

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

int Database::insertRequirements(map<item::StatType, int> stats)
{
	const char* insert_data_item_sql = "INSERT INTO requirements (req_strength, req_dexterity, req_power, req_knowledge) VALUES (?, ?, ?, ?)";
	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, insert_data_item_sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK)
	{
		std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
		throw std::runtime_error("Statement preparation error");
	}

	int i = 1;
	for (const auto& stat : stats)
	{
		sqlite3_bind_int(stmt, i, stat.second);
		i++;
	}

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		std::cerr << "Error inserting requirements: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_finalize(stmt);
		throw std::runtime_error("Requirements insertion error");
	}

	int _lastId = static_cast<int>(sqlite3_last_insert_rowid(db));
	sqlite3_finalize(stmt);
	std::cout << "Requirements inserted successfully" << std::endl;

	return _lastId;
}

