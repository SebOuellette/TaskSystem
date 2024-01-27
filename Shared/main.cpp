#define CROW_MAIN

#include "headers/db.hpp"
#include "headers/crow_all.h"

#include <regex>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>


// Function Definitions
std::string loadFile(crow::response& res, std::string _folder, std::string _name);
std::string replaceTemplates(std::string htmlString, const char templateStr[], std::string replacement);
bool isAuthorized(ID userID, const crow::request& req);
Task buildTaskFromJson(string reqBody);
crow::json::wvalue buildJsonFromTask(Task& task);
crow::json::wvalue buildJsonFromPart(Part& part);

// Main Function
int main()
{
	srand(time(NULL));

	crow::App<crow::CORSHandler> app;
	// Create and initialize the database
	TaskDb db;
	

	CROW_ROUTE(app, "/") // Index page
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::GET)
        ([&db](const crow::request& req, crow::response& res){
			// Redirect to the cart page
            res.code = 200;

			std::string home = loadFile(res, "", "home.html");

			// Get a current list of parts at the time of request
			std::vector<Part> parts = db.getParts();
			
			// // Add all parts to the part dropdown
			for (int i=0;i<parts.size();i++) {
				std::stringstream result;
				// Build new option, append template at the end to allow for another loop
				result << "<option value=\"" << parts[i].id << "\">" << parts[i].name << " | " << parts[i].serialNumber << "</option>" << PART_TEMPLATE;
				// replace the template with our new html element
				home = replaceTemplates(home, PART_TEMPLATE, result.str());
			}
		
			res.write(home);


            res.end();
        });

	CROW_ROUTE(app, "/help") // Index page
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::GET)
        ([&db](const crow::request& req, crow::response& res){
			// Redirect to the cart page
            res.code = 200;
				
			    res.write(loadFile(res, "", "help.html"));

            res.end();
        });
	
	CROW_ROUTE(app, "/search") // Get a current task list by key in json body
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::GET)
        ([&db](const crow::request& req, crow::response& res){
			crow::json::wvalue jsonResponse;
			if (req.method == crow::HTTPMethod::GET) {
				
				const crow::json::rvalue& parsed = crow::json::load(req.body);
				vector<Task> filteredTasks;
				crow::query_string keyParam = req.url_params;
				auto keys = keyParam.keys();
				auto keyValue = keyParam.get(string(keys[0]));
				string searchKey = string(keyValue);
				filteredTasks =  db.getFilteredTasks(searchKey);

				vector<crow::json::wvalue> jsonFilteredTasks;

				for(int i = 0; i < filteredTasks.size(); i++)
				{
					crow::json::wvalue jsonTask = buildJsonFromTask(filteredTasks[i]);
					jsonFilteredTasks.push_back(jsonTask);
				}
				jsonResponse = std::move(jsonFilteredTasks);

			}
			
			cout << "writing response, ";
			
			res.set_header("Access-Control-Allow-Origin", "127.0.0.1:8080");
			res.add_header("Content-Type", "application/json");

			res.code = 200;
			res.write(jsonResponse.dump());
			res.end();
		});

	CROW_ROUTE(app, "/edit") // Get a current task by id
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::GET, crow::HTTPMethod::PATCH)
        ([&db](const crow::request& req, crow::response& res){
			

			Task submitted = buildTaskFromJson(req.body);
			res.code = 200;

			if (req.method == crow::HTTPMethod::GET) {
				// read database
				Task existingTask = db.getTask(submitted);
				//format a response
				crow::json::wvalue jsonTask = buildJsonFromTask(existingTask);
				cout << "writing response, ";
				res.write(jsonTask.dump());

			} else if (req.method == crow::HTTPMethod::PATCH) {
				// Update only changed elements
				bool exists = db.checkExists(submitted);

				if(exists)
				{
					bool updatStatus = db.updateTask(submitted);
					if(!updatStatus)
						res.code = 409;
				}
				else
					res.code = 409;			
			}
			
			res.end();
		});

	CROW_ROUTE(app, "/add") // Upload a new task
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::POST, crow::HTTPMethod::PUT)
        ([&db](const crow::request& req, crow::response& res){
			// Create task object from JSON data
			Task submitted = buildTaskFromJson(req.body);
			submitted.id = -1; // The id should be autogenerated

			bool exists = db.checkExists(submitted);

			// Perform task based on REST method
			if (req.method == crow::HTTPMethod::POST) {
				//
				//	SUBMIT NEW RECORD (ONLY IF DOES NOT EXIST ALREADY)
				//
				// Return error page, temporary basic string for now
				if (exists) {
					res.code = 409;
					std::ostringstream msg;
					msg << "Error 409 - Conflict. Task for user '" << submitted.user.name << "' for part '[" << submitted.consumedPart.id << "] " << submitted.consumedPart.name << "' Already exists. Duplicates cannot exist." << std::endl;
					res.write(msg.str());
					res.end();
					return;
				}

				// Does not exist, add it
				bool insertRes = db.insertTask(submitted);

				// Success or fail
				if (insertRes == SQLITE_OK) {
					res.code = 200;
				} else {
					res.code = 500;
				}
				
				res.code = 200;
				res.end();
			} else if (req.method == crow::HTTPMethod::PUT) {
				//
				// UPDATE EXISTING RECORD (ONLY IF EXISTS)
				//
				// Return error page, temporary basic string for now
				// Does not exist, so we can't edit it
				if (!exists) {
					res.code = 404;
					std::ostringstream msg;
					msg << "Error 404 - Not Found. Task for user '" << submitted.user.name << "' for part '[" << submitted.consumedPart.id << "] " << submitted.consumedPart.name << "' Must first exist before it can be edited. It does not exist." << std::endl;
					res.write(msg.str());
					res.end();
					return;
				}

				// Replace existing task with provided data
				bool updateRes = db.updateTask(submitted);

				// Success or fail
				if (updateRes == SQLITE_OK) {
					res.code = 200;
				} else {
					res.code = 500;
				}

				res.end();
			}

			res.end();
			
		});

	CROW_ROUTE(app, "/delete") // Replace exisitng task
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::DELETE)
        ([&db](const crow::request& req, crow::response& res){

			// Load JSON body
			const crow::json::rvalue& parsed = crow::json::load(req.body);

			int id = parsed["id"].i();


			// Delete task if exists in database
			bool deleteRes = db.deleteTask(std::to_string(id));

			// Check if the query was successful
			if (deleteRes == false) { // no error
				res.code = 200;
			} else { // yes error
				res.code = 500;
			}

			res.end();

		});
	// OPTIONS /
	// GET /edit/<int>
	// PATCH /edit/<int>
	// POST  /add 				// add if it doesn't exist, otherwise return 409
	// PUT /add   				// replace existing entry
	// DELETE /delete/<int> 	// delete entry
	 
		
	app.port(8080).multithreaded().run();
	return 1;
}
crow::json::wvalue buildJsonFromPart(Part& part)
{
	crow::json::wvalue jsonPart;
	jsonPart["partid"] = to_string(part.id);
	jsonPart["name"] = string(part.name);

	return jsonPart;
}
crow::json::wvalue buildJsonFromTask(Task& task)
{
	crow::json::wvalue jsonTask;
	jsonTask["taskid"] = to_string(task.id);
	jsonTask["title"] = string(task.title);
	jsonTask["description"] = string(task.description);
	jsonTask["datecreated"] = string(task.datecreated);
	jsonTask["partid"] = to_string(task.consumedPart.id);
	jsonTask["part"] = string(task.consumedPart.name);
	jsonTask["assigned"] = to_string(task.user.id);
	jsonTask["assignedName"] = string(task.user.name);

	return jsonTask;
}

