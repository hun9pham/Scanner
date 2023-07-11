# Python code transmits a byte to Arduino /Microcontroller
import serial
import time

MPU_NOTIFYLOGIN = "2\n"
MPU_REQSCREENSHOT = "#1\r\n"
MPU_CORMFIRM = "1\n"
MPU_OUTOFPAPPER = "#2\r\n"
MPU_PAPERJAM = "#3\r\n"
MPU_WEBTIMEOUT = "#0\n"

serialPort = serial.Serial(
    port="/dev/ttyUSB0", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)
serialString = ""  # Used to hold data coming over UART
serialPort.write(MPU_NOTIFYLOGIN)
while 1:
    # Wait until there is data waiting in the serial buffer
    if serialPort.in_waiting > 0:
        # Read data out of the buffer until a carraige return / new line is found
        serialString = serialPort.readline()

        if serialString == MPU_REQSCREENSHOT:
            print(MPU_REQSCREENSHOT)
            serialPort.write(MPU_CORMFIRM)
        elif serialString == MPU_OUTOFPAPPER:
            print(MPU_OUTOFPAPPER)
            serialPort.write(MPU_NOTIFYLOGIN)
        elif serialString == MPU_PAPERJAM:
            print(MPU_PAPERJAM)
    else:
        print(".")