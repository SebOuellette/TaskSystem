#pragma once
#ifndef PART_HPP
#define PART_HPP

#include "defines.hpp"
#include <chrono>
#include <ctime>

typedef struct _Part {
	ID id;
	char name[PART_NAME_LENGTH];
	char serialNumber[PART_SERIAL_LENGTH];

	std::chrono::system_clock::time_point timeAdded;
	enum COLUMNS { IDCOL, NAME, SERIALNUMBER };

	_Part() : timeAdded(std::chrono::system_clock::now()) {}

	_Part& operator=(const _Part& other)
	{
		if (this == &other)
			return *this;

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


#endif