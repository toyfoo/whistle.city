/// version 0.9 - 20 januari 2015
/// by thomas laureyssens t@toyfoo.com

#pragma once
#include "ofMain.h"
#include "ofxSerial.h"
#include "ofMain.h"
#include "ofxPlaylist.h"
#include "Pixel.h"
#include "ofxGui.h"
#include "ofxSvg.h"
#include <deque>
#include "ofxTime.h"

#include <fftw3.h>

//WHISTLEDETECTOR
//#include "ofxIntegratedWhistleDetector.h"
#include <ofxIntegratedWhistleDetector.h>
#include <ofxWhistleSequenceDetector.h>

#define pixelAmount 50 // the amount of LED's on a sparrow


class SerialMessage
{
public:
    SerialMessage(): fade(0)
    {
    }

    SerialMessage(const std::string& _message,
                  const std::string& _exception,
                  int _fade):
    message(_message),
    exception(_exception),
    fade(_fade)
    {
    }

    std::string message;
    std::string exception;
    int fade;
};

class ofApp : public ofBaseApp{

    //WHISTLEDETECTOR
    ofxIntegratedWhistleDetector detector;
    ofxWhistleSequenceDetector sequenceDetector;  // state machine
    deque<pair<ofxWhistleSequenceDetector::Transition, ofxIntegratedWhistleDetector::Whistle> > transitions;


	static const size_t MaxTransitions = 20; // Count of last transitions and transition queue (with whistles those cause transitions) to be printed on screen


    int eyePixel;
    int beakPixel;
    int maintenancePixel;

    ofxColorSlider colorEye;
    ofxColorSlider colorBeak;
    ofxColorSlider colorBody;
    ofxToggle editColor;
    ofxToggle listen;
    ofxButton activateSerialButton;
    ofxButton whistleButton;
    ofxLabel screenSize;
    ofxPanel gui;
    ofxPanel guiColor;
    ofxIntSlider HzPerWindow;
    ofxIntSlider powerThresholdPercent;
    ofxIntSlider msecsPerWhistleDuration;
    ofxIntSlider hzPerFrequencyDeviation;
    ofxLabel whistleLabel;
    ofxToggle debugToggle;
    ofxFloatSlider debugWhistleCertainty;
    ofxIntSlider debugWhistleFrequency;
    ofxToggle chromeToggle;


    ofSerial serial;
    bool serialActivated;
    int serialSleepMillis;

    ofXml settingsXML;
    ofXml logXML;
    string xmlStructure;
    string xmlMessage;

    vector<Pixel*> pixels;
    int whistlesToSend;


    ofColor maintenanceNoError;
    ofColor maintenanceError;
    ofColor eyeWhistleDetected;
    ofColor eyeWhistleUncertain;
    ofColor eyeListening;
    ofColor beakConnectionError;
    ofColor beakPointSent;
    ofColor beakPointEnabled;
    ofColor profileColor;
    ofColor status200; // success
    ofColor status400; // connection error - orange
    ofColor status400severe; // connection error - about to set whistles to zero
    ofColor status400off; // connection error - no longer records whistles
    ofColor status400webserver;
    
    ofColor status461; // no active campaign
    

    //vector<ofColor> randomBodyColors;

    vector<int> randomPixelOrder;

    ofxPlaylist controlPlaylist;
    ofxPlaylist animPlaylist;
    ofxPlaylist connectionPlaylist;

    int animateRandomPixelUntil;

    int whistleState;
    bool freshMessage;
    bool isLoading; //is connecting to server
    bool hasConnectionError; // the most general error, includes connection and webserver error
    bool hasSevereConnectionError; // sustained internet connectivity issues
    bool webserverDown; // only the sparrows webserver down
    
    
    bool noActiveCampaign;
    bool timeIsOff;
    int serverTimeOffHack;


    string objectID; //hash, object identifier to send to server
    int onHour;
    int offHour;
    int gameType;

    float timeoutTimeStates;
    float timeoutTimeWhistles;
    float fadeOutTime;
    float fadeInTime;
    float fadeVariationTime;

    int discoLevel; // NEW* used to change the speed of the disco animation
    int discoChance;
    int discoTried;


    unsigned long long previousDebugWhistleTime;
    unsigned long long previousWhistleTime;
    unsigned long long connectionDownTime; // to keep track of when webserver goes down
    bool whistleIsCertain;

    float lastWhistleTime; // in seconds, ofGetElapsedTimef()
    //int whistlesToSend; // the whistles to be sent (introducted for in case connection is down)
    ofTrueTypeFont font;

    //ofxserial
    ofx::IO::PacketSerialDevice device;
    std::vector<SerialMessage> serialMessages;
    bool returnMessageReceived;

    float lastWhistleFrequency; // used to log the frequency, once the timestamp is returned from the server.


public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);

    void activateSerial();
    void whistleButtonPressed();
    void onWhistle( double frequency, float certainty );
    void sendPixels(); // to arduino
    void onKeyframe(ofxPlaylistEventArgs& args);
    ofColor shiftColor(ofColor inputColor);
    ofColor shiftHue(ofColor inputColor);
    unsigned char copyArray (unsigned char array[], int from, int to);
    void urlResponse(ofHttpResponse & response);
    void sendMessageToServer(string type);
    void showWhistleDetectorValues();
    void onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args);
    void onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args);
    void loadXMLSettings();
    void drawSparrowGraphic();
    void loadSparrowGraphic();
    void parseServerResponse(ofXml serverResponse);

    void newDiscoColors(); //helper for animating
    void seedRandomPixelOrder();
    void animateFillDisco(int level);
    void animateFillSimpleColors();
    void animateFillShiftPushColors();
    void animateFillShiftHueColors();
    void animateFillPercentage(ofColor fillColor, float percentage);
    void animateBeakAndEyeAmbientAnim();
    bool interactionAllowed();
    void addLogItem(string datetime, string animation, string serverresponse, float frequency);
    
    

    //WHISTLEDETECTOR
    void setupDetector();
	void setupSequenceDetector();
	string getDetectorParamsAsString() const;
	string getSequenceDetectorParamsAsString() const;
	string getTransitionsAsString() const;


};
