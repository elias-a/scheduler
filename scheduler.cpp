#include "scheduler.h"

Scheduler::Scheduler(
  int weeks_,
  std::vector<std::string> entities_,
  MatchupConstraints constraints_,
  ScheduleConstraints scheduleConstraints_,
  int weeksBetweenMatchups_,
  std::string logoPath_,
  std::string title_
) : weeks(weeks_)
  , entities(entities_)
  , constraints(constraints_)
  , constraintsCheck(constraints_)
  , scheduleConstraints(scheduleConstraints_)
  , weeksBetweenMatchups(weeksBetweenMatchups_)
  , logoPath(logoPath_)
  , title(title_) {
    cleanOutputDirectory("output");
    loadScoringCriteria("data/scoring-criteria.txt");
}

void Scheduler::cleanOutputDirectory(std::string outputPath) {
  std::filesystem::path outputDir(outputPath);
  for (const auto& entry : std::filesystem::directory_iterator(outputDir)) {
    std::filesystem::remove_all(entry.path());
  }
}

std::string Scheduler::createScheduleID(int n) {
  std::string id;
  ++n;
  while (n > 0) {
    int r = (n - 1) % 26;
    id = static_cast<char>('A' + r) + id;
    n = (n - 1) / 26;
  }
  return id;
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

void Scheduler::printSchedule(Schedule& sched) {
    for (int week = 1; week <= weeks; week++) {
        std::cout << "Week " << week << std::endl;

        for (const auto &matchup : sched[week - 1]) {
            std::cout << "\t";
            std::cout << matchup.first << " - ";
            std::cout << matchup.second << std::endl;
        }
    }
}

void Scheduler::createSchedules(int n) {
  srand(time(0));

  std::vector<ScoredSchedule> schedules;
  while (schedules.size() < n) {
        initializeSchedule();
        insertScheduleConstraints();
        scheduleWeek(1);

        if (validateSchedule()) {
          bool alreadyFound = std::any_of(schedules.begin(), schedules.end(),
                            [this](const ScoredSchedule& sched) {
                                return sched.schedule == schedule;
                            });
          if (alreadyFound) continue;

          ScoredSchedule scoredSchedule = scoreSchedule(schedule);
          schedules.push_back(scoredSchedule);
        } else {
            std::cout << "Not a valid schedule" << std::endl;
        }
    }

  std::sort(schedules.begin(), schedules.end(), 
            [](const ScoredSchedule& sched1, const ScoredSchedule& sched2) {
              return sched1.score > sched2.score;
            });

  int numFinalSchedules = std::min(n, 10);
  for (int i = 0; i < numFinalSchedules; ++i) {
    //printScoring(schedules[i]);
    //printSchedule(schedules[i].schedule);
    std::string filePath = "output/schedule" + createScheduleID(i) + "-" + std::to_string(schedules[i].score);
    generateOutput(schedules[i], filePath);
  }
}

void Scheduler::insertScheduleConstraints() {
  for (auto [week, matchups] : scheduleConstraints) {
    for (auto [entity, opponent] : matchups) {
      schedule[week - 1][entity] = opponent;
      --constraints[entity][opponent];
    }
  }
}

// This method schedules a matchup for all entities
// for the given week.
void Scheduler::scheduleWeek(int week) {
    if (week > weeks) {
        return;
    } else if (scheduleConstraints.contains(week)) {
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
      if (scheduleConstraints.contains(week)) {
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
    MatchupConstraints testConstraints;

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
    for (auto [week, matchups] : scheduleConstraints) {
      for (auto [entity, opponent] : matchups) {
        if (schedule[week - 1][entity] != opponent) {
          return false;
        }
      }
    }

    return true;
}

void Scheduler::generateOutput(ScoredSchedule& sched, std::string filePath) {
    generateCsv(sched, filePath);
    //generatePdf(filePath);
}

void Scheduler::generateCsv(ScoredSchedule& sched, std::string filePath) {
  std::ofstream file(filePath + ".csv");

  file << "Score: " << sched.score << "\n";
  file << "Matched Criteria:" << "\n";
  for (auto [week, entity1, entity2] : sched.matchedCriteria) {
    file << "\tWeek " << week << "\t" << entity1 << " vs. " << entity2 << "\n";
  }
  file << "\n";

  file << "Week";
  for (const auto &entity : entities) {
    file << "," + entity;
  }
  file << "\n";

  for (int week = 1; week <= weeks; week++) {
    file << week;
    for (const auto &entity : entities) {
      file << "," << sched.schedule[week - 1][entity];
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

void Scheduler::loadScoringCriteria(std::string scoringCriteriaPath) {
  std::ifstream scoringCriteriaFile(scoringCriteriaPath);

  int week = 0;  // Weeks start at 1
  std::string line;
  while (std::getline(scoringCriteriaFile, line, '\n')) {
    if (line.size() > 0) {
      int delimiterIndex = line.find("|");

      if (delimiterIndex < 0) {
        // This line is a week number.
        week = stoi(line);
      } else {
        // This line is a matchup, e.g., Team1|Team2.
        std::string entity = line.substr(0, delimiterIndex);
        std::string opponent = line.substr(delimiterIndex+1);

        // TODO: Check that `entity` and `opponent` are valid entities.

        if (week > 0) {
          scoringCriteria.emplace_back(week, entity, opponent);
        } else {
          throw std::invalid_argument(
            std::string("Error reading scoring criteria file: no week ") + 
            "is associated with the matchup " + entity + " vs. " + 
            opponent + "."
          );
        }
      }
    }
  }

  scoringCriteriaFile.close();
}

ScoredSchedule Scheduler::scoreSchedule(Schedule& sched) {
  int score = 0;
  Criteria matchedCriteria;

  for (auto [week, entity1, entity2] : scoringCriteria) {
    if (sched[week-1][entity1] == entity2) {
      ++score;
      matchedCriteria.emplace_back(week, entity1, entity2);
    }
  }

  return ScoredSchedule{sched, score, matchedCriteria};
}

void Scheduler::printScoring(ScoredSchedule& schedule) {
  std::cout << "Score: " << schedule.score << std::endl;
  std::cout << "Matched Criteria:" << std::endl;
  for (auto [week, entity1, entity2] : schedule.matchedCriteria) {
    std::cout << "\tWeek " << week << "\t" << entity1 << "\t" << entity2 << std::endl;
  }
}
