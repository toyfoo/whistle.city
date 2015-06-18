/// sparrows
/// version 0.9 - 20 january 2015
/// by thomas laureyssens t@toyfoo.com


#include "ofApp.h"

void ofApp::setup()
{

    // sparrow specific settings
    objectID = "";

    //sparrow design settings
    eyePixel = 48;
    beakPixel = 47;
    maintenancePixel = 49;

    //gamesettings
    gameType = 1;
    timeoutTimeStates = 9000.f;
    timeoutTimeWhistles = 20000.f;
    serverTimeOffHack = 0; // the hours the server is off
    discoChance = 5;  // percentage chance there is discolight (after a minimum of 15 whistles)

    // animation settings
    fadeOutTime = 1000.f;
    fadeInTime = 500.f;
    fadeVariationTime = 30.f;

    maintenanceNoError = ofColor::green;
    maintenanceError = ofColor::red;
    eyeWhistleDetected = ofColor::green;
    eyeWhistleUncertain = ofColor::orange;
    eyeListening = ofColor::grey;
    beakConnectionError = ofColor::red;
    beakPointSent = ofColor::green;
    beakPointEnabled = ofColor::brown;

    status200 = ofColor::green;; // success
    status461 = ofColor::blueViolet;; // no active campaign
    
    
    status400 = ofColor::darkorange; // error
    status400severe = ofColor::red; // connection error - about to set whistles to zero
    status400webserver = ofColor::magenta; // webserver down

    // ofApp settings
    ofSetVerticalSync(true);
    //ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(25);
    ofSetWindowTitle("Sparrows");

    // other settings
    serialSleepMillis = 5; // amount of pause not to choke the serial connection

    //zeroing variables
    serialActivated = false;
    webserverDown = false;
    lastWhistleTime = 0;
    whistlesToSend = 0; //todo: save this to an xml file in case the app restarts
    isLoading = false;
    
    noActiveCampaign = false;
    freshMessage = false;
    returnMessageReceived = 0;
    whistleState = 0;
    whistleIsCertain = false;
    previousWhistleTime = 0;
    
    //firstConnectionTest = false;
    
    hasConnectionError = false;
    hasSevereConnectionError = false;
    connectionDownTime = 0;
    
    timeIsOff = true; //by default, the time can be off so we're sure the whistle function runs until the time is synchronised with the time servers by the system
    lastWhistleFrequency = 0;
    discoLevel = 0;
    discoTried = 0;

    // event registration
    ofxKeyframeAnimRegisterEvents(this);
    ofRegisterURLNotification(this);

    //*
    // GUI
    whistleButton.addListener(this,&ofApp::whistleButtonPressed);
    activateSerialButton.addListener(this,&ofApp::activateSerial);

    // setting gui size 300, 18
    ofxGuiSetDefaultWidth(250);
    ofxGuiSetDefaultHeight(18);

	gui.setup("Sparrow Configuration", "settings.xml", 170, 3);
    //gui.add(HzPerWindow.setup("HzPerWindow", 100, 10, 500)); // a window width in frequency domain within signal energy is integrated; then ratio of windowed energy to entire signal energy is used as a spike value; the window floats across frequency axis. A good whistle has very narrow window ~10 Hz. Whistle on spectrogram looks as line. The line thickness shows frequency band the whistle occupies. Good whistle has very narrow band ~10 Hz , bad - 100s of Hz. In ideal, Spike Window must exactly cover a whistle, but we don't know the whistle in advance. Spike Window lower values - more matching for good whistles, higher values - decreasing certainty of good whistles but allowing bad whistles. Spike Window = 100 Hz is reasonable value from my experimentation.

    //gui.add(powerThresholdPercent.setup("powerThresholdPercent", 80, 1, 100)); // minimal value of spike energy to be considered;
    // Look the output: Whistle detected: frequency 1754.96Hz, certainty (Ewin/Eblock) 0.858128
    // In this case Ewin/Eblock = 0.858128 and this is an averaged ratio of signal energy within Spike Window to energy of entire signal block (FFT frame). Another words, this ratio shows how much energy is contained in the whistle. In noisy environment this value would be lower, in clean - higher. The threshold discards signal with such ratio below.

    //gui.add(msecsPerWhistleDuration.setup("msecsPerWhistleDuration", 500, 50, 1000)); // minimal whistle duration (msecs.) to be considered
    // If we detect a whistle (it may be false whistle) that has less duration, this whistle is discarded.  At a small value ~10s msecs there a lot of false positives from ordinal speech. Values ~100s msecs stop such alarms. A simple way to increase reliability is to set relatively big value, 250 msecs or 500 msecs or even 1000 msecs.

    //gui.add(hzPerFrequencyDeviation.setup("hzPerFrequencyDeviation", 100, 20, 500)); // maximal allowed frequency shift between two consequitive (in time) spikes. Between two FFT frames whistle frequency may change (e.g. increasing or decreasing tone whistling - looks on spectrogram as a line with some angle to horizontal axis). The parameters cuts off too steep lines (discards respective whistles).

    gui.add(debugToggle.setup("Debug", true));
    gui.add(activateSerialButton.setup("Activate Serial"));
    gui.add(debugWhistleCertainty.setup("debugWhistleCertainty", 0.0, 0, 1));
    //gui.add(debugWhistleFrequency.setup("debugWhistleFrequency", 10000, 8000, 20000));
    gui.add(whistleButton.setup("whistle"));




    font.loadFont("DIN.otf", 10);

    
    // drawing pixels on screen for debug purposes
    int xposstart = 3;
    int xpos = xposstart;
    int ypos = 3;
    for(int i = 0; i < pixelAmount; i++){
        pixels.push_back(new Pixel());
        pixels[i] -> ledpos = i; // adding the position in the LED strip
        pixels[i] -> set(xpos,ypos,12,12);
        xpos += 15;
        if(xpos >= xposstart + 150) {
            xpos = xposstart;
            ypos += 15;
        }
        pixels[i] -> setup();
    }
    pixels[beakPixel]->partType = "beak";
    pixels[eyePixel]->partType = "eye";
    pixels[maintenancePixel]->partType = "maintenance";


    // load xml settings
    if (settingsXML.load("sparrowsSettings.xml") ){
        ofLog(OF_LOG_NOTICE, "sparrowsSettings.xml loaded!");
	} else {
        settingsXML.addChild("Sparrows");
        settingsXML.setTo("Sparrows");
        ofLog(OF_LOG_NOTICE, "unable to load sparrowsSettings.xml check data/ folder!");
	}

    if(!settingsXML.exists("//objectId")) {
        settingsXML.addChild("objectId");
    }
    if (settingsXML.getValue("//objectId") == "") {
        settingsXML.setValue("//objectId", "undefined_in_sparrow_code");
    } else {
        objectID = settingsXML.getValue("//objectId");
    }

    if(!settingsXML.exists("//profileColor")) {
        settingsXML.addChild("profileColor");
    }
    if (settingsXML.getValue("//profileColor") == "") {
        settingsXML.setValue("//profileColor", "#FF8000");
    } else {
        string bodycolorstring = settingsXML.getValue("//profileColor");
        bodycolorstring = bodycolorstring.replace(0, 1, "0x");
        profileColor = ofColor::fromHex(ofHexToInt(bodycolorstring));
    }
    if(!settingsXML.exists("//onHour")) {
        settingsXML.addChild("onHour");
    }
    if (settingsXML.getValue("//onHour") == "") {
        settingsXML.setValue("//onHour", ofToString(onHour));
    } else {onHour = settingsXML.getValue<int>("//onHour");}

    if(!settingsXML.exists("//offHour")) {
        settingsXML.addChild("offHour");
    }
    if (settingsXML.getValue("//offHour") == "") {
        settingsXML.setValue("//offHour", ofToString(offHour));
    } else {offHour = settingsXML.getValue<int>("//offHour");}

    if(!settingsXML.exists("//gameType")) {
        settingsXML.addChild("gameType");
    }
    if (settingsXML.getValue("//gameType") == "") {
        settingsXML.setValue("//gameType", ofToString(gameType));
    } else {gameType = settingsXML.getValue<int>("//gameType");}

    settingsXML.save("sparrowsSettings.xml");



    // load xml settings
    if (logXML.load("log.xml") ){
        ofLog(OF_LOG_NOTICE, "log.xml loaded!");
	} else {
        logXML.addChild("sparrowslog");
        logXML.setTo("sparrowslog");
        ofLog(OF_LOG_NOTICE, "unable to load log.xml check data/ folder!");
	}

    if(!logXML.exists("//objectId")) { //the double forward slash //RED means: search from the root of the xml for a tag could RED
        logXML.addChild("objectId");
    }
    logXML.setValue("//objectId", objectID);

    if(!logXML.exists("//unsent")) { //the double forward slash //RED means: search from the root of the xml for a tag could RED
        logXML.addChild("unsent");
    }
    if (logXML.getValue("//unsent") == "") {
        logXML.setValue("//unsent", ofToString(whistlesToSend));
    } else {
        whistlesToSend	= logXML.getValue<int>("//unsent");
        
        // starting the code using .bashprofile loads the code faster than the system can join a wifi network.
        // this might block this application when starting up and no network connection is available
        
        //if (whistlesToSend > 0) {
            //sendMessageToServer("whistle_fromFailedConnection");
        //}
    }

    if(!logXML.exists("//log")) {
        logXML.addChild("log");
    }
    logXML.save("log.xml");
    
    ofLogToFile("ofLogFile.txt", true);

    // starting the code using .bashprofile loads the code faster than the system can join a wifi network.
    // this might block this application when starting up and no network connection is available
    //sendMessageToServer("onStartup");

    seedRandomPixelOrder(); //first seed of the randompixelorder
    
    // WHISTLEDETECTOR
//    setupDetector();
//    setupSequenceDetector();
    
    //activateSerial();
    
    
    animateBeakAndEyeAmbientAnim();
    
}

