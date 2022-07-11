#pragma once

#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <cpr/cpr.h>
#include "tinyxml2.h"
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
        Constraints getConstraints();
    private:
        std::string leagueId;
        bool update;
        int year;
        std::unordered_map<std::string, std::string> managers;
        std::unordered_map<std::string, int> standings;
        void scrape(std::string url, std::string &text);
        void cleanHtml(std::string &text);
        void traverseXml(tinyxml2::XMLElement *e, void (Nfl::*)(tinyxml2::XMLElement *));
        void managerSearch(tinyxml2::XMLElement *);
        void standingsSearch(tinyxml2::XMLElement *);
        void getStandings();
};
