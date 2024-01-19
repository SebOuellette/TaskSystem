#pragma once
#ifndef TASK_HPP
#define TASK_HPP

#include "part.h"
#include "defines.hpp"
#include <chrono>
#include <ctime>

typedef struct _Task {
	ID id;
	char title[TASK_TITLE_LENGTH];
	char description[DESCRIPTION_LENGTH];
	Part consumedPart;
	int quantityConsumed;

	std::chrono::system_clock::time_point timeAdded;
	//0: id, 1: title, 2: description, 3: partid
	enum COLUMNS {IDCOL, TITLE, DESCRIPTION, PARTID};

	_Task() : timeAdded(std::chrono::system_clock::now()) {}

	_Task(Part& part) {
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

		std::copy(other.title, other.title + TASK_TITLE_LENGTH, this->title);
		std::copy(other.description, other.description + DESCRIPTION_LENGTH, this->description);
		this->consumedPart = other.consumedPart;
		this->quantityConsumed = other.quantityConsumed;
		return *this;
	}

	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const {
		return id;
	}
} Task;


#endif