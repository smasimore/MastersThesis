# FlightSoftware
All code flying on the rocket must be in this repository.

## Building the code
Right click the project in Eclipse. Under **Build Configurations** > **Set Active** ensure **Debug** or **Release** is selected.
Right click the project again and select **Build Project** (also **ctrl-b**).

## Deploying and Running
Ensure Eclipse is connected to the sbRIO [see section 6 of [this guide from NI](http://www.ni.com/tutorial/14625/en/)].
 Right click the project and select **Run As** > **FlightSoftwareMainDebug** or **FlightSoftwareMainRelease**. 
 Stdout will be displayed in the Eclipse console window. 
 
## Running Tests
Follow the instructions above for building and deploying, but select **Test** as the build configuration and **FlightSoftwareTest** as the run configuration.
