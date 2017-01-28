/* 
   Compile with:
   g++ -o Space2Ctrl Space2Ctrl.cpp -W -Wall -L/usr/X11R6/lib -lX11 -lXtst

   To install libx11:
   in Ubuntu: sudo apt-get install libx11-dev

   To install libXTst:
   in Ubuntu: sudo apt-get install libxtst-dev

   Needs module XRecord installed. To install it, add line Load "record" to Section "Module" in
   /etc/X11/xorg.conf like this:

   Section "Module"
   Load  "record"
   EndSection

*/

#include <iostream>
#include <X11/Xlibint.h>
#include <X11/keysym.h>
#include <X11/extensions/record.h>
#include <X11/extensions/XTest.h>
#include <sys/time.h>
#include <signal.h>

using namespace std;

struct CallbackClosure {
  Display *ctrlDisplay;
  Display *dataDisplay;
  int curX;
  int curY;
  void *initialObject;
};

typedef union {
  unsigned char type;
  xEvent event;
  xResourceReq req;
  xGenericReply reply;
  xError error;
  xConnSetupPrefix setup;
} XRecordDatum;

#define QUOTE_KEYCODE 48
#define CAPS_KEYCODE 66
// Milliseconds before considering a keypress
#define PRESS_THRESHOLD 300

class Space2Ctrl {

  string m_displayName;
  CallbackClosure userData;
  std::pair<int,int> recVer;
  XRecordRange *recRange;
  XRecordClientSpec recClientSpec;
  XRecordContext recContext;

  void setupXTestExtension() {
    int ev, er, ma, mi;
    if ( ! XTestQueryExtension(userData.ctrlDisplay, &ev, &er, &ma, &mi)) {
      cout << "%sThere is no XTest extension loaded to X server.\n" << endl;
      throw exception();
    }
  }

  void setupRecordExtension() {
    XSynchronize(userData.ctrlDisplay, True);

    // Record extension exists?
    if ( ! XRecordQueryVersion(userData.ctrlDisplay, &recVer.first, &recVer.second)) {
      cout << "%sThere is no RECORD extension loaded to X server.\n"
        "You must add following line:\n"
        "   Load  \"record\"\n"
        "to /etc/X11/xorg.conf, in section `Module'.%s" << endl;
      throw exception();
    }

    recRange = XRecordAllocRange ();
    if (!recRange) {
      // "Could not alloc record range object!\n";
      throw exception();
    }
    recRange->device_events.first = KeyPress;
    recRange->device_events.last = ButtonRelease;
    recClientSpec = XRecordAllClients;

    // Get context with our configuration
    recContext = XRecordCreateContext(userData.ctrlDisplay, 0, &recClientSpec, 1, &recRange, 1);
    if (!recContext) {
      cout << "Could not create a record context!" << endl;
      throw exception();
    }
  }

  static int diff_ms(timeval t1, timeval t2) {
    return ( ((t1.tv_sec - t2.tv_sec) * 1000000)
             + (t1.tv_usec - t2.tv_usec) ) / 1000;
  }

