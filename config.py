import os
import confuse

def config():
    config = confuse.Configuration('PartsLogger', __name__)
    workdir = os.getcwd()
    config.set_file(workdir+'/config.yaml')
    return config


