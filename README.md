ofxWin8Touch
============

This addon provides OpenFrameworks touch events on Windows 8 and above. It uses the new `WM_POINTER` events
to obtain more detailed information about the touch, as well as providing improved touch event stability and precision (compared with `WM_TOUCH`).

The development of this addon was inspired by `ofxWin7` and later `ofxWinTouch`. The big difference is that this addon
uses `WM_POINTER` (available since Windows 8), not `WM_TOUCH` (available since Windows 7). For some devices, this makes a big
difference: `WM_TOUCH` may sometimes drop or mangle touch events which are passed through fine by `WM_POINTER`. You should try
this addon if `ofxWinTouch` is giving you weird results (such as missing touches or "stuck" touches).

It is dead simple to use: add the five standard touch event handlers to your `ofApp` (with matching implementations in `ofApp.cpp`):

	void touchDown(ofTouchEventArgs & touch);
	void touchMoved(ofTouchEventArgs & touch);
	void touchUp(ofTouchEventArgs & touch);
	void touchDoubleTap(ofTouchEventArgs & touch);
	void touchCancelled(ofTouchEventArgs & touch);

the following `#include`:

	#include "ofxWin8Touch.h"

and the following to `setup`:

	ofxWin8TouchSetup();
	ofRegisterTouchEvents(this);

See `example_basic` for simple usage (tested with a Perceptive Pixel 55" touchscreen).

HighDPI Awareness
=================

Some devices (like the Windows Surface Pro) have a default "High DPI" setting to make text more readable on the screen. 
On the Surface Pro, the default settings is 150% DPI - which means a display resolution of 1920x1080 will report a screen size of 1280x720 to openFrameworks, but still report touches on a 1920x1080 grid.
Thankfully, there's an easy fix:
 * Open Project Properties
 * Navigate to Configuration Properties > Manifest Tool > Input and Output
 * Set 'Enable DPI Awareness' to Yes
