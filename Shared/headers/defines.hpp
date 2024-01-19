#ifndef DEFINES_HPP
#define DEFINES_HPP

#include "curl/curl.h"

#include "crow_all.h"
using namespace crow;
using namespace std;


// Program Defines
#define TABLES 5
#define DB_PATH "/Shared/cart.db" // will create cart.db in the shared folder


// Module IP Addresses
#define PROFILE "http://39.106.68.52:8111"
#define CHECKOUT "http://108.168.12.210:8080"
#define CART "https://shoppingcart.honeybeeks.net"
#define AD "http://10.144.124.53:7015"
#define ANALYSIS " https://analysisreportingdatabasemodulegroup1.azurewebsites.net"
#define PRODUCT "http://172.105.25.146:8080"
#define HOME "http://172.105.25.146:8081"

// Product Defines
#define NAME_LENGTH 128
#define PASSWD_HASH_LENGTH 1024
#define DESCRIPTION_LENGTH 4096
#define URL_LENGTH 512

// ID variable type is an unsigned 64-bit integer
typedef std::string ID;

// HTML templates. Used for adding dynamically generated content to the html pages from server-side
#define PRODUCT_TEMPLATE "<!--[[NEXT-CART-ITEM]]-->"
#define TOTAL_COST_TEMPLATE "[[TOTAL_COST]]"
#define USER_ID_TEMPLATE "[[USER_ID]]"
#define AD_TEMPLATE "[[AD-MODULE]]"
#define HOME_LINK_TEMPLATE "[[HOME_LINK]]"
#define PRODUCTS_LINK_TEMPLATE "[[PRODUCTS_LINK]]"


#endif
