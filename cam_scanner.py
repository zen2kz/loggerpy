import _thread
import time
from models.stats_manager import StatsManager

def scan_image():
    scan = True
    statsMgr = StatsManager.instance()
    while scan :
        stats = statsMgr.get_stats()
        stats.startProcessing()
        statsMgr.set_stats(stats)
        print ("scan")
        time.sleep(3)
        stats.processed()
        statsMgr.set_stats(stats)
        time.sleep(1)


def start_scan(val):
    try:
        _thread.start_new_thread(scan_image, ()) #, ("Thread-1", 2, ) 
    except Exception as ex:
        print(ex)