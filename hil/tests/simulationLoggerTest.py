import sys
sys.path.insert(1, '../src') # Inserts src folder so we can import the code to test
from simulationLogger import SimulationLogger, DEFAULT_TEMPLATE, DEFAULT_LOGGING_PORT, DEFAULT_COMMAND_SHEET_NAME, DEFAULT_TELEMETRY_SHEET_NAME
from openpyxl import Workbook, load_workbook
import socket, struct, random, time, os

### Global Variable Defines
DEFAULT_IP = "127.0.0.1"
DIGITS_TO_ROUND_TO = 4  
### End of Global Variable Defines

# SimulationLoggerTester tests the command and telemetry logging
# capabailities of simulationLogger and 
class SimulationLoggerTester():

    ### Start of static methods ###

    # Generate a data vector based on the given template with random elements
    def generateDataVector(template = DEFAULT_TEMPLATE):
        dataVector = []
        for char in template:
            element = SimulationLoggerTester.generateElementOfDataVector(char)
            dataVector.append(element)
        return dataVector

    # Returns a random value of the data type specified in char
    def generateElementOfDataVector(char):  
        if char == 'f':                                                                         # Float
            return  round(random.random() * 99, DIGITS_TO_ROUND_TO)
        elif char == 'i':                                                                       # Integer
            return random.randint(0, 100)
        elif char == '?':                                                                       # Boolean
            return random.randint(0, 1)
        elif char == 'c':                                                                       # Character
            return chr(random.randint(65, 90))
        elif char == 'd':                                                                       # Double
            return  round(random.random() * 99, DIGITS_TO_ROUND_TO)
        elif char == 'L':                                                                       # Unsigned Long
            return random.randint(100000, 1000000)
        elif char == 'l':                                                                       # Signed Long
            return random.randint(-1000000, 0)
        elif char == 'h':                                                                       # Short
            return random.randint(0, 24)

    # During transfer some decimal places starting around 10^-8 and lower get messed up
    # So each float inside the DataVector being checked is rounded off before assertion
    def cleanVector(vector):                    
        for i, elem in enumerate(DEFAULT_TEMPLATE):
            if elem == 'f':
                vector[i] = round(vector[i], DIGITS_TO_ROUND_TO)
        
        return vector

    ### End of static methods ###

    ### Start of instance methods ###
    
    def __init__(self, IP = DEFAULT_IP, port = DEFAULT_LOGGING_PORT, numIterations = 100):
        self.logger = SimulationLogger()
        self.logger.setDaemon(True)                                                             # Prevent the logger from forcing the system to hang by making it a daemon thread
        self.logger.start()
        print("Logger Initialized")
        time.sleep(.3)                                                                          # Sleep for a split second to let the logger open up it's socket
        self.IP = IP
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)                            # UDP socket
        print("Connected to server, telemetry test ready")
        self.numIterations = numIterations

    # Runs through all units tests 
    def runTests(self):
        print("Beginning telemetry testing")
        self.testTelemetry()
        print("Telemetry test successful, moving to command logging")
        self.testCommandLogging()
        print("Command Input test successful, moving to log review")
        self.reviewLogs()
        print("No errors were thrown during testing: TEST PASSED")

    # Generates and sends random telemetry packets to the logger to log
    def testTelemetry(self):                        
        self.dataVectors = [SimulationLoggerTester.generateDataVector() for i in range(self.numIterations)]
        
        for i in range(self.numIterations):
            byteMessage = struct.pack(DEFAULT_TEMPLATE, *self.dataVectors[i])                   # Create a byte array following the logger's DataVector template
            self.sock.sendto(byteMessage, (self.IP, self.port))                     
            time.sleep(.1)                                                                      # Have to sleep to give time for the logger to receive the data vector and update the one in memory
            receivedVector = self.logger.readDataVector()[:-1]
            receivedVector = SimulationLoggerTester.cleanVector(receivedVector)                 # The logger doesn't round when receiving float data and can only guarantee equality to a certain precision
            assert(self.dataVectors[i] == receivedVector)

    # Requests for several random commands to be logged
    def testCommandLogging(self):                                           
        self.commands = ["Random Command {0}".format(random.randint(0, 50)) for i in range(self.numIterations)]
        [self.logger.logCommandInput(command) for command in self.commands] 

    # Checks that all logs have the correct packets and logging order
    def reviewLogs(self):
        logPath = self.logger.getLogPath()
        workbook = load_workbook(filename= logPath)                                             # All logs are stored in one workbook for each instance of a logger               
        
        sheet = workbook[DEFAULT_TELEMETRY_SHEET_NAME]
        self.reviewTelemetryLog(sheet)
        sheet = workbook[DEFAULT_COMMAND_SHEET_NAME]
        self.reviewCommandLog(sheet)

    # Checks that the telemetry packets were all logged correctly
    def reviewTelemetryLog(self, sheet):                                                        # Check that all data vectors were logged correctly
        for i in range(len(self.dataVectors)):
            row = [elem.value for elem in sheet[i + 1]][:-1]
            row = SimulationLoggerTester.cleanVector(row)                                       # Have to round here for the same reason we do in the telemetry test
            for j in range(len(self.dataVectors[i])):
                assert(self.dataVectors[i][j] == row[j])

    # Checks that the commands were logged properly
    def reviewCommandLog(self, sheet):                                                          # Check all commands were logged correctly
        for i in range(len(self.commands)):
            assert(self.commands[i] == sheet[i + 1][0].value)

    ### End of Instance Methods ###

if __name__ == "__main__":
    tester = SimulationLoggerTester()
    tester.runTests()