void ofApp::update()
{
    
    

    controlPlaylist.update();
    animPlaylist.update();
    connectionPlaylist.update();
    
    // WHISTLE DETECTOR CODE
    
    //*
    // Get next whistle, it may be null whistle (Whistle::isNull()) if no whistle is detected
	const ofxIntegratedWhistleDetector::Whistle whistle = detector.getNextWhistle();

	// Regardless of we got a whistle or not, we need to update whistle sequence detector
	const ofxWhistleSequenceDetector::Transition transition =
    sequenceDetector.update(!whistle.isNull());

	// Check if we below the final state, i.e. below 100% certainty (this is mode 1 - collecting
	// 100% certainty)
	if (!transition.isToFinal()) { // == transition.certaintyPercent() < 100
		// There is no actual transition. In this case we have two or more whistles those look like
		// one already counted whistle, i.e. pause between the whistles less then
		// ofxWhistleSequenceDetector::msecsPerStateGap() msecs.
		// Nothing needs to be done in this case
		if (!transition.hasTransition())
			;
		// Else we have real transition from one state to another. Here it is not possible to meet
		// final 100% certainty state, because we check this condition on upper level
		else {
			// Initial state. We go to initial state after the first line timeout
			// ofxWhistleSequenceDetector::msecsPerStateTimeout() or the second line timeout
			// ofxWhistleSequenceDetector::msecsPerFinalStateTimeout()
			if (transition.isToInitial()) {
                // TurnOffAllLEDs();
				// ...
                
				ofLog() << "back to initial " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S");
                
                if (whistleState == 5) { // extra reset for disco mode
                    
                    for (int i = 0; i < randomPixelOrder.size(); i++) {
                        int cPixel = randomPixelOrder[i];

                        if (pixels[cPixel] -> partType == "body") {
                            pixels[cPixel] -> clearCuelist();
                            pixels[cPixel] -> addFade(ofColor::black, 0.f, true);
                            pixels[cPixel] -> play();
                        }
                    }
                }
                
                
                whistleState = 0;
                discoTried = 0;
                discoTried = 0;
                
				seedRandomPixelOrder();
     
                animateBeakAndEyeAmbientAnim();


				// If we go to initial state from final 100% certainty state, i.e. from
				// mode 2 - playing, do something
				if (transition.isFromFinal()) {
                    
                    ofLog() << "transition.isFromFinal() - saving the logfile";
                    logXML.save("log.xml"); // we save this here not to overload the system when the file gets too big
                    settingsXML.save("sparrowsSettings.xml");
                    //ofLog() << "logfile saved";

				}
				// Here we go to initial state from non-final state, i.e. we do not reach 100%
				// certainty and begin from zero due to timeout
				else {
					//int x = 0; (void)x;
					// May be some processing is needed here
				}
			}
			// Here only can be some non-initial and non-final state
			else {
     
                animPlaylist.clear();
     
                // TurnOnSomeLEDs(transition.certaintyPercent() - describes how many LEDs);
                ofLog() << "whistle: initial state. " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S");
                
                if (interactionAllowed()) {
                    pixels[eyePixel]->addBlink(2, eyeWhistleUncertain);
                    pixels[maintenancePixel]->addBlink(1, eyeWhistleUncertain); // white blink if uncertain -- similar to filling up
                    animateFillPercentage(ofColor::white, transition.certaintyPercent()/100.f);
                }

            }
        }
    }
	// Here we are in final state, after 100% certainty (this is mode 2 - playing). In this state
	// we will stay exactly ofxWhistleSequenceDetector::msecsPerFinalStateTimeout() msecs. (this is
	// the second line timeout) regardless of any whistles
	else {
		// If we detect a separate whistle, do some LED animation
		if (!whistle.isNull()) {

            if (interactionAllowed()) {

                lastWhistleFrequency = whistle.frequency(); // log it here?
                //cout << "freq: " << lastWhistleFrequency << endl;

                pixels[eyePixel]->addBlink(2, eyeWhistleDetected);

                if (hasConnectionError) {
                    if (!hasSevereConnectionError) {
                        whistlesToSend++; //don't send it to the server, but save it locally
                        logXML.setValue("//unsent", ofToString(whistlesToSend));
                        logXML.save("log.xml");
                    }
                    addLogItem(ofGetTimestampString("%Y-%n-%eT%H:%M:%S:%i%z"), ofToString(whistleState), "http-1", whistle.frequency());
                } else if (noActiveCampaign) {
                    addLogItem(ofGetTimestampString("%Y-%n-%eT%H:%M:%S:%i%z"), ofToString(whistleState), "461", whistle.frequency());
                    // TODO add the out of campaign possibility
     
                    sendMessageToServer("whistle"); // still send a whistle, because we might be in a new campaign now
     
                } else { // in a campaign, and no connection error
                    pixels[maintenancePixel]->addBlink(1, ofColor::white);
                    sendMessageToServer("whistle");
                    addLogItem(ofGetTimestampString("%Y-%n-%eT%H:%M:%S:%i%z"), ofToString(whistleState), "200", whistle.frequency());
                }

                
                if (whistleState < 4) {
                    whistleState++;
                }

                if (whistleState == 5) {
                    discoLevel ++;
                }

                if (whistleState == 4) {
                    if ((ofRandom(100) > (100 - discoChance)) && discoTried > 15 ) {
                        discoLevel = 0;
                        discoTried = 0;
                        whistleState = 5;
                    } else {
                        discoTried++;
                    }
                }

                ofLog() << "state: " << whistleState;

                switch (whistleState) {
                    case 1: // animation of whistle states below 100% certainty.
                        //ofLogNotice("finalState - animation 1");
                        animateFillPercentage(ofColor::white, 1.f);
                    case 2: // animation of whistle state 100%: simple colour
                        whistleState++;
                        //ofLogNotice("finalState - animation 2");
                        animateFillSimpleColors();
                        //cout << "animate simple color, start lapse: " << ofGetElapsedTimeMillis() << endl;
                        break;

                    case 3: // animation of whistle state 100%: first modification of colors
                        //ofLogNotice("finalState - animation 3");
                        animateFillShiftPushColors();
                        break;

                    case 4: // animation of whistle state 100%: second modification of colors
                        //ofLogNotice("finalState - animation 4");
                        animateFillShiftHueColors();
                        break;

                    case 5: // animation of whistle state 100%: second modification of colors
                        //ofLogNotice("finalState - animation 4");
                        animateFillDisco(discoLevel);
                        break;

                    default:
                        break;
                }

            }
		}
	}
     
     //*/


}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(ofColor::gray);

    //sendPixels();

    gui.draw();


    if (debugToggle){

        ofSetColor(0);

        std::stringstream ss;

        ss << "Sparrows v.0.9 " << std::endl;
        ss << "FPS: " << ofGetFrameRate() << std::endl;
        ss << "Serial: " << device.getPortName();

        ofDrawBitmapString(ss.str(), ofVec2f(3, 90));

    }
}


