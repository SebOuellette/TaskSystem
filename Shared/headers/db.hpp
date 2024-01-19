#ifndef DB_HPP
#define DB_HPP

#include <sqlite3.h>
#include "defines.hpp"
#include "product.hpp"
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
// Using sqlite https://www.geeksforgeeks.org/sql-using-c-c-and-sqlite/

// debug
#define LOG_SUCCESSFUL_QUERIES

class DB {
private:
	sqlite3* _db;

public:
	DB() {
		this->init();
	}

	~DB() {
		sqlite3_close(_db);
	}
	
	// General purpose query. You can use a labmda function as the second argument
	// The lambda function parameters are (void* data, int argc, char** argv, char** columnNames)
	//			The lambda function is what this \/ \/ \/ \/ \/ mess is about
	bool run(string queryString, int(*callback)(void*, int, char**, char**) = NULL, void* data = NULL) {
		char* errorMsg;
		
		// Perform the query
		int exRet = sqlite3_exec(this->_db, queryString.c_str(), callback, data, &errorMsg);

		// Check if an error was reported
		if (exRet != SQLITE_OK) {
			cerr << "Error performing query: " << errorMsg << std::endl;
			sqlite3_free(errorMsg);
		}
#ifdef LOG_SUCCESSFUL_QUERIES
		else {
			cerr << "Query Successful." << endl;
		}
#endif

		// Returns the exit status. AKA the status of the last query
		return (exRet == SQLITE_OK);
	};

	// Initialize the database table
	bool init() {
		string createTables[TABLES] = {
			"CREATE TABLE IF NOT EXISTS Users (id TEXT NOT NULL UNIQUE, cartid int NOT NULL UNIQUE, name varchar(128) NOT NULL UNIQUE, passhash varchar(1024) NOT NULL)",
			"CREATE TABLE IF NOT EXISTS Carts (id INTEGER NOT NULL UNIQUE, userid TEXT NOT NULL UNIQUE, PRIMARY KEY(id AUTOINCREMENT))",
			"CREATE TABLE IF NOT EXISTS Wishlists (id INTEGER NOT NULL UNIQUE, userid TEXT NOT NULL UNIQUE, PRIMARY KEY(id AUTOINCREMENT))",
			"CREATE TABLE IF NOT EXISTS Products (id TEXT NOT NULL, cartid int NOT NULL, sellerid TEXT NOT NULL, name varchar(128) NOT NULL, description varchar(4096) NOT NULL, quantity int NOT NULL, unitcost double NOT NULL, imgurl varchar(512) NOT NULL, time NUMERIC NOT NULL)",
			"CREATE TABLE IF NOT EXISTS WantedProducts (id TEXT NOT NULL, cartid int NOT NULL, sellerid TEXT NOT NULL, name varchar(128) NOT NULL, description varchar(4096) NOT NULL, quantity int NOT NULL, unitcost double NOT NULL, imgurl varchar(512) NOT NULL, time NUMERIC NOT NULL)"
		};

		// Open Database
		int exit = sqlite3_open(DB_PATH, &(this->_db));

		// Ensure the database opens correctly
		if (exit != SQLITE_OK) {
			cerr << "Database failed to open." << endl;
			
			throw 1;
		}

		// Now attempt to run the defined queries
		for (int i=0;i<TABLES;i++) {
			// Just run each query without a callback, there's no need here
			this->run(createTables[i], NULL);
		}

		// Returns the final exit status. AKA the status of the last query
		return (exit == SQLITE_OK);
	}
	//upoload new products to the cart
	bool uploadProducts(ID userID, Product_s p, bool wishlist=false) {

		std::string CartTable = wishlist ? "Wishlists" : "Carts";
		std::string ProductTable = wishlist ? "WantedProducts" : "Products";

		// Check if user and cart already exist in system, if not, add them
		stringstream checkQuery;
		checkQuery << "INSERT INTO "<<CartTable<<" (userid) SELECT \"" << userID <<"\" WHERE NOT EXISTS (SELECT 1 FROM "<<CartTable<<" WHERE userid = \"" << userID << "\");";
		this->run(checkQuery.str());

        stringstream query;
        stringstream selectQuery;
        selectQuery<<"SELECT "<< CartTable <<".id FROM "<< CartTable <<" where "<<CartTable<<".userid=\""<<userID<<"\";";
        //cout << "Running: " << query.str() << endl;
        string cartid;

        this->run(selectQuery.str(), [](void* data, int argc, char* argv[], char* colNames[]){
            // This same block of code will run for each SQL result
            //cout << "Found: " << argc << " rows" << endl;

            string* cartid=(string*) data;

            // Loop through each row
            for (int r=0;r<argc;r++) {

                // Build a product
                if (strcmp(colNames[r], "id") == 0) {
                    *cartid = argv[r];
				}

            }
            return 0;
        }, (void*)&cartid);

        query << "INSERT INTO " <<ProductTable << "(id,cartid,sellerid,name,description,quantity,unitcost,imgurl,time) VALUES(\""<<p.id<<"\","<< cartid <<",\""<<p.sellerID<<"\",\"" << p.name << "\",\"" << p.description << "\","<<to_string(p.quantity) << "," <<to_string(p.price)<<", \"" << p.imgurl << "\","<<to_string(std::chrono::duration_cast<std::chrono::milliseconds>(p.timeAdded.time_since_epoch()).count())<<");";
		//cout << query.str() << endl;
	    return this->run(query.str(),NULL);

        // Return the list of products
        //return products;
    }

