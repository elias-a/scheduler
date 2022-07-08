#pragma once

#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cpr/cpr.h>
#include "tinyxml2.h"

struct HtmlElement {
    const char *tag;
    const char *attributeName;
    const char *attributeValue;
};

class Nfl {
    public:
        Nfl(std::string id, bool u);
        std::vector<std::string> getManagers();
    private:
        std::string leagueId;
        bool update;
        std::vector<std::string> managers;
        void scrape(std::string url, std::string &text);
        void cleanHtml(std::string &text);
        void traverseXml(tinyxml2::XMLElement *e);
        void extractData(tinyxml2::XMLElement *e, HtmlElement *d);
};
