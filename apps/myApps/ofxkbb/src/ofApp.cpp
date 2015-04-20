#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
    //reduce screen tearing / verbose debug mode
    //ofSetVerticalSync(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(60);
    
#ifdef USE_HOSTMODE
    camera.setup();
#endif

	//enable depth->video image calibration
	kinect.setRegistration(true);
	kinect.init();
	kinect.open(1);	// open host kinect
    
#ifdef USE_TWO_KINECTS
    //enable depth->video image calibration
    kinect2.setRegistration(true);
    kinect2.init();
    kinect2.open(0); // open guest kinect
#endif
    
    // default values for near / far threshold / point size (adjusted later via GUI controls)
	nearThreshold = 255;
    farThreshold = 167;
    pointSize = 3;
    depthLimit = 1100;
    
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
    kinect.update();
#ifdef USE_TWO_KINECTS
    kinect2.update();
#endif
}

//--------------------------------------------------------------
void ofApp::draw() {
	
	ofSetColor(255, 255, 255);
    
#ifdef USE_PHOTOBOOTH
        // 2D stars draw
        stars.draw(0, 0, ofGetWidth(), ofGetHeight());
        easyCam.begin();
    #ifdef USE_KINECT
        // Kinect Point Cloud
        drawHostPointCloud();
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
    int ww = 640;
    int hh = 480;
    ofMesh mesh2;
    mesh2.setMode(OF_PRIMITIVE_POINTS);
    int step = 1;
    for(int yy = 0; yy < hh; yy += step) {
        for(int xx = 0; xx < ww; xx += step) {
            if(kinect2.getDistanceAt(xx, yy) < depthLimit) {
                mesh2.addColor(kinect2.getColorAt(xx,yy));
                mesh2.addVertex(kinect2.getWorldCoordinateAt(xx, yy));
            }
        }
    }
    // set point cloud point size (taken from keyboard controls)
    glPointSize(pointSize);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    ofScale(1, -1, 1);
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
        // go full screen
        case OF_KEY_F1:
            ofToggleFullscreen();
            break;
			
		case OF_KEY_F4:
            bReviewLastShot = false;
			break;
        
        // image saver keystroke
        case OF_KEY_F5:
            bSnapshot = true;
            break;
	
        // image review keystroke
        case OF_KEY_F6:
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
			
		case '`':
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
