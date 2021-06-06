import _thread
import threading
import gpiozero
import time
from models.stats_manager import StatsManager
import gpiozero.pins.mock

def scan_image(mock):
    scan = True
    sensor = nil
    if mock:
        sensor = gpiozero.LineSensor(26, True, pin_factory=mock.MockFactory())
    else:
        sensor = gpiozero.LineSensor(26, True)
    statsMgr = StatsManager.instance()
    active = False
    while scan :
        stats = statsMgr.get_stats()
        if sensor.is_active:
            if active == False:
                stats.processed()
                statsMgr.set_stats(stats)
                active = True
        else:
            if active:
                stats.startProcessing()
                statsMgr.set_stats(stats)
                print ("scan",  threading.current_thread().ident )
                active = False

    
        time.sleep(0.1)


def start_scan(mock):
    try:
        id = _thread.start_new_thread(scan_image, (mock)) #, ("Thread-1", 1, )
        print("starting thread id=", id) 
    except Exception as ex:
        print(ex)