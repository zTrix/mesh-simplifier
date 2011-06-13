#include "stdafx.h"
#include <sys/stat.h>
#include "mesh_simplifier.h"
#include "mesh.h"
#include "GLWindow.h"
#include "Commdlg.h"
#include "ProgressiveMesh.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")
#define MAX_LOADSTRING 100

#define ID_SPLITTER 500
#define SLD_TOP 420
#define SLD_LEFT 20
#define SLD_RIGHT 620
#define SLD_BOTTOM 460
#define SLD_WIDTH (SLD_RIGHT-SLD_LEFT)
#define SLD_HEIGHT (SLD_BOTTOM-SLD_TOP)

HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

Mesh * pMesh = NULL;
ProgressiveMesh * pm = NULL;

GLWindow * pWin = NULL;
HWND hWndSlider;
DWORD pos = 100;
bool lock = false;

char objFileName[256] = {0};

ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MSG msg;
    HACCEL hAccelTable;
    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MESH_SIMPLIFIER));

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MESH_SIMPLIFIER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    InitCommonControls();

    hInst = hInstance;

    int width = 640;
    int height = 480;
    unsigned char depth = 16;

    pWin = new GLWindow();
    if (!pWin || !pWin->createMyWindow(width,height,depth, (LPCSTR)IDC_MENU)) {
        return FALSE;
    }

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(pWin->getHWnd(), hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pWin->killMyWindow();
    delete pm;
    delete pMesh;

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MESH_SIMPLIFIER));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_MESH_SIMPLIFIER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

