import sys
import os
import csv
from datetime import date
from pathlib import Path

class CsvFile:
    __workdir = ""
    __cfg = None

    def  __init__(self, cfg): 
        self.__cfg = cfg
        self.__workdir = os.getcwd()

    
    def file_name(self, shift, summary): 
        today = date.today()
        postfix = "" if not summary else "_summary" 
        return "{workdir}/logs/{date}_{shift}{postfix}.csv".format(workdir=self.__workdir, date=today.strftime("%Y%m%d"), shift = shift, postfix=postfix)

    def write(self, shift_id, part_no, processing_sec, dstart, tstart, dend, tend, idle_sec):
        fname = self.file_name(shift_id, False)
        path = Path(fname)
        add_header = not path.is_file()
            
        with open(fname, 'a') as csvfile:
            writer = csv.writer(csvfile, dialect=csv.excel)
            if add_header:
                writer.writerow(['Part#','Processing (sec)','Start Date','Start Time', 'End Date', 'End Time', 'Idle (sec)'])
            writer.writerow([part_no, processing_sec, dstart, tstart, dend, tend, idle_sec])

    def write_summary(self, shift_id, stats):
        fname = self.file_name(shift_id, True)
            
        with open(fname, 'w') as csvfile:
            writer = csv.writer(csvfile, dialect=csv.excel)
            
            writer.writerow(["Parts processed", stats.getPartsProcessed()])
            if stats.getIdleTimeMinutes()+stats.getProcessingTimeMinutes() !=0:
                writer.writerow(["Idle time (minutes)", '{0:.2g}'.format(stats.getIdleTimeMinutes()), '{0:.2g}'.format(stats.getIdleTimePercent())+"%"])
                writer.writerow(["Processing time (minutes)", '{0:.2g}'.format(stats.getProcessingTimeMinutes()), '{0:.2g}'.format(stats.getProcessingTimePercent())+"%"])
            else:
                writer.writerow(["Idle time (minutes)", '{0:.2g}'.format(stats.getIdleTimeMinutes())])
                writer.writerow(["Processing time (minutes)", '{0:.2g}'.format(stats.getProcessingTimeMinutes())])
            
            writer.writerow(["Total time (minutes)", '{0:.2g}'.format(stats.getTotalTimeMinutes())])
            writer.writerow(["Longest idle time (sec)", '{0:.2g}'.format(stats.getLongestIdleTimeSeconds())])
            if stats.getShortestIdleTimeSeconds()!=999999:
                writer.writerow(["Shortest idle time (sec)", '{0:.2g}'.format(stats.getShortestIdleTimeSeconds())])
            else: 
                writer.writerow(["Shortest idle time (sec)", 0])
            writer.writerow(["Longest processing time (sec)", '{0:.2g}'.format(stats.getLongestProcessingTimeSeconds())])
            if stats.getShortestProcessingTimeSeconds()!=999999:
                writer.writerow(["Shortest processing time (sec)", '{0:.2g}'.format(stats.getShortestProcessingTimeSeconds())])
            else: 
                writer.writerow(["Shortest processing time (sec)", 0])
            if stats.getPartsProcessed()>0 :
                writer.writerow(["Average processing time (sec)", '{0:.2g}'.format(stats.getAverageProcessingTimeSeconds())])
            else:
                writer.writerow(["Average processing time (sec)", 0])
