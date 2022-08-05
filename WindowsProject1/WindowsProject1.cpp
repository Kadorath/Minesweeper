// WindowsProject1.cpp : Defines the entry point for the application.

#include "WindowsProject1.h"
#include <string>
#include "main.c"

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
BOOL                InitBoard(HWND, field_t *);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    GameStartProc(HWND, UINT, WPARAM, LPARAM);

field_t *board;
int gameActive; //0 is active, 1 is loss, 2 is victory
bool makeNewGame;
int boardLength;
int boardWidth;
int totalMineCt;
int flagCt;
bool isFlagging;
int result;

HBRUSH hbrLightGrey = CreateSolidBrush(RGB(230, 230, 230));
HBRUSH hbrDarkGrey = CreateSolidBrush(RGB(200, 200, 200));
HBRUSH hbrBlack = CreateSolidBrush(RGB(0, 0, 0));

HICON hIconMine;
HICON hIconFlag;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    board = (field_t *)malloc(sizeof(field_t));
    boardLength = 10;
    boardWidth = 10;
    totalMineCt = (boardLength * boardWidth) / 5;
    flagCt = 0;
    isFlagging = false;

    hIconMine = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE));
    hIconFlag = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FLAG));

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    GetMessage(&msg, nullptr, 0, 0);


    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MINE));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

BOOL InitBoard(HWND hWnd, field_t *board) {
    initialize_board(board, boardLength, boardWidth, hWnd);
    flagCt = 0;
    totalMineCt = (boardLength * boardWidth) / 5;
    set_mines(board, totalMineCt);
    set_hints(board);
    find_starting_area(board);

    isFlagging = false;

    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            result = 0;
            if (wmId >= 1000 && gameActive == 0) {

                SetFocus(hWnd);

                int selectedID = wmId - 1000;
                //Setting down flags
                if (isFlagging) {
                    result = flag_tile(board, selectedID % (boardWidth + 2), selectedID / (boardWidth + 2));
                    if (result == 1) { flagCt++; }
                    else if (result == 2) { flagCt--; }
                }
                //Selecting tiles
                else {
                    result = select_tile(board, selectedID % (boardWidth + 2), selectedID / (boardWidth + 2));
                    update_text(board);
                }

                

                //Loss
                if (result == -1) {
                    for (int i = 0; i < board->length * board->width; i++) {
                        if (board->data[i] == -1) {
                            board->visible[i] = 2;
                            DrawIcon(GetDC(board->buttons[i]), -4, -3, hIconMine);
                        }
                    }
                    gameActive = 1;
                }

                //Win
                if (check_win(board)) {
                    for (int i = 0; i < board->length * board->width; i++) {
                        if (board->data[i] == -1) {
                            board->visible[i] = 2;
                            DrawIcon(GetDC(board->buttons[i]), -4, -3, hIconMine);
                        }
                        else if (board->visible[i] == 0 && board->data[i] != -2) {
                            board->visible[i] = 1;
                            InvalidateRect(hWnd, NULL, FALSE);
                        }
                    }
                    gameActive = 2;
                }

                RECT textRC = { 125, 30, 240, 50 };
                InvalidateRect(hWnd, &textRC, TRUE);

                UpdateWindow(hWnd);
            }
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_NEW:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_START), hWnd, GameStartProc);
                if (makeNewGame) {
                    for (int i = 0; i < board->length * board->width; i++) {
                        DestroyWindow(board->buttons[i]);
                    }
                    free(board->buttons);
                    free(board->data);
                    free(board->visible);

                    InitBoard(hWnd, board);
                }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                PostMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_KEYDOWN:
        if (wParam == VK_SPACE && gameActive == 0) {
            isFlagging = !isFlagging;

            RECT flagIconRC = { 100, 30, 125, 60 };
            InvalidateRect(hWnd, &flagIconRC, TRUE);
            UpdateWindow(hWnd);
        }
        break;
    case WM_NOTIFY: 
        {
            LPNMHDR some_item = (LPNMHDR)lParam;
            if (some_item->idFrom >= 1000 && some_item->code == NM_CUSTOMDRAW) {
                LPNMCUSTOMDRAW item = (LPNMCUSTOMDRAW)some_item;
                HWND button = board->buttons[some_item->idFrom - 1000];
                if (board->data[some_item->idFrom - 1000] == -2) {
                    FillRect(item->hdc, &item->rc, hbrBlack);
                    
                }
                if (board->visible[some_item->idFrom - 1000] == 2) {
                    DrawIcon(item->hdc, -4, -3, hIconMine);
                }
                if (board->visible[some_item->idFrom - 1000] == 1) {
                    FillRect(item->hdc, &item->rc, hbrDarkGrey);
                }
                else if (board->visible[some_item->idFrom - 1000] == -1) {
                    DrawIcon(item->hdc, -4, -3, hIconFlag);
                }
            }
            return CDRF_DODEFAULT;
        }
        break;
    case WM_CREATE: {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_START), hWnd, GameStartProc);
        InitBoard(hWnd, board);       
        break;
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

            if (isFlagging) {
                DrawIcon(hdc, 100, 23, hIconFlag);
            }

            WCHAR* pwcsMineCt;
            pwcsMineCt = (WCHAR*)malloc(sizeof(WCHAR) * 3);
            _itow_s(totalMineCt - flagCt, pwcsMineCt, sizeof(WCHAR) * 3, 10);
            if (gameActive == 0) {
                TextOutW(hdc, 125, 30, L"Mines Left: ", 11);
                int x = _tcslen(pwcsMineCt);
                TextOutW(hdc, 205, 30, pwcsMineCt, x);
            }
            else if (gameActive == 2) {
                TextOutW(hdc, 125, 30, L"You win! ", 8);
            }
            else if (gameActive == 1) {
                TextOutW(hdc, 125, 30, L"You lose :(", 11);
            }
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
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