//tween(duration, TweenTarget, targetValue, BezierTween, startValue, delay_duration);
//tween type
//      TWEEN_BACK:
//      TWEEN_BOUNCE:
//      TWEEN_CIRC:
//      TWEEN_CUBIC:
//      TWEEN_ELASTIC:
//      TWEEN_EXPO:
//      TWEEN_LIN:
//      TWEEN_QUAD:
//      TWEEN_QUART:
//      TWEEN_QUINT:
//      TWEEN_SIN:

//      //tween transitions
//      TWEEN_PAUSE
//      TWEEN_EASE_IN
//      TWEEN_EASE_IN_OUT
//      TWEEN_EASE_OUT
//eyePlaylist.addToKeyFrame(	Action::tween(200.f, &rectPos.y, targetPosition.y));


//--------------------------------------------------------------
void ofApp::seedRandomPixelOrder() {

    randomPixelOrder.clear(); // creating a new pixelRandomOrder
    animateRandomPixelUntil = 0;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            randomPixelOrder.push_back(i);
        }
    }
    ofRandomize(randomPixelOrder);

}

//--------------------------------------------------------------
void ofApp::animateFillSimpleColors() {

    for (int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel

        pixels[cPixel] -> addPause(fadeVariationTime * i, true);
        pixels[cPixel] -> addFade(profileColor, fadeInTime, true); // fadintime was 100.f
        //if (i==0) {pixels[randomPixelOrder[i]] -> addCoutElapsedTime();}
        pixels[cPixel] -> addPause(timeoutTimeWhistles, false);
        //if (i==0) {pixels[randomPixelOrder[i]] -> addCoutElapsedTime();}
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
    }


}

