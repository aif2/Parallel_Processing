
#pragma once

#include <memory>

#include "cs477.h"

#include "../sqlite/sqlite3.h"
#pragma comment(lib, "sqlite.lib")


namespace cs477
{

	namespace data
	{


		class database;
		class statement;
		class table;

		std::shared_ptr<database> open(const char *path);

		class database
		{
		public:
			database();
			~database();

			database(database &&db);
			database &operator =(database &&db);

			database(const database &) = delete;
			database &operator =(const database &) = delete;

		public:
			statement new_statement(const char *sql);

			void execute(const char *sql);
			std::string execute_query(const char *sql);

		private:
			sqlite3 *db;

			friend std::shared_ptr<database> open(const char *);
		};


		class statement
		{
		public:
			statement();
			~statement();

			statement(statement &&stmt);
			statement &operator =(statement &&stmt);

			statement(const statement &) = delete;
			statement &operator =(const statement &) = delete;

		public:
			void bind(int index, const char *value);
			void bind(int index, const std::string &value);

			void execute();
			std::string execute_query();

		private:
			sqlite3_stmt *stmt;

			friend class database;
		};


	}

}


#include "data.inl"
