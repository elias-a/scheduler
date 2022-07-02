#pragma once

#include <string>
#include <vector>
#include <unordered_map>

typedef std::unordered_map<std::string, std::unordered_map<std::string, int>> Constraints;
typedef std::unordered_map<std::string, std::string> Matchups;

class Scheduler {
    public:
        Scheduler(int w, std::vector<std::string> e, Constraints c, std::vector<Matchups> sc, int wbm);
        void createSchedules(int n);
        void printSchedule();
    private:
        int weeks;
        std::vector<std::string> entities;
        Constraints constraints;
        std::vector<Matchups> scheduleConstraints;
        int weeksBetweenMatchups;
        Constraints constraintsCheck;
        std::vector<Matchups> schedule;

        void initializeSchedule();
        void insertScheduleConstraints();
        void scheduleWeek(int w);
        std::string getOpponent(int w, std::string e);
        void alterSchedule(int w, std::string e, std::string o, std::vector<std::string> &ue);
        void cleanup(int w, int n);
        bool checkMatchup(int w, std::string e, std::string o);
        bool validateSchedule();
};
