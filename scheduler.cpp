#include "scheduler.h"

Scheduler::Scheduler(int w, std::vector<std::string> e, Constraints c, std::vector<Matchups> sc, int wbm, std::string lp, std::string t) {    
    weeks = w;
    entities = e;
    constraints = c;
    constraintsCheck = c;
    scheduleConstraints = sc;
    weeksBetweenMatchups = wbm;
    logoPath = lp;
    title = t;
}

void Scheduler::initializeSchedule() {
    schedule.clear();
    constraints = constraintsCheck;

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
    for (int index = 1; index <= n; index++) {
        srand(time(0));
        initializeSchedule();
        insertScheduleConstraints();
        scheduleWeek(1);

        if (validateSchedule()) {
            int score = scoreSchedule();
            printSchedule();

            std::string filePath = "output/schedule" + std::to_string(index);
            generateOutput(filePath);
        } else {
            std::cout << "Not a valid schedule" << std::endl;
        }
    }
}

void Scheduler::insertScheduleConstraints() {
    for (int index = 0; index < weeks; index++) {
        if (scheduleConstraints.size() <= index) {
            break;
        }

        for (const auto &matchup : scheduleConstraints[index]) {
            std::string entity = matchup.first;
            std::string opponent = matchup.second;

            schedule[index][entity] = opponent;
            constraints[entity][opponent]--;
        }
    }
}

// This method schedules a matchup for all entities
// for the given week.
void Scheduler::scheduleWeek(int week) {
    if (week > weeks) {
        return;
    } else if (
        scheduleConstraints.size() > week - 1 &&
        scheduleConstraints[week - 1].size() > 0
    ) {
        // If the week is determined by schedule constraints,
        // move to the next week.
        return scheduleWeek(week + 1);
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
        if (
            scheduleConstraints.size() > week - 1 &&
            scheduleConstraints[week - 1].size() > 0
        ) {
            // Do not modify weeks that are determined by
            // schedule constraints.
            continue;
        }

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

    // Check that any schedule constraints are met.
    for (int index = 0; index < scheduleConstraints.size(); index++) {
        for (const auto &matchup : scheduleConstraints[index]) {
            std::string entity = matchup.first;
            std::string opponent = matchup.second;

            if (schedule[index][entity].compare(opponent) != 0) {
                return false;
            }
        }
    }

    return true;
}

void Scheduler::generateOutput(std::string filePath) {
    generateCsv(filePath);
    generatePdf(filePath);
}

void Scheduler::generateCsv(std::string filePath) {
    std::ofstream file(filePath + ".csv");

    file << "Week";
    for (const auto &entity : entities) {
        file << "," + entity;
    }
    file << "\n";

    for (int week = 1; week <= weeks; week++) {
        file << week;
        for (const auto &entity : entities) {
            file << "," << schedule[week - 1][entity];
        }
        file << "\n";
    }

    file.close();
}

void Scheduler::generatePdf(std::string filePath) {
    std::string columnWidth = std::to_string(std::floor(100 / (entities.size() + 1)));

    std::string headerHtml = "<th style='width:" + columnWidth + "%;'>Week</th>";
    for (const auto &entity : entities) {
        headerHtml += "<th style='width:" + columnWidth + "%;'>" + entity + "</th>";
    }

    std::string tableHtml = "";
    for (int week = 1; week <= weeks; week++) {
        tableHtml += "<tr><td>" + std::to_string(week) + "</td>";

        for (const auto &entity : entities) {
            tableHtml += "<td>" + schedule[week - 1][entity] + "</td>";
        }

        tableHtml += "</tr>";
    }    

    std::unordered_map<std::string, std::string> mapping = {
        { "%%LOGO_PATH%%", logoPath },
        { "%%TITLE%%", title },
        { "%%HEADER%%", headerHtml },
        { "%%TABLE%%", tableHtml },
    };

    std::ifstream templateFile("schedule-template.html");
    std::ofstream htmlFile("schedule.html");

    std::string line;
    while (std::getline(templateFile, line, '\n')) {
        for (const auto &pair : mapping) {
            while (true) {
                std::string templateTag = pair.first;
                int index = line.find(templateTag);

                if (index >= 0) {
                    std::string replacement = pair.second;
                    line.replace(index, templateTag.length(), replacement);
                } else {
                    break;
                }
            }
        }

        htmlFile << line << "\n";
    }

    templateFile.close();
    htmlFile.close();

    std::string command = "wkhtmltopdf --enable-local-file-access schedule.html " + filePath + ".pdf";
    system(command.c_str());

    // Delete the created html file.
    system("rm schedule.html");
}

int Scheduler::scoreSchedule() {
    std::ifstream scoringCriteriaFile("data/scoring-criteria.txt");

    Criteria scoringCriteria;
    std::string scoringCriteriaLine;
    int week = -1;
    while (std::getline(scoringCriteriaFile, scoringCriteriaLine, '\n')) {
        if (scoringCriteriaLine.size() > 0) {
            int delimiterIndex = scoringCriteriaLine.find("|");

            if (delimiterIndex < 0) {
                week = stoi(scoringCriteriaLine);

                if (scoringCriteria.find(week) == scoringCriteria.end()) {
                    scoringCriteria[week] = {};
                }
            } else {
                std::string entity = scoringCriteriaLine.substr(0, delimiterIndex);
                std::string opponent = scoringCriteriaLine.substr(delimiterIndex + 1);

                if (week > -1) {
                    scoringCriteria[week][entity] = opponent;
                    scoringCriteria[week][opponent] = entity;
                }
            }
        }
    }

    scoringCriteriaFile.close();

    int score = 0;
    for (int week = 1; week <= weeks; week++) {
        if (scoringCriteria.find(week) != scoringCriteria.end()) {
            for (auto it = scoringCriteria[week].begin(); it != scoringCriteria[week].end(); ++it) {
                if (schedule[week - 1][it->first] == it->second) {
                    ++score;
                }
            }
        }
    }

    return score;
}
