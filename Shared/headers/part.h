#pragma once
#ifndef PART_HPP
#define PART_HPP

#include "defines.hpp"
#include <chrono>
#include <ctime>

typedef struct _Part {
	ID id;
	char name[PART_NAME_LENGTH] ={0};
	char serialNumber[PART_SERIAL_LENGTH]={0};

	
	enum COLUMNS { IDCOL, NAME, SERIALNUMBER };
	std::chrono::system_clock::time_point timeAdded;

	_Part() : timeAdded(std::chrono::system_clock::now()), id(0) {}

	_Part& operator=(const _Part& other)
	{
		if (this == &other)
			return *this;

		this->id = other.id;
		std::copy(other.name, other.name + PART_NAME_LENGTH, this->name);
		std::copy(other.serialNumber, other.serialNumber + PART_SERIAL_LENGTH, this->serialNumber);
		return *this;
	}
	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const {
		return id;
	}
} Part;

// Function for easily creating a new part
Part newPart(ID id, std::string name, std::string serialNum) {
	Part newp;

	// copy id
	newp.id = id;
	
	// copy name and serial number
	memcpy(newp.name, name.c_str(), PART_NAME_LENGTH);
	memcpy(newp.serialNumber, name.c_str(), PART_SERIAL_LENGTH);

	// return newly created struct
	return newp;
}

#endif