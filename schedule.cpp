#include <fstream>
#include <ctime>
#include "scheduler.h"
#include "nfl.h"

int main() {
    std::ifstream configFile("config.txt");

    std::string leagueId;
    std::getline(configFile, leagueId, '\n');

    std::string configUpdate;
    std::getline(configFile, configUpdate, '\n');
    bool update = configUpdate.compare("true") == 0;

    std::time_t time = std::time(nullptr);
    const std::tm *timeInfo = std::localtime(&time);
    int previousYear = 1900 + timeInfo->tm_year - 1;

    Nfl nfl(leagueId, update, previousYear);
    std::vector<std::string> entities = nfl.getManagers();
    Constraints constraints = nfl.getConstraints();

    int weeks = 13;
    int weeksBetweenMatchups = 2;

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

    std::string logoPath;
    std::string title;

    std::getline(configFile, logoPath, '\n');
    std::getline(configFile, title, '\n');

    configFile.close();

    Scheduler scheduler(weeks, entities, constraints, scheduleConstraints, weeksBetweenMatchups, logoPath, title);
    scheduler.createSchedules(1);

    return 0;
}
