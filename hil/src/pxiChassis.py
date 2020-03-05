import socket, threading, struct, queue

### Global Variable Define Statements
MAX_CHASSIS_MESSAGE_SIZE    = 1024           # Max number of bytes a message can be
DEFAULT_IP          = "127.0.0.1"            # IP Address the communication lib will try and connect to the chassis on
DEFAULT_TX_PORT     = 8081                   # Ports for the TX and RX socket respectively
DEFAULT_RX_PORT     = 8082
DEFAULT_ACK_MESSAGE = "OK"                   # By default communication with the chassis is synchronous on ack messages, can be turned off
### End of Global Defines

# PxiChassis is an abstraction for handling communication with the PXI Chassis.
# Communication is done via two open sockets, one tx and one rx socket for two.
# way communication. Each thread has a message buffer that acts as a FIFO queue.
# The rx buffer holds incoming messages waiting to be read by the simulation.
# The tx buffer holds outgoing messages waiting to be sent to the PXI chassis.
# There is a handler thread for each buffer. The tx thread sends each outgoing
# message one at a time to the chassis, while the rx thread sits listening on
# the rx socket for new messages and puts each new message into the rx buffer.
class PxiChassis():

    ### Start of static methods ###

    # Adds error handling for socket handlers in case of networking issues
    def threadMainDecorator(threadFunc): 
        def errorHandlingWrapper(connection, parent):
            while True:
                try:
                    threadFunc(connection, parent)
                except (ConnectionAbortedError, IOError):
                    connection.close()
                    break
                
        return errorHandlingWrapper

    # Polls the transmit buffer until a message is given to send to the chassis
    @threadMainDecorator
    def txThreadMain(connection, parent):                           
        while parent.txBuffer.empty() is True :
            pass
        message = parent.txBuffer.get()
        connection.send(message.encode('utf-8'))
        if parent.ackRequired:
            ack = connection.recv(MAX_CHASSIS_MESSAGE_SIZE).decode('utf-8')                     # Receiving ack to synch communication
            assert(ack == DEFAULT_ACK_MESSAGE)                                          # Double checking we really get an ack 

    # Polls the rx socket for incoming messages that then get stored in the rx buffer
    @threadMainDecorator
    def rxThreadMain(connection, parent):
        message = connection.recv(MAX_CHASSIS_MESSAGE_SIZE).decode('utf-8')
        if parent.ackRequired: connection.send(DEFAULT_ACK_MESSAGE.encode('utf-8'))     # Sending ack to synch communication
        parent.rxBuffer.put(message)

    ### End of static methods ###

    ### Start of instance methods ###
        
    def __init__(self, IP = DEFAULT_IP, txPort = DEFAULT_TX_PORT, rxPort = DEFAULT_RX_PORT, ackRequired = True):
        self.IP = IP
        self.txPort = txPort
        self.rxPort = rxPort 
        self.ackRequired = ackRequired
        
        self.txBuffer = queue.Queue()                                                   # Using a thread safe queue implementation for each buffer
        self.rxBuffer = queue.Queue()

        self._initializeCommunicationToChassis()

    # Creates two threads to monitor communication on the rx and tx sockets respectively
    def _initializeCommunicationToChassis(self): 
        self._initializeSockets()
        self.txHandlerThread = threading.Thread(target = PxiChassis.txThreadMain, args = (self.txSocket, self))
        self.rxHandlerThread = threading.Thread(target = PxiChassis.rxThreadMain, args = (self.rxSocket, self))
        self.txHandlerThread.start()
        self.rxHandlerThread.start()
        
    # Opens up the tx and rx sockets to the PXI chassis
    def _initializeSockets(self):                
        self.txSocket = socket.socket()
        self.rxSocket = socket.socket()
        self.txSocket.connect((self.IP, self.txPort))                                   # If these ports are in use an error will be thrown and the simulation will crash
        self.rxSocket.connect((self.IP, self.rxPort))

    # Once the message is in the tx buffer the tx thread will pop it out and send it when available
    # The buffer is unbounded and so this call will never block or throw an error on a write
    def write(self, message):
        self.txBuffer.put(message)           

    # Messages are stored as a queue, so read returns the oldest message received from the chassis
    # read on an empty queue will block the caller until the queue is no longer empty
    def read(self):
        while self.rxBuffer.empty() is True:
            pass
        message = self.rxBuffer.get()          
        return message
    
    ### End of instance methods ###


# Basic test code where the user inputs a message and the chassis will echo it back out
# Just used as a simple  test to see if basic packets are getting sent correctly
if __name__ == "__main__":
    chassis = PxiChassis()
    while True:
        messageToSend = input("Enter a message to send>>")
        chassis.write(messageToSend)
        print("Message sent")
        reply = chassis.read()
        input("Message received from chassis: {0}".format(reply))
    
