from gpiozero import LED
from tkinter import *
import threading
import tkinter.font

import RPi.GPIO
import time
import datetime

RPi.GPIO.setmode(RPi.GPIO.BCM)


class ValveMux:
    '''
    Long story short, this class will do everything
    it will toggle in the round robin way
    it will store the delta wait
    '''

    def __init__(self, pins, deltaT=[5, 5, 5, 5]):
        self.__stopFlag = False
        self.__pinIT = 0
        self.__pins = []
        self.incMul = 5


        if (len(pins) != (len(deltaT))):
            print("we have a problem!!!")
            self.deltaT = []
            for pinD in pins:
                self.__deltaT.append(1)

        for pin in pins:
            self.__pins.append(LED(pin))

        self.__nPins = len(pins)
        self.__deltaT = deltaT

    def tick(self):
        # go for the better safe than sorry approach
        # there is no guarantee im the only one controlling the pins

        # TODO: move to the appropriate position
        self.giveReport()

        self.clear_all()
        if self.__deltaT[self.__pinIT] != 0:
            self.__pins[self.__pinIT].on()
        self.__pinIT += 1
        if self.__pinIT == self.__nPins:
            self.__pinIT = 0

    def __run(self):
        self.tick()
        if (self.__deltaT[self.__pinIT-1] != 0):
            time.sleep(self.__deltaT[self.__pinIT - 1])

    def run(self, cycles=0):
        if cycles == 0:
            while not self.__stopFlag:
                self.__run()
            else:
                return
        else:
            for x in range(1, cycles):
                for y in range(1, self.__nPins):
                    if not self.__stopFlag:
                        self.__run()
                    else:
                        return

    def stop(self):
        self.__stopFlag = True
        self.clear_all()

    def start(self):
        self.__stopFlag = False
        self.clear_all()

    def clear_all(self):
        for pin in self.__pins:
            pin.off()

    def get_delta(self, i):
        return self.__deltaT[i]

    def set_delta(self, pinPoint, t):
        self.__deltaT[pinPoint] += t*self.incMul
        if (self.__deltaT[pinPoint] < 0):
            self.__deltaT[pinPoint] = 0

    def set_incMul(self, mul):
        self.incMul = mul

    def getActiveValve(self):
        return self.__pinIT + 1

    def toggleValve(self, pinID):
        self.stop()
        self.__pins[pinID].toggle()

    def setActiveValve(self, pinID):
        self.stop()
        self.__pins[pinID].on()

    def giveReport(self):
        filename = "log.txt"
        file = open(filename, "a")
        file.write(str(datetime.datetime.now()) + ", " + str(self.getActiveValve())+"\n")
        file.close()


def close(mux, thread):
    mux.stop()
    RPi.GPIO.cleanup()
    win.destroy()


def muxHelpler(mux, x, deltadelta):
    mainMux.set_delta(x, deltadelta)
    labelArray[x]["text"] = mux.get_delta(x)


if __name__ == "__main__":
    outList = [5, 6, 13, 26]
    # you can also specify the intial period
    mainMux = ValveMux(outList, [5, 5, 5, 5])

    mainMux = ValveMux(outList)

    # run after we are done with everything
    t1 = threading.Thread(target=mainMux.run)
    t1.daemon = True
    t1.start()

    incrementVal = 1

    # define GUI context
    win = Tk()
    win.title("Gas Sample Selector")
    win.geometry('1000x500')
    myFont = tkinter.font.Font(family="Helvetica", size=12, weight="bold")

    # create GUI
    ExitButton = Button(win, text='Exit', font=myFont, command=lambda: close(mainMux, t1), bg='red', height=5, width=10)
    ExitButton.grid(row=1, column=5)


    # increment size buttons
    oneSec = Button(win, text='5s', font=myFont, command=lambda: mainMux.set_incMul(5) , bg='RoyalBlue2', height=5, width=5)
    oneSec.grid(row=2, column=5)

    oneMin = Button(win, text='1m', font=myFont, command=lambda: mainMux.set_incMul(60), bg='RoyalBlue2', height=5, width=5)
    oneMin.grid(row=3, column=5)

    fiveMin = Button(win, text='5m', font=myFont, command=lambda: mainMux.set_incMul(300), bg='RoyalBlue2', height=5, width=5)
    fiveMin.grid(row=4, column=5)


    # buttons per valve!
    buttonArray = []
    ctrl_buttonArray = []
    labelArray = []


    for pinPointer in range(len(outList)):
        labelArray.append(Label(win, text=mainMux.get_delta(pinPointer)))
        # valve incremnet text
        labelArray[pinPointer].grid(row=1, column=pinPointer + 1)

        buttonArray.append(
            Button(win, text='Valve ' + str(pinPointer+1) + ' on', font=myFont,
                   command=lambda x=pinPointer: (mainMux.toggleValve(x)),
                   bg='lightgreen', height=5, width=10))
        buttonArray[pinPointer].grid(row=2, column=pinPointer + 1)

        #  increment/ decrement buttons
        ctrl_buttonArray.append(
            Button(win, text='+', font=myFont,
                   command=lambda x=pinPointer: muxHelpler(mainMux, x, 1),
                   bg='lightgreen', height=5, width=5))
        ctrl_buttonArray[pinPointer * 2].grid(row=3, column=pinPointer + 1)

        ctrl_buttonArray.append(
            Button(win, text='-', font=myFont,
                   command=lambda x=pinPointer: muxHelpler(mainMux, x, -1),
                   bg='lightgreen', height=5, width=5))
        ctrl_buttonArray[(pinPointer * 2) + 1].grid(row=4, column=pinPointer + 1)

    win.protocol("WM_DELETE_WINDOW", close)
