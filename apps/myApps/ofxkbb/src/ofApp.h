#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxAssimpModelLoader.h"
#include "ofVboMesh.h"
#include "ofAppGlutWindow.h"
#include "ofxGameCamera.h"
#include "ofxUI.h"

// uncomment this to read from two kinects simultaneously
//#define USE_TWO_KINECTS

//uncomment this to enable gamepad mode...do not use unless you have Xbox 360 controller / driver setup properly!!!
//#define USE_GAMEPAD

//uncomment this to enable the kinect point cloud (OR COMMENT THIS IF NO KINECT PLUGGED IN)
//#define USE_KINECT

//uncomment this to enable PHOTOBOOTH mode
#define USE_PHOTOBOOTH

//uncomment this to enable HOT DOCS HOST mode
//#define USE_HOSTMODE

#ifdef USE_GAMEPAD
    #include "ofxGamepadHandler.h"
#endif

class ofApp : public ofBaseApp {
    
public:
	void setup();
	void update();
	void draw();
	void exit();
	
    // point cloud classes
	void drawPointCloud();
    void drawPointCloud2();
    
    // input classes
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    // Reza's GUI classes
    void photoBoothGUI();
    ofxUISuperCanvas *pboothGUI;
    ofxUITextInput *email;
    string emailFile;
    bool hideGUI;
    void guiEvent(ofxUIEventArgs &e);
    
	
    // KINECT VARIABLES
    ofxKinect kinect;
    #ifdef USE_TWO_KINECTS
        ofxKinect kinect2;
    #endif
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
    
    // image filer writer variables
    int snapCounter;
    char snapString[255];
    ofImage img;
    bool bSnapshot;
    bool bReviewLastShot;
    bool bDiagnosticsMode;
    
#ifdef USE_PHOTOBOOTH
    // simpler camera for photobooth
    ofEasyCam easyCam;
    
    // Universe Within 2D environment assets
    ofImage buildings;
    ofImage stars;
#endif

#ifdef USE_HOSTMODE
    // FPS camera
    ofxGameCamera camera;
    
    #ifdef USE_GAMEPAD
        // gamepad classes
        void axisChanged(ofxGamepadAxisEvent &e);
        void buttonPressed(ofxGamepadButtonEvent &e);
        void buttonReleased(ofxGamepadButtonEvent &e);
    #endif
    
    // Universe Within 3D environment assets
    ofxAssimpModelLoader towers;
    ofxAssimpModelLoader sphere;
#endif
    
};
