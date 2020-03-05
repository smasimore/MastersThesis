import sys
sys.path.insert(1, '../src')  # Inserts src folder so we can import the code to test
from pxiChassis import PxiChassis, DEFAULT_ACK_MESSAGE, DEFAULT_TX_PORT, DEFAULT_RX_PORT
import socket, time, threading, random, string

### Global Variable Define Statements
DEFAULT_MAX_TEST_STRING_LENGTH = 10   # Max length of a test packet
### End of Global Defines

# PxiChassisTester is responsible for testing the PxiChassis lib.
# Both single packet and burst packet communication are tested and
# can be configured to test under synchronous and asynchronous modes
class PxiChassisTester():

    ### Start of static methods ###

    # Generates Random String of the input length
    def generateRandomString(stringLength = DEFAULT_MAX_TEST_STRING_LENGTH):                    # Generate a random string of random length
        """Generate a random string of fixed length """
        letters = string.ascii_lowercase
        return ''.join(random.choice(letters) for i in range(random.randint(1, stringLength)))

    # Main that runs on the chassis test thread,
    # initializes a chassis object to talk with
    def chassisMain(ackRequired, numIterations, burstLength):
        chassis = PxiChassis(ackRequired= ackRequired)
        for i in range(numIterations):                                                          # Handle a number of transfers consisting of a specified number of packets
            messages = []
            for j in range(burstLength):                                                        # burst length controls how many packets are in each transfer
                messages.append(chassis.read())
            for message in messages:                                                            # Send the messages back to double check both communication paths are working
                chassis.write(message)
        exit()

    ### End of static methods ###

    ### Start of instance methods ###

    def  __init__(self, ackRequired = True, numIterations = 100, burstLength = 5):
        self.ackRequired = ackRequired
        self.numIterations = numIterations
        self.burstLength = burstLength
        self.initializeSockets()

    # Binds two open server sockets waiting for a PXI chassis object to connect
    def initializeSockets(self):                                                                # The real life chassis will act as a server waiting for the simulator to connect, this mimics that behavior
        print('Initializing sockets')
        self.testTxSocket = socket.socket()
        self.testTxSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)                 # This allows the socket to be reopened shortly after closing, prevents "Socket in use errors"
        self.testRxSocket = socket.socket()
        self.testRxSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        self.testTxSocket.bind(('0.0.0.0', DEFAULT_TX_PORT))                                    # 0.0.0.0 means accept all connections on this port
        self.testRxSocket.bind(('0.0.0.0', DEFAULT_RX_PORT))

        self.testTxSocket.listen()
        self.testRxSocket.listen()
        print('Both sockets listening binded and listening')

    # Runs through each unit test and reports results
    def communicationTest(self):
        print('Testing single packet communication')
        self.testSinglePacketCom()
        print('Single packet communication successful, moving on to burst communication')
        self.testBurstPacketCom()
        print('Burst communication successful')
        print('No errors thrown during testing, TEST PASSED')
        self.closeSockets()

    # Blocks until both the tx and rx sockets receive connections from the chassis
    def connectToChassis(self):
        self.txCon, _ = self.testTxSocket.accept()                                              # Accepting a connection returns a connection object that we can talk too,
        print("Tx Socket connected")                                                            # we can't send packets via the server socket, only the connection object
        self.rxCon, _ = self.testRxSocket.accept()
        print("Rx Socket connected")

    # Call when done with all tests to close the server sockets
    def closeSockets(self):
        self.testTxSocket.close()
        self.testRxSocket.close()

    # Call after each test in order to shutdown the connection to a chassis object
    def closeConnections(self):
        self.txCon.close()
        self.rxCon.close()

    # Creates a thread that initializes a chassis object to test
    def initializeChassis(self, burstLength = 1):                                               # Creates a thread that initializes the chassis since initializing the chassis is a blocking operation
        print('Staring chassis initialization')
        self.thread = threading.Thread(target = PxiChassisTester.chassisMain, args = [self.ackRequired, self.numIterations, burstLength])
        self.thread.setDaemon(True)                                                             # Daemon threads do not prevent the main program from finishing, used in case of missed hanging threads
        self.thread.start()

    # Packets are sent as strings and cast to the correct data types in the chassis
    def sendPacket(self, packet):
        self.rxCon.send(packet.encode('utf-8'))                                                 # Little confusing here, the rx connection is connected to the rx socket in the chassis, so we send packets there
        if self.ackRequired:
            ack = self.rxCon.recv(1024).decode('utf-8')
            assert(ack == DEFAULT_ACK_MESSAGE)                                                  # Double checking we really get an ack

    # Packets are received and returned as strings and must be casted to the correct data types
    def receivePacket(self):
        packet = self.txCon.recv(1024).decode('utf-8')                                          # Little confusing here, the tx connection is connected to the tx socket in the chassis, so we receive packets from there
        if self.ackRequired: self.txCon.send(DEFAULT_ACK_MESSAGE.encode('utf-8'))
        return packet

    # Sends several random individual pakets and checks they were received correctly
    def testSinglePacketCom(self):
        print('Starting Single Packet Communication Test')
        self.testCommunication()
        print('Single Packet Test Finished\n')

    # Sends random burst of packets to test the message buffering of the chassis
    def testBurstPacketCom(self):
        print('Starting Burst Packet Communication Test')
        self.testCommunication(self.burstLength)
        print('Burst Packet Test Finished\n')

    # Starts a test with a specified burst length over a specified number of total transfers
    def testCommunication(self, burstLength = 1):
        self.initializeChassis(burstLength)
        self.connectToChassis()
        self.sendTestPackets(burstLength)
        self.thread.join()                                                                      # Wait for chassis thread to finish before continuing
        self.closeConnections()

    def sendTestPackets(self, burstLength = 1):
        print('Starting Packet Transfer')
        for i in range(self.numIterations):                                                     # Runs for the specified number of transfers in num iterations
            messages = [PxiChassisTester.generateRandomString() for i in range(burstLength)]    # Each message is a random string to ensure a varierty of test inputs

            for message in messages:                                                            # Each transfer consists of the number of packets specified in burst length
                self.sendPacket(message)

            for message in messages:
                assert(message == self.receivePacket())                                         # The chassis sends back all the messages in the order received to check everythings works both ways
        print('Packet Transfer Complete')

    ### End of instance methods ###

if __name__ == "__main__":
    tester = PxiChassisTester()
    tester.communicationTest()