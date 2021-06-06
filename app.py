from flask import Flask
import _thread
from config import config
import cam_scanner
import confuse
import os
from routes import app
from models.stats_manager import StatsManager
from models.stats import Stats
import platform

started = False

def run(cfg):
    if not started:
        print("run")
        StatsManager.instance().set_stats(Stats(cfg))
        try:
            mock = platform.processor == "x86_64"
            cam_scanner.start_scan(mock)
        except Exception as ex:
            print(ex)
    else: 
        print("already started")


if __name__ == '__main__':
    cfg = config()
    run(cfg)
    app.run(debug=False, host='0.0.0.0')