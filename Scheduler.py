import csv
from copy import deepcopy
from pathlib import Path
from weasyprint import HTML

class Scheduler:
    def __init__(self, entities, weeks, logoPath, title, outputPath):
        self._entities = entities
        self._weeks = weeks
        self._logoPath = logoPath
        self._title = title
        self._outputPath = outputPath

    def _readSchedule(self, index):
        return {}

    def generateOutput(self):
        for index in range(1):
            schedule = self._readSchedule(index)

            filePath = f"{self._outputPath}schedule-{index + 1}.csv"
            self.generateCsv(filePath, schedule)
            self.generatePdf(filePath, schedule)

    def generateCsv(self, filePath, schedule):
        with open(filePath, "wt") as f:
            fields = ["Week"]
            fields += list(schedule[1].keys())
            writer = csv.DictWriter(f, fieldnames=fields)

            writer.writeheader()
            for week in schedule:
                weekSchedule = deepcopy(schedule[week])
                weekSchedule["Week"] = week
                writer.writerow(weekSchedule)

    def generatePdf(self, filePath, schedule):
        header = "".join([f"<th style='width:12%;'>{entity}</th>" for entity in self._entities])
        table = "".join([f"""<tr><td>{week}</td>
                {"".join([f"<td>{schedule[week][entity]}</td>" 
                for entity in self._entities])}</tr>""" 
                for week in range(1, self._weeks + 1)])

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
                <img id="logo" src="{self._logoPath}">
                <h1 id="title">{self._title}</h1>
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
        Path(filePath).write_bytes(pdf)

    def _printSchedule(self, schedule):
        for week in schedule:
            print(f"Week {week}")
            for entity in schedule[week]:
                print(f"\t{entity} - {schedule[week][entity]}")
