#pragma once

#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

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
