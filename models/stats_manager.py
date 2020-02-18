import threading

class StatsManager:
   
    __lock = threading.Lock()
    
    # private class instance may not necessarily need name-mangling
    __instance = None
    
    @classmethod
    def instance(cls):
        if not cls.__instance:
            with cls.__lock:
                if not cls.__instance:
                    cls.__instance = cls()
        return cls.__instance
    
    def __init__(self) :
        self.__stats = None
        self.__lock = threading.Lock()

    
    def get_stats(self):
        self.__lock.acquire()
        message = self.__stats
        self.__lock.release()
        return message

    def set_stats(self, stats):
        self.__lock.acquire()
        self.__stats = stats
        self.__lock.release()