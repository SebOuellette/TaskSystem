#ifndef DEFINES_HPP
#define DEFINES_HPP

// Program Defines
#define TABLES 3
#define DB_PATH "/Shared/task.db" // will create task.db in the shared folder
#define EMTPY ""

// Task Defines
#define TASK_TITLE_LENGTH 128
#define DESCRIPTION_LENGTH 4096
#define DATE_LENGTH 23

// Part Defines
#define PART_NAME_LENGTH 25
#define PART_SERIAL_LENGTH 50
#define PART_DISPLAY_LIMIT 500

//User Defines
#define USER_NAME_LENGTH 25

// ID variable type is an unsigned 64-bit integer
typedef int ID;

// HTML templates. Used for adding dynamically generated content to the html pages from server-side
#define TASK_TEMPLATE "<!--[[NEXT-TASK-ITEM]]-->"
#define PART_TEMPLATE "<!--[[PARTS_LIST]]-->"
#define HOME_LINK_TEMPLATE "[[HOME_LINK]]"



#endif