from flask import Flask, render_template
import _thread
import cam_scanner

app = Flask(__name__)

#@app.route('/')
#def index():
#        return 'Hello world'

@app.route('/')
def hello_name():
    user = "testpy"
    return render_template('./index.html', name = user)


def run(): 
    try:
        cam_scanner.start_scan(True)
    except Exception as ex:
        print(ex)


if __name__ == '__main__':
    run()
    app.run(debug=True, host='0.0.0.0')