
#include "stdafx.h"
#include "GLWindow.h"
#include "ProgressiveMesh.h"
#include <assert.h>
#include <string>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLu32.lib")

extern ProgressiveMesh* pm; 
extern HINSTANCE hInst;

#define BUF_SIZE 1024
char buf[BUF_SIZE];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Display title text for window
void GLWindow::displayWindowTitle(char *fmt, ...) {
    if (pm)
        sprintf_s(buf, BUF_SIZE, "Mesh Simplification - Triangles: %d -- ", pm->visibleTriangleNumber());
    else 
        sprintf_s(buf, BUF_SIZE, "Mesh Simplification - ");
    int i;
    for (i = 0; i < BUF_SIZE; i++) 
        if (!buf[i]) break;

    va_list ap;
    va_start(ap, fmt);
    vsprintf_s(&buf[i], BUF_SIZE - i,  fmt, ap);
    SetWindowText(getHWnd(), buf);
}

void GLWindow::displayWindowTitle() {
    if (pm)
        sprintf_s(buf, BUF_SIZE, "Mesh Simplification - Triangles: %d", pm->visibleTriangleNumber());
    else
        sprintf_s(buf, BUF_SIZE, "Mesh Simplification ");
    SetWindowText(getHWnd(), buf);
}

// Resize the OpenGL window
GLvoid GLWindow::reSizeScene(GLsizei width, GLsizei height)
{
    if (height==0) height = 1; // height == 0 not allowed

    glViewport(0,0,width,height);

    width_ = width;
    height_ = height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate The Perspective of the winodw
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,MIN_DISTANCE,MAX_DISTANCE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Initialize OpenGL
int GLWindow::initOpenGL(GLvoid)
{
    glShadeModel(GL_SMOOTH); // Gouraud shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Enable lights and lighting
    glEnable(GL_LIGHT0); // default value is (1.0, 1.0, 1.0, 1.0)
    glEnable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE); // backface culling
    
    return true;
}

bool GLWindow::displayMesh() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    gluLookAt(0,0,dist_, 0,0,0, 0,1,0);

    glRotatef(elevation_,1,0,0);
    glRotatef(azimuth_,0,1,0);
    
    glShadeModel(GL_FLAT);
    glPolygonMode( GL_FRONT, GL_FILL );

    if (pm) {
        glColor3ub(128, 128, 128);
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < pm->triangleNumber(); i++) {
            float a[3];
            triangle t = pm->getTriangle(i);
            if (t.active) {
                const float* pFltArrayN = t.getNormal();
                glNormal3fv(pFltArrayN);
                const float* pFltArray1 = t.getVert1();
                glVertex3fv(pFltArray1);
                const float* pFltArray2 = t.getVert2();
                glVertex3fv(pFltArray2);
                const float* pFltArray3 = t.getVert3();
                glVertex3fv(pFltArray3);
    
                a[0] = -pFltArrayN[0];
                a[1] = -pFltArrayN[1];
                a[2] = -pFltArrayN[2];
                glNormal3fv(a);
                glVertex3fv(pFltArray3);
                glVertex3fv(pFltArray2);
                glVertex3fv(pFltArray1);
            }
        }
        glEnd();
    }
    
    SwapBuffers(hDC_);
    return true;
}

// shut down OpenGL window
GLvoid GLWindow::killMyWindow(GLvoid) {
    if (hGLRC_) {
        if (!wglMakeCurrent(NULL,NULL)) {
            assert(false);
        }

        if (!wglDeleteContext(hGLRC_)) {
            assert(false);
        }
        hGLRC_ = NULL;
    }

    if (hDC_ && !ReleaseDC(hWnd_,hDC_)) {
        assert(false);
        hDC_ = NULL;
    }

    if (hWnd_ && !DestroyWindow(hWnd_)) {
        assert(false);
        hWnd_ = NULL;
    }

    if (!UnregisterClass((LPCTSTR)szClassName_,hInst)) {
        assert(false);
        hInst = NULL;
    }
}

int GLWindow::createMyWindow(int width, int height, unsigned char bits, LPCSTR menu) {
    width_ = width;
    height_ = height;

    GLuint      PixelFormat;
    WNDCLASS    wc;
    DWORD       dwExStyle;
    DWORD       dwStyle;
    RECT        WindowRect;
    WindowRect.left = 0;
    WindowRect.right = width;
    WindowRect.top = 0;
    WindowRect.bottom = height;

    wc.style            = CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = NULL;

    wc.lpszMenuName     = menu;
    wc.lpszClassName    = szClassName_;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, ("Failed To Register The Window Class."),"ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwStyle=WS_OVERLAPPEDWINDOW;


    AdjustWindowRectEx(&WindowRect, dwStyle, false, dwExStyle);     // Adjust Window To True Requested Size


    hWnd_=CreateWindowEx(dwExStyle,
                            szClassName_,
                            szAppName_,
                            dwStyle |   
                            WS_CLIPSIBLINGS |
                            WS_CLIPCHILDREN,
                            0, 0,
                            WindowRect.right - WindowRect.left,
                            WindowRect.bottom - WindowRect.top,
                            NULL,
                            NULL,
                            hInst,
                            NULL);
    if (!hWnd_)
    {
        killMyWindow(); // window creation failed
        MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd=  
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        bits,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0, 
        16, 
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0 
    };

    hDC_=GetDC(hWnd_);  // device context
    if (!hDC_)
    {
        killMyWindow(); 
        MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    PixelFormat = ChoosePixelFormat(hDC_,&pfd);
    if (!PixelFormat) // get pixel format
    {
        killMyWindow();
        MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if(!SetPixelFormat(hDC_,PixelFormat,&pfd))  // set pixel format
    {
        killMyWindow();
        MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    hGLRC_ = wglCreateContext(hDC_);
    if (!hGLRC_)    // get rendering context
    {
        killMyWindow();
        MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if(!wglMakeCurrent(hDC_,hGLRC_)) // make it the current rendering context
    {
        killMyWindow();
        MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    ShowWindow(hWnd_,SW_SHOW);

    SetForegroundWindow(hWnd_);
    SetFocus(hWnd_);
    reSizeScene(width, height);

    if (!initOpenGL())
    {
        killMyWindow();
        MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    InvalidateRect(hWnd_, NULL, TRUE); // repaint window

    return true;
}

void GLWindow::mouseMotion(int x, int y, bool leftButton, bool rightButton) {
    oldX_ = newX_; 
    oldY_ = newY_;
    newX_ = x;  
    newY_ = y; 

    if (newX_ & 0x1000) newX_ = -(0xFFFF - newX_); // when move mouse up, turns from 0 to 65535
    if (newY_ & 0x1000) newY_ = -(0xFFFF - newY_); // turn an unsigned value to a signed value so math works

    float RelX = (newX_ - oldX_) / (float)width_;
    float RelY = (newY_ - oldY_) / (float)height_;

    if (rightButton) {
        dist_ += 10 * RelY;

        if (dist_ > MAX_DISTANCE) dist_ = MAX_DISTANCE;
        if (dist_ < MIN_DISTANCE) dist_ = MIN_DISTANCE;
    } else if (leftButton) { 
        azimuth_ += (RelX*180);
        elevation_ += (RelY*180);
    }
}