Task buildTaskFromJson(string reqBody)
{
				
	Task t;
	const crow::json::rvalue& parsed = crow::json::load(reqBody);
	
	Part* p = &t.consumedPart;
	User* u = &t.user;

	///    Convert JSON data to raw data for Task struct
	// Parts
	try{
		//part id
		p->id = parsed["partid"].i(); //atoi(id.c_str());
	}
	catch(...)
	{
		std::cerr <<"could not find part id in req body\n";
	}
	
	try
	{
		//part name
		memcpy(p->name,			parsed["part"].s().s_,			PART_NAME_LENGTH);
	}
	catch(...)
	{
		std::cerr <<"couldnt find part name in req body\n";
	}
	
	try
	{
		//User Id
		u->id = parsed["assigned"].i();
	}
	catch(...)
	{
		std::cerr <<"could not find user id in req body\n";
	}

	try
	{
		// User name
		memcpy(u->name,			parsed["assignedName"].s().s_,	USER_NAME_LENGTH);
	}
	catch(...)
	{
		std::cerr <<"could not find user name in req body\n";
	}
	
	try
	{
		//task id
		t.id = parsed["taskid"].i();
	}
	catch(...)
	{
		std::cerr <<"could not find task id in req body\n";
	}
	try
	{
		// Task title
		memcpy(t.title,			parsed["title"].s().s_,			TASK_TITLE_LENGTH);
	}
	catch(...)
	{
		std::cerr <<"could not find task title in req body\n";
	}

	try
	{
		// Task Descriptoin
		memcpy(t.description,	parsed["description"].s().s_,	DESCRIPTION_LENGTH);
	}
	catch(...)
	{
		std::cerr <<"could not find task description in req body\n";
	}
	return t;
}

