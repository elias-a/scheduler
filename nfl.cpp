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
}

void Nfl::traverseXml(tinyxml2::XMLElement *element) {
    if (element == NULL) {
        return;
    }

    tinyxml2::XMLElement *next;

    next = element->FirstChildElement();
    if (next) {
        traverseXml(next);
    }

    next = element->NextSiblingElement();
    if (next) {
        traverseXml(next);
    }

    const char *tag = "td";
    const char *attributeName = "class";
    const char *attributeValue = "teamOwnerName";
    HtmlElement data = {
        tag,
        attributeName,
        attributeValue
    };
    extractData(element, &data);

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

std::vector<std::string> Nfl::getManagers() {
    std::string url = "https://fantasy.nfl.com/league/" + leagueId + "/owners";
    std::string text;
    scrape(url, text);
    cleanHtml(text);
    
    tinyxml2::XMLDocument html;
    html.Parse(text.c_str());

    tinyxml2::XMLElement *rootElement = html.FirstChildElement();
    traverseXml(rootElement);

    return managers;
}
