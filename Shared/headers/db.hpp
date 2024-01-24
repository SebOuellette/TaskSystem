#pragma once

#ifndef DB_HPP
#define DB_HPP

#include "user.h"
#include <string.h>
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
#include <cstring>
// Using sqlite https://www.geeksforgeeks.org/sql-using-c-c-and-sqlite/

// debug
#define LOG_SUCCESSFUL_QUERIES

using namespace std;

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
			std::cerr << "Error performing query: " << errorMsg << ". Retcode: " << to_string(exRet) << std::endl;
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
			"CREATE TABLE IF NOT EXISTS Tasks (id INTEGER NOT NULL UNIQUE, title varchar(128) NOT NULL, description varchar(4096) NULL, datecreated datetime NOT NULL, partid int NULL, userid int NOT NULL, PRIMARY KEY(id AUTOINCREMENT))",
			"CREATE TABLE IF NOT EXISTS Parts (id INTEGER NOT NULL UNIQUE, name varchar(25) NOT NULL, serialnumber varchar(50) NOT NULL, PRIMARY KEY(id AUTOINCREMENT))",
			"CREATE TABLE IF NOT EXISTS Users (id INTEGER NOT NULL UNIQUE, name varchar(50) NOT NULL, PRIMARY KEY(id AUTOINCREMENT))"
		};

		// Open Database
		int exit = sqlite3_open(DB_PATH, &(this->_db));

		// Ensure the database opens correctly
		if (exit != SQLITE_OK) {
			std::cerr << "Database failed to open. Retcode: " << exit << std::endl;

			throw 1;
		}

		// Now attempt to run the defined queries
		for (int i = 0; i < TABLES; i++) {
			// Just run each query without a callback, there's no need here
			this->run(createTables[i]);
		}

		seedData();

		// Returns the final exit status. AKA the status of the last query
		return (exit == SQLITE_OK);
	}

	void seedData()
	{
		stringstream seed;
		string insert1 = "INSERT INTO Parts (name, serialnumber) VALUES (\"relay\", \"jl8d8890\"); ";
		string insert2 = "INSERT INTO Parts (name, serialnumber) VALUES (\"seal\", \"jzj000500\"); ";
		string insert3 = "INSERT INTO Parts (name, serialnumber) VALUES (\"pump\", \"KY-34jjk\"); ";
		string insert4 = "INSERT INTO Parts (name, serialnumber) VALUES (\"scanner\", \"SR-1500\"); ";
		string insert5 = "INSERT INTO Parts (name, serialnumber) VALUES (\"sensor\", \"BF-df78ss\"); ";
	
		string insert6 = "INSERT INTO Users (name) VALUES (\"Zebadiah\"); ";
		string insert7 = "INSERT INTO Users (name) VALUES (\"Sebastion\"); ";
		string insert8 = "INSERT INTO Users (name) VALUES (\"Tom\"); ";
		string insert9 = "INSERT INTO Users (name) VALUES (\"Kiana\"); ";

		string insert10 = "INSERT INTO Tasks (title, description, datecreated, partid, userid) VALUES (\"seed task\", \"seed description\", \"2024-01-18\", 1, 1); ";

		seed << insert1 << insert2 << insert3 << insert4 << insert5 << insert6 << insert7 << insert8 << insert9 << insert10;
		this->run(seed.str());
	}
	//upoload new products to the cart
	bool insertTask(Task& t) {

		std::string TaskTable = "Tasks";
		std::stringstream insertQuery;

		insertQuery << "INSERT INTO " << TaskTable << " (title, description, datecreated, partid, userid) VALUES(\"" << t.title << "\"," << "\"" << t.description << "\", \"" << t.datecreated <<"\"," << std::to_string(t.consumedPart.id) << ", " << std::to_string(t.user.id) << ");";
		std::cout << "Running insert query: " << insertQuery.str() << std::endl;
		return this->run(insertQuery.str(), NULL);
	}
	bool checkExists(Task& t)
	{
		string TaskTable = "Tasks";
		Task foundTask;
		stringstream selectQuery;
		selectQuery << "SELECT 1 FROM "<< TaskTable <<" WHERE id == " << to_string(t.id);
		bool exists = false;

		if(t.id != 0)
		{
			exists = this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) 
								{
									Task* foundTask = (Task*)data;
									if(!foundTask.id)
										return 1;

									return 0;
								});
		}
		return exists;
	}
	//get part names from id
	Task getTask(Task& t)
	{
		Task foundTask;
		string TaskTable = "Tasks";

		stringstream selectQuery;
		selectQuery << "SELECT 1 FROM "<< TaskTable <<" WHERE id == " << to_string(t.id);
		
		if (!t.consumedPart.id)
			return Part();

		cout << "Running query: " << selectQuery.str() << std::endl;
		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			Task foundTask;

			for(int row = 0; row < argc; row++)
			{
				
				if (strcmp(colNames[row], "id") == 0) {
					foundTask.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "title") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundTask.title, argv[row], length);
				}
				else if (strcmp(colNames[row], "description") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundTask.description, argv[row], length);
				}
				else if (strcmp(colNames[row], "datecreated") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundTask.datecreated, argv[row], length);
				}
				else if (strcmp(colNames[row], "partid") == 0) {
					foundTask.consumedPart.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "userid") == 0) {
					foundTask.user.id = stoi(argv[row]);
				}
			}

			return 0;
		}, (void*)&foundTask);

		task.consumedPart = this->getPart(task);
		task.user = this->getUser(task.user.id);
		return foundTask;
		
	}
	//get part names from id
	Part getPart(Task& t)
	{
		Part foundPart;
		string PartTable = "Parts";

		stringstream selectQuery;
		selectQuery << "SELECT 1 FROM "<< PartTable <<" WHERE id == " << to_string(t.consumedPart.id);
		
		if (!t.consumedPart.id)
			return Part();

		cout << "Running query: " << selectQuery.str() << std::endl;
		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			for(int row = 0; row < argc; row++)
			{
				Part* foundPart = (Part*)data;
				
				if (strcmp(colNames[row], "id") == 0) {
					foundPart->id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "name") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundPart->name, argv[row], length);
				}
				else if (strcmp(colNames[row], "serialnumber") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundPart->serialNumber, argv[row], length);
				}
			}

			return 0;
		}, (void*)&foundPart);

		return foundPart;
		
	}

	User getUser(int id)
	{
		User foundUser;
		string UsersTable = "Users";
		stringstream selectQuery;

		selectQuery << "SELECT 1 FROM "<< UsersTable <<" WHERE id == \"" << to_string(id) << "\"";

		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			User* foundUser = (User*)data;

			for(int row = 0; row < argc; row++)
			{
				if (strcmp(colNames[row], "id") == 0) {
					foundUser->id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "name") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundUser->name, argv[row], length);
				}
			}
			return 0;
		}, (void*)&foundUser);
		return foundUser;
	}

	//get tasks by filter
	vector<Task> getFilteredTasks(string key, Task::COLUMNS column)
	{
		string TaskTable = "Tasks";
		stringstream selectQuery;
		string colName;

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

		vector<Task> tasks;

		selectQuery << "SELECT * FROM " << TaskTable << " WHERE " << colName << " LIKE " << "'%" << key << "%'";
		cout << "Running filter query: " << selectQuery.str() << std::endl;
		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			vector<Task>* tasks = (vector<Task>*)data;
			cout << "Entered getFilteredTasks lambda, ";
			Task fromDbQuery;

			for(int row = 0; row < argc; row++)
			{
				
				if (strcmp(colNames[row], "id") == 0) {
					fromDbQuery.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "title") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(fromDbQuery.title, argv[row], length);
				}
				else if (strcmp(colNames[row], "description") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(fromDbQuery.description, argv[row], length);
				}
				else if (strcmp(colNames[row], "datecreated") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(fromDbQuery.datecreated, argv[row], length);
				}
				else if (strcmp(colNames[row], "partid") == 0) {
					fromDbQuery.consumedPart.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "userid") == 0) {
					fromDbQuery.user.id = stoi(argv[row]);
				}
			}
			tasks->push_back(fromDbQuery);
			return 0;
		}, (void*)&tasks);
		cout << "Found " << tasks.size() << " matches, ";
		cout << "Getting part data, ";
		for(Task & task : tasks)
		{
			task.consumedPart = this->getPart(task);
			task.user = this->getUser(task.user.id);
		}
		return tasks;
	}
	//Update task
	bool updateTask(Task& withNewDetails)
	{
		string TaskTable = "Tasks";
		stringstream updateQuery;

		updateQuery << "UPDATE " << TaskTable << " SET title = \"" << string(withNewDetails.title) << "\", description = \"" << string(withNewDetails.description) << "\", partid = " << to_string(withNewDetails.consumedPart.id) << ", userid = " << withNewDetails.user.id << " WHERE id == " << to_string(withNewDetails.id);
		cout << "running update query";
		cout << updateQuery.str();
		if(this->run(updateQuery.str()))
			return true;
		else
			return false;
	}
	//Delete a task
	bool deleteTask(string id)
	{
		stringstream deleteQuery;
		string TaskTable = "Tasks";

		if(!id.empty())
		{
			deleteQuery << "DELETE FROM " << TaskTable << " WHERE id == " << id;
			cout << "Running query: " << deleteQuery.str() << std::endl;
			if(this->run(deleteQuery.str()))
				return true;
		}
		return false;
	}
};

#endif

