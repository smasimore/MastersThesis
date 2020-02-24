import socket, threading, struct

### Global Variable Define Statements
DEFAULT_IP        = "127.0.0.1"            # IP Address the communication lib will try and connect to the chassis on
DEFAULT_TX_PORT   = 8081                   # Ports for the TX and RX socket respectively
DEFAULT_RX_PORT   = 8082
TX_LOCK = threading.Lock()                 # Each thread has a lock for their respective buffer to avoid race conditions
RX_LOCK = threading.Lock()
DEFAULT_ACK_MESSAGE = "OK".encode('utf-8') # By default communication with the chassis is synchronous on ack messages, can be turned off
### End of Global Defines

# PxiChassis is responsible for handling communication with the Pxi Chassis during simulation.
#Communication is done via two open sockets, one being a rx socket
# and the other a tx socket to allow for two way communication. Each thread has a message buffer
# attached. The rx buffer acts as a queue for incoming messages waiting to be read.
# The tx buffer acts as a queue where messages are waiting to be sent.
# Each thread is monitored by a seperate thread that deals with emptying and filling their buffers.
class PxiChassis():

    ### Start of static methods ###
    def threadMainDecorator(threadFunc):                            # Adds error handling for socket handlers in case of networking issues
        def errorHandlingWrapper(connection, parent):
            while True:
                try:
                    threadFunc(connection, parent)
                except ConnectionAbortedError:
                    print('Connection to chassis aborted, closing socket')
                    connection.close()
                    break
                
        return errorHandlingWrapper

    @threadMainDecorator
    def txThreadMain(connection, parent):                           # Polls the transmit buffer until a message is given to send to the chassis
        while len(parent.txBuffer) == 0:
            pass
        TX_LOCK.acquire()                                           # Using a lock to prevent race conditions with parent 
        message = parent.txBuffer.pop(0)
        connection.send(message.encode('utf-8'))
        if parent.ackRequired: connection.recv(1024)                # Receiving ack to synch communication
        TX_LOCK.release()

    @threadMainDecorator
    def rxThreadMain(connection, parent):
        message = connection.recv(1024).decode('utf-8')
        if parent.ackRequired: connection.send(DEFAULT_ACK_MESSAGE) # Sending ack to synch communication
        RX_LOCK.acquire()                                           # Using a lock to prevent race conditions with parent
        parent.rxBuffer.append(message)
        RX_LOCK.release()

    ### End of static methods ###

    ### Start of instance methods ###
    def __init__(self, IP= DEFAULT_IP, txPort= DEFAULT_TX_PORT, rxPort= DEFAULT_RX_PORT, ackRequired= True):
        self.IP = IP
        self.txPort= txPort
        self.rxPort = rxPort 
        self.ackRequired= ackRequired
        
        self.txBuffer = []
        self.rxBuffer = []

        self.initializeCommunicationToChassis()

    def initializeCommunicationToChassis(self):  # Creates two threads to monitor communication on the rx and tx sockets respectively
        self.initializeSockets()
        self.txHandlerThread = threading.Thread(target= PxiChassis.txThreadMain, args= (self.txSocket, self))
        self.rxHandlerThread = threading.Thread(target= PxiChassis.rxThreadMain, args= (self.rxSocket, self))
        self.txHandlerThread.start()
        self.rxHandlerThread.start()

    def initializeSockets(self):                 # Opens up the tx and rx sockets to the chassis
        self.txSocket = socket.socket()
        self.rxSocket = socket.socket()
        self.txSocket.connect((self.IP, self.txPort))
        self.rxSocket.connect((self.IP, self.rxPort))


    def write(self, message):
        TX_LOCK.acquire()
        self.txBuffer.append(message)           # Once the message is in the tx buffer the tx thread will pop it out and send it when available
        TX_LOCK.release()

    def read(self):
        while len(self.rxBuffer) == 0:
            pass
        RX_LOCK.acquire()
        message = self.rxBuffer.pop(0)          # Messages are stored as a queue, so read returns the oldest message from the chassis
        RX_LOCK.release()
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
    

