# FlightSoftware
All code flying on the rocket must be in this repository.  
  
&nbsp;

&nbsp;
  
## Environment Setup
### 1 Install Eclipse and NI Linux Real-Time toolchain

#### Windows
Follow the instructions in sections 1-3 of [this guide from NI](http://www.ni.com/tutorial/14625/en/)	

#### Linux
[Install Eclipse](https://www.eclipse.org/downloads/)  

Then install the [nilrt toolchain](http://www.ni.com/download/labview-real-time-module-2017/6760/en/)   

Next, install the Target Management package for Eclipse. To do this, open Eclipse and navigate to **Help** > **Install New Software**.  

Click **Add**. Enter any name, and in the location box enter `https://download.eclipse.org/tm/updates/4.5.100-SNAPSHOT/repository/`.  

Select all of the software packages and click **Next**.  

Follow the instructions to finish installing the pakages. 

&nbsp;


### 2 Clone the repository

*You must clone all Eclipse projects to your Eclipse workspace directory*

#### Windows 

```
cd c:\users\<username>\workspace
git clone git@github.austin.utexas.edu:trel/FlightSoftware.git
```

#### Linux
```
$ cd ~/eclipse-workspace
$ git clone git@github.austin.utexas.edu:trel/FlightSoftware.git 
```
&nbsp;

### 3 Set environment variables 

#### Windows 
 Open the start menu and search for **environment variables**. 
 
 Select **Edit the System Environment Variables**. 
 
 
 In the window that opens, click the **Environment Variables** button. 
 A second window will open. In the **System Variables** section, click **New**. 
 
 
 Enter `NILRT_CROSS_GCC_PATH` as the variable name and `C:\build\17.0\arm\sysroots\i686-nilrtsdk-mingw32` as the variable value. 
 
 
 Click  **OK** and then click **New** again to create another variable. 
 Enter `NILRT_TOOLCHAIN_PATH` as the name and `C:\build\17.0\arm\sysroots` as the value. 
 Click **OK** on all windows. 
 
#### Linux
No additional setup needed. The `run_eclipse_linux.sh` script will start Eclipse with the correct environment variables set. 

```
$ cd ~/eclipse-workspace/FlightSoftware
$ sh run_eclipse_linux.sh
```

*Note that* `run_eclipse_linux.sh` *assumes that the FlightSoftware repo is in* `~/eclipse-workspace/FlightSoftware` *and eclipse is installed in* `~/eclipse`.

*Also note that the repo* will not build *if Eclipse is opened by some method other than the provided script. The script is required to set the environment variables that point to the nilrt cross compiler.*  
  
&nbsp;

&nbsp;

&nbsp;
    
## Building the Code
Right click the project in Eclipse. Under **Build Configurations** > **Set Active** ensure **Debug** or **Release** is selected.
Right click the project again and select **Build Project** (also **ctrl-b**).  
  
&nbsp;

&nbsp;

&nbsp;
  
## Deploying and Running

### Connecting to the sbRIO
Ensure Eclipse is connected to the sbRIO [see section 6 of [this guide from NI](http://www.ni.com/tutorial/14625/en/)].   
 
Note that on Linux, you may have to set the ethernet connection to be **Link-Local Only** in order to connect to the sbRIO. On Ubuntu 16.04, you can do this in **Settings** > **Network** > **Wired** > **Options** > **IPv4 Settings** and set Method to **Link-Local-Only**.   

 
To check the conenction, try pinging the board.  
 
```
$ ping NI-sbRIO-9637-01dea4db.local
```
  
The last 8 digits of the hostname are the board's serial number and will differ between boards. 

### Adding a Run configuration
The first time you deploy to the sbRIO, you will need to add a run configuration. 

Follow the instructions in section 7 of [this guide from NI](http://www.ni.com/tutorial/14625/en/), creating a run configuration called **FlightSoftwareMainDebug** that points to the `Debug/FlightSoftware` executable (by selecting it in the **C/C++ Application** field). Then create another configuration called **FlightSoftwareRunTest** that points to the `Test/FlightSoftwareTest` executable. 

Note, on Linux, the sbRIO may not available in the **Connection** dropdown. If this happens, you will need to click **New** next to **Connection**, select **SSH** from the dropdown, and click **OK**. Then under **Host** enter the board's hostname (e.g. `NI-sbRIO-9637-01dea4db.local`), under **User** enter `admin`, and then select **Password Based Authentication** and enter the password for the admin user. 


## Running


 Right click the project and select **Run As** > **FlightSoftwareMainDebug**.  
 
If the build config you're looking for doesn't show up under **Run As**, you can find it by clicking **Run As** > **Run Configurations**. 
 
 
Stdout will be displayed in the Eclipse console window.
  
&nbsp;

&nbsp;

&nbsp;
  
## Running Tests
Follow the instructions above for building and deploying, but select **Test** as the build configuration and **FlightSoftwareTest** as the run configuration.  
  
&nbsp;

&nbsp;

&nbsp;
  
## Building and Running for x86 Linux
It is possible to build and run the code on an x86 Linux system (like a PC). Note however that this is only a last resort if access to an NILRT system is not available. 

All tests *must* pass on an sbRIO or NILRT emulator before a pull request can be made. 

To build, use the **Debug_x86** or **Test_x86** configurations. 

To run, run as a **Local C/C++ Application**. 



