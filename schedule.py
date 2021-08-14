import os
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from bs4 import BeautifulSoup
from Scheduler import Scheduler
from config import DRIVER_PATH, UPDATE, LEAGUE_ID, LOGO_PATH, TITLE, OUTPUT_PATH

def scrape(url, filename):
    options = Options()
    options.headless = True

    driver = webdriver.Chrome(options=options, executable_path=DRIVER_PATH)

    driver.get(url)

    with open(filename, 'w') as f:
        f.write(driver.page_source)

    driver.quit()

# Return dictionary of team IDs and manager names. 
def getManagers():
    url = 'https://fantasy.nfl.com/league/' + LEAGUE_ID + '/owners'
    filename = os.path.dirname(os.path.realpath(__file__)) + '/data/managers.txt'

    # Scrape fresh data, if desired. 
    if UPDATE:
        scrape(url, filename)

    f = open(filename, 'r')

    txt = f.read()
    html = BeautifulSoup(txt, 'html.parser')

    data = html.find_all('a', { 'class': 'teamName' })

    managers = {}
    for d in data:
        manager = d.parent.parent.nextSibling.text
        managers[d['class'][1][-1]] = manager

    f.close()

    return managers

def getStandings(year):
    url = f'https://fantasy.nfl.com/league/{LEAGUE_ID}/history/{year}/standings?historyStandingsType=regular'
    filename = os.path.dirname(os.path.realpath(__file__)) + '/data/standings.txt'

    # Scrape fresh data, if desired. 
    if UPDATE:
        scrape(url, filename)

    f = open(filename, 'r')

    txt = f.read()
    html = BeautifulSoup(txt, 'html.parser')

    data = html.find_all('span', { 'class': 'teamRank' })

    standings = {}
    for d in data:
        if '(' not in d.text:
            standings[d['class'][1][-1]] = int(d.text)

    f.close()

    return standings

previousYear = 2020
standings = getStandings(previousYear)
managers = getManagers()

constraints = {}
for key in managers:
    entity = managers[key]
    constraints[entity] = {}
    
    for opponent in managers:
        if key != opponent:
            if standings[key] == standings[opponent]:
                constraints[entity][managers[opponent]] = 1
            else:
                constraints[entity][managers[opponent]] = 2

weeks = 13
entities = list(managers.values())
weeksBetweenMatchups = 2
numSchedulesToGenerate = 10

scheduler = Scheduler(weeks, entities, constraints, weeksBetweenMatchups)
scheduler.createSchedules(numSchedulesToGenerate, LOGO_PATH, TITLE, OUTPUT_PATH)