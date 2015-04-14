# uwhd

APRIL 14: added switch in ofApp.h - simply comment / uncomment to enable which mode you want to use:
USE_KINECT - if both this and USE_TWO_KINECTS are commented out, it will disable all Kinect stuff for debugging purposes

USE_TWO_KINECTS - use two kinect point clouds

USE_GAMEPAD - use Xbox360 gamepad
*REQUIRES XBOX 360 controller driver to be installed which is in the root folder

USE_PHOTOBOOTH - enable photobooth mode with 2D backgrounds and peasy camera instead of FPS camera

USE_HOSTMODE - enable Hot Docs live host mode


FUNCTION KEYBOARD CONTROLS!

w = walk forward

a = walk left

s = walk backwards

d = walk right

e = elevate camera

c = lower camera


` / tilda = diagnostics mode

F1 = full screen

F4 = point cloud mode


IMAGE WRITER:

F5 = save PNG of entire OF window

F6 = review last saved file (saved into the data/bin folder)

F4 = go back to live point cloud (during review mode)


POINT CLOUD ADJUSTMENTS:

[ = reduce point cloud size

] = increase point cloud size

, = decrease depth "far" distance threshold (i.e. show less of the surrounding room)

. = incraese depth "far" distance threshold (i.e. show more of the surrounding room)

- = decrease depth "near" distance threshold (i.e. allow things to get closer to the camera)

= = increase depth "near" distance threshold (i.e. restrict things that are too close to the camera)


KINECT MOTOR ADJUSTMENTS:

0 = zero the angle of the kinect

o = return to last set angle

key arrow up = increase angle of Kinect

key arrow down = decrease angle of Kinect