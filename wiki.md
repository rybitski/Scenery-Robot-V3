# Robotski v3 Wiki

Thank you for choosing to use the Robotski system. This wiki will help you set up your system as quickly as possible, answer (most) of your questions, and give you a little bit of development history.

## History
(chris)
### Development of Version 3
A few factors led to the overhaul of the scenery robot that ended up becoming v3. The Intel Edison chip that was being used became obsolete, thus developing on it was not ideal. Additionally, the current software was unfit for a cue based system, the end goal of the project. Finally, the working group decided that an "Internet of Things" approach was best. Therefore, some major decisions were made, ie switching to an Arduino YUN and to rebuild the entire control interface, along with altering the communications framework. For a simple overview of how this system works, just look at this image below!

(andy pic)

#### An Exploration of Data Flow for Networked Stage Robotics

The Internet of Things (IoT) design paradigm is the central motivation behind the networked data flow of this stage robot iteration. Radio-based RC options provide low latency, low infrastructure deployment capabilities, but they lack the backend to support features like cueing, on-the-fly traversal updates, homing, and dynamic collision avoidance. By allowing the robot to interact with a WiFi network, the computing required to support a robust feature set can be distributed across devices to simultaneously optimize for latency and processing efficiency. The IoT approach also allows for multiple devices to interact over a single network, opening the doors for multiple robots, visualizers, and interfaces to synchronize as the stage robotics project continues to advance in the future.

Before the flow of data in this networked solution can be explained, it is first beneficial to explore each piece of hardware in the system to understand why it is responsible for the various modes of processing in the system. Beginning with the lowest level of compute-enabled hardware, the first item on the list is the Arduino processor. Arduino provides a well-supported environment for low-level device control and, in conjunction with libraries like Software Serial and Sabertooth, allows for intuitive programming for device drive control. Unfortunately, the Arduino’s device facing processor, the ATmega32u4, hereon the “Arduino processor,” is too lightweight to support the more complex network tasks required of the robot.

Enter the Arduino YUN, a device with the same hardware interfacing capabilities of an Arduino Uno but with a Linux environment directly on board. The YUN is not to be mistaken for a more powerful Uno, but rather it keeps the Arduino processor but adds an Atheros AR9331 MIPS processor, hereon the “Linux processor,” to support the network-facing capabilities of the board. The two processor are connected with a data bridge, so information from the Linux processor can be piped down to the Arduino processor, and likewise data from the Arduino processor can be sent upstream to the Linux environment. In the deployment used for the stage robot, the Arduino processor is responsible for device-facing tasks like driving the wheels and collecting encoder data. Notably, the Arduino processor does not determine how to drive the robot, it simply passes along and formats the drive command received from the Linux processor.

The Linux processor uses its network capabilities to communicate with the web server, where the cue control data structure is hosted alongside other server functions including a manual control interface. The Linux processor on the robot is but one device communicating with the server; other computers on the network can also see the control data for monitoring, visualization, and control purposes. The networked applications merely provide different interpretations of the same server data to suit the needs of the specific interface goal. The server is hosted on a Raspberry Pi physically connected to a router, though there are many alternatives to hosting a web page on a local network as well.

With the hardware laid out, it is now possible to examine the flow of data from the user to the robot’s hardware. There are many possible control interfaces for stage robotics, and if desired, it is still possible to drive the robot manually with a remote control. To accomplish this task, a computer on the network with a controller attached simply needs to log into the web interface so that controller data can be posted on the server.  An alternative option that is one of the more exciting features of the robot in its third revision is to control based of off a cue system, where traversals can be plotted out and saved in the web interface. This interface uses the capability of the cue data structure to calculate movement vectors based on end-point locations.

The choice of a web interface is key here. In V2, the application itself was a C-based executable runnable only on Windows machines. In order to make our platform as universal as possible, we decided to make it a Node.js based web interface, meaning that any computer with a web browser will be able to host and run our app.

Regardless of the source of the input, all control data eventually is posted on the server. Cue data is posted in the JSON format on a page, manual data is posted as a set of drive values on a separate page, and flags are hosted on yet another page. There is also a page set up for devices to talk back to the server without using a front end. This page is used by the robot to post encoder data and status reports, which can be interpreted for the front end by the server.

