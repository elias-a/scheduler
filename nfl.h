#pragma once

#include <iostream>
#include <string>
#include <cpr/cpr.h>

class Nfl {
    public:
        Nfl(std::string id);
        void getManagers();
    private:
        std::string leagueId;
        void scrape(std::string url);
};
