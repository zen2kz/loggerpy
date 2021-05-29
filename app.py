from flask import Flask
import _thread
import cam_scanner
import confuse
import os
from routes import app
from models.stats_manager import StatsManager
from models.stats import Stats

started = False

def run(cfg):
    if not started:
        print("run")
        StatsManager.instance().set_stats(Stats(cfg))
        try:
            cam_scanner.start_scan()
        except Exception as ex:
            print(ex)
    else: 
        print("already started")

def config():
    config = confuse.Configuration('PartsLogger', __name__)
    workdir = os.getcwd()
    config.set_file(workdir+'/config.yaml')
    return config


if __name__ == '__main__':
    cfg = config()
    run(cfg)
    app.run(debug=False, host='0.0.0.0')