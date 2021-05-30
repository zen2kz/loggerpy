from os import listdir
import os
from os.path import isfile, join

class Downloads:

    def get_files(self) :
        path = os.getcwd() + "/logs"
        files = []
        for f in listdir(path):
            if isfile(join(path, f)):
                files.append(f)
       
        return files