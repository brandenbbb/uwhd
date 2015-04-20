#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
    /* reduce screen tearing / verbose debug mode
    ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
    */
    
    ofSetFrameRate(60);
    
#ifdef USE_HOSTMODE
    camera.setup();
#endif
	
	//enable depth->video image calibration
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
    
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
    
#ifdef USE_TWO_KINECTS
    colorImg.allocate(kinect2.width, kinect2.height);
    grayImage.allocate(kinect2.width, kinect2.height);
    grayThreshNear.allocate(kinect2.width, kinect2.height);
    grayThreshFar.allocate(kinect2.width, kinect2.height);
#endif
    
    // default values for near / far threshold / point size (adjusted later via GUI controls)
	nearThreshold = 255;
    farThreshold = 167;
    pointSize = 3;
    depthLimit = 1100;
    
    bThreshWithOpenCV = false;
	
    // diagnostics mode condition
    bDiagnosticsMode = false;
    
	// start in point cloud mode
	bDrawPointCloud = true;
    
    // image file writer settings
    snapCounter = 0;
    bSnapshot = false;
    bReviewLastShot = false;
    memset(snapString, 0, 255);		// clear the string by setting all chars to 0
    
#ifdef USE_PHOTOBOOTH
    //2D load universe within images
    buildings.loadImage("images/towers16by9.png");
    stars.loadImage("images/bg_16by9.png");
    
    photoBoothGUI();
    pboothGUI->loadSettings("photoBoothSettings.xml");
#endif
    
#ifdef USE_HOSTMODE
    // 3d model load assets
    towers.loadModel("images/3d/towersandtrees.dae");
    sphere.loadModel("images/3d/skysphere.dae");
#endif
}



