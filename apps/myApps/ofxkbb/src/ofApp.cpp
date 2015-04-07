#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	
	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
	
	// print the intrinsic IR sensor values
	if(kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}
    
#ifdef USE_TWO_KINECTS
    kinect2.init();
    kinect2.open();
#endif
    
    /* 2D load universe within images
    buildings.loadImage("images/buildingsBottom.png");
    stars.loadImage("images/bg.png");
	*/
    
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
    
    // default values for near / far threshold / point size (adjusted later via GUI controls)
	nearThreshold = 255;
    farThreshold = 167;
    pointSize = 3;
    depthLimit = 1100;
    
    bThreshWithOpenCV = false;
	
    // diagnostics mode condition
    bDiagnosticsMode = false;
    
	ofSetFrameRate(60);
	
	/*
     // zero the tilt on startup
     angle = 0;
     kinect.setCameraTiltAngle(angle);
	*/
    
	// start in point cloud mode
	bDrawPointCloud = true;
    
    // image file writer settings
    // need to add code to not overwrite files
    snapCounter = 0;
    bSnapshot = false;
    bReviewLastShot = false;
    memset(snapString, 0, 255);		// clear the string by setting all chars to 0
    
    // 3d model load assets
    towers.loadModel("images/img/3d/towersandtrees.dae");
    sphere.loadModel("images/img/sphere/skysphere.dae");
}

#ifdef USE_TWO_KINECTS
    kinect2.update();
#endif

//--------------------------------------------------------------
void ofApp::update() {
	
	//ofBackground(0, 0, 0);
	
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {
			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage.getPixels();
			
			int numPixels = grayImage.getWidth() * grayImage.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
			}
		}
		
		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	
	ofSetColor(255, 255, 255);
	
    if(bDrawPointCloud == true) {
        /* OLD 2D stars draw
         stars.draw(0,0,0);
         */
        
        // anything within the easyCam begin / end section will move relative to the 3D camera...neato!
        easyCam.begin();
        
            // 3D star skydome
            sphere.setScale(10,10,10);
            sphere.drawFaces();
            // Kinect Point Cloud
            drawPointCloud();
            // 3D towers!
            towers.setScale(.5, -.5, .5);
            towers.setPosition(0, -100, 300);
            towers.drawFaces();
        
        easyCam.end();
        
        /* OLD 2D buildings
        buildings.draw(-200,220);
        */
        
            // image file writer code
            // need to add in code so you can enter in name via GUI / not overwrite older files
            if (bSnapshot == true){
                // capture entire OF screen; image is same resolution as OF window
                img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
                string fileName = "NFB_HIGHRISE_Universe_Within_Hot_Docs_"+ofToString(snapCounter)+".png";
                img.saveImage(fileName);
                sprintf(snapString, "saved %s", fileName.c_str());
                snapCounter++;
                bSnapshot = false;
            }
        
            // show the framegrab on screen if the review photo button is pressed
            // need to add code to display the filename
            if (bReviewLastShot == true){
                ofDrawBitmapString(snapString, ofGetWidth(), ofGetHeight());
                
                ofSetHexColor(0xFFFFFF);
                if(snapCounter > 0) {
                    img.draw(0,0,ofGetWidth(),ofGetHeight());
                }
            }
	}

    if (bDiagnosticsMode == true) {
		// diagnostics mode display - shows depth / camera mode for establishing near / far threshold values
		kinect.drawDepth(10, 0, 400, 300);
		kinect.draw(420, 10, 400, 300);
		grayImage.draw(10, 320, 400, 300);
		contourFinder.draw(10, 320, 400, 300);
        
        #ifdef USE_TWO_KINECTS
            kinect2.draw(420, 320, 400, 300);
        #endif
        
        // on screen text GUI for diagnostic mode settings
        ofSetColor(255, 255, 255);
        stringstream reportStream;
        
        if(kinect.hasAccelControl()) {
            reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
            << ofToString(kinect.getMksAccel().y, 2) << " / "
            << ofToString(kinect.getMksAccel().z, 2) << endl;
        } else {
            reportStream << "Note: this is a newer Xbox Kinect or Kinect For Windows device," << endl
            << "motor / led / accel controls are not currently supported" << endl << endl;
        }
        
        reportStream << "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
        << "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
        << "set near threshold " << nearThreshold << " (press: + -)" << endl
        << "set far threshold " << depthLimit << " (press: < >) num blobs found " << contourFinder.nBlobs << endl
        << "set point cloud point size " << pointSize << " (press: [ to decrease or ] to increase)  " << endl
        << ", fps: " << ofGetFrameRate() << endl
        << "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl;
        
        if(kinect.hasCamTiltControl()) {
            reportStream << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
            << "press 1-5 & 0 to change the led mode" << endl;
        }
        
        ofDrawBitmapString(reportStream.str(), 20, 652);
	}
	
}


//--------------------------------------------------------------

void ofApp::drawPointCloud() {
	int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			if(kinect.getDistanceAt(x, y) < depthLimit) {
				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
	// set point cloud point size (taken from keyboard controls)
    glPointSize(pointSize);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards' 
	ofScale(1, -1, -1);
	ofTranslate(0, 0, -1000); // center the points a bit
	ofEnableDepthTest();
	mesh.drawVertices();
	ofDisableDepthTest();
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::exit() {
	// kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();

#ifdef USE_TWO_KINECTS
    kinect2.close();
#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch (key) {
		// diagnostics mode
        case 'd':
            bDiagnosticsMode = true;
			break;
        case ' ':
            bThreshWithOpenCV = !bThreshWithOpenCV;
            break;
        // go full screen
        case 'f':
            ofToggleFullscreen();
            break;
			
		case'p':
            bDiagnosticsMode = false;
            bReviewLastShot = false;
            bDrawPointCloud = true;
			break;
        
        // image saver keystroke
        case'l':
            bSnapshot = true;
            break;
	
        // image review keystroke
        case'r':
            bDiagnosticsMode = false;
            bReviewLastShot = true;
            break;
            
		case '>':
		case '.':
			//farThreshold ++;
			//if (farThreshold > 255) farThreshold = 255;
            depthLimit = depthLimit + 100;
			break;
			
		case '<':
		case ',':
			//farThreshold --;
			//if (farThreshold < 0) farThreshold = 0;
            if (depthLimit < 0) depthLimit = 0;
            depthLimit = depthLimit - 100;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
        
        // decrease / increase point cloud point size controls
        case '[':
            pointSize --;
            if (pointSize < 0) pointSize = 0;
            break;
        case ']':
            pointSize ++;
            break;
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'o':
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;
			
		case 'c':
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
			break;
			
		case '1':
			kinect.setLed(ofxKinect::LED_GREEN);
			break;
			
		case '2':
			kinect.setLed(ofxKinect::LED_YELLOW);
			break;
			
		case '3':
			kinect.setLed(ofxKinect::LED_RED);
			break;
			
		case '4':
			kinect.setLed(ofxKinect::LED_BLINK_GREEN);
			break;
			
		case '5':
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;
			
		case '0':
			kinect.setLed(ofxKinect::LED_OFF);
			break;
			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{}
