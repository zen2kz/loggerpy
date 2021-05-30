import os
from flask import Flask, render_template
from flask.helpers import send_file
from models.stats_manager import StatsManager
from models.downloads import Downloads

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

@app.route('/settings')
def settings():
    return render_template('./settings.html' )

@app.route('/camera')
def camera():
    user = "testpy"
    return render_template('./camera.html', name = user)


@app.route('/files/<name>')
def files(name):
    path = os.getcwd()+ "/logs/"+ name
    return send_file(path, as_attachment=True)