#define CROW_MAIN

#include "headers/db.hpp"
#include <regex>
#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include "headers/crow_all.h"

using namespace crow;
using namespace std;

// Function Definitions
std::string loadFile(response& res, std::string _folder, std::string _name);
std::string replaceTemplates(std::string htmlString, const char templateStr[], std::string replacement);
bool isAuthorized(ID userID, const request& req);

// Main Function
int main()
{
	srand(time(NULL));

	crow::SimpleApp app;
	// Create and initialize the database
	TaskDb db;


	CROW_ROUTE(app, "/") // Index page
	.methods(HTTPMethod::Options, HTTPMethod::Get)
        ([&db](const request& req, response& res){
			// Redirect to the cart page
            res.code = 200;
		
			res.write(loadFile(res, "", "home.html"));

            res.end();
        });

	CROW_ROUTE(app, "/help") // Index page
	.methods(HTTPMethod::Options, HTTPMethod::Get)
        ([&db](const request& req, response& res){
			// Redirect to the cart page
            res.code = 200;
		
			res.write(loadFile(res, "", "help.html"));

            res.end();
        });


	CROW_ROUTE(app, "/edit/<int>") // Get a current task by id
	.methods(HTTPMethod::Options, HTTPMethod::Get, HTTPMethod::Patch)
        ([&db](const request& req, response& res, int id){

			if (req.method == HTTPMethod::Get) {
				// read database

			} else if (req.method == HTTPMethod::Patch) {
				// Update only changed elements
			}

			
		});

	CROW_ROUTE(app, "/add") // Upload a new task
	.methods(HTTPMethod::Options, HTTPMethod::Post, HTTPMethod::Put)
        ([&db](const request& req, response& res){
			if (req.method == HTTPMethod::Post) {
				// Check if task exists
				// If it does, return 409
			} else if (req.method == HTTPMethod::Put) {
				// Check if task exists
				// if not, return "not found"

				// Replace existing task with provided data
			}

			
		});

	CROW_ROUTE(app, "/delete") // Replace exisitng task
	.methods(HTTPMethod::Options, HTTPMethod::Delete)
        ([&db](const request& req, response& res){

			// Delete task if exists in database


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


// Function Definitions
string loadFile(response& res, std::string _folder, std::string _name) {
	std::string path = "/Shared/public/" + _folder + _name;

	ifstream file(path, ifstream::in);

	if (file) {
		ostringstream contents;
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
	stringstream result;
	// Prefix the before string, then add our new code, then the rest of the html
	result << before << replacement << after;

	return result.str();
}

// Check if a request is authorized to access page for some userID.
bool isAuthorized(ID userID, const request& req) {
	/// --- How to check for authorization ---
		// https://crowcpp.org/master/guides/auth/
	// Example of an authorization header entry
	/// Authorization: Basic bXlVbmlxdWVVc2VybmFtZTpteVBhc3N3b3JkCg==

	// Where...
	/// [Authorization:]  =   The header name
	/// [Basic ]   =   A prefix to the data, signifying the data is base64 encoded
	/// [bXlVbmlxdWVVc2VybmFtZTpteVBhc3N3b3JkCg==]    =    "myUniqueUsername:myPassword" (without "") encoded in base64

	// Get the full contents of the authorization header
	string authHeader = req.get_header_value("Authorization");
	// WHat if we weren't given a header?

	// Remove the "Basic " keyword
	string base64 = authHeader.substr(6);
	// What if the data length is less than 6?

	// Decode the base64
	string rawAuth = crow::utility::base64decode(base64, base64.size());
	// What if we weren't given valid base64???

	/// Now split the credentials into username and password
	unsigned int split = rawAuth.find(':');
	string username = rawAuth.substr(0, split);
	string pass = rawAuth.substr(split+1);


	// Now, verify that userID, username, and pass align with each other in the database


	// Debug return false always
	return false;

}