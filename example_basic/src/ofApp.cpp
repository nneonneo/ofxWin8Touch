#include "ofApp.h"
#include "ofxWin8Touch.h"

//--------------------------------------------------------------
void ofApp::setup(){
	/* Essential setup */
	ofxWin8TouchSetup();
	ofRegisterTouchEvents(this);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	/* Draw touches */
	ofBackground(0);
	ofNoFill();
	ofSetColor(255);
	for(auto &i : touchMap) {
		auto &touch = i.second;
		ofPushMatrix();
		ofTranslate(touch.x, touch.y);
		ofRotate(90-touch.angle);
		ofRect(-touch.width/2, -touch.height/2, touch.width, touch.height);
		ofEllipse(0, 0, touch.width * 5, touch.height * 5);
		ofDrawBitmapString(ofVAArgsToString("%d", touch.id), 0, 0);
		ofPopMatrix();
    }
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

//--------------------------------------------------------------
void ofApp::touchDown(ofTouchEventArgs & touch){
    touchMap[touch.id] = touch;
}

//--------------------------------------------------------------
void ofApp::touchMoved(ofTouchEventArgs & touch){
    touchMap[touch.id] = touch;
}

//--------------------------------------------------------------
void ofApp::touchUp(ofTouchEventArgs & touch){
    touchMap.erase(touch.id);
}

//--------------------------------------------------------------
void ofApp::touchDoubleTap(ofTouchEventArgs & touch){

}

//--------------------------------------------------------------
void ofApp::touchCancelled(ofTouchEventArgs & touch){
    touchMap.erase(touch.id);    
}
