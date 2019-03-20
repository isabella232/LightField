import glob
import os
import sys
import time

from threading import Thread

sys.path.append('/usr/share/lightfield/libexec/printrun')

from Util.constants import MOTOR_AXIS

from printrun import printcore
from printrun.eventhandler import PrinterEventHandler

class printprocess():
    def __init__(self, printer, showcb=None, hidecb=None, startcb=None, donecb=None):
        self.printer=printer
        self.showcb=showcb
        self.donecb=donecb
        self.hidecb=hidecb
        self.startcb=startcb
        self.files=[]
        self.index=0
        self.brightness=127
        self.layerthickness=0.1
        self.exptime=1.0
        self.print_thread=None
        self._stop=True
        self.printer.liftdone=self._liftdone
        self.liftdone=False

    def _liftdone(self):
        self.liftdone=True

    def print_proc(self):
        if(self.startcb):
            self.startcb()
        self._stop=False
        self.liftdone=False
        self.printer.home()
        if(self.hidecb):
            self.hidecb()
        self.printer.lift(layerthickness,2)
        while(not liftdone):
            time.sleep(0.1)

        while not self._stop and self.index<len(self.files):
            self.liftdone=False
            if(self.showcb):
                self.showcb(self.files[self.index], self.brightness, self.index, len(self.files))
            time.sleep(self.exptime)
            if(self.hidecb):
                self.hidecb()
            self.printer.lift(self.layerthickness, 2.0)
            while (not self.liftdone):
                time.sleep(0.1)
            self.index+=1
            if(self.index==len(self.files)):
                if(self.donecb):
                    self.donecb()
                index=0
                return;
        if(self.donecb):
            self.donecb()
        self.print_thread=None

    def stop(self):
        self._stop=True

    def print(self, files=[], layerthickness=0.1, brightness=127, exptime=1.0):
        if self.printer.p.online and not self.print_thread:
            self.files=files
            self.index=0
            self.brightness=brightness
            self.layerthickness=layerthickness
            self.exptime=exptime
            self.print_thread=threading.Thread(target=self.print_proc)
            self.print_thread.start()
            return True
        else:
            return False

class printer(PrinterEventHandler):
    def __init__(self, onlinecb=None, offlinecb=None, positioncb=None, tempcb=None, receivecb=None, sendcb=None):
        self.p=printcore.printcore()
        self.p.addEventHandler(self)
        self.onlinecb=onlinecb
        self.offlinecb=offlinecb
        self.positioncb=positioncb
        self.tempcb=tempcb
        self.receivecb=receivecb
        self.sendcb=sendcb
        self.liftdone=None
        self.port=None

    def on_disconnect(self):
        if(self.offlinecb):
            self.offlinecb()

    def on_recv(self, line):
        l=line.strip()
        if self.receivecb:
            self.receivecb(l)
        else:
            if 'Z:' in line and "E:" in line:
                pos=float(l.split("Z:")[1].split("E:")[0])
                if(self.positioncb):
                    self.positioncb(pos)
                if(self.liftdone):
                    self.liftdone()

    def on_online(self):
        if(self.onlinecb):
            self.onlinecb()

    def connect(self,port=None,baud=250000):
        baselist=[]
        for g in ['/dev/ttyUSB*', '/dev/ttyACM*']:
            baselist += glob.glob(g)
        p=port
        if p is None and len(baselist):
            p=baselist[0]
        if p is None:
            return False
        self.port=p
        self.p.connect(p,baud)
        return True

    def disconnect(self):
        self.p.disconnect()

    def send_noisy(self, output):
        if self.sendcb:
            self.sendcb(output)
        self.p.send_now(output)

    def move(self, d=1.0, axis=MOTOR_AXIS):
        if(self.p.online):
            self.send_noisy("G91")
            self.send_noisy("G0 %s%f F50"%(axis,d))
            self.send_noisy("M400")
            self.send_noisy("M114")

    def moveto(self, d=1.0, axis=MOTOR_AXIS):
        if(self.p.online):
            self.send_noisy("G90")
            self.send_noisy("G0 %s%f"%(axis,d))
            self.send_noisy("M400")
            self.send_noisy("M114")

    def home(self, axis=MOTOR_AXIS):
        if(self.p.online):
            self.send_noisy("G28 "+axis)
            self.send_noisy("M400")
            self.send_noisy("M114")

    def lift(self, d=0.1, l=2.0, axis=MOTOR_AXIS):
        if(self.p.online):
            self.send_noisy("G91")
            self.send_noisy("G0 %s%f"%(axis,l))
            self.send_noisy("G0 %s%f"%(axis,d-l))
            self.send_noisy("G90")
            self.send_noisy("M400")
            self.send_noisy("M114")

    def asktemp(self):
        if self.p.online:
            self.send_noisy("M105");

    def on_temp(self, line):
        if(self.tempcb):
            self.tempcb(line)

    def send(self,gcode):
        if self.p.online:
            self.send_noisy(gcode)

if __name__=="__main__":

    import time

    _online=False
    _position=0
    def online():
        global _online
        _online=True
    def offline():
        global _online
        _online=False
    def position(val):
        print(val)
        global _position
        _position=val

    p=printer(online,offline,position)
    p.connect("/dev/ttyACM0",115200)
    p.asktemp()
    while not _online:
        time.sleep(0.1)
    p.asktemp()
    p.move(2)
    while not _position==2:
        time.sleep(0.1)
    p.move(-2)
    while not _position==0:
        time.sleep(0.1)
    p.lift()
    while not _position==0.1:
        time.sleep(0.1)
    p.lift()
    while not _position==0.2:
        time.sleep(0.1)
    p.lift()
    while not _position==0.3:
        time.sleep(0.1)
    p.disconnect()
