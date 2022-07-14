#include "nfl.h"

Nfl::Nfl(std::string id, bool u, int y) {
    leagueId = id;
    update = u;
    year = y;
}

void Nfl::scrape(std::string url, std::string &text) {
    cpr::Response response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200) {
        text = response.text;
    } else {
        std::cout << "Request not successful" << std::endl;
        return;
    }
}

void Nfl::cleanHtml(std::string &text) {
    // Remove <head>...</head>.
    std::regex head(R"(<head([\s\S]*?)</head>)");
    text = std::regex_replace(text, head, "");

    // Remove <script> tags.
    std::regex script(R"(<script([\s\S]*?)</script>)");
    text = std::regex_replace(text, script, "");

    // Remove comments from the html.
    std::regex htmlComment(R"(<!--([\s\S]*?)-->)");
    text = std::regex_replace(text, htmlComment, "");

    // Remove <!doctype.
    std::regex doctype("<!doctype html>");
    text = std::regex_replace(text, doctype, "");

    std::regex htmlTag("<html.*>");
    text = std::regex_replace(text, htmlTag, "<html>");

    // Remove img, link and a tags, for now, in order to
    // easily modify unquoted attributes in the html.
    // TODO: Circumvent the need to remove these tags.
    std::regex imgTag(R"(<img([\s\S]*?)/>)");
    text = std::regex_replace(text, imgTag, "");

    std::regex linkTag(R"(<link([\s\S]*?)/>)");
    text = std::regex_replace(text, linkTag, "");

    std::regex aTag(R"(<a ([\s\S]*?)>)");
    text = std::regex_replace(text, aTag, "<a>");

    // The HTML standard does not require quotes around attribute
    // values, but the XML parser requires them.
    // TODO: Speed this up.
    std::regex unquotedAttribute(R"((<[\s\S]+=(?:(?!["'])))([^\s>]*))");
    while (std::regex_search(text, unquotedAttribute)) {
        text = std::regex_replace(text, unquotedAttribute, "$1\"$2\"");
    }
}

void Nfl::traverseXml(tinyxml2::XMLElement *element, void (Nfl::*f)(tinyxml2::XMLElement *)) {
    if (element == NULL) {
        return;
    }

    tinyxml2::XMLElement *next;

    next = element->FirstChildElement();
    if (next) {
        traverseXml(next, f);
    }

    next = element->NextSiblingElement();
    if (next) {
        traverseXml(next, f);
    }

    (this->*f)(element);

    return;
}

void Nfl::managerSearch(tinyxml2::XMLElement* element) {
    const char *tag = "tr";
    const char *attributeName = "class";
    const char *attributeValue = "team-";
    HtmlElement data = {
        tag,
        attributeName,
        attributeValue
    };

    if (strcmp(element->Name(), data.tag) == 0) {
        const tinyxml2::XMLAttribute *attribute = element->FindAttribute(data.attributeName);
        if (attribute) {
            if (strstr(attribute->Value(), data.attributeValue)) {
                tinyxml2::XMLElement *e = element->FirstChildElement()->NextSiblingElement();
                // The text is inside the element 3 nodes down the tree.
                tinyxml2::XMLElement *managerElement = e;
                for (int i = 0; i < 3; i++) {
                    managerElement = managerElement->FirstChildElement();
                    if (managerElement == NULL) {
                        std::cout << "Manager name not found where expected" << std::endl;
                        return;
                    }
                }

                // Extract the manager's name and ID.
                std::string classes = attribute->Value();
                std::string id = classes.substr(classes.find("-") + 1, 1);
                std::string name = managerElement->GetText();
                managers[id] = name;
            }
        }
    }
}

void Nfl::standingsSearch(tinyxml2::XMLElement *element) {
    const char *tag = "span";
    const char *attributeName = "class";
    const char *attributeValue = "teamRank ";
    HtmlElement data = {
        tag,
        attributeName,
        attributeValue
    };

    if (strcmp(element->Name(), data.tag) == 0) {
        const tinyxml2::XMLAttribute *attribute = element->FindAttribute(data.attributeName);
        if (attribute) {
            if (strstr(attribute->Value(), data.attributeValue)) {
                std::string rank = element->GetText();
                if (!strstr(rank.c_str(), "(")) {
                    std::string classes = attribute->Value();
                    std::string id(1, classes.back());
                    standings[id] = atoi(rank.c_str());
                }
            }
        }
    }
}

std::vector<std::string> Nfl::getManagers() {
    std::vector<std::string> managerNames;
    std::string filePath = "data/entities.txt";
    struct stat buffer;

    if (update || stat(filePath.c_str(), &buffer) != 0) {
        std::string url = "https://fantasy.nfl.com/league/" + leagueId + "/owners";
        std::string text;
        scrape(url, text);
        cleanHtml(text);
        
        tinyxml2::XMLDocument html;
        html.Parse(text.c_str());

        tinyxml2::XMLElement *rootElement = html.FirstChildElement();
        traverseXml(rootElement, &Nfl::managerSearch);

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
    std::string text;
    scrape(url, text);
    cleanHtml(text);

    tinyxml2::XMLDocument html;
    html.Parse(text.c_str());

    tinyxml2::XMLElement *rootElement = html.FirstChildElement();
    traverseXml(rootElement, &Nfl::standingsSearch);
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
