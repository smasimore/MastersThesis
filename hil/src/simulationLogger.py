import socket, threading, struct, os
from openpyxl import Workbook, load_workbook
from datetime import datetime

### Global variable define statements
DEFAULT_TEMPLATE = "fff?iLlhd"
DEFAULT_LOG_DIRECTORY_PATH = "SimulationLogs-"
DEFAULT_LOG_NAME = "SimulationLog"
DEFAULT_COMMAND_SHEET_NAME = "Command History"
DEFAULT_TELEMETRY_SHEET_NAME = "DataVector Logs"
DEFAULT_EXTENSION = '.xlsx'
DEFAULT_LOGGING_PORT = 8080
### End of Global Variable Defines

# SimulationLogger is responsible for keeping a time stamped log of
# everything going on in the simulation that it was insantiated in.
# It has an open socket accepting ethernet packets from the flight
# computer and keeps the most recent data vector in memory. 
class SimulationLogger(threading.Thread):

    ### Start of instance methods ###
    def __init__(self, template= DEFAULT_TEMPLATE, IP= "0.0.0.0", port= DEFAULT_LOGGING_PORT, backlogs= 1): # IP 0.0.0.0 means accept all incoming connections regardless of IP
        self.sock = socket.socket()             
        self.sock.bind((IP, port))                                                  # The logger acts as a server waiting for the flight computer to connect to it
        self.sock.listen(backlogs)                                                  # Backlogs is the number of incoming connections that can be queued before any more connections are refused
        
        self.template = template                                                    # The template is a description of what data types the Data Vector consists of
        self.currentDataVector = []                                                 # Stores the last received Data Vector from the flight computer
        self.initializeSimulationLog()

        self.lock = threading.Lock()                                                # A lock is used when updating the current data vector to avoid race conditions
        super(SimulationLogger, self).__init__()

    def run(self):
        self.con, __ = self.sock.accept()                                           # Accept generates a connection object that we interface with, we don't receive packets directly through the socket object
        
        while True:
            dataVector = self.receiveDataVector()
            self.updateCurrentDataVector(dataVector)
            self.logDataVector(dataVector)

    
    def initializeSimulationLog(self):
        self.initializeLogDirectory()                                              
        timeStamp = str(datetime.time(datetime.now())).split('.')[0]
        timeStamp = timeStamp.replace(':', '-', 2)                                  # Replacing invalid naming characters
        self.logName = DEFAULT_LOG_NAME + '-' + timeStamp + DEFAULT_EXTENSION       # The workbook name for this simulation is time stamped with the current hh-mm--ss time
        self.workbook = Workbook()
        self.workbook.create_sheet(DEFAULT_COMMAND_SHEET_NAME, 0)                   # Stores commands issued by the simulato    
        self.workbook.create_sheet(DEFAULT_TELEMETRY_SHEET_NAME, 0)                 # Stores all received Data Vectors from the flight computer
    
    def initializeLogDirectory(self):                                               # Creates a directory time stamped with todays date to store the logs
        date = str(datetime.date(datetime.today()))
        self.dirName = DEFAULT_LOG_DIRECTORY_PATH + date
        if not os.path.isdir(self.dirName):                                         # Don't make the dir if it already exists
            os.mkdir(self.dirName)

    def receiveDataVector(self):
        byteData = self.con.recv(1024)
        dataVector = list(struct.unpack(self.template, byteData))                   # Unpacks the byteData into an array of data types detailed in the template
        return dataVector

    def updateCurrentDataVector(self, dataVector):
        self.lock.acquire()
        self.currentDataVector = dataVector                                         
        self.lock.release() 

    def logDataVector(self, dataVector):
        self.logToSheet(DEFAULT_TELEMETRY_SHEET_NAME, dataVector)

    def readDataVector(self):
        self.lock.acquire()
        dataVector = self.currentDataVector
        self.lock.release()
        return dataVector

    def logCommandInput(self, command):
        newRow = [command]
        self.logToSheet(DEFAULT_COMMAND_SHEET_NAME, newRow)      

    def logToSheet(self, sheetName, row):
        sheet = self.workbook[sheetName]
        timeStamp = str(datetime.time(datetime.now()))                              # Time stamp each log entry for a time record of the simulation
        row.append(timeStamp)        
        sheet.append(row)
        self.workbook.save(filename = os.path.join(self.dirName, self.logName))          

    def getLogPath(self):
        return os.path.join(self.dirName, self.logName)

    ### End of Instance Methods ###
    
if __name__ == "__main__":
    logger = SimulationLogger()
    logger.start()

    dataArray = [3.0, 4.0, 5.0, 48, 6]      # Example Data Vector to send
    sock = socket.socket()
    sock.connect(("127.0.0.1", 8080))
    print("Connected to server")

    ## Basic test code where the user can manually watch the unpacking of a specified Data Vector
    while True:
        input("Press enter to send next message")
        sock.send(struct.pack(DEFAULT_TEMPLATE, *dataArray))
        input("Message sent, press enter to continue")
        print("Message received by logger: {0}".format(logger.readDataVector()))
        logger.logCommandInput("Random Command")
        print("Command Input logged")
        
