#!/bin/env python2.6

import os, sys, signal

from pkg_resources import require
require("cothread")

from PyQt4 import QtCore, QtGui
from PyQt4.Qt import *
from PyQt4.Qwt5 import *

# Import the cothread library in each module that uses it.
import cothread
from cothread.catools import *
from numpy import *

# Enable Qt processing
qApp = cothread.iqt()

# Import the ui form
from form_ui import Ui_Form

class Spy(QtCore.QObject):
    
    def __init__(self, parent):
        QtCore.QObject.__init__(self, parent)
        parent.setMouseTracking(True)
        parent.installEventFilter(self)

    def eventFilter(self, _, event):
        if event.type() == QtCore.QEvent.MouseMove:
            self.emit(QtCore.SIGNAL("MouseMove"), event.pos())
        return False


class plot(QwtPlot):
    def updateLayout(self, *args, **kwargs):
        QwtPlot.updateLayout(self, *args, **kwargs)
        if self.autoscaleButton is not None:
            pos = QtCore.QPoint(self.legend.pos().x() + self.legend.size().width(), self.legend.pos().y())
            self.autoscaleButton.move(pos)
            self.autoscaleButton.resize(self.axisWidget(Qwt.QwtPlot.yRight).size().width(), self.legend.size().height())
            self.autoscaleButton.show()

    def __init__(self, parent):
        self.autoscaleButton = None
        QwtPlot.__init__(self, parent)
        self.setCanvasBackground(Qt.white)    
        self.autoscale = False    
        
        # legend
        self.legend = QwtLegend()
        self.legend.setFrameStyle(QFrame.Box | QFrame.Sunken)
        self.legend.setItemMode(QwtLegend.ReadOnlyItem)
        self.insertLegend(self.legend, QwtPlot.BottomLegend)
        
                
        # grid
        self.grid = QwtPlotGrid()
        self.grid.enableXMin(True)
        self.grid.setMajPen(QPen(Qt.black, 0, Qt.DotLine))
        self.grid.setMinPen(QPen(Qt.gray, 0 , Qt.DotLine))
        self.grid.attach(self)

        # axes
        self.enableAxis(QwtPlot.yRight)
        self.setAxisMaxMajor(QwtPlot.xBottom, 6)
        self.setAxisMaxMinor(QwtPlot.xBottom, 10)
        
        # zooming
        self.zoomers = []
        zoomer = QwtPlotZoomer(
            QwtPlot.xBottom,
            QwtPlot.yLeft,
            QwtPicker.DragSelection,
            QwtPicker.AlwaysOff,
            self.canvas())
        zoomer.setRubberBandPen(QPen(Qt.black))            
        self.zoomers.append(zoomer)
        
        zoomer = QwtPlotZoomer(
            QwtPlot.xTop,
            QwtPlot.yRight,
            QwtPicker.PointSelection | QwtPicker.DragSelection,
            QwtPicker.AlwaysOff,
            self.canvas())
        zoomer.setRubberBand(QwtPicker.NoRubberBand)
        self.zoomers.append(zoomer)
        
        for item in self.legend.legendItems():
            item.curvePen().setWidth(3)         
        
        '''
        pattern = [
            Qwt.QwtEventPattern.MousePattern(Qt.LeftButton, Qt.NoModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.MidButton, Qt.NoModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.RightButton, Qt.NoModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.LeftButton, Qt.ShiftModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.MidButton, Qt.ShiftModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.RightButton, Qt.ShiftModifier)]
        for zoomer in self.zoomers:
            zoomer.setMousePattern(pattern)      
        self.zoomer2 = Qwt.QwtPlotZoomer(Qwt.QwtPlot.xBottom,
                                        Qwt.QwtPlot.yRight,
                                        Qwt.QwtPicker.DragSelection,
                                        Qwt.QwtPicker.AlwaysOff,
                                        self.canvas())
        self.zoomer2.setRubberBandPen(QPen(Qt.black))
        pattern = [
            Qwt.QwtEventPattern.MousePattern(Qt.LeftButton, Qt.NoModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.MidButton, Qt.NoModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.RightButton, Qt.NoModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.LeftButton, Qt.ShiftModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.MidButton, Qt.ShiftModifier),
            Qwt.QwtEventPattern.MousePattern(Qt.RightButton, Qt.ShiftModifier)]
        self.zoomer2.setMousePattern(pattern)                     
        '''

    def setAutoscale(self, autoscale):
        self.autoscale = autoscale
        self.setAxisAutoScale(Qwt.QwtPlot.xBottom)
        self.setAxisAutoScale(Qwt.QwtPlot.yLeft)
        if self.autoscale:
            self.setAxisAutoScale(Qwt.QwtPlot.yRight)        
        else:
            self.setAxisScale(Qwt.QwtPlot.yRight,-1, 1)
        for zoomer in self.zoomers:
            zoomer.setZoomBase()    
        self.replot()             
            
    def replot(self, *args, **kwargs):                                              
        for item in self.legend.legendItems():
            item.curvePen().setWidth(3)                     
        QwtPlot.replot(self, *args, **kwargs)
        
    def makeCurve(self, name, y2=False, col = Qt.blue, width = 1):
        curve = QwtPlotCurve(name)
        curve.setRenderHint(QwtPlotItem.RenderAntialiased);
        pen = QPen(col)
        pen.setWidth(width)
        curve.setPen(pen)
        if y2:
            curve.setYAxis(QwtPlot.yRight)
        else:
            curve.setYAxis(QwtPlot.yLeft)
        curve.attach(self)        
        return curve

                                        