The Linux processor runs a Python script that scrapes these web pages and interprets the information into specific drive commands for the Arduino processor. For example, the user may update the cue list, which will cause the server to set a “load cue” flag that the Linux processor will see. When the flag is set, the Linux processor will scrape the JSON cue control data and interpret it into the cue data structure in the Linux system memory. The Linux processor will also communicate with the Arduino processor to terminate any drives currently in motion and set an LED indicator color. When the parsing of cue data is complete, the Linux processor posts to the server indicating a successful processing of data, prompting the server to revert the “load cue” flag.

Important considerations throughout the system are bandwidth, latency, and processing power, each of which vary between connection points in the data flow. For this reason, the Linux processor is also responsible for mathematically determining the execution of a cue in terms of specific motor drive values while balancing the equation with live encoder data send up from the Arduino processor. While the server is the most powerful device in the flowchart, with the possible exception of a client computer, it is too far abstracted from the hardware of the robot to calculate these drive commands fast enough, and even if it could, the Linux processor would become responsible for much more intense web parsing that may fall outside of its hardware capabilities. The cue vector is thus interpreted by the Linux processor into drive commands representing the acceleration, deceleration, and total drive time segments of the cue based on real time progress towards the destination. Encoder values are regularly posted to the network for monitoring. The drive data itself is sent down the bridge to the Arduino processor.

The Arduino processor’s algorithm is built with safety at the forefront. On every loop execution, the robot will immediately deploy its breaks unless it is explicitly told to power the drive wheels by the Linux environment. If there is a break anywhere in the chain of communication, the risk of a rouge drive is significantly mitigated by this algorithm’s halt assumption. Ideally, the Arduino processor would be responsible for processing the encoder data to avoid latency introduction from the two-way bridge travel, but the Arduino is not powerful enough to handle all of the different consequences of the data, so it must be sent up stream for processing.

In summary, a GUI is hosted on the local network for users to input cues for the robot’s movement. The user may decide to input direct measurements, draw the path over a ground plan, or even drive the robot freely with a controller. The cue list itself is a data structure consisting of vectors with an acceleration and deceleration, which are extracted from the path. The GUI input, whatever its source, is interpreted into the vector data structure, which is hosted on a Pi running a webpage on the local network. This page can be interpreted in different ways bet different devices. Multiple robots could derive their instructions from a common source, or a mobile application could provide real-time updates on expected movement all from the data on this page. Once the robot has copied the cue list, any cue can be triggered with a lightweight “Go” web update. A python script running on the YUN repeatedly check the local web pages for movement instructions and interprets them into specific motor control arguments for the adjacent microprocessor. The python script is also responsible for managing communication across the Bridge to the microprocessor, informing the network as needed. The YUN’s on-board Arduino processor reads values off of the bridge and applies them to the proper hardware devices using a Software Serial implementation of the Sabertooth library. The encoder devices on the robot report to the Arduino script, which in turn hands the required information back to the Bridge for processing on the Linux device. A custom PCB directs values from the YUN pinout an activates the motors according to the planned path of traversal in conjunction with an emergency stop system build outside of this dataflow.

#### Where We Go Next
The underlying greatest aspect of the IOT approach is its scalability. Future plans include the ability to host multiple robots in the same application, the ability to have more than one one laptop on the network without crossover issues, and adding functionality for mobile devices so that technicians will only have to open an app on their smartphone to cue up the robot and monitor its location. 

## Arduino Yun

### Introduction

The Arduino YUN is an extraordinary piece of hardware that is made infuriatingly difficult to use because of a poor software deployment and objectively false documentation. Regardless of its many issues, the YUN is the right tool for many networked robotics projects. This section of the Stage Robot Documentation attempts to alleviate the stress on the developer by explaining the exact deployment of the YUN that was found to be reliable. The developer will hopefully be able to create a mental model of the operation of the YUN through exploration of this guide. It is important to read this section of the documentation completely through before attempting to follow the setup instructions.

### Device Overview – What is the YUN?

The Arduino YUN Rev2 (not to be confused with the Rev1, the YUN Shield, or the YUN mini, all of which are commonly referred to as “the YUN” online, much to the dismay of debuggers) is a microcontroller consisting of a ATmega32u4 processor for Arduino sketches and a Atheros AR9331 MIPS processor responsible for running a Linux distribution, a modified flavor of OpenWRT. The ATmega32u4, hereon the “Arduino processor,” controls the device-facing side of the YUN, controlling pin voltages and serial communication as any Arduino Uno does. The Atheros AR9331, heron the “Linux processor,” controls the world-facing functionality of the YUN, which includes the SD card, the USB port, the flash file system, and most notably, the WiFI module.

