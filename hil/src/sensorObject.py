### Global variable defines
COMMON_NAME_INDEX                   = 0
SENSOR_NAME_INDEX                   = 1
SENSOR_TYPE_INDEX                   = 2
SENSOR_COMMUNICATION_PROTOCOL_INDEX = 3
SENSOR_PIN_INDEX                    = 4
###

# Sensor Object serves as a wrapper around data retreived from the
# spec sheet to abstract the interface between the the simulator and
# how it access sensor information
class SensorObject():

    def __init__(self, info):
        self.info = info # Info is an array of all the info about the sensor

    def getSensorSpecs(self):
        return self.info

    def getSpec(self, index):
        return self.info[index]

    def getCommonName(self):
        return self.getSpec(COMMON_NAME_INDEX)

    def getSensorName(self):
        return self.getSpec(SENSOR_NAME_INDEX)
        
    def getSensorType(self):
        return self.getSpec(SENSOR_TYPE_INDEX)

    def getSensorCommunicationProtocol(self):
        return self.getSpec(SENSOR_COMMUNICATION_PROTOCOL_INDEX)

    def getSensorPin(self):
        return self.getSpec(SENSOR_PIN_INDEX)

    def __str__(self):
        string = ''
        for spec in self.info:
            string += str(spec) + ' '
        return string.strip()
    
if __name__ == "__main__":
    sensor = SensorObject(["ParachuteSensor", "esp32", "analog", "NA", "2"])
    print(sensor.getSensorSpecs())
    print(sensor.getCommonName())
    print(sensor.getSensorName())
    print(sensor.getSensorType())
    print(sensor.getSensorCommunicationProtocol())
    print(sensor.getSensorPin())
    print(sensor)
