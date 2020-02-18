from flask import Flask
import _thread
import cam_scanner
from routes import app
from models.stats_manager import StatsManager
from models.stats import Stats

def run():
    StatsManager.instance().set_stats(Stats("testfile", "6AM"))
    try:
        cam_scanner.start_scan(True)
    except Exception as ex:
        print(ex)


if __name__ == '__main__':
    run()
    app.run(debug=True, host='0.0.0.0')