The genius of the YUN is that both of these processors can communicate with each other over a modified serial connection called the “Bridge.” This opens the door to a massive world of possibilities for networked robotics projects. Obviously, being able to access web data from an Arduino sketch is a major benefit of this setup, but having a full Linux deployment in close communication with the Arduino microcontroller means that complex scripts can be written to interpret data from the world-facing devices and produce incredibly low-level results through execution from the Arduino processor.

The Achilles Heel of the YUN is its documentation, or lack thereof. The information published on the Arduino website is often factually incorrect about facets of current YUN devices being shipped, and many critical components of operating the YUN to its fullest potential are missing from the documentation and have instead been inferred from a web of forum posts where YUN users try to piece the project back together. Some have even gone as far as to author entirely new system images for the YUN, a fact this guide will exploit in the setup of the Stage Robot. Thankfully, the Arduino-side of the YUN shares the majority of its documentation with the YUN, and while few YUN-specific examples worth anything exist on the Arduino website, the API for Arduino sketches is fairly complete.

### Into the Tutorial – What You Will Need

In order to complete this tutorial, please secure the following resources if you have not used them yet in other sections of the documentation:

1.  An Arduino YUN Rev2
    
2.  A network router with both WiFi 2.4GHz and LAN connections and an outbound
    
    connection to the Internet (for now, and this can also be bridged from another computer).
    
3.  A Windows computer on the network with the following software installed:
	
    	a. Arduino IDE 
	    b. WinSCP
	    c. PuTTY  
	    d. TFTPUtil

4.  An Ethernet cable connecting the YUN to the router
    
5.  Your robot, configured following this documentation
    
6.  Completed download of the YUN images, pip archive file, and Python and Arduino code
    

#### Phase 0 – Get the YUN on the Correct Network

If you have not played with the Arduino YUN yet, you need to get it on your robot’s network before you can follow this tutorial. By default, the YUN will emit a local WiFi hotpot if it does not already have a network to connect to (or if that network disappears at any time). The YUN will remain “stranded” if this happens until explicitly reminded of the correct network to connect to.

Connect to the Arduino’s hotspot, and then access its web control interface by either typing in http://arduino.local into a web browser or, more reliably, entering the default IP address of the YUN, which is 192.168.240.1 . The default password is “arduino” without the quotation marks. Inside the editor, you must configure the YUN to connect to your network. You should also give the YUN a name, such as myYUN or robotBrain, and set a new password. After you fill out this information, the YUN will prompt you to change networks, and if you do this before the progress bar completes, you might be able to access the web interface again by clicking the hyperlink, but if that fails, you need to figure out what the IP of the YUN was set to by your router so you can access the web interface again in the future. Note this IP, as it will be needed later as well.

#### Phase 1 – Flash the YUN

The version of the YUN that ships at the time of the writing of this documentation does not seem to comply with any known documentation, and for this purpose, it is essential to flash the YUN to a known-working state where the rest of the tutorial can build from.

This phase draws from this tutorial, though not exactly:

https://www.arduino.cc/en/Tutorial/YunUBootReflash

The YUN has three sections of flash data, each of which needs to be erased and replaced in order. Follow the instructions for setting up a TFTP server on your computer, but target the images from this repository instead of the ones provided by Arduino.

With your TFTP server online and connected to the robot’s network and the YUN connected to your computer over USB and to the network over both WiFi and Ethernet, upload the included YunSerialMonitor sketch to the YUN board and open the serial monitor window.

At this time, possibly by using the ipconfig command in Windows Command Prompt, determine the IP address of your computer.

The following steps occur in quick succession. With serial monitor open, press the physical button on the YUN labeled “YUN Reset” or some equivalent. When you press this button, the serial monitor should rapidly populate, and you have around four seconds to type “ard” (or whatever the last prompt on screen says depending on your version) and enter it into the YUN. If you are successful, you will get access to the bootloader’s terminal.

