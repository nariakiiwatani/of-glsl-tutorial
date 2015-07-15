#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	particle_.setup(1024);
	particle_.addUpdater(&update_gravity_);
	particle_.addUpdater(&update_position_);
	particle_.addUpdater(&update_damping_);
	
	param_.setup("settings");
	param_.addSlider("simulation speed", update_position_.time_step_, 0.f, 10.f);
	param_.addSlider("damping", update_damping_.damping_, 0.f, 0.1f);
	param_.addSlider("attraction strength", update_gravity_.force_, 0.f, 500.f);
	param_.addSlider("attraction attenuation", update_gravity_.attenuation_, 0.f, 0.001f);
	param_.load();
	param_.close();
	
	ofBackground(0);
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
	ofVec3f mouse(ofGetMouseX(), ofGetMouseY() , 0.f);
	update_gravity_.position_.set(mouse);
	particle_.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	particle_.draw();
	if(param_.isOpen()) {
		particle_.drawDebug();
		param_.draw();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key) {
		case 'd':
			param_.toggleOpen();
			break;
		case 'r':
			particle_.setup(1024);
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
