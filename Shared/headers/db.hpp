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

typedef struct _SEARCH_TERM {
	Task::COLUMNS col;
	std::string key;
} SearchTerm;

typedef enum _SEARCH_TYPE {
	S_AND, S_OR
} SearchType;

typedef struct _SEARCH {
	std::vector<SearchTerm> searches;
	SearchType type = S_AND;
} Search;


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
			return 1; // true indicates error
		}
#ifdef LOG_SUCCESSFUL_QUERIES
		else {
			std::cerr << "Query Successful." << std::endl;
		}
#endif

		// Returns the exit status. AKA the status of the last query
		return exRet != SQLITE_OK; // return 0 if all good, 1 if not good;
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
		string insert7 = "INSERT INTO Users (name) VALUES (\"Sebastian\"); ";
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
	// Return a list of all parts in the system
	/// Used when displaying all parts in the home.html dropdown
	std::vector<Part> getParts() {
		// Prepare list of parts
		std::vector<Part> parts;

		stringstream selectQuery;
		selectQuery << "SELECT * FROM Parts LIMIT " << PART_DISPLAY_LIMIT << ";";

		cout << "Running query: " << selectQuery.str() << std::endl;
		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {
			// Local pointer to parts list
			std::vector<Part>* parts = (std::vector<Part>*)data;
			Part foundPart;

			// Parsing for various variables within the part struct
			for(int row = 0; row < argc; row++)
			{
				if (strcmp(colNames[row], "id") == 0) {
					foundPart.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "name") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundPart.name, argv[row], length);
				}
				else if (strcmp(colNames[row], "serialnumber") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundPart.serialNumber, argv[row], length);
				}
			}

			// Part is constructed, push it to the list
			parts->push_back(foundPart);

			return 0;
		}, (void*)&parts);

		// Return the final list
		return parts;
	}
	
	bool checkExists(Task& t)
	{
		string TaskTable = "Tasks";
		stringstream selectQuery;
		selectQuery << "SELECT 1 FROM "<< TaskTable << " WHERE ";
		if (t.id == -1) {
			// Sometimes, the id is unknown (when adding a new record)
			// In this case, records are identified using userid and partid. 
			 // In an event where the task iD is unknown, the user must know the unique combination of userid and partid. This is used when adding a new task, for example
			selectQuery <<"userid == " << to_string(t.user.id) << " AND partid == " << to_string(t.consumedPart.id);
		} else {
			// When we know the task ID, we can simply query for ID, saving performance
			selectQuery << "id == " << std::to_string(t.id);
		}

		bool exists = false;

		if(t.id != 0)
		{
			// The return type of DB::run is not set within the lambda, that's set within the actual run function. The return value of the lambda is the status code.
			// To get data from the lambda, you must pass a pointer to a variable as the last argument, and cast it to a void pointer like so.
			this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) 
				{
					// Read exists from the passed argument
					bool* exists = (bool*)data;

					// If we found results, true, if not, false
					*exists = argc > 0;

					return 0;
				}, (void*)&exists);
		}
		return exists;
	}
	//get task from id
	Task getTask(Task& t)
	{
		Task foundTask;
		string TaskTable = "Tasks";
		string PartTable = "Parts";
		string UserTable = "Users";

		stringstream selectQuery;
		selectQuery << "SELECT 1 FROM "<< TaskTable <<" WHERE id == " << to_string(t.id);
		
		if (!t.consumedPart.id)
			return Part();

		cout << "Running query: " << selectQuery.str() << std::endl;
		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			Task* foundTask = (Task*)data;

			for(int row = 0; row < argc; row++)
			{
				
				if (strcmp(colNames[row], "id") == 0) {
					foundTask->id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "title") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundTask->title, argv[row], length);
				}
				else if (strcmp(colNames[row], "description") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundTask->description, argv[row], length);
				}
				else if (strcmp(colNames[row], "datecreated") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(foundTask->datecreated, argv[row], length);
				}
				else if (strcmp(colNames[row], "partid") == 0) {
					foundTask->consumedPart.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "userid") == 0) {
					foundTask->user.id = stoi(argv[row]);
				}
			}

			return 0;
		}, (void*)&foundTask);

		if(foundTask.id)
		{
			cout << "Retrieved a taskId > 0, ";
			foundTask.consumedPart = this->getPart(foundTask);
			foundTask.user = this->getUser(foundTask.user.id);
			return foundTask;
		}
		
		return Task();
		
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

		//get part names from id
	vector<Part> getAllParts()
	{
		vector<Part> allParts;
		string PartTable = "Parts";

		stringstream selectQuery;
		selectQuery << "SELECT * FROM "<< PartTable;

		cout << "Running query: " << selectQuery.str() << std::endl;
		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {
		
			vector<Part>* allParts = (vector<Part>*)data;

			for(int row = 0; row < argc; row++)
			{
				
				Part entry;

				if (strcmp(colNames[row], "id") == 0) {
					entry.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "name") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(entry.name, argv[row], length);
				}
				else if (strcmp(colNames[row], "serialnumber") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(entry.serialNumber, argv[row], length);
				}
				allParts->push_back(entry);
			}

			return 0;
		}, (void*)&allParts);

		return allParts;
		
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

	vector<User> getAllUsers(int id)
	{
		vector<User> foundUsers;
		string UsersTable = "Users";
		stringstream selectQuery;

		selectQuery << "SELECT * FROM "<< UsersTable;

		this->run(selectQuery.str(), [](void* data, int argc, char** argv, char** colNames) {

			vector<User>* foundUsers = (vector<User>*)data;

			for(int row = 0; row < argc; row++)
			{
				User entry;
				if (strcmp(colNames[row], "id") == 0) {
					entry.id = stoi(argv[row]);
				}
				else if (strcmp(colNames[row], "name") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(entry.name, argv[row], length);
				}
				foundUsers->push_back(entry);
			}
			return 0;
		}, (void*)&foundUsers);
		return foundUsers;
	}

	//get tasks by filter
	vector<Task> getFilteredTasks(string& key)
	{
		string TaskTable = "Tasks";
		stringstream selectQuery;
		string colName;

		vector<Task> tasks;
		cout << "searching for key: " << key << ", ";
		//SELECT ALL DISTINCT 
		selectQuery << "SELECT Tasks.id,title,description,datecreated,partid,userid,Users.name as username,Parts.name as partname, Parts.serialnumber as serialnumber FROM Tasks INNER JOIN Users ON Users.id = Tasks.id INNER JOIN Parts ON Parts.id == Tasks.id WHERE title LIKE '%" << key << "%' OR description LIKE '%" << key << "%'";
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
				else if (strcmp(colNames[row], "username") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(fromDbQuery.user.name, argv[row], length);
				}
				else if (strcmp(colNames[row], "partname") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(fromDbQuery.consumedPart.name, argv[row], length);
				}
				else if (strcmp(colNames[row], "serialnumber") == 0) {
					int length = strlen(argv[row]) + 1;
					strncpy(fromDbQuery.consumedPart.serialNumber, argv[row], length);
				}
			}
			tasks->push_back(fromDbQuery);
			return 0;
		}, (void*)&tasks);

		cout << "Found " << tasks.size() << " matches, ";
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
			deleteQuery << "DELETE FROM " << TaskTable << " WHERE id=\"" << id << "\"";
			cout << "Running query: " << deleteQuery.str() << std::endl;
			if(this->run(deleteQuery.str()))
				return true;
		}
		return false;
	}
};

#endif

