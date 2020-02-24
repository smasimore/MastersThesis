import sys
sys.path.insert(1, '../src') # Inserts src folder so we can import the code to test
from simulationLogger import SimulationLogger, DEFAULT_TEMPLATE, DEFAULT_LOGGING_PORT, DEFAULT_COMMAND_SHEET_NAME, DEFAULT_TELEMETRY_SHEET_NAME
from openpyxl import Workbook, load_workbook
import socket, struct, random, time, os

### Global Variable Defines
DIGITS_TO_ROUND_TO = 4
### End of Global Variable Defines

# SimulationLoggerTester tests the command and telemetry logging
# capabailities of simulationLogger
class SimulationLoggerTester():

    ### Start of static methods ###
    def generateDataVector(template= DEFAULT_TEMPLATE):     # Generate a data vector based on the given template with random elements
        dataVector = []
        for char in template:
            element = SimulationLoggerTester.generateElementOfDataVector(char)
            dataVector.append(element)
        return dataVector
     
    def generateElementOfDataVector(char):                  # Returns a random value of the data type specified in char
        if char == 'f': # Float
            return  round(random.random() * 99, DIGITS_TO_ROUND_TO)
        elif char == 'i': # Integer
            return random.randint(0, 100)
        elif char == '?': # Boolean
            return random.randint(0, 1)
        elif char == 'c': # Character
            return chr(random.randint(65, 90))
        elif char == 'd': # Double
            return  round(random.random() * 99, DIGITS_TO_ROUND_TO)
        elif char == 'L': # Unsigned Long
            return random.randint(100000, 1000000)
        elif char == 'l': # Signed Long
            return random.randint(-1000000, 0)
        elif char == 'h': # Short
            return random.randint(0, 24)

    def cleanVector(vector):                        # During transfer some decimal places starting around 10^-8 and lower get messed up
        for i, elem in enumerate(DEFAULT_TEMPLATE):
            if elem == 'f':
                vector[i] = round(vector[i], DIGITS_TO_ROUND_TO)
        
        return vector

    ### End of static methods ###

    ### Start of instance methods ###
    def __init__(self, port= DEFAULT_LOGGING_PORT, numIterations= 100):
        self.logger = SimulationLogger()
        self.logger.start()
        print("Logger Initialized")
        time.sleep(.1)                              # Sleep for a split second to let the logger open up it's socket 
        self.sock = socket.socket() 
        self.sock.connect(("127.0.0.1", port))      # Connecting to the logger, 127.0.0.1 means connect to this computer
        print("Connected to server, telemetry test ready")
        self.numIterations = numIterations

    def runTests(self):
        print("Beginning telemetry testing")
        self.testTelemetry()
        print("Telemetry test successful, moving to command logging")
        self.testCommandLogging()
        print("Command Input test successful, moving to log review")
        self.reviewLogs()
        print("No errors were thrown during testing: TEST PASSED")

    def testTelemetry(self):                        # Checks to see if telemetry packets are read and unpacked correctly
        self.dataVectors = [SimulationLoggerTester.generateDataVector() for i in range(self.numIterations)]
        
        for i in range(self.numIterations):
            self.sock.send(struct.pack(DEFAULT_TEMPLATE, *self.dataVectors[i]))
            time.sleep(.1)                                                          # Have to sleep to give time for the logger to receive the data vector and update the one in memory
            receivedVector = self.logger.readDataVector()[:-1]
            receivedVector = SimulationLoggerTester.cleanVector(receivedVector)     # The logger doesn't round when receiving float data and can only guarantee equality to a certain precision
            assert(self.dataVectors[i] == receivedVector)

    def testCommandLogging(self):                                                   # Requests for several random commands to be logged
        self.commands = ["Random Command {0}".format(random.randint(0, 50)) for i in range(self.numIterations)]
        [self.logger.logCommandInput(command) for command in self.commands] 

    def reviewLogs(self):
        logPath = self.logger.getLogPath()
        workbook = load_workbook(filename= logPath)                                 # All logs are stored in one workbook for each instance of a logger               
        
        sheet = workbook[DEFAULT_TELEMETRY_SHEET_NAME]
        self.reviewTelemetryLog(sheet)
        sheet = workbook[DEFAULT_COMMAND_SHEET_NAME]
        self.reviewCommandLog(sheet)

    def reviewTelemetryLog(self, sheet):                                            # Check that all data vectors were logged correctly
        for i in range(len(self.dataVectors)):
            row = [elem.value for elem in sheet[i + 1]][:-1]
            row = SimulationLoggerTester.cleanVector(row)                           # Have to round here for the same reason we do in the telemetry test
            for j in range(len(self.dataVectors[i])):
                assert(self.dataVectors[i][j] == row[j])

    def reviewCommandLog(self, sheet):                                              # Check all commands were logged correctly
        for i in range(len(self.commands)):
            assert(self.commands[i] == sheet[i + 1][0].value)

    ### End of Instance Methods ###

if __name__ == "__main__":
    tester = SimulationLoggerTester()
    tester.runTests()
