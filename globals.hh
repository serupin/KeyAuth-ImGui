#pragma once
#include <unordered_map>
#include "auth/auth.hpp"
#include "skStr.h"

std::string tm_to_readable_time(tm ctx);
static std::time_t string_to_timet(std::string timestamp);
static std::tm timet_to_tm(time_t timestamp);

using namespace KeyAuth;

std::string name = (std::string)skCrypt(""); // application name. right above the blurred text aka the secret on the licenses tab among other tabs
std::string ownerid = (std::string)skCrypt(""); // ownerid, found in account settings. click your profile picture on top right of dashboard and then account settings.
std::string secret = (std::string)skCrypt(""); // app secret, the blurred text on licenses tab and other tabs
std::string version = (std::string)skCrypt(""); // leave alone unless you've changed version on website
std::string url = (std::string)skCrypt("https://keyauth.win/api/1.2/"); // change if you're self-hosting

api KeyAuthApp(name, ownerid, secret, version, url);


class c_globals {
public:
	bool active = true;
	char username[255] = "";
	char password[255] = "";
	char key[255] = "";

	int tab = 0;;

    std::string CreateDate = tm_to_readable_time(timet_to_tm(string_to_timet(KeyAuthApp.data.createdate)));
    std::string LastLogin = tm_to_readable_time(timet_to_tm(string_to_timet(KeyAuthApp.data.lastlogin)));
    std::string Expiry = tm_to_readable_time(timet_to_tm(string_to_timet(KeyAuthApp.data.expiry)));
    std::string subs;
    
};

inline c_globals globals;



std::string tm_to_readable_time(tm ctx) {
    char buffer[80];

    strftime(buffer, sizeof(buffer), "%a %m/%d/%y %H:%M:%S %Z", &ctx);

    return std::string(buffer);
}

static std::time_t string_to_timet(std::string timestamp) {
    auto cv = strtol(timestamp.c_str(), NULL, 10); // long

    return (time_t)cv;
}

static std::tm timet_to_tm(time_t timestamp) {
    std::tm context;

    localtime_s(&context, &timestamp);

    return context;
}
