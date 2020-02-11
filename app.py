from flask import Flask, render_template

app = Flask(__name__)

#@app.route('/')
#def index():
#        return 'Hello world'

@app.route('/')
def hello_name():
    user = "testpy"
    return render_template('./index.html', name = user)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')