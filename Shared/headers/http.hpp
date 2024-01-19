#ifndef HTTP_HPP
#define HTTP_HPP

#include "defines.hpp"
#include <iostream>
#include <string>
#include <vector>

typedef struct _res_t {
    CURLcode code;
    std::string body;
} res_t;


class HTTP {
public:
    static size_t _ResCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t totalSize = size * nmemb;
        
        output->append((char*)contents, totalSize);

        return totalSize;
    }

    // Initialize the curl object
    static int init(CURL** curl) {
        *curl = curl_easy_init();

        if (!curl) {
            std::cerr << "Failed to initialize curl. Failing." << std::endl;
            exit(1);
        }
        return 0;
    }

    // Cleanup a curl object
    static void cleanup(CURL* curl, curl_slist* headers = nullptr) {
        curl_easy_cleanup(curl);

        if (headers != nullptr) {
            curl_slist_free_all(headers);
        }
    }

    // Clean all curl objects
    static void cleanup() {
        curl_global_cleanup();
    }


//public:

    // Raw request
    static res_t request(std::string url, std::string method = "GET", std::vector<string>  headers = {}, std::string postbody = "") {
        
        CURL* curl;
        res_t res;
        HTTP::init(&curl);

        
        /// --- Begin HTTP Requsest ---

        // Set request URL
        CURLcode optRes = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());


        if (optRes != CURLE_OK) {
            fprintf(stderr, "curl_easy_setopt failed: %s\n", curl_easy_strerror(optRes));
        }

        

        // Set request method
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);

        


        // If this is a post request, send post body
        if (method == "POST" || method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postbody.c_str());
        }

        // Add any and all headers
        struct curl_slist* _headers = nullptr;
        for (int i=0;i<headers.size();i++) {
            _headers = curl_slist_append(_headers, headers[i].c_str());
        }

        // If we were given at least one header, then we can actually add headers to the request
        if (headers.size() > 0) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
        }

        // Response Body
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTP::_ResCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);


        // Perform the request
        res.code = curl_easy_perform(curl);

        if (res.code != CURLE_OK) {
            std::cerr << "HTTP Request to " << url << " failed. (" << res.code << ")" << std::endl;
        }

        HTTP::cleanup(curl, _headers);

        return res;
    }


    // Example: "Post request to google. We are authorizing with our token aksdfj, setting the context type to normal text, and sending the word meatballs"
    // res_t res = HTTP::request("https://google.com/", "POST", {"Authorization: Bearer aksdfj", "Context-type: text/plain"}, "meatballs");
    // res.code   is the statuscode
    // res.body   is the body in an std::string


    // Example: "Simple get request"
    // res_t res = HTTP::request("https://google.com/");


    // Example: "Simple get request with headers"
    // res_t res = HTTP::request("https://google.com/", "GET", {"Authorization: Bearer asdf"});



};

#endif