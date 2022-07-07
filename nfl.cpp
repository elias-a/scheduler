#include "nfl.h"

Nfl::Nfl(std::string id) {
    leagueId = id;
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

void Nfl::traverseXml(tinyxml2::XMLElement *element) {
    tinyxml2::XMLElement *next;

    next = element->FirstChildElement();
    if (next) {
        traverseXml(next);
    }

    next = element->NextSiblingElement();
    if (next) {
        traverseXml(next);
    }

    return;
}

void Nfl::getManagers() {
    std::string url = "https://fantasy.nfl.com/league/" + leagueId + "/owners";
    std::string text;
    scrape(url, text);
    
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

    tinyxml2::XMLDocument html;
    html.Parse(text.c_str());

    tinyxml2::XMLElement *rootElement = html.FirstChildElement();
    traverseXml(rootElement);
}
