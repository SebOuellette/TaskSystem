#ifndef PRODUCT_HPP
#define PRODUCT_HPP

#include "defines.hpp"
#include <chrono>
#include <ctime>

typedef struct _Product {
	ID id;
	ID sellerID;
	char name[NAME_LENGTH];
	char description[DESCRIPTION_LENGTH];
    char imgurl[URL_LENGTH];
	double price;
	unsigned int quantity = 1;

	std::chrono::system_clock::time_point timeAdded;

	_Product() : timeAdded(std::chrono::system_clock::now())	{}

	bool isExpired() {
		auto currentTime = std::chrono::system_clock::now();
		auto difference = currentTime - timeAdded;

		auto months = std::chrono::duration_cast<std::chrono::months>(difference);

		if (months.count() < 2)	{
			return 0;
		}
		else {
			return 1;
		}

	}



	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const { 
		return id;
	}
} Product;

typedef struct _Product_s {
	ID id;
	ID sellerID;
	std::string name;
	std::string description;
    std::string imgurl;
	double price;
	unsigned int quantity = 1;

	std::chrono::system_clock::time_point timeAdded;

	_Product_s() : timeAdded(std::chrono::system_clock::now())	{}

	bool isExpired() {
		auto currentTime = std::chrono::system_clock::now();
		auto difference = currentTime - timeAdded;

		auto months = std::chrono::duration_cast<std::chrono::months>(difference);

		if (months.count() < 2)	{
			return 0;
		}
		else {
			return 1;
		}

	}



	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const { 
		return id;
	}
} Product_s;


#endif