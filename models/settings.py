class Settings:
    __cfg = None

    def __init__(self, cfg) :
        self.__cfg = cfg

    def get_shifts(self): 
        shifts = {}
        if self.__cfg["shifts"]!= None:
            i = 0
            for sh in self.__cfg["shifts"]:
                shift = Shift()
                shift.enabled = True
                shift.name = sh["name"]
                shift.startsAt = str(sh["start_time_hrs"].get())+":"+str(sh["start_time_minutes"].get())
                shifts[i] = shift
                i = i+1

        if len(shifts)<3 :
            i = len(shifts)
            while i<3:
                shift = Shift()
                shift.enabled = False
                shifts[i] = shift
                i=i+1

        return shifts

class Shift:
    enabled = False
    name = ""
    startsAtHrs = ""
    startsAtMin = ""
