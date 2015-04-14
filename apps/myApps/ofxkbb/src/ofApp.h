#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxAssimpModelLoader.h"
#include "ofVboMesh.h"
#include "ofxGamepadHandler.h"
#include "ofAppGlutWindow.h"
#include "ofxGameCamera.h"

// Windows users:
// You MUST install the libfreenect kinect drivers in order to be able to use
// ofxKinect. Plug in the kinect and point your Windows Device Manager to the
// driver folder in:
//
//     ofxKinect/libs/libfreenect/platform/windows/inf
//
// This should install the Kinect camera, motor, & audio drivers.
//
// You CANNOT use this driver and the OpenNI driver with the same device. You
// will have to manually update the kinect device to use the libfreenect drivers
// and/or uninstall/reinstall it in Device Manager.
//
// No way around the Windows driver dance, sorry.



// uncomment this to read from two kinects simultaneously
//#define USE_TWO_KINECTS



//uncomment this to enable gamepad mode
#define USE_GAMEPAD


class ofApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void drawPointCloud();
    void drawPointCloud2();
    
    void keyPressed(int key);
    void keyReleased(int key);
    
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void axisChanged(ofxGamepadAxisEvent &e);
    void buttonPressed(ofxGamepadButtonEvent &e);
    void buttonReleased(ofxGamepadButtonEvent &e);
    
    ofxGameCamera camera;
     
	ofxKinect kinect;

#ifdef USE_TWO_KINECTS
    ofxKinect kinect2;
#endif
    
    ofImage buildings;
    ofImage stars;
	
	ofxCvColorImage colorImg;
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
	ofxCvContourFinder contourFinder;

	bool bThreshWithOpenCV;
	bool bDrawPointCloud;
	
	int nearThreshold;
	int farThreshold;
	
	int angle;
    
    int pointSize;
    int depthLimit;
	
	// used for viewing the point cloud
	ofEasyCam easyCam;
    
    //image writer variables
    int snapCounter;
    char snapString[255];
    ofImage img;
    bool bSnapshot;
    bool bReviewLastShot;
    
    bool bDiagnosticsMode;
    
    //3d model variables
    ofxAssimpModelLoader towers;
    ofxAssimpModelLoader sphere;
};
