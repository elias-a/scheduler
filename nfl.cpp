#include "nfl.h"

Nfl::Nfl(std::string id) {
    leagueId = id;
}

void Nfl::scrape(std::string url) {
    cpr::Response response = cpr::Get(cpr::Url{url});
    std::string html;

    if (response.status_code == 200) {
        html = response.text;
    } else {
        std::cout << "Request not successful" << std::endl;
        return;
    }
}

void Nfl::getManagers() {
    std::string url = "https://fantasy.nfl.com/league/" + leagueId + "/owners";
    scrape(url);
}
