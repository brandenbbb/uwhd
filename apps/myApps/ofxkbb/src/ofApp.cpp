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

#ifdef USE_KINECT
	//enable depth->video image calibration
	kinect.setRegistration(true);
	kinect.init();
	kinect.open();	// open host kinect
#endif
    
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
    towers.loadModel("images/3d/untitled.dae");
    sphere.loadModel("images/3d/skysphere.dae");
    
    // 3d object movement GUI setup
    
    moveThings = new ofxUISuperCanvas("3D OBJECTS");
    moveThings ->addLabel("HOST POINT CLOUD");
    moveThings ->addSpacer();
    moveThings ->addSlider("hostTranX", -5000, 5000, hostTranX);
    moveThings ->addSlider("hostTranY", -5000, 5000, hostTranY);
    moveThings ->addSlider("hostTranZ", -5000, 5000, hostTranZ);
    moveThings ->addSpacer();
    moveThings ->addSlider("hostRoX", -180, 180, hostRoX);
    moveThings ->addSlider("hostRoY", -180, 180, hostRoY);
    moveThings ->addSlider("hostRoZ", -180, 180, hostRoZ);
    moveThings ->addSpacer();
    moveThings ->addLabel("GUEST POINT CLOUD");
    moveThings ->addSpacer();
    moveThings ->addSlider("guestTranX", -5000, 5000, guestTranX);
    moveThings ->addSlider("guestTranY", -5000, 5000, guestTranY);
    moveThings ->addSlider("guestTranZ", -5000, 5000, guestTranZ);
    moveThings ->addSpacer();
    moveThings ->addSlider("guestRoX", -180, 180, guestRoX);
    moveThings ->addSlider("guestRoY", -180, 180, guestRoY);
    moveThings ->addSlider("guestRoZ", -180, 180, guestRoZ);
    moveThings ->addLabel("TOWERS");
    moveThings ->addSpacer();
    moveThings ->addSlider("towersScale", 0, 20, towersScale);
    moveThings ->addButton("reset_towersScale", false);
    moveThings ->addSlider("towersTranX", -5000, 5000, towersTranX);
    moveThings ->addSlider("towersTranY", -5000, 5000, towersTranY);
    moveThings ->addSlider("towersTranZ", -5000, 5000, towersTranZ);
    moveThings ->addSpacer();
    moveThings ->autoSizeToFitWidgets();
    ofAddListener(moveThings ->newGUIEvent,this,&ofApp::guiEvent);
    moveThings ->loadSettings("moveThingsSettings.xml");
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
        towers.setScale(towersScale*.5, towersScale*-.5, towersScale*.5);
        towers.setPosition(towersTranX, towersTranY, towersTranZ);
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
            fileName = emailFile.c_str() + ofToString(underScore) + ofToString(snapCounter) + ".jpg";
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
    ofRotateX(hostRoX);
    ofRotateY(hostRoY);
    ofRotateZ(hostRoZ);
    ofTranslate(hostTranX, hostTranY, hostTranZ);
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
    ofScale(1, -1, -1);
    
    ofTranslate(guestTranX, guestTranY, guestTranZ);
    ofRotateX(guestRoX);
    ofRotateX(guestRoY);
    ofRotateZ(guestRoZ);
    
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
    
#ifdef USE_HOSTMODE
    moveThings->saveSettings("moveThingsSettings.xml");
    delete moveThings;
#endif
}


// photo booth writer GUI
//--------------------------------------------------------------

void ofApp::photoBoothGUI(){
    pboothGUI = new ofxUISuperCanvas("EMAIL");
    pboothGUI->addSpacer();
    pboothGUI->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
    email = pboothGUI->addTextInput("EMAIL", "");
    emailFile = ofToString(email);
    ofAddListener(pboothGUI->newGUIEvent,this,&ofApp::guiEvent);
}