	// Get list of products by user id
	vector<Product> loadProducts(ID userID, bool wishlist=false) {
		std::string CartTable = wishlist ? "Wishlists" : "Carts";
		std::string ProductTable = wishlist ? "WantedProducts" : "Products";

		stringstream query;
		query << "SELECT "<<CartTable<<".userid as userid, "<<ProductTable<<".* FROM "<<ProductTable<<" INNER JOIN "<<CartTable<<" ON "<<CartTable<<".id="<<ProductTable<<".cartid WHERE "<<CartTable<<".userid=\"" << userID <<  "\";";
		//cout << "Running: " << query.str() << endl;

		vector<Product> products;

		this->run(query.str(), [](void* data, int argc, char** argv, char** colNames){
			// This same block of code will run for each SQL result
			//cout << "Found: " << argc << " rows" << endl;

			vector<Product>* products = (vector<Product>*)data;

			Product p;

			// Loop through each row
			for (int r=0;r<argc;r++) {
	
				// Build a product
				if (strcmp(colNames[r], "id") == 0) {
					p.id = argv[r];
				} else if (strcmp(colNames[r], "sellerid") == 0) {
					p.sellerID = argv[r];
				} else if (strcmp(colNames[r], "name") == 0) {
					int length = strlen(argv[r]) + 1;
					strncpy(p.name, argv[r], length);
				} else if (strcmp(colNames[r], "description") == 0) {
					int length = strlen(argv[r]) + 1;
					strncpy(p.description, argv[r], length);
				} else if (strcmp(colNames[r], "imgurl") == 0) {
					int length = strlen(argv[r]) + 1;
					strncpy(p.imgurl, argv[r], length);
				} else if (strcmp(colNames[r], "quantity") == 0) {
					p.quantity = atoi(argv[r]);
				} else if (strcmp(colNames[r], "unitcost") == 0) {
					p.price = atof(argv[r]);
				} else if (strcmp(colNames[r], "time") == 0) {
					p.timeAdded = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(stoll(argv[r])));
				} 

				//cout << colNames[r] << " : " << argv[r] << endl;
			}

			products->push_back(p);
			return 0;
		}, (void*)&products);

