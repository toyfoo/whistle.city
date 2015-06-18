/// version 0.7.5 - 10 september 2014
/// by thomas laureyssens t@toyfoo.com

#pragma once

#include "ofxMSAInteractiveObject.h"
#include "ofxPlaylist.h" // in ofxPlaylist.h - define "bool doNext()" as public

class ColorTime {

public:

    ColorTime() {
        color = ofColor::black;
        duration = 0.f;
        action = "none";
        has_priority = false;
        to_delete = false;
        // tween
    }

    void setColorTime (ofColor _color, float _duration) {
        color = _color;
        duration = _duration;
    }
    void setAction (string _action) {
        action = _action;
        //_tween = tween;
    }
    void hasPriority (bool _has_priority) {
        has_priority = _has_priority;
    }

    ofColor color;
    float duration;
    string action;
    bool has_priority;
    bool to_delete;
    //string tween;
};


class Pixel  : public ofxMSAInteractiveObject {

public:

    Pixel(){
        pos.set( 30, 30, 0 );
        vel.set( 0, 0, 0);
        color = ofColor::black;
        colorFrom = ofColor::black;
        colorTo = ofColor::black;
        ledpos = -1;
        //font.loadFont("DIN.otf", 6);
        partType = "body"; //default to body
        changeEnabled = true;
        playing = false;

        fadeAmount = 0;
    }

    ofPoint pos; // position on screen
    ofVec3f vel; // used later in animation (maybe)
    ofColor color;
    ofColor colorFrom;
    ofColor colorTo;

    ofColor colorDisco;
    ofColor colorHueShifted;
    ofColor colorSimpleShifted;

    ofxPlaylist playlist;
    vector<ColorTime> colorTimeCue;
    //vector< ofPtr<ColorTime> > colorTimeCue;

    float fadeAmount;
    int ledpos;
    //ofTrueTypeFont font;
    string partType; // types: body, eye, beak
    
    float blinkTweenIn; // for standard blinking actions, tweening in/out
    float blinkTimeSustain;
    float blinkTweenOut;
    float blinkTimePause;

    bool changeEnabled;
    bool isPausing;
    bool playlistToClear;
    bool playing;


    void setup() {
        //printf("Pixel::setup() - hello!\n");

        //enableMouseEvents();
		//enableKeyEvents();
        ofxKeyframeAnimRegisterEvents(this);
        
        blinkTweenIn = 100.f;
        blinkTimeSustain = 500.f;
        blinkTweenOut = 10.f;
        blinkTimePause = 200.f;
    
	}

    void update(){
        color = colorFrom.getLerped(colorTo, fadeAmount);
        playlist.update();
    }


    void draw() {
        /*
		if(isMousePressed()) ofSetHexColor(DOWN_COLOR);
		else if(isMouseOver()) ofSetHexColor(OVER_COLOR);
		else ofSetColor(color);
		*/

		ofSetColor(color);
		ofRect(x, y, width, height);

        //ofSetColor(ofColor::grey);

        //ofDrawBitmapString(ofToString(ledpos), ofVec2f(x+7, y+17));
        //or:
        //font.drawString(ofToString(ledpos), x+7, y+17);

	}

    void play() {
        using namespace Playlist;
        if (!playing) {
            playlist.addKeyFrame(Action::event(this, "nextInCue"));
            playing = true;
        }
    }

    void cueColorTime(ofColor _colorTo, float _duration, string _action, bool _priority) {
        ColorTime ct;
        ct.color = _colorTo;
        ct.duration = _duration;
        ct.action = _action;
        ct.has_priority = _priority;
        colorTimeCue.push_back(ct);
    }

    void cuePause(float _duration, bool _priority) {
        ColorTime ct;
        ct.action = "pause";
        ct.duration = _duration;
        ct.has_priority = _priority;
        colorTimeCue.push_back(ct);
    }
    
    void cueSetColor(ofColor _colorTo, bool _priority) {
        ColorTime ct;
        ct.color = _colorTo;
        ct.action = "setColor";
        ct.has_priority = _priority;
        colorTimeCue.push_back(ct);
    }
    
    void cueAction(string _action, bool _priority) {
        ColorTime ct;
        ct.action = _action;
        ct.has_priority = _priority;
        colorTimeCue.push_back(ct);
    }
    
    void clearCuelist(){
        colorTimeCue.clear();
    }

    void clearPlaylist(){
        //cout << ledpos << " clearing playlist" << endl;
        playlist.clear();
        colorTimeCue.clear();
        playing = false;
    }
    
    void setColor(ofColor col) {
        colorFrom = col;
        colorTo = col;
    }

    void addPause(float _dur, bool _priority){
        if (_priority && isPausing) {
            playlist.doNext();
        }
        cuePause(_dur, _priority);
    }

    void addFade(ofColor _color, float _duration, bool _priority) {
        if (_priority && isPausing) {
            playlist.doNext();
        }
        cueColorTime(_color, _duration, "tween", _priority);
    }
    
    void addSetColor(ofColor _color, bool _priority) {
        if (_priority && isPausing) {
            playlist.doNext();
        }
        cueSetColor(_color, _priority);
    }