class gui(QtGui.QMainWindow):
    def __init__(self, prefix):
        QtGui.QMainWindow.__init__(self)
        self.prefix = prefix
        # setup the ui
        self.ui = Ui_Form()    
        self.ui.setupUi(self)
        # setup the button actions
        self.bActions = {self.ui.go: (self.go, "go"), self.ui.stop: (self.stop, "stop")}
        pre = os.path.dirname(sys.argv[0])+"/button_"
            
        # and connect them
        for button, f, name in [(self.ui.go, self.go, "go"), 
                                (self.ui.stop, self.stop, "stop")]:
            self.connect(button, QtCore.SIGNAL("clicked()"), f)
            pix = QPixmap(pre + name + ".png")
            button.releasedIcon = QIcon(pix)
            button.pressedIcon = QIcon(pre + name + "_down.png")
            button.setIcon(button.releasedIcon)
            button.setIconSize(pix.size())
            button.setMask(QRegion(pix.mask()).translated(8,6))            
            def pressedAction(button = button):
                button.setIcon(button.pressedIcon)
            button.pressedAction = pressedAction        
            self.connect(button, QtCore.SIGNAL("pressed()"), button.pressedAction)                    
            def releasedAction(button = button):
                button.setIcon(button.releasedIcon)
            button.releasedAction = releasedAction            
            self.connect(button, QtCore.SIGNAL("released()"), button.releasedAction)                                                
        # setup the lineEdit actions
        self.lActions = {}
        for line, pv, pvrbv in \
                [ (self.ui.V,  self.prefix + ":MOTOR.VELO", self.prefix + ":MOTOR.VELO"),
                  (self.ui.TA, self.prefix + ":MOTOR.ACCL", self.prefix + ":MOTOR.ACCL"),
                  (self.ui.P,  self.prefix + ":P", self.prefix + ":P:RBV"),
                  (self.ui.I,  self.prefix + ":I", self.prefix + ":I:RBV")]:
            def f(string, self = self, pv = pv):
                caput(pv, float(string))
                self.computeTime()
            self.lActions[line] = f
            def monitor(value, self = self, line = line):    
                if not line.isModified() or not line.hasFocus():            
                    line.setText(str(value))
                self.computeTime()                    
            camonitor(pvrbv, monitor)
        # and connect them            
        for line, f in self.lActions.items():
            self.connect(line, QtCore.SIGNAL("textChanged(const QString &) "), f)        
        # make a position plot
        playout = QtGui.QVBoxLayout(self.ui.pFrame)                
        self.pPlot = plot(self.ui.pFrame)
        playout.addWidget(self.pPlot)    
        self.pPlot.setAxisTitle(QwtPlot.xBottom, "Time (s)")
        self.pPlot.setAxisTitle(QwtPlot.yLeft, 'Position (mm)')
        self.pPlot.setAxisTitle(QwtPlot.yRight, "Position Error (mm)")                
        # make a velocity plot
        vlayout = QtGui.QVBoxLayout(self.ui.vFrame)
        self.vPlot = plot(self.ui.vFrame)
        vlayout.addWidget(self.vPlot)    
        self.vPlot.setAxisTitle(QwtPlot.xBottom, "Time (s)")
        self.vPlot.setAxisTitle(QwtPlot.yLeft, 'Velocity (mm/s)')        
        # store the arrays
        self.arrays = {}        
        # set some monitors on the array
        self.arrayFuncs = {}
        for i,pv in enumerate([self.prefix + ":GATHER:DEMANDPOSN",
                               self.prefix + ":GATHER:POSN",
                               self.prefix + ":GATHER:DEMANDVELO",
                               self.prefix + ":GATHER:VELO",
                               self.prefix + ":GATHER:FERR",                               
                               self.prefix + ":GATHER:TIME"]):
            def f(value, self=self, i=i):
                if i in (0,1,4):
                    factor = caget(self.prefix + ":MOTOR.MRES")
                elif i in (2,3):
                    factor = 1000 * caget(self.prefix + ":MOTOR.MRES")
                else:
                    factor = 0.001
                if i==4 and self.arrays.has_key(4) and max(abs(self.arrays[4])) > 0.0:
                    # old following error
                    self.arrays[6] = self.arrays[4]
                self.arrays[i] = value *  factor
                self.updateArray(i)
            self.arrayFuncs[pv] = f
        # plot some curves
        self.olderror = self.pPlot.makeCurve("Old Posn Error", y2=True, col=Qt.darkRed, width = 1)
        self.curves = [
            self.pPlot.makeCurve("Demand Posn", col=Qt.darkBlue, width = 3),
            self.pPlot.makeCurve("Actual Posn", col=Qt.green, width = 1),
            self.vPlot.makeCurve("Demand Velocity", col=Qt.darkBlue, width = 3),      
            self.vPlot.makeCurve("Actual Velocity", col=Qt.green, width = 1),
            self.pPlot.makeCurve("Posn Error", y2=True, col=Qt.red, width = 2)]
        for pv, f in self.arrayFuncs.items():
            camonitor(pv, f)
        # add tracking
        self.connect(Spy(self.pPlot.canvas()), QtCore.SIGNAL("MouseMove"), self.showPCoordinates) 
        self.connect(Spy(self.vPlot.canvas()), QtCore.SIGNAL("MouseMove"), self.showVCoordinates) 
        self.should_go = False        
        # connect up the other buttons
        self.pPlot.autoscaleButton = QtGui.QPushButton(self.pPlot)
        self.pPlot.autoscaleButton.setText("Autoscale")
        self.pPlot.autoscaleButton.setCheckable(True)
        font = QtGui.QFont()
        font.setFamily("Sans Serif")
        font.setPointSize(10)
        self.pPlot.autoscaleButton.setFont(font)
        self.connect(self.ui.printScreen, QtCore.SIGNAL("clicked()"), self.printScreen)
        self.connect(self.pPlot.autoscaleButton, QtCore.SIGNAL("toggled(bool)"), self.pPlot.setAutoscale)  
        self.connect(self.ui.defaults, QtCore.SIGNAL("clicked()"), self.defaults)        
        self.ui.D.setFocus()      
        self.go_timer = QtCore.QTimer()               
     
    def computeTime(self):
        vals = dict(D=100.0, V=1.0, TA=0.0)
        for k in vals:
            v = self.ui.__dict__[k].text()
            try:
                vals[k] = float(v)
            except ValueError:
                pass 
        if vals["V"] == 0.0:
            vals["V"] = 0.00001
        self.ui.time.setText("%.2f" % ((vals["D"] / vals["V"] + vals["TA"]) * 2))
                      
    def printScreen(self):
        self.ui.progressBar.setValue(0)      
        printer = QtGui.QPrinter(QtGui.QPrinter.HighResolution)
        printer.setOrientation(printer.Landscape)
        printer.setPrinterName("dh.g.col.2")
        p = QPainter()
        p.begin(printer)
        p.setFont(QtGui.QFont("arial",16,QtGui.QFont.Bold))
        r = printer.pageRect()
        pix = QPixmap.grabWidget(self)        
        sf = min(float(r.size().width()) / pix.size().width(), float(r.size().height()) / pix.size().height())
        p.drawPixmap(0, 0, int(pix.size().width() * sf), int(pix.size().height() * sf), pix)
        p.end()
        self.ui.progressBar.setValue(100)    
        self.statusBar().showMessage("Screenshot will be printed shortly")            
        
                                               
    def updateArray(self, i):
        pchanged = False
        vchanged = False
        pvals = (0,1,4)
        if i != 5:
            if self.arrays.has_key(5):
                self.curves[i].setData(self.arrays[5], self.arrays[i])
                if i==4 and self.arrays.has_key(6):
                    self.olderror.setData(self.arrays[5], self.arrays[6])                
                if i in pvals:
                    pchanged = True
                else:
                    vchanged = True                    
        else:
            for i in range(5):
                if self.arrays.has_key(i):
                    self.curves[i].setData(self.arrays[5], self.arrays[i])
                    if i in pvals:
                        pchanged = True
                    else:
                        vchanged = True
            if self.arrays.has_key(6):
                self.olderror.setData(self.arrays[5], self.arrays[6])
                        
        if pchanged:            
            self.pPlot.setAutoscale(self.pPlot.autoscale)
            if self.arrays.has_key(4):
                self.ui.error.setText("%.4f" % abs(self.arrays[4]).mean())            
        if vchanged:
            self.vPlot.setAutoscale(self.vPlot.autoscale)
                                                          
    def tick(self):
        state = caget(self.prefix + ":GATHER:STATE")
        if state == "MONITOR_INPUT":            
            self.ui.progressBar.setValue(100)
            self.stop()
            self.timer.stop()
        elif self.ui.progressBar.value() < 99:
            self.ui.progressBar.setValue(self.ui.progressBar.value() + 1)

    def defaults(self):
        self.ui.V.setText(str(50))
        self.ui.TA.setText(str(1))
        self.ui.P.setText(str(0))
        self.ui.I.setText(str(0))

    def go(self):
        if self.ui.progressBar.value() != 100:
            return
        caput(self.prefix + ":GATHER:PORT", "pmac1port")
        caput(self.prefix + ":GATHER:MOTOR", self.prefix + ":MOTOR")
        caput(self.prefix + ":MOTOR", 0)
        self.ui.progressBar.setValue(0)  
        self.go_countdown = 50       
        self.connect(self.go_timer, QtCore.SIGNAL("timeout()"), self.do_go)
        self.go_timer.start(200)        

    def do_go(self):
        if caget(self.prefix + ":MOTOR.RBV") < 0.001:
            self.go_countdown = 0
        if self.go_countdown == 0:
            try:
                val = float(str(self.ui.D.text()))
            except ValueError:
                val = 10
                self.ui.D.setText("10")
            caput(self.prefix + ":GATHER:DEMAND", val)
            tSample = caget(self.prefix + ":GATHER:TSAMPLE.B")
            accl = caget(self.prefix + ":MOTOR.ACCL")
            tMove = max(accl + (val / caget(self.prefix + ":MOTOR.VELO")), 2*accl)
            tMove = 2*tMove + caget(self.prefix + ":GATHER:DELAY") + 0.5
            sPeriod = int(tMove * 1000.0 / (1024.0 * tSample)) + 1
            caput(self.prefix + ":GATHER:SPERIOD", sPeriod)
            cothread.Sleep(0.2)                
            # timer tick in ms
            tick = sPeriod * tSample * 10.24 + 30
            self.timer = QtCore.QTimer()              
            self.connect(self.timer, QtCore.SIGNAL("timeout()"), self.tick)
            caput(self.prefix + ":GATHER:STATE", "EXECUTE")
            caput(self.prefix + ":GATHER:EXECUTE", 1)
            self.timer.start(tick)
            self.go_countdown = 50            
            self.go_timer.stop()
        else:
            self.go_countdown -= 1
        
    def stop(self):
        caput(self.prefix + ":GATHER:ASYN.AOUT", "&1A")     
        caput(self.prefix + ":MOTOR:KILL.PROC", 1)            
        caput(self.prefix + ":MOTOR.STOP", 1)    
        caput(self.prefix + ":GATHER:EXECUTE", 0)                     
        self.ui.progressBar.setValue(100)        

    def showCoordinates(self, plot, text, position):
        self.statusBar().showMessage(text % 
            (plot.invTransform(Qwt.QwtPlot.xBottom, position.x()),
             plot.invTransform(Qwt.QwtPlot.yLeft, position.y())))        
                            
    def showPCoordinates(self, position):
        self.showCoordinates(self.pPlot, 'Time = %f, Position = %f', position)

    def showVCoordinates(self, position):
        self.showCoordinates(self.vPlot, 'Time = %f, Velocity = %f', position)
                

if __name__ == "__main__":                
    # create and show form
    if len(sys.argv) != 2:
        print "Usage: %s <prefix>\nE.g. prefix = BLxxI-MO-PMAC-01"
        sys.exit(1)
    QtCore.QObject.connect(qApp, QtCore.SIGNAL("lastWindowClosed()"), qApp.quit)    
    g = gui(sys.argv[1])
    g.ui.teamName.setText("Team %s" % sys.argv[1].title().replace("stage", " Stage"))
    def quit(*args, **kwargs):
        cothread.Quit()
    signal.signal(signal.SIGINT, quit)    
    g.show()
    # main loop
    cothread.WaitForQuit()
        
