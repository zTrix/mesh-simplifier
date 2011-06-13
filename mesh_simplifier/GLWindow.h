#pragma once

#include <gl\gl.h> // OpenGL32 
#include <gl\glu.h> // GLu32

// orientation
const float ORIG_ELEVATION = 0.0f;
const float ORIG_AZIMUTH = 0.0f;
const float ORIG_DIST = 3.0f;

const float MIN_DISTANCE = 0.1f;
const float MAX_DISTANCE = 100.0f;

class GLWindow {
public:

    // constructor
    GLWindow() : hWnd_(NULL), hDC_(NULL), hGLRC_(NULL),
                        szClassName_("MeshSimp"), 
                        szAppName_("Mesh Simplification"),
                        width_(0), height_(0), oldWidth_(0), oldHeight_(0),
                        oldX_(0), oldY_(0), newX_(0), newY_(0) {
        resetOrientation();
    };

    ~GLWindow() {};

    GLvoid reSizeScene(GLsizei width, GLsizei height);
    int initOpenGL(GLvoid);
    bool displayMesh();
    GLvoid killMyWindow(GLvoid);
    int createMyWindow(int width, int height, unsigned char bits, const char * menu);
    HWND getHWnd() {return hWnd_;};
    void mouseMotion(int x, int y, bool leftButton, bool rightButton);
    void setNewXY(int x, int y) {newX_ = x; newY_ = y;};
    void resetOrientation() {   
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        elevation_ = ORIG_ELEVATION;
        azimuth_ = ORIG_AZIMUTH;
        dist_ = ORIG_DIST;
    }

    void displayWindowTitle();
    void displayWindowTitle(char *fmt, ...);

private:
    HWND    hWnd_;
    HDC     hDC_;
    HGLRC   hGLRC_;

    const char *szClassName_;
    const char *szAppName_;

    int width_;
    int height_;

    int oldWidth_;
    int oldHeight_;

    int oldX_, oldY_, newX_, newY_;

    float elevation_;
    float azimuth_;
    float dist_;

    // no assignment, copy ctor allowed (implementation not provided).
    GLWindow(const GLWindow&);
    GLWindow& operator=(const GLWindow&);
};
