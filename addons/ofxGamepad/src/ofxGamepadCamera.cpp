#include "ofxGamepadCamera.h"

ofxGamepadCamera::ofxGamepadCamera():pad(NULL),speedRotation(20), speedMove(200), useAnalogueDolly(false) {
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
	pan(-pad->getAxisValue(XB_STICK_R_X)*speedRotation*mult);
	tilt(-pad->getAxisValue(XB_STICK_R_Y)*speedRotation*mult);
    
    truck((pad->getAxisValueU(XB_STICK_L_X)-.5)*speedMove*mult);
    dolly((pad->getAxisValueU(XB_STICK_L_Y)-.5)*speedMove*mult);
    

    if(pad->getButtonValue(XB_STICK_LT)) {
        boom(-speedMove*2*mult);
    }
    if(pad->getButtonValue(XB_STICK_RT)) {
        boom(speedMove*2*mult);
    }

	if(pad->getButtonValue(XB_BTN_LB)) {
		roll(-speedRotation*.7*mult);
	}
	if(pad->getButtonValue(XB_BTN_RB)) {
		roll(speedRotation*.7*mult);
	}

    if(pad->getButtonValue(XB_BTN_BACK)){
		reset();
    }

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
