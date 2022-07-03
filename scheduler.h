#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cmath>

typedef std::unordered_map<std::string, std::unordered_map<std::string, int> > Constraints;
typedef std::unordered_map<std::string, std::string> Matchups;

class Scheduler {
    public:
        Scheduler(int w, std::vector<std::string> e, Constraints c, std::vector<Matchups> sc, int wbm, std::string lp, std::string t);
        void createSchedules(int n);
        void printSchedule();
        void generateOutput(std::string fp);
        void generateCsv(std::string fp);
        void generatePdf(std::string fp);
    private:
        int weeks;
        std::vector<std::string> entities;
        Constraints constraints;
        std::vector<Matchups> scheduleConstraints;
        int weeksBetweenMatchups;
        Constraints constraintsCheck;
        std::vector<Matchups> schedule;
        std::string logoPath;
        std::string title;

        void initializeSchedule();
        void insertScheduleConstraints();
        void scheduleWeek(int w);
        std::string getOpponent(int w, std::string e);
        void alterSchedule(int w, std::string e, std::string o, std::vector<std::string> &ue);
        void cleanup(int w, int n);
        bool checkMatchup(int w, std::string e, std::string o);
        bool validateSchedule();
};
