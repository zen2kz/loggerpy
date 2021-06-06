import wifi
from wireless import Wireless
from wifi import Cell, Scheme

class Wifi:

    def get_networks(self):
        wireless = Wireless() 
        ifs = wireless.interfaces()
        if len(ifs) == 0:
            return []

        res = []
        for c in Cell.all(ifs[0]):
            res.append(c.ssid)


        return res