//--------------------------------------------------------------
void ofApp::animateFillShiftPushColors() {

    for (int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> addPause(fadeVariationTime * i, true);
        pixels[cPixel] -> addFade(shiftColor(profileColor), fadeInTime, true); // fadintime was 100.f
        pixels[cPixel] -> addPause(timeoutTimeWhistles, false);
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
    }

}

//--------------------------------------------------------------
void ofApp::animateFillShiftHueColors() {

    vector<ofColor> tempToColors;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            tempToColors.push_back(pixels[i] -> colorTo);
        }
    }

    ofRandomize(tempToColors); // identical to std::random_shuffle (tempToColors.begin(), tempToColors.end());

    int it = 0;

    for (int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> addPause(fadeVariationTime * i, true);
        pixels[cPixel] -> addFade(shiftHue(tempToColors[it]), fadeInTime/2, true);
        pixels[cPixel] -> addPause(timeoutTimeWhistles + fadeVariationTime * i, false);
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
        it++;
    }

}

//--------------------------------------------------------------
void ofApp::animateFillDisco(int mode) {

    float pause;
    float fade = 0.f;
    float speedvar = 0.60;

    if (mode > 12) {
        mode = 12;
    }

    pause = 700.f - 50.f * mode;


    int animationamount = floor(timeoutTimeWhistles*speedvar/(pause+fade));
    //cout << "animationamount " << animationamount << endl;


    vector<ofColor> tempToColors;

    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            tempToColors.push_back(pixels[i] -> colorTo);
        }
    }


    tempToColors[0] = ofColor::white;


    ofRandomize(tempToColors); // identical to std::random_shuffle (tempToColors.begin(), tempToColors.end());

    for (int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> clearCuelist();
    }

    for (int j = 0; j < animationamount; j++) {
        int it = 0;
        for (int i = 0; i < randomPixelOrder.size(); i++) {
            int cPixel = randomPixelOrder[i]; //current pixel
            pixels[cPixel] -> addFade(tempToColors[it], fade, true);
            pixels[cPixel] -> addPause(pause, true);
            it++;
        }
        ofRandomize(tempToColors);
    }

    for (int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> addPause(fadeVariationTime * i, false);
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
    }
}




//--------------------------------------------------------------
void ofApp::animateFillPercentage(ofColor fillColor, float percentage) {

    //cout << "animate fill percentage " << percentage << endl;
    int previousAnimateRandomPixelUntil = animateRandomPixelUntil; // in case this animation has ran before, we want to know
    animateRandomPixelUntil = floor(randomPixelOrder.size() * percentage);

    if (previousAnimateRandomPixelUntil <= animateRandomPixelUntil) { // progressing: more whistle certainty

        for (int i = 0; i < animateRandomPixelUntil; i++) {
            int cPixel = randomPixelOrder[i]; //current pixel
            //cout << "i: " << i << " cPixel " << cPixel << endl;
            if (i < previousAnimateRandomPixelUntil) { // previous pixels stay white but get a new fade-out value
                pixels[cPixel] -> clearPlaylist();
                pixels[cPixel] -> addPause(timeoutTimeStates + fadeVariationTime * i, false);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime + fadeVariationTime * i, false);
                pixels[cPixel] -> play();
            } else {
                pixels[cPixel] -> addPause(fadeVariationTime * (i - previousAnimateRandomPixelUntil), true);
                pixels[cPixel] -> addFade(fillColor, fadeInTime, true);
                pixels[cPixel] -> addPause(timeoutTimeStates, false);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
                pixels[cPixel] -> play();
            }
        }
    } else { // detector is less certain about the whistle - turn pixels to black again

        for (int i = previousAnimateRandomPixelUntil; i >= 0; i--) {
            int cPixel = randomPixelOrder[i]; //current pixel

            if (i >= animateRandomPixelUntil) {
                pixels[cPixel] -> addPause(fadeVariationTime * i, true);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, true);
                pixels[cPixel] -> play();
            } else {  // previous pixels stay white but get a new fade-out value
                pixels[cPixel] -> clearPlaylist();
                pixels[cPixel] -> addPause(timeoutTimeStates + fadeVariationTime * i, false);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
                pixels[cPixel] -> play();
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::animateBeakAndEyeAmbientAnim() {
    
    using namespace Playlist;
    
        pixels[eyePixel] -> addFade(ofColor::white, 10000.f, false);
        pixels[eyePixel] -> addFade(ofColor::black, 10000.f, false);
        pixels[eyePixel] -> play();

    
        pixels[beakPixel] -> addSetColor(ofColor::white, false);
        pixels[beakPixel] -> addPause(50.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::black, false);
        pixels[beakPixel] -> addPause(100.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::white, false);
        pixels[beakPixel] -> addPause(50.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::black, false);
        pixels[beakPixel] -> addPause(100.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::white, false);
        pixels[beakPixel] -> addPause(50.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::black, false);
        pixels[beakPixel] -> addPause(19050.f, false);
        pixels[beakPixel] -> play();
    
        animPlaylist.addKeyFrame(Action::pause(20200.f));
        animPlaylist.addKeyFrame(Action::event(this, "animateBeakAndEyeAmbientAnim"));
}

//--------------------------------------------------------------
bool ofApp::interactionAllowed() {
    return true;
    
//    if ((ofGetHours() > onHour && ofGetHours() <= offHour) || /*debugToggle || */timeIsOff) {
//        return true;
//    } else {
//        return false;
//    }
    
}


//--------------------------------------------------------------
void ofApp::onWhistle( double frequency, float certainty) {
    using namespace Playlist;

    lastWhistleFrequency = frequency;


    if (interactionAllowed()) {

        //        cout << "[notice ] onWhistle certainty: " << certainty << " isWhistleCertain: " << whistleIsCertain << " whistleState: " << whistleState << " time interval: " << whistleInterval << endl;


        if (certainty > 0.9) {

            if (whistleState >= 2 && whistleState < 4) {
                whistleState++;
            }

            // NEW* ----

            if (whistleState == 5) {
                discoLevel ++;
            }

            if (whistleState == 4) {
                if ((ofRandom(100) > (100 - discoChance)) && discoTried > 10 ) {
                    discoLevel = 0;
                    discoTried = 0;
                    whistleState = 5;
                } else {
                    discoTried++;
                }
            }

            if (certainty > 0.9 && certainty < 1 ) { // if it's almost certainly a whistle, that's ok.
                certainty = 1;
            }

            sendMessageToServer("whistle");
            pixels[eyePixel]->addBlink(2, eyeWhistleDetected);

        } else {
            pixels[eyePixel]->addBlink(2, eyeWhistleUncertain);
        }

        // Sparrows main animation
        switch (whistleState) {
            case 0: // creating new random values for the animation to look organic
            {
                seedRandomPixelOrder();
                logXML.save("log.xml");
            }
            case 1: // animation of whistle states below 100% certainty.
            {
                ofLogNotice("whistleState 1");
                whistleState = 1; // in case whistleState was 0, we're advancing a state (case 0 has no break)

                animateFillPercentage(ofColor::white, certainty);

                if (certainty < 0.9) {
                    break;
                }
            }
            case 2: { // animation of whistle state 100%: simple colour

                ofLogNotice("whistleState 2");
                whistleState = 2;

                animateFillSimpleColors();

                controlPlaylist.update(); // is needed for the following clear() to work, since we already added elements in this same frame (we get into case#2 from case#1 when certainty is > 0.9)
                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles + fadeVariationTime * randomPixelOrder.size()));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;
            }

            case 3: // animation of whistle state 100%: first modification of colors
                ofLogNotice("whistleState 3");

                // simple shift of color

                animateFillShiftPushColors();

                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;

            case 4: // animation of whistle state 100%: second modification of colors

                ofLogNotice("whistleState 4");

                animateFillShiftHueColors();

                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;

            case 5: //

                ofLogNotice("whistleState 5");

                animateFillDisco(discoLevel);

                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;
                // more cases will be added

            default:
                break;
        }

    } else {ofLog(OF_LOG_NOTICE, "whistle - but too late or too early");}
}



