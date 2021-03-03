# Runner-Assistant
 
Vision system for autonomous robot supporting runners in traning by driving in front of them with helps to keep proper running pace. 
Advanced lines tracking connected with PID speed and position controll acquire optimall support to realise traning goals. 

Program is running on Rasbperry Pi 3B+ with is connected by USB with Arduino Mega.
Vision system is based on C++ and OpenCv liblary.

## Image Processing in few steps
Image is procssing in many steps, the most important are mentioned above:
* Orgnial image
* ROI - Region of Intrests
* Grayscale
* Tresholing
* Bluring
* Next tresholing
* Canny operation - to find lines edges
* Hough Transform - to find straight lines
* Collection lines into vector
* Lines filtration
* Spliting lines into groups besed on their slope
* Finding lines with belongs to 1 real line
* Finding centr of the track
* Counting difference between actual position and optimal position


After that operation data is send to the Arduino. uC recalculates value in pixels into the turning angle of robot.
Moreover it Arduino is responsible for:
-speed controlling
-sensors support
-communication with mobile app
-carrying out traning program

 
