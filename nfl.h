#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <cpr/cpr.h>
#include "tinyxml2.h"

class Nfl {
    public:
        Nfl(std::string id);
        void getManagers();
    private:
        std::string leagueId;
        void scrape(std::string url, std::string &text);
};