// Function Definitions
std::string loadFile(crow::response& res, std::string _folder, std::string _name) {
	std::string path = "/Shared/public/" + _folder + _name;

	std::ifstream file(path, std::ifstream::in);

	if (file) {
		std::ostringstream contents;
		contents << file.rdbuf();
		file.close();
		return contents.str();
	}
	else {
		res.set_header("Content-Type", "text/plain");
		res.code = 404;
		return path + "Not Found";
	}
}

std::string replaceTemplates(std::string htmlString, const char templateStr[], std::string replacement) {
	const int templateSize = strlen(templateStr);

	// Find the location of the first occurance of the product template
	size_t loc = htmlString.find(templateStr);
	if (loc <= templateSize) {
		return htmlString;
	}

	// Split the html in two, using the product template as a delimeter
	std::string before = htmlString.substr(0, loc);
	std::string after = htmlString.substr(loc + templateSize);


	// Build the final result
	std::stringstream result;
	// Prefix the before string, then add our new code, then the rest of the html
	result << before << replacement << after;

	return result.str();
}

// Check if a request is authorized to access page for some userID.
bool isAuthorized(ID userID, const crow::request& req) {
	/// --- How to check for authorization ---
		// https://crowcpp.org/master/guides/auth/
	// Example of an authorization header entry
	/// Authorization: Basic bXlVbmlxdWVVc2VybmFtZTpteVBhc3N3b3JkCg==

	// Where...
	/// [Authorization:]  =   The header name
	/// [Basic ]   =   A prefix to the data, signifying the data is base64 encoded
	/// [bXlVbmlxdWVVc2VybmFtZTpteVBhc3N3b3JkCg==]    =    "myUniqueUsername:myPassword" (without "") encoded in base64

	// Get the full contents of the authorization header
	std::string authHeader = req.get_header_value("Authorization");
	// WHat if we weren't given a header?

	// Remove the "Basic " keyword
	std::string base64 = authHeader.substr(6);
	// What if the data length is less than 6?

	// Decode the base64
	std::string rawAuth = crow::utility::base64decode(base64, base64.size());
	// What if we weren't given valid base64???

	/// Now split the credentials into username and password
	unsigned int split = rawAuth.find(':');
	std::string username = rawAuth.substr(0, split);
	std::string pass = rawAuth.substr(split+1);


	// Now, verify that userID, username, and pass align with each other in the database


	// Debug return false always
	return false;

}