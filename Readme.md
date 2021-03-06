# Runner-Assistant
 
Vision system for autonomous robot supporting runners in training. Robot is driving in front of the runner on the track and keep proper pace with helps to acquire training goals. Device is using advanced lines tracking connected with PID speed and position control.

<p align="center">
  <img src="https://user-images.githubusercontent.com/62110076/109811145-be69af80-7c2a-11eb-88a9-19722bf09a8f.png"/ width="40%" > 
</p>


Program is running on Rasbperry Pi 3B+ with is connected by USB with Arduino Mega.
Vision system is based on C++ and OpenCv liblary.
### Elements and connections plan
<p align="center">
  <img src="https://user-images.githubusercontent.com/62110076/109812419-53b97380-7c2c-11eb-95f4-8dd0b00b5d90.png"/> 
</p>

## Image Processing in few steps

Image is processing in many steps, the most important are mentioned above:
* Orgnial image
* ROI - Region of Interests
* Grayscale
* Tresholing
* Bluring
* Next tresholing
* Canny operation - to find lines edges
* Hough Transform - to find straight lines
* Collection lines into vector
* Lines filtration
* Splitting lines into groups based on their slope
* Finding lines with belongs to 1 real line
* Finding centre of the track
* Counting difference between actual position and optimal position

### Example steps on the simulated image
![itscreen](https://user-images.githubusercontent.com/62110076/109809584-c1fc3700-7c28-11eb-9409-9d83b883d08b.png)

After that operations information is sended to the Arduino. uC recalculates value in pixels into the turning angle of robot. Moreover,  Arduino is responsible for:
* speed controlling
* sensors support
* communication with mobile app
* carrying out training program

## Validation on the real track

 ### Good quality surface
 ![image](https://user-images.githubusercontent.com/62110076/109810296-a9d8e780-7c29-11eb-945e-0fb9a665b833.png)
 
 ### Poor quality surface 
 ![image](https://user-images.githubusercontent.com/62110076/109810810-4b603900-7c2a-11eb-8188-ab57c769f656.png)
 
 ### Driving video
![Hnet-image (2)](https://user-images.githubusercontent.com/62110076/109814860-4782e580-7c2f-11eb-89b7-ba5c2ac5c78c.gif)



 

