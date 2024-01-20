#pragma once
#ifndef USER_HPP
#define USER_HPP

#include "defines.hpp"
#include <iostream>

typedef struct _User {
    ID id;
    char name[USER_NAME_LENGTH];
    
    _User(){}

    _User& operator=(const _User& other){
		if (this == &other)
			return *this;

        this->id = other.id;
		std::copy(other.name, other.name + USER_NAME_LENGTH, this->name);
		return *this;
	}

	// Add conversion from a product to an int
	// We can only have one conversion method here or else errors up the wazoo
	operator ID() const {
		return id;
	}
}User;


#endif