//--------------------------------------------------------------
void ofApp::onKeyframe(ofxPlaylistEventArgs& args){

    //ofLog(OF_LOG_VERBOSE) << "onKeyframe " << args.message << ": " << ofGetFrameNum();

    using namespace Playlist;

    // this check is only necessary if you want to be absolutely sure that
    // the onKeyFrame Event was sent by the same object as the receiver.
    if (args.pSender != static_cast<void*>(this)) return;


    if (args.message == "whistleState 0") {
        ofLogNotice("onKeyFrame main playlist  | whistleState is now 0");
        whistleState = 0;
    }
    
    if (args.message == "send_whistle_delayed") {
        ofLogNotice("sendMessageToServer - delayed");
        sendMessageToServer("whistle_delayed");
    }
    
    if (args.message == "animateBeakAndEyeAmbientAnim") {
        ofLogNotice("animateBeakAndEyeAmbientAnim");
        animateBeakAndEyeAmbientAnim();
    }
    
    if (args.message == "retryWebserverConnection") {
        ofLogNotice("retryWebserverConnection");
        
        if (whistlesToSend > 0) {
            sendMessageToServer("whistle_fromFailedConnection");
        }
    }
    
    if (args.message == "retryInternetConnection") {
        ofLogNotice("retryInternetConnection");
        
        ofLoadURLAsync("http://google.com/robots.txt", "connectionTest");
    }
}

// communication functions
//--------------------------------------------------------------

void ofApp::sendPixels (){

    if (serialActivated) {
        vector<unsigned char> allColorChars;

        for(int i = 0; i < pixelAmount; i++){
            allColorChars.push_back(pixels[i]->color.r);
            allColorChars.push_back(pixels[i]->color.g);
            allColorChars.push_back(pixels[i]->color.b);
        }

        ofx::IO::ByteBuffer buffer(allColorChars);
        device.send(buffer);

    }
}

//--------------------------------------------------------------
void ofApp::activateSerial(){

    if (!serialActivated) {

        std::vector<ofx::IO::SerialDeviceInfo> devicesInfo = ofx::IO::SerialDeviceUtils::listDevices();

        ofLogNotice("ofApp::setup") << "Connected Devices: ";

        for (std::size_t i = 0; i < devicesInfo.size(); ++i)
        {
            ofLogNotice("ofApp::setup") << "\t" << devicesInfo[i];
        }

        if (!devicesInfo.empty())
        {
            // Connect to the first matching device.
            bool success = device.setup(devicesInfo[0], 115200);

            if(success)
            {
                device.registerAllEvents(this);
                ofLogNotice("ofApp::setup") << "Successfully setup " << devicesInfo[0];
            }
            else
            {
                ofLogNotice("ofApp::setup") << "Unable to setup " << devicesInfo[0];
            }
        }
        else
        {
            ofLogNotice("ofApp::setup") << "No devices connected.";
        }

        serialActivated = true;
    }
}

