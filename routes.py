from flask import Flask, render_template
from models.stats_manager import StatsManager

app = Flask(__name__)

@app.route('/')
def home():
    stats = StatsManager.instance().get_stats()
    return render_template('./index.html', stats = stats)

@app.route('/downloads')
def downloads():
    user = "testpy"
    return render_template('./downloads.html', name = user)

@app.route('/settings')
def settings():
    user = "testpy"
    return render_template('./settings.html', name = user)

@app.route('/camera')
def camera():
    user = "testpy"
    return render_template('./camera.html', name = user)