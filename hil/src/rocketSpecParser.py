from openpyxl import Workbook, load_workbook
from sensorObject import SensorObject
import pandas as pd

### Global variable define statements
DEFAULT_WORKBOOK_NAME = 'Sensor_Workbook'
DEFAULT_SHEET_NAME = "Sensor_Info"
DEFAULT_EXTENSION = '.xlsx'
### 

# Parses the sensor spec sheet into a data frame and can provide
# sensor info per request to the simulation. All sensor info is
# wrapped by a SensorObject that helps make the interface cleaner
class RocketSpecParser():

    # The spec sheet is stored in a data frame to speed up accessing time
    def __init__(self):
        self.sensorSpecs = pd.read_excel(DEFAULT_WORKBOOK_NAME + DEFAULT_EXTENSION)  

    # Returns a sensor object that acts as a wrapper around accessing the sensor info
    def getSensorInfo(self, sensorName):   
        row  = self.sensorSpecs.loc[self.sensorSpecs['Common Name'] == sensorName].iloc[0].values
        sensor = SensorObject(row)
        return sensor

if __name__ == "__main__":
    parser = RocketSpecParser()
    print(parser.getSensorInfo("ParachuteSensor"))
    