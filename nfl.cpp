#include "nfl.h"

Nfl::Nfl(std::string id, bool u, int y) {
    leagueId = id;
    update = u;
    year = y;
}

xmlDoc *Nfl::scrape(std::string url) {
    curlpp::Easy request;
    request.setOpt(curlpp::options::Url(url));

    std::ostringstream responseStream;
    curlpp::options::WriteStream streamWriter(&responseStream);
    request.setOpt(streamWriter);

    request.perform();
    std::string response = responseStream.str();

	xmlDoc *html = htmlReadDoc((xmlChar*)response.c_str(), NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
	return html;
}

void Nfl::parseManagers(xmlDoc *html) {
	xmlXPathContextPtr context = xmlXPathNewContext(html);

	xmlChar *trXPath = (xmlChar *)"//tr[contains(@class, 'team-')]";
	xmlXPathObjectPtr trs = xmlXPathEvalExpression(trXPath, context);

	xmlChar *spanXPath= (xmlChar *)"//span[contains(@class, 'userName')]";
	for (int i = 0; i < trs->nodesetval->nodeNr; i++) {
		xmlDocSetRootElement(html, trs->nodesetval->nodeTab[i]);
		xmlXPathObjectPtr spans = xmlXPathEvalExpression(spanXPath, context);

		if (xmlXPathNodeSetIsEmpty(spans->nodesetval)) continue;
		std::string manager = std::string((char *)xmlNodeGetContent(spans->nodesetval->nodeTab[0]));

		struct _xmlAttr *trAttributes = trs->nodesetval->nodeTab[i]->properties;
		while (trAttributes) {
			if (strcmp((const char *)trAttributes->name, "class") == 0) {
				std::string attributeValue = std::string((char *)trAttributes->children->content);
				std::string id = attributeValue.substr(attributeValue.find("-") + 1, 1);
                managers[id] = manager;
			}

			trAttributes = trAttributes->next;
		}

		xmlXPathFreeObject(spans);
	}

	xmlXPathFreeObject(trs);
}

void Nfl::parseStandings(xmlDoc *html) {
	xmlXPathContextPtr context = xmlXPathNewContext(html);

	xmlChar *spanXPath = (xmlChar *)"//span[contains(@class, 'teamRank ')]";
	xmlXPathObjectPtr spans = xmlXPathEvalExpression(spanXPath, context);

	for (int i = 0; i < spans->nodesetval->nodeNr; i++) {
		std::string rank = std::string((char *)xmlNodeGetContent(spans->nodesetval->nodeTab[i]));
		if (strstr(rank.c_str(), "(")) continue;

		struct _xmlAttr *attributes = spans->nodesetval->nodeTab[i]->properties;
		while (attributes) {
			if (strcmp((const char *)attributes->name, "class") == 0) {
				std::string attributeValue = std::string((char *)attributes->children->content);
				std::string id(1, attributeValue.back());
                standings[id] = atoi(rank.c_str());
			}

			attributes = attributes->next;
		}
	}

	xmlXPathFreeObject(spans);
}

std::vector<std::string> Nfl::getManagers() {
    std::vector<std::string> managerNames;
    std::string filePath = "data/entities.txt";
    struct stat buffer;

    if (update || stat(filePath.c_str(), &buffer) != 0) {
        std::string url = "https://fantasy.nfl.com/league/" + leagueId + "/owners";
		xmlDoc *html = scrape(url);
		parseManagers(html);
		xmlFreeDoc(html);

        std::ofstream file(filePath);

        for (const auto &manager : managers) {
            std::string name = manager.second;
            managerNames.push_back(name);
            file << name << "\n";
        }

        file.close();
    } else {
        std::ifstream file(filePath);

        std::string line;
        while (std::getline(file, line, '\n')) {
            if (line.compare("") != 0) {
                managerNames.push_back(line);
            }
        }

        file.close();
    }

    return managerNames;
}

void Nfl::getStandings() {
    std::string url = "https://fantasy.nfl.com/league/" + leagueId + 
        "/history/" + std::to_string(year) + 
        "/standings?historyStandingsType=regular";
	xmlDoc *html = scrape(url);
	parseStandings(html);
	xmlFreeDoc(html);
}

Constraints Nfl::getMatchupConstraints() {
    Constraints constraints;
    std::string filePath = "data/constraints.txt";
    struct stat buffer;

    if (update || stat(filePath.c_str(), &buffer) != 0) {
        getStandings();

        for (const auto &manager : managers) {
            std::string id = manager.first;
            std::string name = manager.second;

            for (const auto &opponent : managers) {
                std::string opponentId = opponent.first;
                std::string opponentName = opponent.second;

                if (id.compare(opponentId) != 0) {
                    if (standings[id] == standings[opponentId]) {
                        constraints[name][opponentName] = 1;
                    } else {
                        constraints[name][opponentName] = 2;
                    }
                }
            }
        }

        std::ofstream file(filePath);

        for (const auto &c : constraints) {
            file << c.first << "\n";

            for (const auto &m : c.second) {
                file << m.first << "|" << m.second << "\n";
            }
        }

        file.close();
    } else {
        std::ifstream file(filePath);

        std::string constraintsLine;
        std::string entity = "";
        while (std::getline(file, constraintsLine, '\n')) {
            if (constraintsLine.size() > 0) {
                int delimiterIndex = constraintsLine.find("|");

                if (delimiterIndex < 0) {
                    entity = constraintsLine;
                    constraints[entity] = {};
                } else {
                    std::string opponent = constraintsLine.substr(0, delimiterIndex);
                    int numMatchups = stoi(constraintsLine.substr(delimiterIndex + 1));

                    if (constraints.find(entity) != constraints.end()) {
                        constraints[entity][opponent] = numMatchups;
                    }
                }
            }
        }

        file.close();
    }

    return constraints;
}

std::vector<Matchups> Nfl::getScheduleConstraints(int weeks) {
    std::ifstream scheduleFile("data/schedule-constraints.txt");

    std::vector<Matchups> scheduleConstraints;
    for (int index = 0; index < weeks; index++) {
        scheduleConstraints.push_back({});
    }

    std::string scheduleLine;
    int week = -1;
    while (std::getline(scheduleFile, scheduleLine, '\n')) {
        if (scheduleLine.size() > 0) {
            int delimiterIndex = scheduleLine.find("|");

            if (delimiterIndex < 0) {
                week = stoi(scheduleLine);
                scheduleConstraints[week] = {};
            } else {
                std::string entity = scheduleLine.substr(0, delimiterIndex);
                std::string opponent = scheduleLine.substr(delimiterIndex + 1);

                if (week > -1 && scheduleConstraints.size() >= week) {
                    scheduleConstraints[week - 1][entity] = opponent;
                    scheduleConstraints[week - 1][opponent] = entity;
                }
            }
        }
    }

    scheduleFile.close();

    return scheduleConstraints;
}
