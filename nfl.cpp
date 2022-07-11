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

void Nfl::extractData(tinyxml2::XMLElement *element, HtmlElement *data) {
    if (strcmp(element->Name(), data->tag) == 0) {
        const tinyxml2::XMLAttribute *attribute = element->FindAttribute(data->attributeName);
        if (attribute) {
            if (strstr(attribute->Value(), data->attributeValue)) {
                // The text is inside the element 3 nodes down the tree.
                tinyxml2::XMLElement *managerElement = element;
                for (int i = 0; i < 3; i++) {
                    managerElement = managerElement->FirstChildElement();
                    if (managerElement == NULL) {
                        std::cout << "Manager name not found where expected" << std::endl;
                        return;
                    }
                }

                std::string manager = managerElement->GetText();
                if (std::find(managers.begin(), managers.end(), manager) == managers.end()) {
                    managers.push_back(manager);
                }
            }
        }
    }
}

void Nfl::managerSearch(tinyxml2::XMLElement* element) {
    const char *tag = "td";
    const char *attributeName = "class";
    const char *attributeValue = "teamOwnerName";
    HtmlElement data = {
        tag,
        attributeName,
        attributeValue
    };
    extractData(element, &data);
}

void Nfl::standingsSearch(tinyxml2::XMLElement *element) {}

std::vector<std::string> Nfl::getManagers() {
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
            file << manager << "\n";
        }

        file.close();
    } else {
        std::ifstream file(filePath);

        std::string line;
        while (std::getline(file, line, '\n')) {
            if (line.compare("") != 0) {
                managers.push_back(line);
            }
        }

        file.close();
    }

    return managers;
}

void Nfl::getStandings() {
    std::string filePath = "data/standings.txt";
    struct stat buffer;

    if (update || stat(filePath.c_str(), &buffer) != 0) {
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
    } else {

    }

    return;
}
