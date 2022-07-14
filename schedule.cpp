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
    Constraints matchupConstraints = nfl.getMatchupConstraints();

    std::string configWeeks;
    std::string configWeeksBetweenMatchups;
    std::getline(configFile, configWeeks, '\n');
    std::getline(configFile, configWeeksBetweenMatchups, '\n');
    int weeks = stoi(configWeeks);
    int weeksBetweenMatchups = stoi(configWeeksBetweenMatchups);

    std::vector<Matchups> scheduleConstraints = nfl.getScheduleConstraints(weeks);

    std::string configNumSchedules;
    std::getline(configFile, configNumSchedules, '\n');
    int numSchedules = stoi(configNumSchedules);

    std::string logoPath;
    std::string title;

    std::getline(configFile, logoPath, '\n');
    std::getline(configFile, title, '\n');

    configFile.close();

    Scheduler scheduler(weeks, entities, matchupConstraints, scheduleConstraints, weeksBetweenMatchups, logoPath, title);
    scheduler.createSchedules(numSchedules);

    return 0;
}
