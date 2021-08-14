import csv
from random import shuffle
from copy import deepcopy
from pathlib import Path
from weasyprint import HTML

import sys
sys.setrecursionlimit(20000)

class Scheduler:

    def __init__(self, weeks, entities, constraints, weeksBetweenMatchups):
        self.validateArgs()

        self.weeks = weeks
        self.entities = entities
        self.constraints = constraints
        self.weeksBetweenMatchups = weeksBetweenMatchups

        # Save a copy of the constraints to use for validation. 
        self.constraintsCheck = deepcopy(constraints)
        
        self.schedule = dict([(week + 1, dict([(entity, '') for entity in entities])) for week in range(weeks)])

    def createSchedules(self, numSchedules, logo, title, filePath):
        for index in range(numSchedules):
            self.scheduleWeek(1)
            self.fileName = f'schedule-{index + 1}'

            if self.validateSchedule():
                self.generateOutput(logo, title, filePath)
            else:
                print('Not a valid schedule.')

    # This function schedules a matchup for all entities
    # for the given week. 
    def scheduleWeek(self, week):
        if week > self.weeks:
            return 

        # Keep track of which entities have not been
        # scheduled this week. 
        unscheduledEntities = deepcopy(self.entities)

        # Iterate over the entities that do not have a
        # scheduled matchup this week. 
        while len(unscheduledEntities) > 0:
            entity = unscheduledEntities[0]
            opponent = self.getOpponent(week, entity)

            if opponent != None:
                self.alterSchedule(week, entity, opponent, unscheduledEntities)
            else:
                # If there are no possible opponents, we need to 
                # backtrack, since this is a dead-end. 
                backtrack = 4
                newWeek = max([week - backtrack, 1])
                self.cleanup(week, newWeek)
                return self.scheduleWeek(newWeek)

            # When all entities have a scheduled matchup
            # this week, advance to the next week. 
            if len(unscheduledEntities) == 0:
                return self.scheduleWeek(week + 1)

    # Returns a randomly selected opponent, or None
    # if there are no valid matchups. 
    def getOpponent(self, week, entity):
        possibleOpponents = [opponent for opponent in self.constraints[entity] 
                             if self.checkMatchup(week, entity, opponent)]

        # Choose a random opponent from the list
        # of possible opponents. 
        if len(possibleOpponents) > 0:
            shuffle(possibleOpponents)
            return possibleOpponents[0]
        else:
            return None

    # Saves the matchup of entity vs. opponent for the given week, and 
    # removes both entities from the list of unscheduled entities. 
    def alterSchedule(self, week, entity, opponent, unscheduledEntities):
        self.schedule[week][entity] = opponent
        self.schedule[week][opponent] = entity
        
        # Both entity and opponent now have
        # scheduled matchups this week. 
        unscheduledEntities.remove(entity)
        unscheduledEntities.remove(opponent)

        # Decrement the number of times entity
        # and opponent need to play each other. 
        self.constraints[entity][opponent] -= 1
        self.constraints[opponent][entity] -= 1

    # When the search hits a dead-end, backtrack. 
    def cleanup(self, currentWeek, newWeek):
        # Iterate over the weeks between the new week 
        # and the current week, and undo any changes that 
        # have been made to instance variables. 
        for week in range(newWeek, currentWeek + 1):
            for entity in self.schedule[week]:
                opponent = self.schedule[week][entity]
                if opponent != '':
                    self.constraints[entity][opponent] += 1
                    self.schedule[week][entity] = ''

    # Checks whether the given matchup is valid. 
    def checkMatchup(self, week, entity, opponent):
        
        # Avoid scheduling an entity against itself. 
        if entity == opponent:
            return False

        # Check if any matchups remain between entity 
        # and opponent. 
        if self.constraints[entity][opponent] <= 0:
            return False

        # Check if opponent already has a scheduled matchup
        # this week. 
        if self.schedule[week][opponent] != '':
            return False

        # Check whether entity and opponent played during the
        # last 2 weeks. 
        startWeek = max(week - self.weeksBetweenMatchups, 1)
        if any([self.schedule[w][entity] == opponent for w in range(startWeek, week)]):
            return False

        return True

    # Checks whether the arguments to the constructor are valid. 
    def validateArgs(self):
        pass

    # Checks whether the created schedule meets the given constraints. 
    def validateSchedule(self):
        valid = True
        constraintsCheck = dict([(entity, dict([(opponent, 0) 
                            for opponent in self.entities if entity != opponent])) 
                            for entity in self.entities])

        # Check whether the schedule length differs from the requested
        # number of weeks. 
        if len(self.schedule) != self.weeks:
            valid = False

        for week in self.schedule:
            for entity in self.schedule[week]:
                opponent = self.schedule[week][entity]

                # Keep track of how many times each entity is 
                # matched up against each of the other entities. 
                constraintsCheck[entity][opponent] += 1

                # Check whether any matchup pair exists more than once
                # in any (self.weeksBetweenMatchups + 1) week span. 
                startWeek = max(week - self.weeksBetweenMatchups, 1)
                if any([self.schedule[w][entity] == opponent for w in range(startWeek, week)]):
                    valid = False
                    break
            
            if not valid:
                break

        # Check whether each entity is matched up against each of 
        # the other entities the correct number of times. 
        for entity in constraintsCheck:
            if not all([constraintsCheck[entity][opponent] == self.constraintsCheck[entity][opponent] for opponent in constraintsCheck[entity]]):
                valid = False
        
        return valid

    def generateOutput(self, logo, title, filePath):
        self.generateCsv(filePath)
        self.generatePdf(logo, title, filePath)

    def generateCsv(self, filePath):
        with open(f'{filePath}{self.fileName}.csv', 'w') as f:
            fields = ['Week']
            fields += list(self.schedule[1].keys())
            writer = csv.DictWriter(f, fieldnames=fields)

            writer.writeheader()
            for week in self.schedule:
                weekSchedule = deepcopy(self.schedule[week])
                weekSchedule['Week'] = week
                writer.writerow(weekSchedule)

    def generatePdf(self, logo, title, filePath):
        header = ''.join([f'<th style="width:12%;">{entity}</th>' for entity in self.entities])
        table = ''.join([f"""<tr><td>{week}</td>
                {"".join([f"<td>{self.schedule[week][entity]}</td>" 
                for entity in self.entities])}</tr>""" 
                for week in range(1, self.weeks + 1)])

        html = f"""
        <!doctype html>
        <html>
        <head>
            <style>
                #logo {{
                    width: 100px;
                    display: inline-block; 
                }}

                #title {{
                    font-family: Times New Roman;
                    float: right;
                    display: inline-block;
                    text-align: right;
                    margin-right: 100px;
                }}

                #schedule {{
                    font-family: Times New Roman;
                    border-collapse: collapse;
                    width: 100%;
                    margin-top: 50px;
                }}

                #schedule td, #schedule th {{
                    border: 1px solid #DDD;
                    padding: 8px;
                    text-align: center;
                    height: 30px;
                }}

                #schedule tr:nth-child(even) {{ background-color: #F2F2F2; }}

                #schedule th {{
                    padding-top: 12px;
                    padding-bottom: 12px;
                    background-color: #0000CD;
                    color: #FFF;
                }}
            </style>
        </head>
        <body>
            <div>
                <img id="logo" src="{logo}">
                <h1 id="title">{title}</h1>
            </div>
            <table id="schedule">
                <thead>
                    <tr>
                        <th style="width:12%;">Week</th>
                        {header}
                    </tr>
                </thead>
                <tbody>
                    {table}
                </tbody>
            </table>
        </body>
        </html>
        """

        htmldoc = HTML(string=html, base_url="")
        pdf = htmldoc.write_pdf()
        Path(f'{filePath}{self.fileName}.pdf').write_bytes(pdf)

    def printSchedule(self):
        for week in self.schedule:
            print(f'Week {week}')
            for entity in self.schedule[week]:
                print(f'\t{entity} - {self.schedule[week][entity]}')