import _thread
import threading
import time
from models.stats_manager import StatsManager

def scan_image():
    scan = True
    statsMgr = StatsManager.instance()
    while scan :
        stats = statsMgr.get_stats()
        stats.startProcessing()
        statsMgr.set_stats(stats)
        print ("scan",  threading.current_thread().ident )
        time.sleep(3)
        stats.processed()
        statsMgr.set_stats(stats)
        time.sleep(1)


def start_scan():
    try:
        id = _thread.start_new_thread(scan_image, ()) #, ("Thread-1", 1, )
        print("starting thread id=", id) 
    except Exception as ex:
        print(ex)