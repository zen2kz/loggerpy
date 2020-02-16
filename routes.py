from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def home():
    user = "testpy"
    return render_template('./index.html', name = user)

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