    void addBlink(int amount, ofColor col) {
        using namespace Playlist;
         colorFrom = ofColor::black;
         colorTo = col;

            for (int i = 0; i < amount ; i++) { // 5 points = 5 blinks
                playlist.addKeyFrame(Action::tween(100.f, &fadeAmount, 1, TWEEN_LIN));
                playlist.addKeyFrame(Action::pause(500.f));
                playlist.addKeyFrame(Action::tween(10.f, &fadeAmount, 0, TWEEN_LIN));
                playlist.addKeyFrame(Action::pause(200.f));
            }
        playlist.addKeyFrame(Action::event(this, "checkNext"));
    }

    void addBlinkLoop(ofColor col, float _blinkTweenIn, float _blinkTimeSustain, float _blinkTweenOut, float _blinkTimePause) {
        colorFrom = ofColor::black;
        colorTo = col;
        blinkTweenIn = _blinkTweenIn;
        blinkTimeSustain = _blinkTimeSustain;
        blinkTweenOut = _blinkTweenOut;
        blinkTimePause = _blinkTimePause;
        addBlinkLoopKeyframes();
    }
    
    void coutElapsedTime() {
        cout<< "keyframe ElapsedTime " << ofGetElapsedTimeMillis() << endl;
    }

    void addCoutElapsedTime() {
        cueColorTime(ofColor::white, 10, "coutElapsedTime", false);
        //playlist.addKeyFrame(Action::event(this, "coutElapsedTime"));
    }

    void addBlinkLoopKeyframes() {
        using namespace Playlist;
        playlist.addKeyFrame(Action::tween(blinkTweenIn, &fadeAmount, 1, TWEEN_LIN));
        playlist.addKeyFrame(Action::pause(blinkTimeSustain));
        playlist.addKeyFrame(Action::tween(blinkTweenOut, &fadeAmount, 0, TWEEN_LIN));
        playlist.addKeyFrame(Action::pause(blinkTimePause));
        playlist.addKeyFrame(Action::event(this, "addBlinkLoopKeyframes"));
    }

    void onKeyframe(ofxPlaylistEventArgs& args){

        using namespace Playlist;

        // this check is only necessary if you want to be absolutely sure that
        // the onKeyFrame Event was sent by the same object as the receiver.
        if (args.pSender != static_cast<void*>(this)) return; // needed here!

        if (args.message == "nextInCue") {

            if (!colorTimeCue.empty()) {

                //cout << ledpos << " nextInCue priority: " << colorTimeCue[0].has_priority << " duration: " << colorTimeCue[0].duration  << " action: " << colorTimeCue[0].action << endl;

                // checking if there are priority elements present, and if so, delete the first
                bool hasPriorityElements = false;
                for(vector<ColorTime>::iterator it = colorTimeCue.begin(); it != colorTimeCue.end(); ++it){
                    if ((*it).has_priority) {
                        hasPriorityElements = true;
                    }
                }
                if (hasPriorityElements) {
                    //cout << ledpos << " has priority elements!" << endl;
                    vector<ColorTime>::iterator it = colorTimeCue.begin();
                    while (!(*it).has_priority) {
                            colorTimeCue.erase(it); // erasing non-priority elements in front of the cue
                    }
                }

                if (colorTimeCue[0].action == "pause") {
                    //cout  << ledpos << " adding a pause" << endl;
                    playlist.addKeyFrame(Action::event(this, "pause true"));
                    playlist.addKeyFrame(Action::pause(colorTimeCue[0].duration));
                    playlist.addKeyFrame(Action::event(this, "pause false"));
                    playlist.addKeyFrame(Action::event(this, "nextInCue"));
                }
                else if (colorTimeCue[0].action == "tween") {
                    //cout  << ledpos << " adding a tween. color: " << colorTimeCue[0].color << endl;
                    colorFrom = color;
                    colorTo = colorTimeCue[0].color;
                    fadeAmount = 0;
                    playlist.addKeyFrame(Action::tween(colorTimeCue[0].duration, &fadeAmount, 1.f, TWEEN_LIN));
                    playlist.addKeyFrame(Action::event(this, "nextInCue"));
                }
                else if (colorTimeCue[0].action == "setColor") {
                    colorFrom = color;
                    colorTo = colorTimeCue[0].color;
                    fadeAmount = 1;
                    playlist.addKeyFrame(Action::event(this, "nextInCue"));
                }
                else if (colorTimeCue[0].action == "coutElapsedTime") {
                    cout << "coutElapsedTime " << ofGetElapsedTimeMillis() << endl;
                    playlist.addKeyFrame(Action::event(this, "nextInCue"));
                }

                colorTimeCue.erase(colorTimeCue.begin());


            } else {
                playing = false;
                //cout << "cue is empty" << endl;
            }
        }

        else if (args.message == "pause true") {
            isPausing = true;
        } else if (args.message == "pause false") {
            isPausing = false;
        } else if (args.message == "addBlinkLoopKeyframes") {
            addBlinkLoopKeyframes();
        }
    }

    void exit () {
        ofxKeyframeAnimUnRegisterEvents(this);
    }