// move things GUI
//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e)
{
    string name = e.getName();
    int kind = e.getKind();
    
    if(name == "EMAIL"){
        ofxUITextInput *email = (ofxUITextInput *) e.widget;
        emailFile = email->getTextString();
    }
    
    // HOST REPOSITIONING
    if(name == "hostTranX")
    {
        ofxUISlider *hostTranX_Slider = (ofxUISlider *) e.widget;
        hostTranX = hostTranX_Slider->getScaledValue();
    }
    if(name == "hostTranY")
    {
        ofxUISlider *hostTranY_Slider = (ofxUISlider *) e.widget;
        hostTranY = hostTranY_Slider->getScaledValue();
    }
    if(name == "hostTranZ")
    {
        ofxUISlider *hostTranZ_Slider = (ofxUISlider *) e.widget;
        hostTranZ = hostTranZ_Slider->getScaledValue();
    }
    if(name == "hostRoX")
    {
        ofxUISlider *hostRoX_Slider = (ofxUISlider *) e.widget;
        hostRoX = hostRoX_Slider->getScaledValue();
    }
    if(name == "hostRoY")
    {
        ofxUISlider *hostRoY_Slider = (ofxUISlider *) e.widget;
        hostRoY = hostRoY_Slider->getScaledValue();
    }
    if(name == "hostRoZ")
    {
        ofxUISlider *hostRoZ_Slider = (ofxUISlider *) e.widget;
        hostRoZ = hostRoZ_Slider->getScaledValue();
    }
    
    // GUEST REPOSITIONING
    if(name == "guestTranX")
    {
        ofxUISlider *guestTranX_Slider = (ofxUISlider *) e.widget;
        guestTranX = guestTranX_Slider->getScaledValue();
    }
    if(name == "guestTranY")
    {
        ofxUISlider *guestTranY_Slider = (ofxUISlider *) e.widget;
        guestTranY = guestTranY_Slider->getScaledValue();
    }
    if(name == "guestTranZ")
    {
        ofxUISlider *guestTranZ_Slider = (ofxUISlider *) e.widget;
        guestTranZ = guestTranZ_Slider->getScaledValue();
    }
    if(name == "guestRoX")
    {
        ofxUISlider *guestRoX_Slider = (ofxUISlider *) e.widget;
        guestRoX = guestRoX_Slider->getScaledValue();
    }
    if(name == "guestRoY")
    {
        ofxUISlider *guestRoY_Slider = (ofxUISlider *) e.widget;
        guestRoY = guestRoY_Slider->getScaledValue();
    }
    if(name == "guestRoZ")
    {
        ofxUISlider *guestRoZ_Slider = (ofxUISlider *) e.widget;
        guestRoZ = guestRoZ_Slider->getScaledValue();
    }
    
    // TOWERS REPOSITIONING
    if(name == "towersScale"){
        ofxUISlider *towersScale_Slider = (ofxUISlider *) e.widget;
        towersScale = towersScale_Slider->getScaledValue();
    }
    if(name == "reset_towersScale"){
        ofxUIButton *button = (ofxUIButton *) e.getButton();
        if (button->getValue() == true){
            towersScale = 1;
        }
    }
    if(kind == OFX_UI_WIDGET_BUTTON)
    {
        ofxUIButton *button = (ofxUIButton *) e.widget;
        cout << name << "\t value: " << button->getValue() << endl;
    }
    if(name == "towersTranX")
    {
        ofxUISlider *towersTranX_Slider = (ofxUISlider *) e.widget;
        towersTranX = towersTranX_Slider->getScaledValue();
    }
    if(name == "towersTranY")
    {
        ofxUISlider *towersTranY_Slider = (ofxUISlider *) e.widget;
        towersTranY = towersTranY_Slider->getScaledValue();
    }
    if(name == "towersTranZ")
    {
        ofxUISlider *towersTranZ_Slider = (ofxUISlider *) e.widget;
        towersTranZ = towersTranZ_Slider->getScaledValue();
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
        
        // hide GUI
        case 'h':
            moveThings->toggleVisible();
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
			
		case 'n':
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case 'm':
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
