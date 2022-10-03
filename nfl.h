#pragma once

#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "scheduler.h"

struct HtmlElement {
    const char *tag;
    const char *attributeName;
    const char *attributeValue;
};

class Nfl {
    public:
        Nfl(std::string id, bool u, int y);
        std::vector<std::string> getManagers();
        Constraints getMatchupConstraints();
        std::vector<Matchups> getScheduleConstraints(int weeks);
    private:
        std::string leagueId;
        bool update;
        int year;
        std::unordered_map<std::string, std::string> managers;
        std::unordered_map<std::string, int> standings;
		xmlDoc *scrape(std::string url);
		void parseManagers(xmlDoc *html);
		void parseStandings(xmlDoc *html);
        void getStandings();
};