		// Return the list of products
		return products;
	}

	void removeExpired(ID userID) {
		
		std::stringstream queryCheck;
		queryCheck << "SELECT Carts.userid as userid, Products.* FROM Products INNER JOIN Carts ON Carts.id=Products.cartid WHERE Carts.userid=\"" << userID <<  "\";";

		this->run(queryCheck.str(), [](void* data, int argc, char** argv, char** colNames){
			Product_s p;
			ID userID;
			DB* thisDB = (DB*)data;

			for (int r=0;r<argc;r++) {
				if (strcmp(colNames[r], "id") == 0) {
					p.id = argv[r];
				} else if (strcmp(colNames[r], "time") == 0) {
					p.timeAdded = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(stoll(argv[r])));
				} else if (strcmp(colNames[r], "userid") == 0) {
					userID = argv[r];
				}
			}


			if (p.isExpired())
			{
				std::stringstream query;
				query << "DELETE FROM Products WHERE cartid=(SELECT Carts.id FROM Carts WHERE userid=\"" << userID << "\") AND Products.id=\"" << p.id << "\";";
				bool worked = thisDB->run(query.str());
			}
			return 0;
		}, (void*)this);
	}
	
	void increaseProductQuantity(ID productID, int wishlist) {
		std::string ProductTable = wishlist ? "WantedProducts" : "Products";

		stringstream query;
		query << "UPDATE "<< ProductTable << " SET quantity = quantity + 1 WHERE id = \"" << productID << "\";";
		bool worked = this->run(query.str());
	}

	void decreaseProductQuantity(ID productID, int wishlist) {
		std::string ProductTable = wishlist ? "WantedProducts" : "Products";

		stringstream query;
		stringstream query2;
		query << "UPDATE "<< ProductTable << " SET quantity = quantity - 1 WHERE id = \"" << productID << "\";";
		bool worked = this->run(query.str());		
		query2 << "DELETE FROM "<< ProductTable << " WHERE quantity < 1;";
		worked = this->run(query2.str());
	
	}

	bool checkIfExists(ID userID, ID productID, int wishlist) {
		std::string CartTable = wishlist ? "Wishlists" : "Carts";
                std::string ProductTable = wishlist ? "WantedProducts" : "Products";

                stringstream query;
                query << "SELECT "<<CartTable<<".userid as userid, "<<ProductTable<<".* FROM "<<ProductTable<<" INNER JOIN "<<CartTable<<" ON "<<CartTable<<".id="<<ProductTable<<".cartid WHERE "<<CartTable<<".userid=\"" << userID <<  "\" AND " << ProductTable << ".id=\"" << productID << "\";";
                //cout << "Running: " << query.str() << endl;

                vector<Product> products;

		// We actually search for a product quantity variable, since that's what we want
		bool foundProduct = false;

                this->run(query.str(), [](void* data, int argc, char** argv, char** colNames){
                        // This same block of code will run for each SQL result
                        //cout << "Found: " << argc << " rows" << endl;
			bool* foundProduct = (bool*)data;

                        // Loop through each row
                        for (int r=0;r<argc;r++) {

                                // Build a product
                                if (strcmp(colNames[r], "quantity") == 0) {
                                        *foundProduct = true;
				}
			}

			return (int)*foundProduct;
		}, (void*)&foundProduct);

		return foundProduct;
	}

};

#endif

// Useful queries

	// Example 1: Find user using product ID
	// SELECT Users.* FROM Products INNER JOIN Carts ON Carts.id=Products.cartid INNER JOIN Users ON Users.id=Carts.userid WHERE Products.id=334;

	// Example 2: Find all products in a cart using the userID
	// SELECT Products.* FROM Products INNER JOIN Carts ON Carts.id=Products.cartid where Carts.userid=323

	// Example 3: Find all products in a cart using the username
	// SELECT Products.* FROM Products INNER JOIN Carts ON Carts.id=Products.cartid INNER JOIN Users ON Users.id=Carts.userid WHERE Users.name="John Ron"
