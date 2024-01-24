#define CROW_MAIN

#include "headers/db.hpp"
#include "headers/crow_all.h"

#include <regex>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>
#include "headers/crow_all.h"

#define PART_COUNT 5

// Function Definitions
std::string loadFile(crow::response& res, std::string _folder, std::string _name);
std::string replaceTemplates(std::string htmlString, const char templateStr[], std::string replacement);
bool isAuthorized(ID userID, const crow::request& req);
Task buildTaskFromJson(string reqBody);
crow::json::wvalue buildJsonFromTask(Task& task)

// Main Function
int main()
{
	srand(time(NULL));

	crow::SimpleApp app;
	// Create and initialize the database
	TaskDb db;


	// List of parts in the system (example)
	Part parts[PART_COUNT];
	parts[0] = newPart(1, "Part1", "SN-29494");
	parts[1] = newPart(2, "Part2", "SN-4377593");
	parts[2] = newPart(3, "Part3", "SN-6947493");
	parts[3] = newPart(4, "Part4", "SN-5857374");
	parts[4] = newPart(5, "Part5", "SN-6474757");
	

	CROW_ROUTE(app, "/") // Index page
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::GET)
        ([&db, &parts](const crow::request& req, crow::response& res){
			// Redirect to the cart page
            res.code = 200;

			std::string home = loadFile(res, "", "home.html");
			
			// // Add all parts to the part dropdown
			for (int i=0;i<PART_COUNT;i++) {
				std::stringstream result;
				// Build new option, append template at the end to allow for another loop
				result << "<option value=\"" << parts[i].id << "\">" << parts[i].name << "</option>" << PART_TEMPLATE;
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
        ([&db](const crow::request& req){
		
			json::rvalue jsonResponse;
			const crow::json::rvalue& parsed = crow::json::load(reqBody);
			vector<Task> filteredTasks;
			vector<string> keys;

        	for(auto& j: parsed["key"].array_items()) {
            	keys.push_back(j.dump());
				vector<Task> returnedTasks =  db.getFilteredTasks(keyValue);
				filteredTasks.insert(filteredTasks.end(), returnedTasks.begin(), returnedTasks.end());
        	}

			jsonResponse = std::move(filteredTasks);
			return crow::response(std::move(jsonResponse));
		}

	CROW_ROUTE(app, "/edit") // Get a current task by id
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::GET, crow::HTTPMethod::PATCH)
        ([&db](const crow::request& req, crow::response& res){
			

			Task submitted = buildTaskFromJson(req.body);
			res->code = 200;

			if (req.method == crow::HTTPMethod::GET) {
				// read database
				Task existingTask = db.getTask(submitted);
				//format a response
				crow::json::wvalue jsonTask = buildJsonFromTask(existingTask);
				res->write(jsonTask);

			} else if (req.method == crow::HTTPMethod::PATCH) {
				// Update only changed elements
				bool exists = db.checkExists(submitted);

				if(exists)
				{
					bool updatStatus = db.updateTask(submitted);
					if(!updatStatus)
						res->code = 409;
				}
				else
					res->code = 409;		
			}

			res.end();
		});

	CROW_ROUTE(app, "/add") // Upload a new task
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::POST, crow::HTTPMethod::PUT)
        ([&db](const crow::request& req, crow::response& res){
			if (req.method == crow::HTTPMethod::POST) {
				// Check if task exists
				// If it does, return 409
				// Otherwise continue

				// build JSON object from body
				const crow::json::rvalue& parsed = crow::json::load(req.body);
				// Create task object from JSON data
				Task t;
				Part* p = &t.consumedPart;
				User* u = &t.user;

				///    Convert JSON data to raw data for Task struct
				// Parts
				p->id = parsed["id"].i(); //atoi(id.c_str());
				memcpy(p->name,			parsed["part"].s().s_,			PART_NAME_LENGTH);
				
				// User
				u->id = parsed["assigned"].i();
				memcpy(u->name,			parsed["assignedName"].s().s_,	USER_NAME_LENGTH);

				// Task
				memcpy(t.title,			parsed["title"].s().s_,			TASK_TITLE_LENGTH);
				memcpy(t.description,	parsed["description"].s().s_,	DESCRIPTION_LENGTH);


				// Debug print everything
				std::cout << "  Part:" << std::endl << "Part ID:" << p->id << std::endl << "Part Name: " << p->name << std::endl <<
				"  User:" << std::endl << "User ID: " << u->id << std::endl << "User Name:" << u->name << std::endl << 
				"  Task: " << std::endl << "Task Title: " << t.title << std::endl << "Description: " << t.description << std::endl;
			} else if (req.method == crow::HTTPMethod::PUT) {
				// Check if task exists
				// if not, return "not found"

				// Replace existing task with provided data


			}

			res.end();
			
		});

	CROW_ROUTE(app, "/delete/<string>") // Replace exisitng task
	.methods(crow::HTTPMethod::OPTIONS, crow::HTTPMethod::DELETE)
        ([&db](const crow::request& req, crow::response& res, std::string id){

			// Delete task if exists in database
			db.deleteTask(id);

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

crow::json::wvalue buildJsonFromTask(Task& task)
{
	crow::json::wvalue jsonTask;
	jsonTask["taskid"] = json::value::int(task.id);
	jsonTask["title"] = json::value::string(task.title);
	jsonTask["description"] = json::value::string(task.description);
	jsonTask["datecreated"] = json::value::string(task.datecreated);
	jsonTask["partid"] = json::value::int(task.consumedPart.id);
	jsonTask["part"] = json::value::string(task.consumedPart.name);
	jsonTask["assigned"] = json::value::string(task.user.id);
	jsonTask["assignedName"] = json::value::string(task.user.name);

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