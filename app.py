from flask import Flask
import _thread
import cam_scanner
from routes import app



def run(): 
    try:
        cam_scanner.start_scan(True)
    except Exception as ex:
        print(ex)


if __name__ == '__main__':
    run()
    app.run(debug=True, host='0.0.0.0')