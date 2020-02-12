import _thread
import time

def scan_image():
    scan = True
    while scan :
        print ("scan")
        time.sleep(0.5)

def start_scan(val):
    print('test 11')
    try:
        _thread.start_new_thread(scan_image, ()) #, ("Thread-1", 2, ) 
    except Exception as ex:
        print(ex)