//--------------------------------------------------------------
void ofApp::sendMessageToServer(string type){

    //return;
    

    ofLog() << "send servermessage " + type + " " + ofGetTimestampString("%Y-%n-%e T%H:%M:%S");

    if (type == "whistle") {
        whistlesToSend++;
        logXML.setValue("//unsent", ofToString(whistlesToSend));
        //logXML.save("log.xml");
        if (!isLoading) {
            freshMessage = true;
            //cout << "whistle: http://sparrows.meteor.com/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType) << endl;
            ofLoadURLAsync("http://sparrows.meteor.com/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType), "whistle");
            isLoading = true;
        }
    }
    if (type == "whistle_fromFailedConnection") {
        if (!isLoading) {
            //cout << "whistle_fromFailedConnection: http://sparrows.meteor.com/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType) << endl;
            ofLoadURLAsync("http://sparrows.meteor.com/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType), "whistle_fromFailedConnection");
            isLoading = true;
        }
    }
    if (type == "whistle_delayed") {
        if (!isLoading) {
            ofLoadURLAsync("http://sparrows.meteor.com/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType), "whistle_delayed");
            isLoading = true;
        }
    }
    if (type == "onStartup") {
        if (!isLoading) {
            //ofLoadURLAsync("http://sparrows.meteor.com/activityapi?objectId=" + objectID + "&activityType=ping&gameType=" + ofToString(gameType), "onStartup");
            isLoading = true;
        }
    }
}



//--------------------------------------------------------------
void ofApp::addLogItem(string datetime, string animation, string serverresponse, float frequency){

    ofXml logAddition;

    logAddition.addChild("w");
    logAddition.setTo("w");

    logAddition.addValue("dt", datetime); // ISO8601_FORMAT
    logAddition.addValue("f", frequency); // whistle frequency
    //logAddition.addValue("t", gameType);
    //logAddition.addValue("ID", objectID);
    logAddition.addValue("a", animation);
    logAddition.addValue("s", serverresponse);

    if(!logXML.exists("//log")) {
        logXML.addChild("log");
    }
    //cout << "addLog 1: " << logXML.getName() << endl;
    logXML.setTo("//log");
    logXML.addXml(logAddition);
    //cout << "addLog 2: " << logXML.getName() << endl;
    logXML.setToParent();
    //cout << "addLog 3: " << logXML.getName() << endl;


    // placing the save in the whistle update loop to occur after whistling stops, so that we dont' hog the system saving the logfile all too often.
    //logXML.save("log.xml");
}


//--------------------------------------------------------------
void ofApp::urlResponse(ofHttpResponse & response){
    using namespace Playlist;

    if(response.request.name == "connectionTestFirst"){ // just pinged google to see if the internet connection works
        
        
            if (response.status == 200) { // google works
                webserverDown = true; // the sparrows webserver
                pixels[maintenancePixel] -> clearPlaylist();
                pixels[maintenancePixel] -> addBlinkLoop(status400webserver, 100.f, 500.f, 10.f, 200.f);
                
                connectionPlaylist.addKeyFrame(Action::pause(20000.f));
                connectionPlaylist.addKeyFrame(Action::event(this, "retryWebserverConnection"));
                
                //todo: send email to sparrows admin
            }
            
            else { // internet connection is down
                
                ofRemoveAllURLRequests();
                
                pixels[maintenancePixel] -> clearPlaylist();
                pixels[maintenancePixel] -> addBlinkLoop(status400, 100.f, 500.f, 10.f, 200.f);
                
                connectionDownTime = ofGetElapsedTimeMillis();
                
                
                // try again
                connectionPlaylist.addKeyFrame(Action::pause(20000.f));
                connectionPlaylist.addKeyFrame(Action::event(this, "retryInternetConnection"));
            }
            
    } else if(response.request.name == "connectionTest"){ // subsequent connection tests
        
            
            if (response.status == 200) { // great, google works, so connection is back on
                connectionPlaylist.addKeyFrame(Action::pause(20000.f));
                connectionPlaylist.addKeyFrame(Action::event(this, "retryWebserverConnection"));
            }
            
            else { // google still doesn't work
            
                unsigned long long currentTime = ofGetElapsedTimeMillis();
                if (currentTime - connectionDownTime > 3 * 86400000) { // 3 days
                    hasSevereConnectionError = true;
                    
                    // indicator light red -- needs restart -- doesn't save anymore
                    pixels[maintenancePixel] -> clearPlaylist();
                    pixels[maintenancePixel] -> setColor(status400severe);
                    
                    
                    
                    if (whistlesToSend > 0) {
                        whistlesToSend = 0;
                        logXML.setValue("//unsent", ofToString(whistlesToSend));
                        logXML.save("log.xml");
                    }

                } else if (currentTime - connectionDownTime > 2 * 86400000) {
                    pixels[maintenancePixel] -> clearPlaylist();
                    pixels[maintenancePixel] -> addBlinkLoop(status400severe, 50.f, 200.f, 10.f, 100.f); //blinking fast
                    
                } else if (currentTime - connectionDownTime > 86400000) { // 86400000 day of miliseconds

                        pixels[maintenancePixel] -> clearPlaylist();
                        pixels[maintenancePixel] -> addBlinkLoop(status400severe, 100.f, 500.f, 10.f, 200.f); // normal blink
                }
                
                // try again
                connectionPlaylist.addKeyFrame(Action::pause(20000.f));
                connectionPlaylist.addKeyFrame(Action::event(this, "retryInternetConnection"));
            }
        
        
    } else if (response.status == 200) { // HTTP response of all other (whistle & delayed whistle) connections
        
        isLoading = false;

        if (hasConnectionError) {
            hasConnectionError = false;
            hasSevereConnectionError = false; // sustained internet connectivity issues
            webserverDown = false;
            pixels[maintenancePixel] -> clearPlaylist();
            pixels[maintenancePixel] -> setColor(ofColor::black);
        }

        ofXml serverResponseXML;

        serverResponseXML.loadFromBuffer(response.data.getText());

        //cout << serverResponseXML.toString() << endl;
        
        ofLog() << "http response " << response.status << " " << response.error << " " << response.request.name << "ServerMessage: " << serverResponseXML.getValue("//status") << " time: " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S");

        if(serverResponseXML.exists("//onHour")) {
            if (serverResponseXML.exists("//onHour") && serverResponseXML.exists("//offHour") && serverResponseXML.getValue("//onHour") != "" && serverResponseXML.getValue("//offHour") != "") {
                onHour = serverResponseXML.getValue<int>("//onHour");
                offHour = serverResponseXML.getValue<int>("//offHour");
                settingsXML.setValue("//onHour", serverResponseXML.getValue("//onHour"));
                settingsXML.setValue("//offHour", serverResponseXML.getValue("//offHour"));
            }
            if (serverResponseXML.exists("//gameType") && serverResponseXML.getValue("//gameType") != "" && serverResponseXML.getValue("//gameType") != "0") {
                gameType = serverResponseXML.getValue<int>("//gameType");
                settingsXML.setValue("//gameType", serverResponseXML.getValue("//gameType"));
            }
            if (serverResponseXML.exists("//profileColor") && serverResponseXML.getValue("//profileColor") != "") {
                settingsXML.setValue("//profileColor", serverResponseXML.getValue("//profileColor"));
                string bodycolorstring = serverResponseXML.getValue("//profileColor");
                bodycolorstring = bodycolorstring.replace(0, 1, "0x");
                profileColor = ofColor::fromHex(ofHexToInt(bodycolorstring));
            }
            if (serverResponseXML.exists("//serverTime") && serverResponseXML.getValue("//serverTime") != "") { //ISO8601 - eg. 2014-08-21T12:29:51.652Z
                Poco::DateTime max1;
                try
                {
                    int tz = 0;
                    Poco::DateTime serverTime;
                    Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, serverResponseXML.getValue("//serverTime"), serverTime, tz);
                    //serverTime.makeLocal(tz);
                    //serverTime.makeUTC(tz);
                    //cout << "Server Hour (UTC) is " << serverTime.hour() << " computer hour is " << ofGetHours() << endl; // seems to be 2 hours off
                    if ((serverTime.hour() + serverTimeOffHack) != ofGetHours()) {
                        int minutesdiff = serverTime.minute() - ofGetMinutes();
                        if ( minutesdiff > 10 || minutesdiff < -10) {
                            timeIsOff = true;
                            //cout << "setting time is off!" << endl;
                        }
                    } else {
                        timeIsOff = false;
                        //cout << "setting time is false!" << endl;
                    }
                }
                catch (const Poco::SyntaxException& exc)
                {
                    ofLogError("ofApp::urlResponse()") << "Syntax exception: " << exc.displayText();
                }

            }
            if (serverResponseXML.exists("//status") && serverResponseXML.getValue("//status") != "") {

                //cout << "serverResponseXML.getValue(//status) " << serverResponseXML.getValue("//status") << endl;

                // status OK
                if (serverResponseXML.getValue("//status") == "200") { //whistle was correctly received
                
                noActiveCampaign = false;

                    if(response.request.name == "whistle"){
                        whistlesToSend--;
                        logXML.setValue("//unsent", ofToString(whistlesToSend));

                        //addLogItem(serverResponseXML.getValue("//serverTime"), ofToString(whistleState), "200"); // saves the log.xml file //logXML.save("log.xml");

                        if (freshMessage) { //no visual feedback after a connection failure
                            freshMessage = false;
                            pixels[beakPixel] -> addBlink(2, status200);
                            pixels[maintenancePixel] -> addBlink(1, status200);
                        }

                        if (whistlesToSend > 0) { // if there were unsent wistles, send them now. this is delayed, since the website interface flashes with intensive use
                            connectionPlaylist.addKeyFrame(Action::pause(1500.f));
                            connectionPlaylist.addKeyFrame(Action::event(this, "send_whistle_delayed"));
                        }
                    }
                    else if(response.request.name == "whistle_delayed"){
                        whistlesToSend--;
                        logXML.setValue("//unsent", ofToString(whistlesToSend));
                        //logXML.save("log.xml");

                        if (whistlesToSend > 0) {
                            connectionPlaylist.addKeyFrame(Action::pause(1000.f));
                            connectionPlaylist.addKeyFrame(Action::event(this, "send_whistle_delayed"));
                        }
                    }

                }
                else if (serverResponseXML.getValue("//status") == "461") { // status no active campaign

                    noActiveCampaign = true;
                    
                    if (whistlesToSend > 0) { // unsent whistles from this and previous campain should be removed
                        whistlesToSend = 0;
                        logXML.setValue("//unsent", ofToString(whistlesToSend));
                        logXML.save("log.xml");
                    }
                   
                    if(response.request.name == "whistle"){
                        pixels[beakPixel] -> addBlink(2, status461);
                        pixels[maintenancePixel] -> addBlink(1, status461);
                    }
                }
                else if ((serverResponseXML.getValue("//status") == "400" || serverResponseXML.getValue("//status") == "461") && response.request.name == "onStartup") { // status no active campaign
                    ofLogNotice("Server connection works. Loaded settings.");
                }
                else {
                    ofLogError("Unimplemented server return status: " + serverResponseXML.getValue("//status") + " error: " + serverResponseXML.getValue("//error"));
                    addLogItem(serverResponseXML.getValue("//serverTime"), ofToString(whistleState), serverResponseXML.getValue("//status"), 0.f); //logXML.save("log.xml");
                    pixels[beakPixel] -> addBlink(2, status400);
                    pixels[maintenancePixel] -> addBlink(1, status400);
                }
            }
        }
        
    }
    else {
        
        if (!hasConnectionError) { // in case of connection error, run this only once
            cout << "one time connection error run" << endl;
            ofLog() << "connection error. time: " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S"); //ofGetTimestampString("%Y-%n-%e T%H:%M:%S:%i%z")
            hasConnectionError = true;
            
            //figuring out if it's our webserver or the internet connection
            ofLoadURLAsync("http://google.com/robots.txt", "connectionTestFirst");
            
        } else { // connection error is already registered
            
            connectionPlaylist.clear(); // if there is a connection error, try again in 30 seconds
            connectionPlaylist.addKeyFrame(Action::pause(20000.f));
            connectionPlaylist.addKeyFrame(Action::event(this, "connectionTest"));
        }
        
        
        

        if(response.status != -1) isLoading = false;
        
        if(response.status == -1) {
            //cout << "attempting to remove request: " << response.request.getID() << endl;
            ofRemoveAllURLRequests();
            //ofRemoveURLRequest(response.request.getID());
            isLoading = false;
            
//            connectionPlaylist.clear(); // if there is a connection error, try again in 30 seconds
//            connectionPlaylist.addKeyFrame(Action::pause(30000.f));
//            connectionPlaylist.addKeyFrame(Action::event(this, "send_whistle_delayed"));
        }

        if (response.request.name == "whistle" && freshMessage) {
            freshMessage = false;
            pixels[beakPixel] -> addBlink(1, status400);
        }
	}
}



// helper functions
//--------------------------------------------------------------
ofColor ofApp::shiftColor(ofColor inputColor) {
    ofFloatColor randomColor;
    float r = 0.2;
    randomColor.set(ofRandom(-r, r), ofRandom(-r, r), ofRandom(-r, r));
    return inputColor + randomColor;
}

ofColor ofApp::shiftHue(ofColor inputColor) {
    float hueAngle = inputColor.getHueAngle();
    hueAngle += ofRandom(0, 30);
    if (!ofInRange(hueAngle, 0, 360)) {
        hueAngle -= 360;
    }
    inputColor.setHueAngle(hueAngle);
    return inputColor;
}

void ofApp::newDiscoColors() { // disco mode!
    // this code is not ready for beaks, eyes and maintainance pixels at other places than at the last 3 pixels of the LED strand.

    vector<ofColor> tempToColors;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            tempToColors.push_back(pixels[i] -> colorTo);
        }
    }

    ofRandomize(tempToColors); // identical to std::random_shuffle (tempToColors.begin(), tempToColors.end());

    int it = 0;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            pixels[i]->colorFrom = pixels[i]->colorTo;
            pixels[i]->fadeAmount = 0;
            pixels[i]->colorTo = shiftHue(tempToColors[it]);
            it++;
        }
    }
}

