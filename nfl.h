#pragma once

#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
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
        MatchupConstraints getMatchupConstraints();
        ScheduleConstraints getScheduleConstraints(int weeks);
    private:
        std::string leagueId;
        bool update;
        int year;
        std::unordered_map<std::string, std::string> managers;
        std::unordered_map<std::string, int> standings;
		xmlDoc *scrape(const char *url);
		void parseManagers(xmlDoc *html);
		void parseStandings(xmlDoc *html);
        void getStandings();
};
