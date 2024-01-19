#pragma once
#ifndef USER_HPP
#define USER_HPP

#include "defines.hpp"
#include <chrono>
#include <ctime>


typedef struct _User {
    ID id;
    char name[USER_NAME_LENGTH];
    
    _User(){}
}User;


#endif