#pragma once
#ifndef TASK_HPP
#define TASK_HPP

#include "part.h"
#include "defines.hpp"
#include <chrono>
#include <ctime>

typedef struct _Task {
	ID id;
	char title[TASK_TITLE_LENGTH] = {0};
	char description[DESCRIPTION_LENGTH] = {0};
	char datecreated[DATE_LENGTH] = {0};
	Part consumedPart;
	User user;

	std::chrono::system_clock::time_point timeAdded;
	//0: id, 1: title, 2: description, 3: partid, 4: userid
	enum COLUMNS {IDCOL = 0b1, TITLE = 0b10, DESCRIPTION = 0b100, PARTID = 0b1000, USERID = 0b10000};

	_Task() : timeAdded(std::chrono::system_clock::now()), id(0) {}

	_Task(Part part) {
		this->consumedPart = part;
	}

	bool isOverdue() {
		auto currentTime = std::chrono::system_clock::now();
		auto difference = currentTime - timeAdded;

		auto minutes = std::chrono::duration_cast<std::chrono::minutes>(difference);

		if (minutes.count() < 2) {
			return 0;
		}
		else {
			return 1;
		}

	}

	bool hasPart() {
		return consumedPart.id;
	}

	_Task& operator=(const _Task& other){
		if (this == &other)
			return *this;

		this->id = other.id;
		std::copy(other.title, other.title + TASK_TITLE_LENGTH, this->title);
		std::copy(other.description, other.description + DESCRIPTION_LENGTH, this->description);
		std::copy(other.datecreated, other.datecreated + DATE_LENGTH, this->datecreated);
		this->consumedPart = other.consumedPart;
		this->user = other.user;
		return *this;
	}

	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const {
		return id;
	}
} Task;


#endif