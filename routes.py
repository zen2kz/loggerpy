from models.wifi import Wifi
import os
from flask import Flask, render_template, request, redirect
from flask.helpers import make_response, send_file
from models.stats_manager import StatsManager
from models.downloads import Downloads
from models.settings import Settings, Shift
from config import config

app = Flask(__name__)

@app.route('/')
def home():
    stats = StatsManager.instance().get_stats()
    return render_template('./index.html', stats = stats)

@app.route('/downloads')
def downloads():
    downloads = Downloads()
    files = downloads.get_files()
    return render_template('./downloads.html', files = files)

@app.route('/shifts', methods = ['POST'])
def save_shifts():
    try:
        i = 1 
        cfg = config()
        while i<=3:
            enabled = request.form.get("hasShift{idx}".format(idx = i), False)
            name =  request.form["shift{idx}name".format(idx = i)]
            time =  request.form["shift{idx}time".format(idx = i)]
            hm = time.split(':')
            cfg["shifts"][i-1]["enabled"].set(enabled=="on")
            cfg["shifts"][i-1]["name"].set(name)
            cfg["shifts"][i-1]["start_time_hrs"].set(hm[0])
            if len(hm)>1:
                cfg["shifts"].items
                cfg["shifts"][i-1]["start_time_minutes"].set(hm[1])
            else:
                cfg["shifts"][i-1]["start_time_minutes"].set(0)
            i = i +1
        yml = cfg.dump(True)

        workdir = os.getcwd()
        text_file = open(workdir+'/config.yaml', "w")
        text_file.write(yml)
        text_file.close()

        return redirect("/settings", code=302)
    except Exception as e:
        print(e)
        return make_response(e, 400)

@app.route('/settings')
def settings():
    wifi = Wifi()
    cfg = config()
    settings = Settings(cfg)

    return render_template('./settings.html', networks = wifi.get_networks(), shifts = settings.get_shifts() )


@app.route('/camera')
def camera():
    user = "testpy"
    return render_template('./camera.html', name = user)


@app.route('/files/<name>')
def files(name):
    path = os.getcwd()+ "/logs/"+ name
    return send_file(path, as_attachment=True)

@app.errorhandler(404)
def handle_404(e):
    # handle all other routes here
    return 'Not Found, but we HANDLED IT'