    virtual void onPress(int x, int y, int button) {
		printf("Pixel %i ::onPress(x: %i, y: %i, button: %i)\n", ledpos, x, y, button);

	}

	virtual void onRelease(int x, int y, int button) {
		printf("Pixel %i ::onRelease(x: %i, y: %i, button: %i)\n", ledpos, x, y, button);
	}

};

// http://forum.openframeworks.cc/t/making-an-object-e-g-ofimage-draggable/1637/3
/*

 #define		IDLE_COLOR		0xFFFFFF
 #define		OVER_COLOR		0x00FF00
 #define		DOWN_COLOR		0xFF0000

class ofxDraggable : public ofxMSAInteractiveObject {
public:
	ofBaseDraws *content; // this will point to your image, video, grabber etc.

	ofxDraggable() {
		content = NULL;
	}

	void onPress(int mx, int my, int button) {
		// save the offset of where the mouse was clicked...
		// ...relative to the position of the object
		saveX = mx - this->x;
		saveY = my - this->y;
	}

	void onDragOver(int mx, int my, int button) {
		this->x = mx - saveX;    // update x position
		this->y = my - saveY;    // update mouse y position
	}

	void draw() {
		if(content) {
			width = content->getWidth();
			height = content->getHeight();
			content->draw(x, y, width, height);

			// add a border if mouse is pressed or over the object
			if(isMouseDown()) {
				ofNoFill();
				ofSetColor(0xFF0000);
				ofRect(x, y, width, height);
			} else if(isMouseOver()){
				ofNoFill();
				ofSetColor(0x00FF00);
				ofRect(x, y, width, height);
			}
		}
	}

protected:
	int saveX, saveY;
};

// Then in your testApp code you can do something like:


 ofImage myImage;
 ofxDraggable imageDragger;

 ofVideoPlayer myVideo;
 ofxDraggable videoDragger

 imageDragger.content = &myImage; // tell imageDragger to use this image
 videoDragger.content = &videoDragger; // tell videoDragger to use this video


  */




/*
class MyTestObject : public ofxMSAInteractiveObject {
public:
	void setup() {
		printf("MyTestObject::setup() - hello!\n");
		enableMouseEvents();
		enableKeyEvents();
	}


	void exit() {
		printf("MyTestObject::exit() - goodbye!\n");
	}


	void update() {
		//		x = ofGetWidth()/2 + cos(ofGetElapsedTimef() * 0.2) * ofGetWidth()/4;
		//		y = ofGetHeight()/2 + sin(ofGetElapsedTimef() * 0.2) * ofGetHeight()/4;
	}


	void draw() {
		if(isMousePressed()) ofSetHexColor(DOWN_COLOR);
		else if(isMouseOver()) ofSetHexColor(OVER_COLOR);
		else ofSetHexColor(IDLE_COLOR);

		ofRect(x, y, width, height);
	}

	virtual void onRollOver(int x, int y) {
		printf("MyTestObject::onRollOver(x: %i, y: %i)\n", x, y);
	}

	virtual void onRollOut() {
		printf("MyTestObject::onRollOut()\n");
	}

	virtual void onMouseMove(int x, int y){
		printf("MyTestObject::onMouseMove(x: %i, y: %i)\n", x, y);
	}

	virtual void onDragOver(int x, int y, int button) {
		printf("MyTestObject::onDragOver(x: %i, y: %i, button: %i)\n", x, y, button);
	}

	virtual void onDragOutside(int x, int y, int button) {
		printf("MyTestObject::onDragOutside(x: %i, y: %i, button: %i)\n", x, y, button);
	}

	virtual void onPress(int x, int y, int button) {
		printf("MyTestObject::onPress(x: %i, y: %i, button: %i)\n", x, y, button);
	}

	virtual void onRelease(int x, int y, int button) {
		printf("MyTestObject::onRelease(x: %i, y: %i, button: %i)\n", x, y, button);
	}

	virtual void onReleaseOutside(int x, int y, int button) {
		printf("MyTestObject::onReleaseOutside(x: %i, y: %i, button: %i)\n", x, y, button);
	}

	virtual void keyPressed(int key) {
		printf("MyTestObject::keyPressed(key: %i)\n", key);
	}

	virtual void keyReleased(int key) {
		printf("MyTestObject::keyReleased(key: %i)\n", key);
	}

};
*/

/*
 class Pixel {
 public:
 Pixel(){
 pos.set( 30, 30, 0 );
 vel.set( 0, 0, 0);
 color.set(0, 0, 0);
 ledpos = -1;
 font.loadFont("DIN.otf", 12);
 partType = "body"; //default to body
 }

 void update(){
 pos += vel;
 //vel.y += .1;
 }

 void draw() {
 ofSetColor(color);
 ofEllipse(pos.x,pos.y,40,40);
 ofSetColor(ofColor::grey);
 font.drawString(ofToString(ledpos), pos.x, pos.y);

 }




 ofPoint pos; // position on screen
 ofVec3f vel; // used later in animation (maybe)
 ofColor color;
 int ledpos;
 ofTrueTypeFont font;
 string partType; // types: body, eye, beak

 }
 */