//--------------------------------------------------------------
void ofApp::update() {
	
    //ofBackground(0, 0, 0);

    kinect.update();
    
#ifdef USE_TWO_KINECTS
    kinect2.update();
#endif
	
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
    
#ifdef USE_PHOTOBOOTH
        // 2D stars draw
        stars.draw(0, 0, ofGetWidth(), ofGetHeight());
        easyCam.begin();
    #ifdef USE_KINECT
        // Kinect Point Cloud
        drawPointCloud();
    #endif
        easyCam.end();
        // OLD 2D buildings
        buildings.draw(0,0, ofGetWidth(), ofGetHeight());
#endif
            
// anything within the camera begin / end section will move relative to the 3D camera...neato!
#ifdef USE_HOSTMODE
        camera.begin();
     
        // 3D star skydome
        sphere.setScale(10,10,10);
        sphere.drawFaces();
    
        #ifdef USE_KINECT
            // Kinect Point Cloud
            drawHostPointCloud();
        #endif
    
        #ifdef USE_TWO_KINECTS
            // Kinect Point Cloud #2
            drawGuestPointCloud();
        #endif
    
        // 3D towers!
        towers.setScale(.5, -.5, .5);
        towers.setPosition(0, -100, -100);
        towers.drawFaces();
        camera.end();
#endif
        
        
#ifdef USE_PHOTOBOOTH
        // image file writer code
        if (bSnapshot == true){
            // capture entire OF screen; image is same resolution as OF window
            // automatically reviews last frame taken
            img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
            string underScore = "_";
            fileName = emailFile.c_str() + ofToString(underScore) + ofToString(snapCounter) + ".png";
            img.saveImage(fileName);
            sprintf(snapString, "saved %s", fileName.c_str());
            snapCounter++;
            pboothGUI->addLabel(fileName, OFX_UI_FONT_SMALL);
            pboothGUI->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
            bSnapshot = false;
            bReviewLastShot = true;
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
#endif
        
        
        //gamepad GUI for diagnostics
    #ifdef USE_GAMEPAD
        ofxGamepadHandler::get()->draw(10,10);
    #endif
         
    }

    if (bDiagnosticsMode == true) {
		// diagnostics mode display - shows depth / camera mode for establishing near / far threshold values
		kinect.drawDepth(10, 0, 400, 300);
		kinect.draw(10, 300, 400, 300);
		//grayImage.draw(10, 320, 400, 300);
		//contourFinder.draw(10, 320, 400, 300);
        
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

void ofApp::drawHostPointCloud() {
	int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 1;
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
    
#ifdef USE_HOSTMODE
	ofTranslate(0, 0, 0); // center the points a bit
#endif
    
#ifdef USE_PHOTOBOOTH
    ofTranslate(0, 0, -1000); // center the points a bit
#endif
    
	ofEnableDepthTest();
	mesh.drawVertices();
	ofDisableDepthTest();
	ofPopMatrix();
}

#ifdef USE_TWO_KINECTS
//--------------------------------------------------------------
void ofApp::drawGuestPointCloud() {
    int w = 640;
    int h = 480;
    ofMesh mesh2;
    mesh2.setMode(OF_PRIMITIVE_POINTS);
    int step = 1;
    for(int y = 0; y < h; y += step) {
        for(int x = 0; x < w; x += step) {
            if(kinect2.getDistanceAt(x, y) < depthLimit) {
                mesh2.addColor(kinect2.getColorAt(x,y));
                mesh2.addVertex(kinect2.getWorldCoordinateAt(x, y));
            }
        }
    }
    // set point cloud point size (taken from keyboard controls)
    glPointSize(pointSize);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    ofScale(-1, -1, 1);
    ofTranslate(0, 0, 0); // center the points a bit;
    ofEnableDepthTest();
    mesh2.drawVertices();
    ofDisableDepthTest();
    ofPopMatrix();
}
#endif

//--------------------------------------------------------------
void ofApp::exit() {
    // kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();

#ifdef USE_TWO_KINECTS
    kinect2.close();
#endif
    
#ifdef USE_PHOTOBOOTH
    pboothGUI->saveSettings("photoBoothSettings.xml");
    delete pboothGUI;
#endif
}


//--------------------------------------------------------------

void ofApp::photoBoothGUI(){
    pboothGUI = new ofxUISuperCanvas("EMAIL");
    
    pboothGUI->addSpacer();
    pboothGUI->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
    email = pboothGUI->addTextInput("EMAIL", "");
    emailFile = ofToString(email);
    
    ofAddListener(pboothGUI->newGUIEvent,this,&ofApp::guiEvent);
}


//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e)
{
    string name = e.getName();
    int kind = e.getKind();
    cout << "got event from: " << name << endl;
    
    if(name == "EMAIL")
    {
        ofxUITextInput *email = (ofxUITextInput *) e.widget;
        if(email->getInputTriggerType() == OFX_UI_TEXTINPUT_ON_ENTER)
        {
            cout << "ON ENTER: ";
        }
        emailFile = email->getTextString();
        cout << emailFile << endl;
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch (key) {
		// diagnostics mode
        case '`':
            bDiagnosticsMode = true;
			break;
        case ' ':
            bThreshWithOpenCV = !bThreshWithOpenCV;
            break;
        // go full screen
        case OF_KEY_F1:
            ofToggleFullscreen();
            break;
			
		case OF_KEY_F4:
            bDiagnosticsMode = false;
            bReviewLastShot = false;
            bDrawPointCloud = true;
			break;
        
        // image saver keystroke
        case OF_KEY_F5:
            bSnapshot = true;
            break;
	
        // image review keystroke
        case OF_KEY_F6:
            bDiagnosticsMode = false;
            bReviewLastShot = true;
            break;
            
		case '.':
			//farThreshold ++;
			//if (farThreshold > 255) farThreshold = 255;
            depthLimit = depthLimit + 100;
			break;
			
		case ',':
			//farThreshold --;
			//if (farThreshold < 0) farThreshold = 0;
            if (depthLimit < 0) depthLimit = 0;
            depthLimit = depthLimit - 100;
			break;
			
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
            
		case 'o':
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;
			
		case '0':
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
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
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
}