Send the following commands over the serial monitor in order, waiting for the previous to complete and prompt you to enter the next command until you finish this list:

    setenv serverip 192.168.1.4;replace this IP with your computer’s IP, where TFTP server runs setenv ipaddr 192.168.1.7;the YUN’s IP  
    tftp 0x80060000 openwrt-ar71xx-generic-linino-u-boot.bin;
    
    erase 0x9f000000 +0x40000;  
    cp.b $fileaddr 0x9f000000 $filesize; erase 0x9f040000 +0x10000;
    
    tftp 0x80060000 openwrt-ar71xx-generic-yun-16M-kernel.bin;
    
    erase 0x9fEa0000 +0x140000;  
    cp.b $fileaddr 0x9fea0000 $filesize;
    
    tftp 0x80060000 openwrt-ar71xx-generic-yun-16M-squashfs-sysupgrade.bin
    
    erase 0x9f050000 +0xE50000;  
    cp.b $fileaddr 0x9f050000 $filesize;
    
    bootm 0x9fea0000;

The YUN will now reboot itself and forget which network it was connected to, so you must repeat Phase 0. The YUN is now flashed to the proper image for this documentation.

#### Phase 2 – Install Python Requests Library

The Linux processor’s feeble Bridge to the Arduino processor is managed by a mysterious Python library called, you guessed it, Bridge. This Bridge library is the main attraction for

development in Python on the YUN, because the lower-level access routines required for C/C++ to use the hardware bridge is usually beyond the scope of hobby electronics. That said, installing Python libraries is tedious. Thankfully, the heavy lifting has been accomplished already during the flash, so it is now possible to install the only library this tutorial requires, requests, somewhat “easily.”

First, connect to the file hierarchy of the YUN using WinSCP. For reference:

File Protocol: SCP  
Host name: the IP address of the YUN  
Port number: 22  
User name: root  
Password: the password you set in the web interface

Go to the usr/lib folder and create a new folder. The tutorial names it “andyCode” for simplicity. Inside andyCode, drag and drop the pip tar file from your downloads. Creating it was a labor of love.

Open a PuTTY session. Assuming you know basic BASH commands, CD to the andyCode folder. If you do not, go learn some basic BASH commands. In your terminal, type:

    opkg update
    opkg install distribute
    okpg install python-openssl
    easy_install pip-6.0.8.tar.gz

These will take a fair amount of time to run. When they complete, delete the pip tar file from the YUN. Now run:

    pip install requests 

This will install the requests library on the YUN.

#### Phase 3 – Migrating the Python Code and Understanding the Bridge

Open the Python code in your IDE of choice, perhaps Visual Studio 2017. It is important to understand how it works in case it needs to be adapted. The first line is required for execution, and the following lines import the required libraries for web and bridge access and configure each. The infinite loop scrapes the robot’s control website and parses it into individual strings separated by commas. There are eight such arguments and an additional clock report at the end. The clock is intended to check if the data has become “stale,” possibly due to a network error. If the data is stale, a set of zeros will be printed, indicating an implicit stop command. Otherwise, the data will be updated to the integer representation offset by 127 to ensure that all values are positive or zero. Note that the buffer is flushed after each print. This is for Arduino processing.

The time is updated at the end of the loop, and a short sleep is added to keep the bridge from becoming backlogged with many more updates than the slower Arduino processor can parse in a reasonable amount of time. The value of this sleep command is entirely experimental and constitutes a careful balancing act between the two processors to make sure the bridge does not hold data between the processors for too long before being acted upon. This supports the design philosophy of only sending data when it is needed, which is very helpful for reducing the latency between a drive command sent from the physical controller and the response from the robot. Note that this example sketch currently ignores encoder value parsing back from the Arduino processor to the Linux processor, and this functionality will be added in a near future update.

Drop the Python middleware code in the andyCode folder on the YUN. It will need privileges to execute:

    chmod +x Middleware_for_Controller.py

Verify that the file is executable (will now appear green when using ls command)

Test it with the code:

    python Middleware_for_Controller.py

and make sure it prints as expected. Interrupt it with CTRL+C when the functionality is confirmed. It will require the robot data server being active.

The Python code is now configured for execution on the YUN’s Linux processor.

#### Phase 4 – Uploading and Understanding the Arduino Sketch

Open the provided Arduino sketch in the Arduino IDE and make sure the serial monitor is open. Understanding this firmware is also important for debugging and modification in the future.

The firmware uses the Software Serial library to implement the Sabertooth library because there is potential for a collision between Sabertooth and the Bridge library that can be avoided by routing through Software Serial. The Bridge library (now the Arduino Bridge library, not the mysterious Python Bridge library) is imported and configured. Process is where most of the Bridge communication will occur, though, so it is also imported.