// debug and log functions
//--------------------------------------------------------------

void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args)
{
    // Decoded serial packets will show up here.
    SerialMessage message(args.getBuffer().toString(), "", 255);
    serialMessages.push_back(message);
}


void ofApp::onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args)
{
    // Errors and their corresponding buffer (if any) will show up here.
    SerialMessage message(args.getBuffer().toString(),
                          args.getException().displayText(),
                          500);

    serialMessages.push_back(message);
}

//--------------------------------------------------------------
void ofApp::whistleButtonPressed(){
    /*
    unsigned long long currentTime = ofGetElapsedTimeMillis();
    if (currentTime - previousDebugWhistleTime < timeoutTimeWhistles) {
        debugWhistleCertainty = debugWhistleCertainty + 0.25;
        if (debugWhistleCertainty > 1){
            debugWhistleCertainty = 0;
        }
    } else {
        debugWhistleCertainty = 0;
    }
    previousDebugWhistleTime = currentTime;
    */
    onWhistle(debugWhistleFrequency, debugWhistleCertainty);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    //*
	if(key == 's') {
		gui.saveToFile("settings.xml");
	}
	if(key == 'l') {
		gui.loadFromFile("settings.xml");
	}
    if(key == 'w') {
		onWhistle(1000, 0.5);
	}
    //*/

}

