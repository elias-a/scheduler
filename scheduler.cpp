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

// This function schedules a matchup for all entities
// for the given week.
void Scheduler::scheduleWeek(int week) {
    if (week > weeks) {
        return;
    }

    // Keep track of which entities have not been
    // scheduled this week.
    std::vector<std::string> unscheduledEntities = entities;

    // Iterate over the entities that do not have a
    // scheduled matchup this week.
    while (unscheduledEntities.size() > 0) {
        std::string entity = unscheduledEntities[0];
        std::string opponent = getOpponent(week, entity);

        if (opponent.length() > 0) {
            alterSchedule(week, entity, opponent, unscheduledEntities);
        } else {
            // If there are no possible opponents, we need to
            // backtrack, since this is a dead-end.
            int backtrack = 4;
            int newWeek = std::max(week - backtrack, 1);
            cleanup(week, newWeek);
            return scheduleWeek(newWeek);
        }

        // When all entities have a scheduled matchup
        // this week, advance to the next week.
        if (unscheduledEntities.size() == 0) {
            return scheduleWeek(week + 1);
        }
    }
}

// Returns a randomly selected opponent, or None
// if there are no valid matchups.
std::string Scheduler::getOpponent(int week, std::string entity) {
    std::vector<std::string> possibleOpponents;
    
    for (const auto &opponent : constraints[entity]) {
        if (checkMatchup(week, entity, opponent.first)) {
            possibleOpponents.push_back(opponent.first);
        }
    }

    // Choose a random opponent from the list
    // of possible opponents.
    if (possibleOpponents.size() > 0) {
        int index = std::rand() % possibleOpponents.size();
        return possibleOpponents[index];
    } else {
        return "";
    }
}

// Saves the matchup of entity vs. opponent for the
// given week and removes both entities from the
// list of unscheduled entities.
void Scheduler::alterSchedule(int week, std::string entity, std::string opponent, std::vector<std::string> &unscheduledEntities) {
    schedule[week - 1][entity] = opponent;
    schedule[week - 1][opponent] = entity;

    // Both entity and opponent now have
    // scheduled matchups this week.
    unscheduledEntities.erase(std::remove(unscheduledEntities.begin(), unscheduledEntities.end(), entity));
    unscheduledEntities.erase(std::remove(unscheduledEntities.begin(), unscheduledEntities.end(), opponent));

    // Decrement the number of times entity
    // and opponent need to play each other.
    constraints[entity][opponent]--;
    constraints[opponent][entity]--;
}

// When the search hits a dead-end, backtrack.
void Scheduler::cleanup(int currentWeek, int newWeek) {
    // Iterate over the weeks between the new week
    // and the current week, and undo any changes that
    // have been made to instance variables.
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

// Checks whether the given matchup is valid.
bool Scheduler::checkMatchup(int week, std::string entity, std::string opponent) {
    // Avoid scheduling an entity against itself.
    if (entity.compare(opponent) == 0) {
        return false;
    }

    // Check if any matchups remain between entity
    // and opponent.
    if (constraints[entity][opponent] <= 0) {
        return false;
    }
    
    // Check if opponent already has a scheduled matchup
    // this week.
    if (schedule[week - 1][opponent] != "") {
        return false;
    }

    // Check whether entity and opponent play during the
    // previous `self.weeksBetweenMatchups` weeks or the 
    // next `self.weeksBetweenMatchups` weeks.
    int startIndex = std::max(week - 1 - weeksBetweenMatchups, 0);
    int endIndex = std::min(week - 1 + weeksBetweenMatchups, weeks - 1);
    for (int i = startIndex; i <= endIndex; i++) {
        if (schedule[i][entity].compare(opponent) == 0) {
            return false;
        }
    }

    return true;
}

// Checks whether the created schedule meets the given constraints.
bool Scheduler::validateSchedule() {
    Constraints testConstraints;

    // Check whether the schedule length differs from the requested
    // number of weeks.
    if (schedule.size() != weeks) {
        return false;
    }

    for (int week = 1; week <= weeks; week++) {
        for (const auto &matchup : schedule[week - 1]) {
            std::string entity = matchup.first;
            std::string opponent = matchup.second;

            // Keep track of how many times each entity is
            // matched up against each of the other entities.
            testConstraints[entity][opponent] += 1;

            // Check whether any matchup pair exists more than once
            // in any `self.weeksBetweenMatchups + 1` week span.
            int startIndex = std::max(week - 1 - weeksBetweenMatchups, 0);
            int endIndex = std::min(week - 1 - weeksBetweenMatchups, weeks - 1);
            for (int i = startIndex; i <= endIndex; i++) {
                if (schedule[i][entity].compare(opponent) == 0) {
                    return false;
                }
            }
        }
    }

    // Check whether each entity is matched up against each of
    // the other entities the correct number of times.
    for (const auto &e : testConstraints) {
        std::string entity = e.first;
        
        for (const auto &c : e.second) {
            std::string opponent = c.first;
            int numMatchups = c.second;

            if (numMatchups != constraintsCheck[entity][opponent]) {
                return false;
            }
        }
    }

    return true;
}