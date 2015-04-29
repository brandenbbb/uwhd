#include "ofxGamepadCamera.h"

ofxGamepadCamera::ofxGamepadCamera():pad(NULL),speedRotation(70), speedMove(200), useAnalogueDolly(false) {
}

ofxGamepadCamera::~ofxGamepadCamera() {
}

void ofxGamepadCamera::setup() {
	//CHECK IF THERE IS A GAMEPAD CONNECTED
	if(ofxGamepadHandler::get()->getNumPads()>0) {
		setGamepad(ofxGamepadHandler::get()->getGamepad(0));
	} else
		ofLogWarning("no gamepad connected in ofxGamepadCAmera::setup");

	ofAddListener(ofEvents().update, this, &ofxGamepadCamera::update);
}

void ofxGamepadCamera::update(ofEventArgs& e) {
	if(pad == NULL)
		return;

	float curTime = ofGetElapsedTimef();

	float mult = curTime - lastTime;
	pan(-pad->getAxisValue(PS3_STICK_R_X)*speedRotation*.25*mult);
	tilt(-pad->getAxisValue(PS3_STICK_R_Y)*speedRotation*.25*mult);

	truck(pad->getAxisValue(PS3_STICK_L_X)*speedMove*.5*mult);
	dolly(pad->getAxisValue(PS3_STICK_L_Y)*speedMove*.5*mult);
    
    if(pad->getButtonValue(PS3_BTN_L1)) {
        roll(-speedRotation*.1*mult);
    }
    if(pad->getButtonValue(PS3_BTN_R1)) {
        roll(speedRotation*.1*mult);
    }
    if(pad->getButtonValue(PS3_BTN_L2)) {
        boom(speedMove*.25*mult);
    }
    if(pad->getButtonValue(PS3_BTN_R2)) {
        boom(-speedMove*.25*mult);
    }
    /*
    if(pad->getButtonValue(PS3_BTN_SELECT)){
		reset();
    }
    */

	lastTime = curTime;
}

void ofxGamepadCamera::setGamepad(ofxGamepad* p) {
	pad = p;
}

void ofxGamepadCamera::setInitialPosition(float x, float y, float z) {
	setInitialPosition(ofVec3f(x, y, z));
}

void ofxGamepadCamera::setInitialPosition(ofVec3f pos) {
	initialPosition = pos;
}

void ofxGamepadCamera::reset() {
	setPosition(initialPosition);
	setOrientation(ofVec3f(0, 0, 0));
    
}
