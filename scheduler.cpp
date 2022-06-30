#include <iostream>
#include "scheduler.h"

Scheduler::Scheduler(int w, std::vector<std::string> e, Constraints c, std::vector<Matchups> sc, int wbm) {    
    weeks = w;
    entities = e;
    constraints = c;
    constraintsCheck = c;
    scheduleConstraints = sc;
    weeksBetweenMatchups = wbm;
}

void Scheduler::initializeSchedule() {
    for (int week = 1; week <= weeks; week++) {
        Matchups matchups;

        for (const auto &entity : entities) {
            matchups[entity] = "";
        }

        schedule.push_back(matchups);
    }
}

void Scheduler::printSchedule() {
    for (int week = 1; week <= weeks; week++) {
        std::cout << "Week " << week << std::endl;
    
        for (const auto &matchup : schedule[week - 1]) {
            std::cout << "\t";
            std::cout << matchup.first << " - ";
            std::cout << matchup.second << std::endl;
        }
    }
}

void Scheduler::createSchedules(int n) {
    for (int index = 0; index < n; index++) {
        srand(time(0));
        initializeSchedule();
        scheduleWeek(1);

        if (validateSchedule()) {
            printSchedule();
        } else {
            std::cout << "Not a valid schedule" << std::endl;
        }
    }
}

void Scheduler::scheduleWeek(int week) {
    if (week > weeks) {
        return;
    }

    std::vector<std::string> unscheduledEntities = entities;

    while (unscheduledEntities.size() > 0) {
        std::string entity = unscheduledEntities[0];
        std::string opponent = getOpponent(week, entity);

        if (opponent.length() > 0) {
            alterSchedule(week, entity, opponent, unscheduledEntities);
        } else {
            int backtrack = 4;
            int newWeek = std::max(week - backtrack, 1);
            cleanup(week, newWeek);
            return scheduleWeek(newWeek);
        }

        if (unscheduledEntities.size() == 0) {
            return scheduleWeek(week + 1);
        }
    }
}

std::string Scheduler::getOpponent(int week, std::string entity) {
    std::vector<std::string> possibleOpponents;
    
    for (const auto &opponent : constraints[entity]) {
        if (checkMatchup(week, entity, opponent.first)) {
            possibleOpponents.push_back(opponent.first);
        }
    }

    if (possibleOpponents.size() > 0) {
        int index = std::rand() % possibleOpponents.size();
        return possibleOpponents[index];
    } else {
        return "";
    }
}

void Scheduler::alterSchedule(int week, std::string entity, std::string opponent, std::vector<std::string> &unscheduledEntities) {
    schedule[week - 1][entity] = opponent;
    schedule[week - 1][opponent] = entity;

    unscheduledEntities.erase(std::remove(unscheduledEntities.begin(), unscheduledEntities.end(), entity));
    unscheduledEntities.erase(std::remove(unscheduledEntities.begin(), unscheduledEntities.end(), opponent));

    constraints[entity][opponent]--;
    constraints[opponent][entity]--;
}

void Scheduler::cleanup(int currentWeek, int newWeek) {
    for (int week = newWeek; week <= currentWeek; week++) {
        for (const auto &matchup : schedule[week - 1]) {
            std::string entity = matchup.first;
            std::string opponent = matchup.second;

            if (opponent.length() > 0) {
                constraints[entity][opponent]++;
                schedule[week - 1][entity] = "";
            }
        }
    }
}

bool Scheduler::checkMatchup(int week, std::string entity, std::string opponent) {
    if (entity.compare(opponent) == 0) {
        return false;
    }

    if (constraints[entity][opponent] <= 0) {
        return false;
    }

    if (schedule[week - 1][opponent] != "") {
        return false;
    }

    int startIndex = std::max(week - 1 - weeksBetweenMatchups, 0);
    int endIndex = std::min(week - 1 + weeksBetweenMatchups, weeks - 1);
    for (int i = startIndex; i <= endIndex; i++) {
        if (schedule[i][entity].compare(opponent) == 0) {
            return false;
        }
    }

    return true;
}

bool Scheduler::validateSchedule() {
    Constraints testConstraints;

    if (schedule.size() != weeks) {
        std:: cout << schedule.size() << "\t" << weeks << std::endl;
        return false;
    }

    for (int week = 1; week <= weeks; week++) {
        for (const auto &matchup : schedule[week - 1]) {
            std::string entity = matchup.first;
            std::string opponent = matchup.second;

            testConstraints[entity][opponent] += 1;

            int startIndex = std::max(week - 1 - weeksBetweenMatchups, 0);
            int endIndex = std::min(week - 1 - weeksBetweenMatchups, weeks - 1);
            for (int i = startIndex; i <= endIndex; i++) {
                if (schedule[i][entity].compare(opponent) == 0) {
                    std::cout << "Week " << week << " Conflicting Week " << i + 1<< "\t" << entity << "\t" << opponent << std::endl;
                    return false;
                }
            }
        }
    }

    for (const auto &e : testConstraints) {
        std::string entity = e.first;
        
        for (const auto &c : e.second) {
            std::string opponent = c.first;
            int numMatchups = c.second;

            if (numMatchups != constraintsCheck[entity][opponent]) {
                std::cout << "constraints issue    " << entity << std::endl;
                return false;
            }
        }
    }

    return true;
}
