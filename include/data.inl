
namespace cs477
{

	namespace data
	{

		inline std::shared_ptr<database> open(const char *path)
		{
			auto db = std::make_shared<database>();
			int err = sqlite3_open(path, &db->db);
			if (err)
			{
				throw std::exception();
			}
			return db;
		}


		inline database::database()
			: db(nullptr)
		{
		}

		inline database::~database()
		{
			if (db)
			{
				sqlite3_close(db);
			}
		}

		inline database::database(database &&db)
			: db(db.db)
		{
			db.db = nullptr;
		}

		inline database &database::operator =(database &&db)
		{
			std::swap(this->db, db.db);
			return *this;
		}

		inline statement database::new_statement(const char *sql)
		{
			statement stmt;
			auto err = sqlite3_prepare(db, sql, -1, &stmt.stmt, nullptr);
			if (err)
			{
				throw std::exception();
			}
			return stmt;
		}

		inline void database::execute(const char *sql)
		{
			auto stmt = new_statement(sql);
			stmt.execute();
		}

		inline std::string database::execute_query(const char *sql)
		{
			auto stmt = new_statement(sql);
			return stmt.execute_query();
		}



		inline statement::statement()
		{
		}

		inline statement::~statement()
		{
			if (stmt)
			{
				sqlite3_finalize(stmt);
			}
		}

		inline statement::statement(statement &&stmt)
			: stmt(stmt.stmt)
		{
			stmt.stmt = nullptr;
		}

		inline statement &statement::operator =(statement &&stmt)
		{
			std::swap(this->stmt, stmt.stmt);
			return *this;
		}

		inline void statement::bind(int index, const char *value)
		{
			auto err = sqlite3_bind_text(stmt, index, value, -1, nullptr);
			if (err)
			{
				throw std::exception();
			}
		}

		inline void statement::bind(int index, const std::string &value)
		{
			bind(index, value.c_str());
		}

		inline void statement::execute()
		{
			int tries = 1;
			int state = 0;
			do
			{
				state = sqlite3_step(stmt);
				if (state == SQLITE_BUSY)
				{
					if (tries < 11)
					{
						Sleep(1 << tries);
						tries++;
					}
				}

				if (state == SQLITE_ROW)
				{
					// TODO
				}

			} while (state == SQLITE_ROW);

			if (state == SQLITE_DONE)
			{
				return;
			}

			throw std::exception();
		}

		inline std::string statement::execute_query()
		{
			std::string json = "{\n";

			int tries = 1;
			int state = 0;

			int rows = 0;
			std::vector<std::string> cols;

			do
			{
				state = sqlite3_step(stmt);
				if (state == SQLITE_BUSY)
				{
					if (tries < 11)
					{
						Sleep(1 << tries);
						tries++;
					}
				}

				if (state == SQLITE_ROW)
				{
					auto count = sqlite3_column_count(stmt);

					if (!rows)
					{
						cols.resize(count);
						for (int i = 0; i < count; i++)
						{
							auto str = sqlite3_column_name(stmt, i);
							if (str)
							{
								cols[i] = str;
							}
						}
					}

					if (rows > 0)
					{
						json.append(",\n");
					}

					json.append("[ ");
					for (int i = 0; i < count; i++)
					{
						auto str = sqlite3_column_text(stmt, i);
						if (str)
						{
							if (i > 0) json.append(", ");
							json.append("'");
							json.append(cols[i]);
							json.append("': '");
							json.append((const char *)str);
							json.append("'");
						}
					}
					json.append(" ]");
					rows++;
				}

			} while (state == SQLITE_ROW);

			if (state == SQLITE_DONE)
			{
				json.append("\n}");
				return json;
			}

			throw std::exception();
		}


	
	}

}


