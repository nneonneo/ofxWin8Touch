/*
 * ofxWin8Touch.cpp
 * Windows 8 touch event handling.
 *
 * Created by Robert Xiao on August 24, 2015.
 */

#pragma once
#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN8
#endif

#include "ofMain.h"
#include "ofEvents.h"

#include "ofxWin8Touch.h"

static WNDPROC prevWndProc;

/* Get window pointer */
struct EnumWindowsCallbackArgs {
    EnumWindowsCallbackArgs( DWORD p ) : pid( p ) { }
    const DWORD pid;
    std::vector<HWND> handles;
};

static BOOL CALLBACK EnumWindowsCallback( HWND hnd, LPARAM lParam ) {
    EnumWindowsCallbackArgs *args = (EnumWindowsCallbackArgs *)lParam;

    DWORD windowPID;
    (void)::GetWindowThreadProcessId( hnd, &windowPID );
    if ( windowPID == args->pid ) {
        args->handles.push_back( hnd );
    }

    return TRUE;
}

static std::vector<HWND> getToplevelWindows() {
    EnumWindowsCallbackArgs args( ::GetCurrentProcessId() );
    if ( ::EnumWindows( &EnumWindowsCallback, (LPARAM) &args ) == FALSE ) {
		return std::vector<HWND>();
    }
    return args.handles;
}

static HWND getOfxWindow() {
	HWND hwnd = ofGetWin32Window();
	if(!hwnd) {
		/* Glut or something */
		vector<HWND> windows = getToplevelWindows();
		HWND consoleWindow = GetConsoleWindow();
		for(HWND h : windows) {
			if(h != consoleWindow) {
				hwnd = h;
				break;
			}
		}
	}
	return hwnd;
}

/* Handle incoming pointer event */
static bool handlePointerEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	UINT32 pointerId = GET_POINTERID_WPARAM(wParam);
	POINTER_INPUT_TYPE pointerType = PT_POINTER;

	if (!GetPointerType(pointerId, &pointerType)) {
		ofLogError() << "GetPointerType failed: 0x" << hex << GetLastError();
		pointerType = PT_POINTER; // default to PT_POINTER
	}

	ofTouchEventArgs touch;

	union {
		POINTER_INFO pointer;
		POINTER_TOUCH_INFO touch;
		POINTER_PEN_INFO pen;
	} info;
	POINTER_INFO *pointer = &info.pointer;

	switch (pointerType) {
	case PT_TOUCH: {
		if(!GetPointerTouchInfo(pointerId, &info.touch)) {
			ofLogError() << "GetPointerTouchInfo failed: 0x" << hex << GetLastError();
			return false;
		}
		pointer = &info.touch.pointerInfo;

		touch.width = info.touch.rcContact.right - info.touch.rcContact.left;
		touch.height = info.touch.rcContact.bottom - info.touch.rcContact.top;
		touch.angle = info.touch.orientation; // degrees 0-359
		touch.pressure = info.touch.pressure; // 0-1024
		touch.majoraxis = max(touch.width, touch.height);
		touch.minoraxis = min(touch.width, touch.height);
		break;
	}
	case PT_PEN: {
		if (!GetPointerPenInfo(pointerId, &info.pen)) {
			ofLogError() << "GetPointerPenInfo failed: 0x" << hex << GetLastError();
			return false;
		}
		pointer = &info.pen.pointerInfo;

		touch.width = touch.height = 0;
		touch.angle = info.pen.rotation; // degrees 0-359
		touch.pressure = info.pen.pressure; // 0-1024
		touch.majoraxis = touch.minoraxis = 0;

		break;
	}
	default:
		return false; /* unsupported pointer type */
	}

	/* Use HIMETRIC positions for improved precision (subpixel) */
	RECT himetricRect, displayRect;
	if(GetPointerDeviceRects(pointer->sourceDevice, &himetricRect, &displayRect)
	  && himetricRect.right - himetricRect.left > 0
	  && himetricRect.bottom - himetricRect.top > 0) {
		double x = ((double)pointer->ptHimetricLocation.x - himetricRect.left) / (himetricRect.right - himetricRect.left);
		double y = ((double)pointer->ptHimetricLocation.y - himetricRect.top) / (himetricRect.bottom - himetricRect.top);
		touch.x = displayRect.left + x * (displayRect.right - displayRect.left) - ofGetWindowPositionX();
		touch.y = displayRect.top + y * (displayRect.bottom - displayRect.top) - ofGetWindowPositionY();
	} else {
		touch.x = pointer->ptPixelLocation.x - ofGetWindowPositionX();
		touch.y = pointer->ptPixelLocation.y - ofGetWindowPositionY();
	}
	touch.id = pointer->pointerId;
	if(pointer->pointerFlags & (POINTER_FLAG_CANCELED | POINTER_FLAG_CAPTURECHANGED)) {
		/* Cancelled */
		touch.type = ofTouchEventArgs::Type::cancel;
		ofNotifyEvent(ofEvents().touchCancelled, touch);
		return true;
	}

	/*
	if(!(pointer->pointerFlags & POINTER_FLAG_CONFIDENCE)) {
		ofLogError() << "pointer id " << touch.id << " not confident: flags 0x" << hex << pointer->pointerFlags;
	}
	*/

	if(pointer->pointerFlags & POINTER_FLAG_DOWN) {
		touch.type = ofTouchEventArgs::Type::down;
		ofNotifyEvent(ofEvents().touchDown, touch);
	}
	if(pointer->pointerFlags & POINTER_FLAG_UPDATE) {
		touch.type = ofTouchEventArgs::Type::move;
		ofNotifyEvent(ofEvents().touchMoved, touch);
	}
	if(pointer->pointerFlags & POINTER_FLAG_UP) {
		touch.type = ofTouchEventArgs::Type::up;
		ofNotifyEvent(ofEvents().touchUp, touch);
	}
	return true;
}

/* Window callback function */
LRESULT APIENTRY pointerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_POINTERUPDATE:
		if(!IS_POINTER_INCONTACT_WPARAM(wParam))
			break;
		/* fall through */
	case WM_POINTERDOWN:
	case WM_POINTERUP:
	case WM_POINTERCAPTURECHANGED:
		if(handlePointerEvent(uMsg, wParam, lParam))
			return 0;
		break; // fail, call CallWindowProc
	}
	// Chain to the next wndproc
	return CallWindowProc(prevWndProc, hwnd, uMsg, wParam, lParam);
}

void ofxWin8TouchSetup() {
	HWND hwnd = getOfxWindow();
	if(!hwnd) {
		ofLogError() << "Failed to get HWND for ofx window - try calling this later in update().";
		return;
	}

	if(prevWndProc) {
		ofLogError() << "Already called ofxWin8TouchSetup!";
		return;
	}

	prevWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)pointerWndProc);
}