static HWND lengthEdit;
static HWND widthEdit;

// Message handler for the game start dialogue box
INT_PTR CALLBACK GameStartProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    TCHAR dimenBuffer[3];
    WORD cchCount;

    switch (message)
    {
    case WM_INITDIALOG:
        lengthEdit = CreateWindowEx(
            0, L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
            70, 15, 20, 17,
            hWnd, (HMENU)IDE_LENGTHEDIT,
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);

        widthEdit = CreateWindowEx(
            0, L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
            70, 45, 20, 17,
            hWnd, (HMENU)IDE_WIDTHEDIT,
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            //Fetching user entered length from IDE_LENGTHEDIT
            cchCount = (WORD)SendDlgItemMessage(hWnd,
                IDE_LENGTHEDIT,
                EM_LINELENGTH,
                (WPARAM)0,
                (LPARAM)0);


            if (cchCount > 2 || cchCount <= 0) {
                MessageBox(hWnd, L"Invalid dimension entry.", L"Error", MB_OK);
                break;
            }

            *((LPWORD)dimenBuffer) = cchCount;

            SendDlgItemMessage(hWnd, IDE_LENGTHEDIT, EM_GETLINE, (WPARAM)0, (LPARAM)dimenBuffer);
            dimenBuffer[cchCount] = 0;

            boardLength = atoi((char *)(dimenBuffer));
            if (cchCount == 2) { boardLength = (boardLength * 10) + atoi((char*)(dimenBuffer)+sizeof(TCHAR)); }
            //Fetching user entered width from IDE_WIDTHEDIT
            cchCount = (WORD)SendDlgItemMessage(hWnd,
                IDE_WIDTHEDIT,
                EM_LINELENGTH,
                (WPARAM)0,
                (LPARAM)0);


            if (cchCount > 2 || cchCount == 0) {
                MessageBox(hWnd, L"Invalid dimension entry.", L"Error", MB_OK);
                break;
            }

            *((LPWORD)dimenBuffer) = cchCount;

            SendDlgItemMessage(hWnd, IDE_WIDTHEDIT, EM_GETLINE, (WPARAM)0, (LPARAM)dimenBuffer);
            dimenBuffer[cchCount] = 0;
            
            boardWidth = atoi((char*)(dimenBuffer));
            if(cchCount == 2) { boardWidth = (boardWidth*10) + atoi((char*)(dimenBuffer)+sizeof(TCHAR)); }

            if (boardWidth <= 0 || boardLength <= 0) {
                MessageBox(hWnd, L"Invalid dimension entry.", L"Error", MB_OK);
                break;
            }

            //Set the game window size
            int x = SetWindowPos(GetParent(hWnd), 
                HWND_TOP, 
                0, 0, ((boardWidth + 2) * 25) + 210, ((boardLength + 2) * 25) + 175, 
                SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);

            gameActive = 0;
            makeNewGame = true;
            EndDialog(hWnd, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) 
        {
            makeNewGame = false;
            EndDialog(hWnd, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}