//--------------------------------------------------------------
void ofApp::exit()
{
    device.unregisterAllEvents(this);

	whistleButton.removeListener(this,&ofApp::whistleButtonPressed);
    activateSerialButton.removeListener(this,&ofApp::activateSerial);
    ofUnregisterURLNotification(this);
    ofxKeyframeAnimUnRegisterEvents(this);
}



// future developement
//--------------------------------------------------------------

/*
 void ofApp::savePixelPositions(){

 //to implement
 // json writer: http://stackoverflow.com/questions/4289986/jsoncpp-writing-to-files
 //http://forum.openframeworks.cc/t/create-your-own-json-data-and-save-json-file/16560

 #include<json/writer.h>
 Code:

 Json::Value event;
 Json::Value vec(Json::arrayValue);
 vec.append(Json::Value(1));
 vec.append(Json::Value(2));
 vec.append(Json::Value(3));

 event["competitors"]["home"]["name"] = "Liverpool";
 event["competitors"]["away"]["code"] = 89223;
 event["competitors"]["away"]["name"] = "Aston Villa";
 event["competitors"]["away"]["code"]=vec;

 std::cout << event << std::endl;


 //https://github.com/jefftimesten/ofxJSON

 }
 */

// windowless: http://forum.openframeworks.cc/t/windowless-of/1876/4

//WHISTLEDETECTOR

//*
void ofApp::setupDetector()
{
    // Set detector parameters
    detector.setHzPerWindow(130);
    detector.setWhistlePowerThresholdPercent(90);
    detector.setMsecsPerWhistleDuration(150);
    detector.setHzPerWhistleFrequencyDeviation(130);
    detector.setFramePowerThreshold(0.001f);
    
    
    
    // Open detector. As a result the detector starts working in separate thread and collects
    // detected whistles in internal buffer. ofxIntegratedWhistleDetector::getNextWhistle() must be
    // used to retrieve available whistles in FIFO order from the buffer
    detector.open();
}

void ofApp::setupSequenceDetector()
{
    // Set sequence detector parameters
    sequenceDetector.setWhistleCountInSequence(4);
    sequenceDetector.setMsecsPerStateGap(1000);
    sequenceDetector.setMsecsPerStateTimeout(timeoutTimeStates);
    sequenceDetector.setMsecsPerFinalStateTimeout(timeoutTimeWhistles);
    
}

string ofApp::getDetectorParamsAsString() const
{
    // Print detector parameters actual values (these are corrected in accordance with FFT
    // resolution and processing step)
    string msg = "Detector Parameters:\n";
    msg += "hzPerWindow = " + ofToString(detector.hzPerWindow()) + "\n";
    msg += "whistlePowerThresholdPercent = " + ofToString(detector.whistlePowerThresholdPercent()) + "\n";
    msg += "msecsPerWhistleDuration = " + ofToString(detector.msecsPerWhistleDuration()) + "\n";
    msg += "hzPerWhistleFrequencyDeviation = " + ofToString(detector.hzPerWhistleFrequencyDeviation()) + "\n";
    return msg;
}

string ofApp::getSequenceDetectorParamsAsString() const
{
    // Print sequence detector parameters actual values (these are corrected in accordance with
    // CLOCKS_PER_SEC (time.h) resolution)
    string msg = "Sequence Detector (State Machine) Parameters:\n";
    msg += "whistleCountInSequence = " + ofToString(sequenceDetector.whistleCountInSequence()) + "\n";
    msg += "msecsPerStateGap = " + ofToString(sequenceDetector.msecsPerStateGap()) + "\n";
    msg += "msecsPerStateTimeout = " + ofToString(sequenceDetector.msecsPerStateTimeout()) + "\n";
    return msg;
}

string ofApp::getTransitionsAsString() const
{
    string msg;
    for (deque<pair<ofxWhistleSequenceDetector::Transition,
         ofxIntegratedWhistleDetector::Whistle> >::const_iterator i = transitions.begin();
         i != transitions.end(); ++i) {
        const ofxWhistleSequenceDetector::Transition& transition = i->first;
        const ofxIntegratedWhistleDetector::Whistle& whistle = i->second;
        msg += transition.toString() + ". \t" +
        (whistle.isNull() ?
         string("Go to initial state due to timeout.") :
         whistle.toString()) +
        "\n";
    }
    return msg;
}

//*/