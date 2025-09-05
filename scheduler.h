#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <algorithm>

struct Matchup {
  int week;
  std::string entity1;
  std::string entity2;

  Matchup(int w, std::string e1, std::string e2) : week(w), entity1(e1), entity2(e2) {}
};

typedef std::unordered_map<std::string, std::unordered_map<std::string, int> > Constraints;
typedef std::unordered_map<std::string, std::string> Matchups;
typedef std::vector<Matchup> Criteria;
typedef std::vector<Matchups> Schedule;

struct ScoredSchedule {
  Schedule schedule;
  int score;
  Criteria matchedCriteria;
};

class Scheduler {
    public:
        Scheduler(int w, std::vector<std::string> e, Constraints c, std::vector<Matchups> sc, int wbm, std::string lp, std::string t);
        void createSchedules(int n);
        void printSchedule(Schedule& s);
        void generateOutput(ScoredSchedule& s, std::string fp);
        void generateCsv(ScoredSchedule& s, std::string fp);
        void generatePdf(std::string fp);
    private:
        int weeks;
        std::vector<std::string> entities;
        Constraints constraints;
        std::vector<Matchups> scheduleConstraints;
        int weeksBetweenMatchups;
        Criteria scoringCriteria;
        Constraints constraintsCheck;
        Schedule schedule;
        std::string logoPath;
        std::string title;

        void cleanOutputDirectory(std::string p);
        std::string createScheduleID(int n);
        void initializeSchedule();
        void insertScheduleConstraints();
        void scheduleWeek(int w);
        std::string getOpponent(int w, std::string e);
        void alterSchedule(int w, std::string e, std::string o, std::vector<std::string> &ue);
        void cleanup(int w, int n);
        bool checkMatchup(int w, std::string e, std::string o);
        bool validateSchedule();

        void loadScoringCriteria(std::string p);
        ScoredSchedule scoreSchedule(Schedule& s);
        void printScoring(ScoredSchedule& s);
};
