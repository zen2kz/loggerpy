import sys
import os
from datetime import date

class CsvFile:
    __workdir = ""
    __cfg = None

    def  __init__(self, cfg): 
        self.__cfg = cfg
        self.__workdir = os.getcwd()

    
    def file_name(self, shift): 
        today = date.today()
        return "{workdir}/logs/{date}_{shift}.csv".format(workdir=self.__workdir, date=today.strftime("%Y%m%d"), shift = shift)

    
