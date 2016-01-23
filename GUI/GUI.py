import sys
import math
import glob
import serial
from PyQt4 import QtGui, QtCore
from PyQt4.QtGui import QApplication, QMainWindow, QTextCursor
from PyQt4.QtCore import QObject, SIGNAL, SLOT, QThread

slide_command = 0
available_port = []


class MotorController(QtGui.QWidget):
    
    def __init__(self):
        super(MotorController, self).__init__()
        self.unitSI = False
        self.COMPort = ""
        self.serialPort = None
        self.mode = 0
        self.initUI()
        self.reader = SerialReader()
        
        QObject.connect(self.reader, SIGNAL("newData(QString)"), self.displayValue)
        
    def initUI( self ):
        self.CreateSISelection()
        self.CreateCOMPortSelection()
        self.CreateSpeedSlider()
        self.CreateSpeedBox()
        self.CreateTachyBox()
        self.CreateButtons()
        self.CreateMinMaxBox()
        
        # x,y position , width,height position
        self.setGeometry(300, 300, 300, 400)
        self.setWindowTitle("Motor Command")
        self.show()
        
        
    def CreateSISelection( self ):
        self.comboSI = QtGui.QComboBox(self)
        self.comboSI.addItem("tr/min")
        self.comboSI.addItem("rd/s")
        self.comboSI.move(200, 50)
        self.comboSI.activated[str].connect(self.onActivated)

    def CreateCOMPortSelection( self ):
        self.comboCOM = QtGui.QComboBox(self)
        for comPort in available_port :
            self.comboCOM.addItem(comPort)
        self.comboCOM.move(230, 10)
        self.comboCOM.activated[str].connect(self.COMUpdate)

    def CreateSpeedSlider( self ):
        self.sld = QtGui.QSlider(QtCore.Qt.Horizontal, self)
        self.sld.setFocusPolicy(QtCore.Qt.NoFocus)
        self.sld.setRange(-8000,8000)
        self.sld.setPageStep(50)
        # x,y position , width,height position
        self.sld.setGeometry(30, 150, 240, 15)
        self.sld.valueChanged[int].connect(self.changeConsigneValue)
        self.sld.sliderReleased.connect(self.updateValue)

    def CreateSpeedBox( self ):
        self.vitesseConsigne = QtGui.QLabel("Consigne", self)
        self.vitesseConsigne.setGeometry(50, 65, 80, 30)
        
        self.textConsigne = QtGui.QLineEdit(self)
        self.textConsigne.setGeometry(100, 65, 80, 30)
        self.textConsigne.setText("0")

    def CreateTachyBox( self ):
        self.vitesseTachy = QtGui.QLabel("Tachymetre", self)
        self.vitesseTachy.setGeometry(40, 30, 80, 30)

        self.textTachy = QtGui.QLineEdit(self)
        self.textTachy.setGeometry(100, 30, 80, 30)
        self.textTachy.setText("0")
        
    def CreateButtons( self ):
        self.btnRAZ = QtGui.QPushButton("Remise a Zero", self)
        self.btnRAZ.setGeometry(100, 100 , 100, 25)
        self.btnRAZ.clicked.connect(self.RAZ)

        self.btnSweep = QtGui.QPushButton("Sweep", self)
        self.btnSweep.setGeometry(100, 260 , 100, 25)
        self.btnSweep.clicked.connect(self.Sweep)

        self.btnStep = QtGui.QPushButton("Step", self)
        self.btnStep.setGeometry(100, 290 , 100, 25)
        self.btnStep.clicked.connect(self.Step)

    def CreateMinMaxBox( self ):
        self.min = QtGui.QLabel("Min", self)
        self.min.setGeometry(50, 180, 80, 30)

        self.max = QtGui.QLabel("Max", self)
        self.max.setGeometry(50, 215, 80, 30)
        
        self.textMin = QtGui.QLineEdit(self)
        self.textMin.setGeometry(100, 180, 80, 30)
        self.textMin.setText("0")

        self.textMax = QtGui.QLineEdit(self)
        self.textMax.setGeometry(100, 215, 80, 30)
        self.textMax.setText("0")


    def changeConsigneValue( self , value ):
        global slide_command
        slide_command = value
        
        textBoxValue = int(value)
        textToSend = textBoxValue
        if self.unitSI:
            textBoxValue = textBoxValue * math.pi / 30
        self.textConsigne.setText(str(int(textBoxValue)))

        
    def displayValue( self , text ):
        received = unicode(text)
        signe_first_value = 1
        signe_second_value = 1
        
        try:
            signe = int(str(ord(received[0])))
            if ( signe & 0b00000100 ) == 0b00000100:
                signe_first_value = -1
            if ( signe & 0b00000001 ) == 0b00000001:
                signe_second_value = -1

            tachyValue_part_1 = int(str(ord(received[1])))
            tachyValue_part_2 = int(str(ord(received[2])))

            tachyValue = ((tachyValue_part_1 & 0b11111110) << 7 ) | (tachyValue_part_2 >> 1)
            tachyValue = tachyValue * signe_first_value

            second_value_part_1 = int(str(ord(received[3])))
            second_value_part_2 = int(str(ord(received[4])))

            value_2 = ((second_value_part_1 & 0b11111110) << 7 ) | (second_value_part_2 >> 1)
            value_2 = value_2 * signe_second_value

            if self.unitSI:
                tachyValue = tachyValue * math.pi / 30
            self.textTachy.setText( str(int(tachyValue)) );
            
        except IndexError:
            pass
        

    def updateValue( self ):
        textToSend = int( self.sld.value() )
        self.mode = 0b0001
        self.sendConsigne( textToSend )


    def sendConsigne( self , textToSend_part1 , textToSend_part2 = "" ):
        to_send = []
        if textToSend_part2 == "":
            textToSend_part2 = textToSend_part1

        firstValueToSend = abs(int(textToSend_part1))
        secondValueToSend = abs(int(textToSend_part2))
        if int(textToSend_part1) < 0:
            signe = 0b0100
        else:
            signe = 0b1000
        if int(textToSend_part2) < 0:
            signe = signe | 0b0001
        else:
            signe = signe | 0b0010
        to_send.append(chr( signe | (self.mode << 4) ))
        to_send.append(chr( firstValueToSend >> 8 ))
        to_send.append(chr( firstValueToSend & 0xFF ))
        to_send.append(chr( secondValueToSend >> 8 ))
        to_send.append(chr( secondValueToSend & 0xFF ))
        print [format(ord(to_send[i]),"02X") for i in range(0,5)]
        self.serialPort.write(to_send)


    def onActivated( self , value ):
        if value == "tr/min":
            self.unitSI = False
        else:
            self.unitSI = True
        self.changeConsigneValue(slide_command)

    def RAZ( self ):
        self.sld.setValue(0)
        self.mode = 0b0001
        self.sendConsigne( "0" )

    def COMUpdate( self , value ):
        if self.serialPort != None:
            try:
                if self.reader.isRunning():
                    self.reader.terminate()
                self.serialPort.close()
            except:
                print "Failed closing " + str(value)

        try:
            self.serialPort = serial.Serial(str(value), 115200)
            print value + " opened successfully !"
        except:
            print "Failed opening " + str(value)
        self.COMPort = value
        self.reader.start( self.serialPort )

    def Sweep( self ):
        minSweep = int(str(self.textMin.text()));
        maxSweep = int(str(self.textMax.text()));
        
        self.mode = 0b0010
        
        if minSweep < maxSweep: 
            self.sendConsigne( minSweep , maxSweep )
        else:
            self.sendConsigne( maxSweep , minSweep )


    def Step( self ):
        minStep = int(str(self.textMin.text()));
        maxStep = int(str(self.textMax.text()));
        
        self.mode = 0b0100
        
        if minStep < maxStep: 
            self.sendConsigne( minStep , maxStep )
        else:
            self.sendConsigne( maxStep , minStep )


class SerialReader( QThread ):
   def start( self , ser , priority = QThread.InheritPriority ):
      self.ser = ser
      QThread.start( self , priority )
      
   def run( self ):
      while 1:
         try:
            data = self.ser.read()
            n = self.ser.inWaiting()
            if n:
               data = data + self.ser.read(n)
            self.emit(SIGNAL("newData(QString)"), data)
         except:
            print "Reader thread has terminated unexpectedly."
            break

   def terminate(self):
      self.wait()
      QThread.terminate(self)


def serial_ports():
    ports = ['COM%s' % (i + 1) for i in range(256)]
    
    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


                
def main():
    global available_port
    available_port = serial_ports()
    
    app = QtGui.QApplication(sys.argv)
    ex = MotorController()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
