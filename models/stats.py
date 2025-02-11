import csv
from datetime import datetime, timedelta
from models.csv_file import CsvFile
import sys
from datetime import date, datetime, timedelta

class Stats:
    __partsProcessed = 0
    __idleTimeMills = 0
    __processingTimeMills = 0
    __lastEvent = datetime.now()
    __lastIdleMills = 0

    __longestIdleTimeMills = 0
    __shortestIdleTimeMills = 0
    __longestProcessingTimeMills = 0
    __shortestProcessingTimeMills = 0

    __state = "idle"
    __cfg = None
    __csv_file = None

    def __init__(self, cfg) :
        self.__cfg = cfg
        self.__csv_file = CsvFile(cfg)

    def get_shift(self): 
        now = datetime.now()
        shift_name = "none"
        if self.__cfg["shifts"]!= None:
            i = 0
            prevDay = True
            for sh in self.__cfg["shifts"]:
                i=i+1
                enabled = sh["enabled"].get()
                if enabled:
                    hrs = sh["start_time_hrs"].get()
                    min = sh["start_time_minutes"].get()
                    d = datetime(now.year, now.month, now.day, hrs, min)
                    td = now-d
                    if td.total_seconds()>0:
                        shift_name = sh["name"].get()
                        prevDay = False
                    else:
                        break

            # if prevDay - get last shift from previous day
            if prevDay:
                for sh in self.__cfg["shifts"]:
                    enabled = sh["enabled"].get()
                    if enabled:
                        shift_name = sh["name"].get()
        
                
        return shift_name, prevDay

    def processed(self):
        self.__partsProcessed += 1
        now = datetime.now()
        tdelta = now - self.__lastEvent
        newDuration = tdelta.total_seconds()*1000.0
        print("processed:"+ str(newDuration))
        self.__processingTimeMills += newDuration
        self.__lastEvent = now

        if self.__longestProcessingTimeMills < newDuration :
            self.__longestProcessingTimeMills = newDuration
        if self.__shortestProcessingTimeMills == 0 :
            self.__shortestProcessingTimeMills = newDuration
        elif self.__shortestProcessingTimeMills > newDuration :
            self.__shortestProcessingTimeMills = newDuration

        self.__state = "idle"

        shift, prevDay = self.get_shift()
        self.__csv_file.write(shift, prevDay, str(self.__partsProcessed), str(newDuration/1000.0), self.__lastEvent.strftime("%m/%d/%Y"), self.__lastEvent.strftime("%H:%M:%S"),  now.strftime("%m/%d/%Y"), now.strftime("%H:%M:%S"), str(self.__lastIdleMills/1000.0))
        self.__csv_file.write_summary(shift, prevDay, self)
    
    def startProcessing(self):
        now = datetime.now()
        tdelta = now - self.__lastEvent
        newDuration = tdelta.total_seconds()*1000.0
        print("start processing:"+ str(newDuration))
        self.__lastIdleMills = newDuration
        self.__idleTimeMills += newDuration
        self.__lastEvent = now

        if self.__longestIdleTimeMills < newDuration :
            self.__longestIdleTimeMills = newDuration
        if self.__shortestIdleTimeMills == 0 :
            self.__shortestIdleTimeMills = newDuration
        elif self.__shortestIdleTimeMills > newDuration :
            self.__shortestIdleTimeMills = newDuration
        
        self.__state = "processing"


    def getPartsProcessed(self):
        return self.__partsProcessed

    def getIdleTimeMinutes(self) :
        return self.__idleTimeMills / (1000.0 * 60.0)

    def getIdleTimePercent(self): 
        if self.__idleTimeMills + self.__processingTimeMills == 0:
            return 0.0
        return (self.__idleTimeMills / (self.__idleTimeMills + self.__processingTimeMills)) * 100.0 

    def getProcessingTimeMinutes(self): 
        return self.__processingTimeMills / (1000.0 * 60.0)

    def getProcessingTimePercent(self): 
        if self.__idleTimeMills + self.__processingTimeMills == 0:
            return 0.0
        return (self.__processingTimeMills / (self.__idleTimeMills + self.__processingTimeMills)) * 100.0

    def getTotalTimeMinutes(self):
        return (self.__processingTimeMills+self.__idleTimeMills) / (1000.0 * 60.0)

    def getLongestIdleTimeSeconds(self):
        return self.__longestIdleTimeMills / 1000.0

    def getShortestIdleTimeSeconds(self):
        return self.__shortestIdleTimeMills / 1000.0

    def getLongestProcessingTimeSeconds(self):
        return self.__longestProcessingTimeMills / 1000.0

    def getShortestProcessingTimeSeconds(self):
        return self.__shortestProcessingTimeMills / 1000.0

    def getAverageProcessingTimeSeconds(self):
        if self.__partsProcessed == 0 :
            return 0.0
        return (self.__processingTimeMills / 1000.0) / self.__partsProcessed

    def getUpdatedAt(self): 
        return self.__lastEvent

    def getFileName(self): 
        shift, prevDay = self.get_shift()
        return self.__csv_file.file_name(shift, prevDay, False)

    def getShiftID(self):
        shift, _ = self.get_shift()
        return shift

    def getState(self):
        return self.__state