Setup completes required tasks for the encoders and motors and runs the Python code on the Linux processor. This is achieved through the Process library’s runShellCommandAsynchronously method, and this will essentially send a shell command to the Linux processor and then continue execution of the Arduino code regardless of what happens with that shell command on the Linux processor. As a result, the code must wait for p.available() to become true, which indicates that values are being printed from the Linux processor to the console (and thus, to the bridge). The code is now ready to enter the loop body.

Emergency stop is checked at the beginning to determine if a flag should be raised to activate the breaks. Note that regardless of if this code is executing properly, the segregated emergency stop system will cut power to the motors. Having the Arduino code running simply adds in an additional breaking action so the robot will stop immediately versus drifting a bit further with momentum.

Each button on the controller has an integer value in the loop, and these are each updated by reading an integers from the bridge in sequential order or by implicitly setting them to zero if no new data is available. If both bumpers are depressed, execute a deliberate break. Otherwise, if the right trigger “dead man” is depressed, unlock the break and execute a drive based on the joystick data. If neither of the above but the aButton is depressed, execute some action for the lift depending on the dPad data. If no condition has yet been met, execute an implicit break.

Congrats! You are finished with the hard part. From here on out setting up your robot will be a walk in the park. 

## The Server

### Introduction
The next step is to set up the server. The server used here is called Express.js and, Express is a minimal and flexible Node.js web application framework that provides a robust set of features for web and mobile applications. For more explanations on exactly what node is, refer to the Control Interface portion.

### What is Express and what is a Pi?
Express is a web application framework that is based on Node.js, which itself is basically more robust way to run Javascript for a website. Express is not a very strong server, focusing more on being lightweight and easy to use. This is ideal for our purposes since we are not sending much data at all, rather we need a server that can work on small devices like the Pi.

What is this Raspberry Pi? It is a small single board computer with a large number of capabilities but a limited performance level. For us, the Pi is just a tiny computer tower. We basically just need it to act as a computer, so the server code can turn it into a server. 

### Making your Server
What you will need to set up a server:

 1. A laptop
 2. A Raspberry Pi
 3. An SD card
 4. The router you already set up
 5. A monitor, keyboard and mouse to connect to the Pi