  // Called from Xserver when new event occurs.
  static void eventCallback(XPointer priv, XRecordInterceptData *hook) {

    if (hook->category != XRecordFromServer) {
      XRecordFreeData(hook);
      return;
    }

    CallbackClosure *userData = (CallbackClosure *) priv;
    XRecordDatum *data = (XRecordDatum *) hook->data;
    static bool quote_down = false;
    static bool caps_down = false;
    static bool key_combo = false;
    static struct timeval startWait, endWait;

    unsigned char t = data->event.u.u.type;
    int c = data->event.u.u.detail;

    cout << c << endl;
    switch (t) {
    case KeyPress:
      {
        // cout << "KeyPress";
        if (c == QUOTE_KEYCODE) {
          quote_down = true;
          gettimeofday(&startWait, NULL);
	} else if (c == CAPS_KEYCODE) {
          caps_down = true;
          gettimeofday(&startWait, NULL);
        } else if ( (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_L))
                 || (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_R)) ) {
          if (quote_down) {
            XTestFakeKeyEvent(userData->ctrlDisplay, 255, True, CurrentTime);
            XTestFakeKeyEvent(userData->ctrlDisplay, 255, False, CurrentTime);
          } else if (caps_down) {
            XTestFakeKeyEvent(userData->ctrlDisplay, 254, True, CurrentTime);
            XTestFakeKeyEvent(userData->ctrlDisplay, 254, False, CurrentTime);
					}
        } else { // another key pressed
          if (quote_down || caps_down) {
            key_combo = true;
          } else {
            key_combo = false;
          }

        }

        break;
      }
    case KeyRelease:
      {
        if (c == QUOTE_KEYCODE) {
          quote_down = false;
          if ( ! key_combo) {
            gettimeofday(&endWait, NULL);
            if ( diff_ms(endWait, startWait) < PRESS_THRESHOLD) {
              // if minimum timeout elapsed since minus was pressed
              XTestFakeKeyEvent(userData->ctrlDisplay, 255, True, CurrentTime);
              XTestFakeKeyEvent(userData->ctrlDisplay, 255, False, CurrentTime);
            }
          }
          key_combo = false;
        } else if (c == CAPS_KEYCODE) {
          caps_down = false;

          if ( ! key_combo) {
            gettimeofday(&endWait, NULL);

            if (diff_ms(endWait, startWait) < PRESS_THRESHOLD) {
              // if minimum timeout elapsed since caps was pressed
              XTestFakeKeyEvent(userData->ctrlDisplay, 254, True, CurrentTime);
              XTestFakeKeyEvent(userData->ctrlDisplay, 254, False, CurrentTime);
            }
          }
          key_combo = false;
        } else if ( (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_L))
                 || (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_R)) ) {
          if (quote_down || caps_down)
            key_combo = true;
        }

        break;
      }
    }

    XRecordFreeData(hook);
  }

public:
  Space2Ctrl() { }
  ~Space2Ctrl() {
    stop();
  }

  bool connect(string displayName) {
    m_displayName = displayName;
    if (NULL == (userData.ctrlDisplay = XOpenDisplay(m_displayName.c_str())) ) {
      return false;
    }
    if (NULL == (userData.dataDisplay = XOpenDisplay(m_displayName.c_str())) ) {
      XCloseDisplay(userData.ctrlDisplay);
      userData.ctrlDisplay = NULL;
      return false;
    }

    // You may want to set custom X error handler here

    userData.initialObject = (void *) this;
    setupXTestExtension();
    setupRecordExtension();

    return true;
  }

  void start() {
    // TODO: document why the following events are needed
    XTestFakeKeyEvent(userData.ctrlDisplay, 255, True, CurrentTime);
    XTestFakeKeyEvent(userData.ctrlDisplay, 255, False, CurrentTime);
    XTestFakeKeyEvent(userData.ctrlDisplay, 254, True, CurrentTime);
    XTestFakeKeyEvent(userData.ctrlDisplay, 254, False, CurrentTime);

    if (!XRecordEnableContext(userData.dataDisplay, recContext, eventCallback,
                              (XPointer) &userData)) {
      throw exception();
    }
  }

  void stop() {
    if (!XRecordDisableContext (userData.ctrlDisplay, recContext)) {
      throw exception();
    }
  }

};

Space2Ctrl* space2ctrl;

void stop(int param) {
  delete space2ctrl;
  if(param == SIGTERM)
    cout << "-- Terminating Space2Ctrl --" << endl;
  exit(1);
}

int main() {
  cout << "-- Starting Space2Ctrl --" << endl;
  space2ctrl = new Space2Ctrl();

  void (*prev_fn)(int);

  prev_fn = signal (SIGTERM, stop);
  if (prev_fn==SIG_IGN) signal (SIGTERM,SIG_IGN);

  if (space2ctrl->connect(":0")) {
    space2ctrl->start();
  }
  return 0;
}
