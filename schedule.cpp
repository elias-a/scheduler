#include <fstream>
#include "scheduler.h"

int main() {
    int weeks = 13;
    int weeksBetweenMatchups = 2;
    
    std::ifstream entitiesFile("data/entities.txt");

    std::vector<std::string> entities;
    std::string entitiesLine;
    while (std::getline(entitiesFile, entitiesLine, '\n')) {
        if(entitiesLine.size() > 0) {
            entities.push_back(entitiesLine);
        }
    }

    entitiesFile.close();

    std::ifstream constraintsFile("data/constraints.txt");

    Constraints constraints;
    std::string constraintsLine;
    std::string entity = "";
    while (std::getline(constraintsFile, constraintsLine, '\n')) {
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

    constraintsFile.close();

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

    std::ifstream configFile("config.txt");

    std::getline(configFile, logoPath, '\n');
    std::getline(configFile, title, '\n');

    configFile.close();

    Scheduler scheduler(weeks, entities, constraints, scheduleConstraints, weeksBetweenMatchups, logoPath, title);
    scheduler.createSchedules(1);

    return 0;
}