#### Step 1 - Setting up the Pi
The Pi does not come preloaded with a robust operating system so we need to actually put something on it that can run. I recommend using Raspbian, which I have found is easy to install and use. To install Raspbian, just refer to their easy guide: [https://www.raspberrypi.org/downloads/raspbian/](https://www.raspberrypi.org/downloads/raspbian/). The one addition I have is that plugging in a keyboard, mouse and monitor to the Pi makes its setup miles easier. 

#### Step 2 - Setting up Node.js on the Pi
At this point, when you boot your Pi, there should be a normal GUI that looks like any other desktop. Now you need to connect to actual wifi (NOT the router that you have yet). Go into the terminal and install npm and node. There are a couple steps to this (most of this tutorial was from biskis from his [page](https://www.instructables.com/id/Install-Nodejs-and-Npm-on-Raspberry-Pi/) . First enter the command:

    uname -m
There are three options: ARMv6, ARMv7 or ARMv8. If the response starts with armv6 than that's the version that you will need. For example for raspberry pi zero W you will need ARMv6. 
Go to  [node.js download page](https://nodejs.org/en/download/)  and check right click on the version of ARM that you need and choose Copy Link address.

After that in the terminal using wget download the tar.gz file for that version. Just type wget, paste the link copied before and make sure the extension is .tar.gz. If it's something else change it to this and it should be ok. For example I will need ARMv6 and I will type this in my terminal:

    wget https://nodejs.org/dist/v8.9.0/node-v8.9.0-linux-armv6l.tar.gz
Using tar that is already installed with the system on your raspberry pi just type this (make sure you change the filename with the file that you have)

    tar -xzf node-v8.9.0-linux-armv6l.tar.gz
Then copy Node.js to /usr/local

    cd node-v6.11.1-linux-armv6l/
    sudo cp -R * /usr/local/
Check if node and npm are installed correctly. This lines should print the version of node and npm installed.

`node -v`  
`npm -v`

#### Step 3 - Getting the Server Script on the Pi
This step is relatively easy. All you have to do is go on to our github and download the "Middleman" folder. Make sure it is in a locale under usr/local (so it has access to Node and npm). 

#### Step 4 - Making the Server "Forever"
An important aspect is making the Pi portable and self sufficient. Therefore, we don't want to have to open up a monitor and use a mouse/keyboard every time we want to start a server. We will use a script called [forever.js](https://github.com/foreverjs/forever) to have the server start on bootup regardless of anything else. All you have to do is open up the terminal and enter: 

`sudo npm install forever -g`

Then navigate (in the terminal) to the Middleman directory and type this into terminal:

`forever start index.js`

#### Step 5 - Setting the correct network
At this point, you no longer need connection to the internet to use the Pi. There are two ways to connect to the robot's wifi. The one we recommend is to hardwire the connection directly from the router to the Pi with an ethernet cable. The option is to go to change the wifi in the top right of the desktop to the correct wifi. Either way, after doing this, navigate to the /etc/wpa_supplicant/wpa_supplicant.conf file in the Pi and delete all other networks.

That's it! You now have a Pi serving as a virtual server!


## Control Interface 
Finally, you're at the GUI portion! 

### Introduction
There's no HTML or CSS in this web app, except for a skeleton page. That's because I'm using a bootstrapped React.js framework. React was originally developed by Facebook then released to the public. React as a standalone library is used as single page or mobile applications, as it's optimal only for its intended use of being the quickest method to fetch rapidly changing data that needs to be recorded, which is clearly our main need (two way parsing of control and encoder data at hight speed). However, fetching data is only the beginning of what happens on a web page, which is why complex React applications usually allow for the use of additional libraries, like Bootstrap, which is a quick way to style a page effectively. The best part of React is that I give you all the libraries you need, so you don't need any drivers or whatever. It just works!

### Getting the App Running

 1. To run this code, you only need Node.js and npm. To install these
    dependencies, please refer to
    https://docs.npmjs.com/downloading-and-installing-node-js-and-npm. 
    
 2. Download the client and middleman folders. The entire "client" folder **needs** to be copied into this "middleman" folder in this repo.  

 3. Now you want to change the proxy setting for the web app. Basically you want to point it to the correct server address. Open up the package.json file in the client folder, **NOT** the middleman one. Find the line   

	   ` "proxy": "http://localhost:5000",`
	   
	   Now change gears, and go to the Pi. Open up the terminal and type in 
	
	   `sudo ifconfig`
	
	Next to the **wlan0** entry you will see inet addr like : **192.168.1.10** (or something). That's the IP address of the Pi. Go back to that line of code in the package.json and change the "localhost" word to that string of numbers.
 4. Now open up the terminal. Navigate into the client directory and type in `npm start`. Your browser should open up with the app! You don't need anything else.

Congratulations, you're ready to start using your robot!

## FAQs 
**What on earth is this terminal and how do I tell it to do things?**
This terminal is a way for you to communicate directly to the computer to tell it to do things like run programs without the need for a graphical interface. It's basically the same for Mac and Linux computers so I suggest taking 5 minutes to look at a handy [tutorial](https://macpaw.com/how-to/use-terminal-on-mac). If you're on Windows in 2019, you're in luck! Windows 10 now offers the ability to run a bash shell natively. Just refer to [this](https://www.howtogeek.com/249966/how-to-install-and-use-the-linux-bash-shell-on-windows-10/). 

**How do I proceed if I cannot locate my YUN on the WiFi network?**

The first and easiest item to check is to see if the YUN is emitting its own WiFi signal. If it is, it has forgotten your WiFi network and will need to be set up again with the network details. The YUN will “strand” itself if it boots up without the WiFi network being available, and it will not make further attempts to connect to the known network even if the YUN is restarted. If the YUN does not emit a WiFi signal, there are many free mobile apps that allow you to scan for devices on the network, and if the YUN is connected, you should be able to see its IP address.

  

**I cannot access the YUN with .local**, **so how do I load the web interface?**

On some configurations, the YUN will not be accessible with “.local” URL formats, so you must type in the IP address of the YUN directly into the web browser. If the YUN is emitting its own WiFi signal, it will use the default IP address of 192.168.240.1 .

  

**How do I perform runtime debugging for my code if it is running on the YUN?**

The Arduino code Process simply triggers a session on the Linux environment where the code executes, so you can see what the code outputs without running it from the Arduino code, instead using SSH over PuTTY, which will show terminal output. Be careful if you use printline debugging, as the console is what the Process library reads to process Bridge data.

**How do I contact you if I have more questions, bugs to report, or general problems with our entire framework?***

If you navigate to [https://github.com/rybitski/Scenery-Robot-V3/issues](https://github.com/rybitski/Scenery-Robot-V3/issues), the issues section of our GitHub, and post the matter at hand, the developer most able will respond as quickly as we can.



