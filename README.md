# uwhd

APRIL 20:
    kinect(1) = host kinect / kinect(0) = guest kinect
    removed diagnostics mode

APRIL 19:

    xbox 360 USB gamepad controls now working if USE_GAMEPAD enabled. FPS style controls.  Just need to customize further!

    also...be sure to build as release as the framerate performance drastically increases

APRIL 14:

    added switch in ofApp.h - simply comment / uncomment to enable which mode you want to use:


USE_KINECT
    if both this and USE_TWO_KINECTS are commented out, it will disable all Kinect stuff for debugging purposes

USE_TWO_KINECTS
    use two kinect point clouds

USE_GAMEPAD
    use Xbox360 gamepad
    *REQUIRES XBOX 360 controller driver to be installed which is in the root folder

USE_PHOTOBOOTH
    enable photobooth mode with 2D backgrounds and peasy camera instead of FPS camera

USE_HOSTMODE
    enable live host mode


KEYBOARD CONTROLS!

    w = walk forward

    a = walk left

    s = walk backwards

    d = walk right

    e = elevate camera

    c = lower camera


GAMEPAD CONTROLS:

    LEFT ANALOGUE STICK = walk forwards / backwards / sideways

    RIGHT ANALOGUE STICK = move camera / look around (like mouse in FPS game)

    LEFT BUTTON = rotate camera counterclockwise

    RIGHT BUTTON = rotate camera clockwise

    LEFT TRIGGER = elevate camera

    RIGHT TRIGGER = lower camera


IMAGE WRITER:

    ENTER person's email address in the text box and press enter

    Then press F5 to take a snapshot...it will automatically save out the PNG file with their email in it as well as a numerator at the end - i.e. user@user.com_01.png.  It will then automatically go into review mode and will display the last image taken and you can see the file name of the image it is displaying in the text box on the left.  You press F4 to go back to the live mode.  The images end up in the /data/ folder.

    ` / tilda = diagnostics mode

    F1 = full screen

    F4 = point cloud mode / live mode
        
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