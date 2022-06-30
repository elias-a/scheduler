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

    std::vector<Matchups> scheduleConstraints = {};

    Scheduler scheduler(weeks, entities, constraints, scheduleConstraints, weeksBetweenMatchups);
    scheduler.createSchedules(1);

    return 0;
}
