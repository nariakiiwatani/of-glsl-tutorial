#include "ofApp.h"
#include "ofShader.h"

ofShader shader_;
//--------------------------------------------------------------
void ofApp::setup(){
	shader_.setupShaderFromSource(GL_VERTEX_SHADER,
#include "shiftX.vert"
								  );
	shader_.setupShaderFromSource(GL_FRAGMENT_SHADER,
#include "swapColor.frag"
								);
	shader_.linkProgram();
}

//--------------------------------------------------------------
void ofApp::update(){
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofPushStyle();
	shader_.begin();
	ofSetColor(ofColor::red);
	ofRect(0,0,ofGetWidth(),ofGetHeight());
	shader_.end();
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
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