void loadObjFile() {
    static char filter[] = "Obj files (*.obj)\0*.obj\0";
    OPENFILENAME name;
    char location[256] = {0};
    ZeroMemory(&name, sizeof(OPENFILENAME));
    name.lStructSize = sizeof(name);
    name.hwndOwner = NULL;
    name.lpstrFilter = filter;
    name.lpstrFile = location;
    name.nMaxFile = sizeof(location);
    name.Flags = OFN_HIDEREADONLY;
    name.lpstrDefExt = "obj";
    name.lpstrTitle = "Open Object file";

    if (!GetOpenFileName(&name)) {
        return;
    }

    struct stat fileStat;
    if (stat(name.lpstrFile, &fileStat)) {
        char errmsg[1024];
        sprintf_s(errmsg, 1024, "%s not found", name.lpstrFile);
        MessageBox(NULL, errmsg, "File Not Found Error", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    delete pMesh;
    pMesh = NULL;
    delete pm;
    pm = NULL;

    pWin->displayWindowTitle("%s"," loading obj file, please wait ...");
    pMesh = new Mesh(name.lpstrFile);
    strcpy_s(objFileName, name.lpstrFile);
    if (pMesh) {
        pMesh->Normalize();
    }
    pWin->displayWindowTitle("%s"," constructing progressive mesh ...");
    pm = new ProgressiveMesh(pMesh);
    pWin->resetOrientation();
    pWin->displayWindowTitle();
    SendMessage(hWndSlider, TBM_SETPOS, TRUE, 100);
}

int handleMenu(WPARAM wParam, LPARAM lParam) {
    int wmId, wmEvent;
    wmId    = LOWORD(wParam);
    wmEvent = HIWORD(wParam);
    const int percent = 5;
    switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), pWin->getHWnd(), About);
            break;

        case IDM_EXIT:
            SendMessage(pWin->getHWnd(), WM_CLOSE, wParam, lParam);
            break;
        
        case ID_OPEN_FILE:
            loadObjFile();
            InvalidateRect(pWin->getHWnd(), NULL, TRUE);
            break;

        case ID_FILE_SAVEOBJFILE:
            if (!pm) {
                MessageBox(NULL, "nothing to save", "nothing to save", MB_OK | MB_ICONINFORMATION);
                break;
            }
            FILE * f = NULL;
            char buf[10] = {0};
            sprintf_s(buf, 10, "%d.obj", pm->visibleTriangleNumber());
            pWin->displayWindowTitle("saving to file %s ", buf);
            fopen_s(&f, buf, "w");
            if (!f) {
                MessageBox(NULL, "cannot open file for writing", "File Not Found Error", MB_OK | MB_ICONINFORMATION);
            }
            for (int i = 0; i < pMesh->getNumVerts(); i++) {
                Tuple & t = pMesh->getVertex(i).getXYZ();
                fprintf_s(f, "v %.7f %.7f %.7f\n", t.x, t.y, t.z);
            }
            for (int i = 0; i < pm->triangleNumber(); i++) {
                triangle t = pm->getTriangle(i);
                if (t.active) {
                    int v[3];
                    t.getVerts(v[0], v[1], v[2]);
                    fprintf_s(f, "f %d %d %d\n", v[0]+1, v[1]+1, v[2]+1);
                }
            }
            fclose(f);
            MessageBox(NULL, "write file succeeded", "success", MB_OK | MB_ICONINFORMATION);
            break;
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    CREATESTRUCT *cs;
    HRGN hrgn;

    switch (message) {
        // MENU
        case WM_COMMAND:
            handleMenu(wParam, lParam);
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            pWin->displayMesh();
            EndPaint(hWnd, &ps);
            break;

        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            SetCapture(pWin->getHWnd());
            pWin->setNewXY(LOWORD(lParam), HIWORD(lParam));

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            ReleaseCapture();
            break;

        case WM_MOUSEMOVE:
            if ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)) {
                bool lb = !!(wParam & MK_LBUTTON);
                bool rb = !!(wParam & MK_RBUTTON);
                pWin->mouseMotion(LOWORD(lParam), HIWORD(lParam), lb, rb);
                InvalidateRect(pWin->getHWnd(), NULL, TRUE);
            }
            break;

        case WM_SIZE:
            pWin->reSizeScene(LOWORD(lParam), HIWORD(lParam));
            MoveWindow(hWndSlider, SLD_LEFT, HIWORD(lParam) - 60, LOWORD(lParam) - 40, SLD_HEIGHT, TRUE);
            hrgn = CreateRoundRectRgn(0, 0, LOWORD(lParam) - 40, SLD_HEIGHT + 5, 8, 8);
            SetWindowRgn(hWndSlider, hrgn, TRUE);
            break;

        case WM_CREATE:
            cs = (LPCREATESTRUCT)lParam;
            hWndSlider = CreateWindow(TRACKBAR_CLASS, "", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                                      SLD_LEFT, SLD_TOP, SLD_RIGHT, SLD_BOTTOM,
                                      hWnd, (HMENU)ID_SPLITTER, cs->hInstance, NULL);
            hrgn = CreateRoundRectRgn(0, 0, SLD_WIDTH + 20, SLD_HEIGHT + 5, 8, 8);
            SetWindowRgn(hWndSlider, hrgn, TRUE);
            SendMessage(hWndSlider, TBM_SETPOS, TRUE, 100);
            break;

        case WM_HSCROLL:
            if (lock) break;
            lock = true;
            switch(LOWORD(wParam)) {
                case TB_THUMBPOSITION:
                case TB_PAGEUP:
                case TB_PAGEDOWN:
                    DWORD last = pos;
                    pos = SendMessage(hWndSlider, TBM_GETPOS, 0, 0);
                    if (pm) {
                        int total = pm->edgeCollapseNumber();
                        double target = (1.0 * pos / 100);
                        double cur = 1.0 * pm->visibleTriangleNumber() / pm->triangleNumber();
                        int size = (int)(total * (cur - target));
                        pm->simplify(size);
                        InvalidateRect(pWin->getHWnd(), NULL, TRUE);
                    }
                    break;
            }
            pWin->displayWindowTitle("percent: %d%%", pos);
            lock = false;
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
