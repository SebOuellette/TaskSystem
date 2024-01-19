#pragma once

#ifndef DB_HPP
#define DB_HPP

#include <vector>
#include "defines.hpp"
#include "task.h"
#include "part.h"
#include <sqlite3.h>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
// Using sqlite https://www.geeksforgeeks.org/sql-using-c-c-and-sqlite/

// debug
#define LOG_SUCCESSFUL_QUERIES

class TaskDb {
private:
	sqlite3* _db;

public:
	TaskDb() {
		this->init();
	}

	~TaskDb() {
		sqlite3_close(_db);
	}

	// General purpose query. You can use a labmda function as the second argument
	// The lambda function parameters are (void* data, int argc, char** argv, char** columnNames)
	//			The lambda function is what this \/ \/ \/ \/ \/ mess is about
	bool run(std::string queryString, int(*callback)(void*, int, char**, char**) = NULL, void* data = NULL) {
		char* errorMsg;

		// Perform the query
		int exRet = sqlite3_exec(this->_db, queryString.c_str(), callback, data, &errorMsg);

		// Check if an error was reported
		if (exRet != SQLITE_OK) {
			std::cerr << "Error performing query: " << errorMsg << std::endl;
			sqlite3_free(errorMsg);
		}
#ifdef LOG_SUCCESSFUL_QUERIES
		else {
			std::cerr << "Query Successful." << std::endl;
		}
#endif

		// Returns the exit status. AKA the status of the last query
		return (exRet == SQLITE_OK);
	};

	// Initialize the database table
	bool init() {
		std::string createTables[TABLES] = {
			"CREATE TABLE IF NOT EXISTS Tasks (id INTEGER NOT NULL UNIQUE, taskid int NOT NULL UNIQUE, title varchar(128) NOT NULL, description varchar(4096) NULL, partid int NULL, PRIMARY KEY(id AUTOINCREMENT))",
			"CREATE TABLE IF NOT EXISTS Parts (id INTEGER NOT NULL UNIQUE, partid int NOT NULL UNIQUE, name varchar(25) NOT NULL, serialnumber varchar(50) NOT NULL, PRIMARY KEY(id AUTOINCREMENT))",
		};

		// Open Database
		int exit = sqlite3_open(DB_PATH, &(this->_db));

		// Ensure the database opens correctly
		if (exit != SQLITE_OK) {
			std::cerr << "Database failed to open." << std::endl;

			throw 1;
		}

		// Now attempt to run the defined queries
		for (int i = 0; i < TABLES; i++) {
			// Just run each query without a callback, there's no need here
			this->run(createTables[i]);
		}

		// Returns the final exit status. AKA the status of the last query
		return (exit == SQLITE_OK);
	}
	//upoload new products to the cart
	bool insertTask(Task& t) {

		std::string TaskTable = "Tasks";

		// Check if task already exist in system, if not, add them
		std::stringstream checkQuery;
		std::stringstream insertQuery;

		checkQuery << "SELECT EXISTS (SELECT 1 FROM "<< TaskTable <<" WHERE taskid == " << std::to_string(t.id) << ");";
		if(this->run(checkQuery.str()))
			return false;

		insertQuery << "INSERT INTO " << TaskTable << " (title,description,partid) VALUES(\"" << t.title << "\"," << "\"" << t.description << "\"," << std::to_string(t.consumedPart.id) << ");";

		return this->run(insertQuery.str(), NULL);
	}
	//get part names from id
	Part& getPart(Task& t)
	{
		Part foundPart;
		std::string PartTable = "Parts";

		std::stringstream selectQuery;
		selectQuery << "SELECT 1 FROM "<< PartTable <<" WHERE partid == \"" << std::to_string(t.consumedPart.id) << "\"";

		if (!this->run(selectQuery.str()))
			return foundPart;

		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			Part foundPart = *(Part*)data;

			return 1;
		}, (void*)&foundPart);

		return foundPart;
		
	}
	//get tasks by filter
	std::vector<Task> getFilteredTasks(std::string key, Task::COLUMNS column)
	{
		std::string TaskTable = "Tasks";
		std::stringstream selectQuery;
		std::string colName;

		switch(column)
		{
		case 0:
			colName = "id";
			break;
		case 1:
			colName = "title";
			break;
		case 2:
			colName = "description";
			break;
		case 3:
			colName = "partid";
			break;
		default: 
			colName = "title";
		}

		std::vector<Task> tasks;

		selectQuery << "SELECT * FROM " << TaskTable << " WHERE " << colName << " LIKE " << "%\"" << key << "%\"";
		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			std::vector<Task>* tasks = (std::vector<Task>*)data;

			for(int row = 0; row < argc; row++)
			{
				Task fromDbQuery;
				
				if (strcmp(colNames[row], "id") == 0) {
					fromDbQuery.id = std::stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "title") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy_s(fromDbQuery.title, argv[row], length);
				}
				else if (strcmp(colNames[row], "description") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy_s(fromDbQuery.description, argv[row], length);
				}
				else if (strcmp(colNames[row], "partid") == 0) {
					fromDbQuery.consumedPart.id = std::stoi(argv[row]);
				}

				tasks->push_back(fromDbQuery);
			}
			return 1;
		}, (void*)&tasks);

		for(Task & task : tasks)
		{
			task.consumedPart = this->getPart(task);
		}

		return tasks;
	}
	//Update task
	bool updateTask(Task& withNewDetails)
	{
		std::string TaskTable = "Tasks";
		std::stringstream updateQuery;

		updateQuery << "UPDATE " << TaskTable << " SET title = \"" << std::string(withNewDetails.title) << "\", description = \"" << std::string(withNewDetails.description) << "\", partid = " << std::to_string(withNewDetails.consumedPart.id) << " WHERE taskid == " << std::to_string(withNewDetails.id);
		if (this->run(updateQuery.str()))
			return true;
		else
			return false;
	}
};

#endif

