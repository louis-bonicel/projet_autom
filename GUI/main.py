import sys
import math
import glob
import serial
from PyQt4 import QtGui, QtCore

slide_command = 0
available_port = []
#COMPort = ""
#serialPort = None


class Example(QtGui.QWidget):
    
    def __init__(self):
        super(Example, self).__init__()
        self.unitSI = False
        self.COMPort = ""
        self.serialPort = None
        self.initUI()
        
        
    def initUI(self):      
        self.comboSI = QtGui.QComboBox(self)
        self.comboSI.addItem("tr/min")
        self.comboSI.addItem("rd/s")
        self.comboSI.move(200, 50)
        self.comboSI.activated[str].connect(self.onActivated)
        

        self.comboCOM = QtGui.QComboBox(self)
        for comPort in available_port :
            self.comboCOM.addItem(comPort)
        self.comboCOM.move(230, 10)
        self.comboCOM.activated[str].connect(self.COMUpdate)


        self.sld = QtGui.QSlider(QtCore.Qt.Horizontal, self)
        self.sld.setFocusPolicy(QtCore.Qt.NoFocus)
        self.sld.setRange(-8000,8000)
        self.sld.setPageStep(50)
        # x,y position , width,height position
        self.sld.setGeometry(30, 150, 240, 15)
        self.sld.valueChanged[int].connect(self.changeConsigneValue)

        self.vitesseConsigne = QtGui.QLabel("Consigne", self)
        self.vitesseConsigne.setGeometry(50, 65, 80, 30)
        
        self.textConsigne = QtGui.QLineEdit(self)
        self.textConsigne.setGeometry(100, 65, 80, 30)
        self.textConsigne.setText("0")

        self.vitesseTachy = QtGui.QLabel("Tachymètre", self)
        self.vitesseTachy.setGeometry(40, 30, 80, 30)

        self.textTachy = QtGui.QLineEdit(self)
        self.textTachy.setGeometry(100, 30, 80, 30)
        self.textTachy.setText("0")
        
        self.btnRAZ = QtGui.QPushButton("Remise a Zero", self)
        self.btnRAZ.move(100, 100)
        self.btnRAZ.clicked.connect(self.RAZ)

        self.setGeometry(300, 300, 300, 200)
        self.setWindowTitle("Motor Command")
        self.show()

    def changeConsigneValue( self , value ):
        global slide_command
        slide_command = value
        
        textBoxValue = int(slide_command)
        textToSend = textBoxValue
        if self.unitSI:
            textBoxValue = textBoxValue * math.pi / 30

        self.textConsigne.setText(str(int(textBoxValue)))

        to_send = [elem.encode("hex") for elem in format(textToSend,"+016d")]
        for i in range(1,16):
            to_send[i] = int(to_send[i])-30
        print to_send
        if to_send[0] == "2b":
            to_send[0] = 0
        else:
            to_send[0] = 1
        print to_send
        self.serialPort.write(to_send)

    def onActivated( self , value ):
        if value == "tr/min":
            self.unitSI = False
        else:
            self.unitSI = True
        self.changeConsigneValue(slide_command)

    def RAZ( self ):
        self.sld.setValue(0)

    def COMUpdate( self , value ):
        if self.serialPort != None:
            try:
                self.serialPort.close()
            except:
                print "Failed closing " + str(value)

        try:
            self.serialPort = serial.Serial(str(value), 115200)
            print value + " opened successfully !"
        except:
            print "Failed opening " + str(value)
        self.COMPort = value


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
    ex = Example()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
