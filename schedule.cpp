#include <ctime>
#include <toml.hpp>
#include "scheduler.h"
#include "nfl.h"

int main() {
    const auto config = toml::parse("config.toml");
    const auto &leagueConfig = toml::find(config, "LEAGUE");
    const std::string leagueId = toml::find<std::string>(leagueConfig, "LEAGUE_ID");
    const auto &scheduleConfig = toml::find(config, "SCHEDULE");
    const bool update = toml::find<bool>(scheduleConfig, "UPDATE_DATA");
    const int weeks = toml::find<int>(scheduleConfig, "NUM_WEEKS");
    const int weeksBetweenMatchups = toml::find<int>(scheduleConfig, "WEEKS_BETWEEN_MATCHUPS");
    const int numSchedules = toml::find<int>(scheduleConfig, "NUM_SCHEDULES");
    const auto &outputConfig = toml::find(config, "OUTPUT");
    const std::string logoPath = toml::find<std::string>(outputConfig, "LOGO_PATH");
    const std::string title = toml::find<std::string>(outputConfig, "SCHEDULE_TITLE");

    std::time_t time = std::time(nullptr);
    const std::tm *timeInfo = std::localtime(&time);
    int previousYear = 1900 + timeInfo->tm_year - 1;

    Nfl nfl(leagueId, update, previousYear);
    std::vector<std::string> entities = nfl.getManagers();
    Constraints matchupConstraints = nfl.getMatchupConstraints();

    std::vector<Matchups> scheduleConstraints = nfl.getScheduleConstraints(weeks);

    Scheduler scheduler(
        weeks,
        entities,
        matchupConstraints,
        scheduleConstraints,
        weeksBetweenMatchups,
        logoPath,
        title);
    scheduler.createSchedules(numSchedules);

    return 0;
}
