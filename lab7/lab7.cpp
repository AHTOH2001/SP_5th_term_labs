// lab7.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "lab7.h"
#define MAX_LOADSTRING 100
#define ID_COMBOBOX_TOOL 10001
#define ID_BUTTON_CLEAR 10002
#define ID_LABEL_SIZE 10003
#define ID_LABEL_COLOR 10004
#define ID_SCROLLBAR_SIZE 10013
#define ID_SCROLLBAR_RED_COLOR 10014
#define ID_SCROLLBAR_GREEN_COLOR 10015
#define ID_SCROLLBAR_BLUE_COLOR 10016

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

HWND hWndComboBoxTool;
HWND hWndButtonClear;
HWND hWnd;
int wmId;
PAINTSTRUCT ps;
HDC hdc;
int countLeftMouse = 0;
const COLORREF WhiteColor = RGB(255, 255, 255);
const int maxColorNum = 255, maxSizeNum = 50;
HWND hRedScroll = NULL, hGreenScroll = NULL, hBlueScroll = NULL, hSizeScroll = NULL;
HBRUSH hBrush;
RECT ColorLineRect; //области отрисовки
const int standartHeight = 23, scrollBarLength = 150, shiftBetween = 10, hatSize = 70;
const int colorStatusRectW = 50, colorStatusRectH = standartHeight * 2 + 150;
bool threadsStopped = false;


int widthOfWindow, heightOfWindow;

enum class TOOLS {
	BRUSH,
	RECT,
	RECT2,
	ELLIPSE,
	ELLIPSE2,
	ERASER,
	FILL,
	GRADIENT
};
enum class SCROLLS {
	RED,
	GREEN,
	BLUE,
	SIZE
};
std::map<int, SCROLLS> scrollsDictionary;
TOOLS toolsDictionary[] = { TOOLS::BRUSH, TOOLS::RECT, TOOLS::RECT2, TOOLS::ELLIPSE, TOOLS::ELLIPSE2, TOOLS::ERASER, TOOLS::FILL, TOOLS::GRADIENT };
int RedNum = 100, GreenNum = 200, BlueNum = 150, SizeNum = 5;
TOOLS currentTool = TOOLS::BRUSH;
HACCEL hAccelTable;
MSG msg;
// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int prevX = 0, prevY = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_LAB7, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB7));

	// Цикл основного сообщения:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB7));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB7);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

struct THREAD_PARAMS
{
	int startX, startY, endX, endY;
	int RColor1, GColor1, BColor1;
	int RColor2, GColor2, BColor2;
};

void StandartFill(int x, int y, COLORREF color)
{
	HDC hdc = GetDC(hWnd);
	HBRUSH br = CreateSolidBrush(color);
	SelectObject(hdc, br);
	ExtFloodFill(hdc, x, y, GetPixel(hdc, x, y), FLOODFILLSURFACE);
	DeleteObject(br);
	ReleaseDC(hWnd, hdc);
}

int PythagoreanDistance(int x1, int y1, int x2, int y2)
{
	return (int)round(sqrt(((double)x2 - x1) * ((double)x2 - x1) + ((double)y2 - y1) * ((double)y2 - y1)));
}

const int dx[] = { 0,0,1,-1 };
const int dy[] = { 1,-1,0,0 };

DWORD WINAPI fillGradient(LPVOID lpParam)
{
	THREAD_PARAMS* params = (THREAD_PARAMS*)lpParam;
	POINT start;
	start.x = params->startX;
	start.y = params->startY;
	COLORREF targetColor;
	int totalDistance = PythagoreanDistance(params->endX, params->endY, start.x, start.y);
	int R = params->RColor1;
	int G = params->GColor1;
	int B = params->BColor1;
	int deltaR = R - params->RColor2;
	int deltaG = G - params->GColor2;
	int deltaB = B - params->BColor2;

	HDC threadHDC;
	threadHDC = GetDC(hWnd);
	targetColor = GetPixel(threadHDC, start.x, start.y);
	StandartFill(start.x, start.y, targetColor);
	POINT point;
	point.x = start.x;
	point.y = start.y;
	std::queue<POINT> qr;
	qr.push(point);
	RECT statusRect;
	statusRect.left = shiftBetween;
	statusRect.top = heightOfWindow - shiftBetween - 20 - hatSize;
	statusRect.right = shiftBetween + 20;
	statusRect.bottom = heightOfWindow - shiftBetween - hatSize;
	Rectangle(threadHDC, statusRect.left, statusRect.top, statusRect.right, statusRect.bottom);
	while (!qr.empty())
	{
		for (int r = 0; r < 4; r++)
		{
			int xx = qr.front().x + dx[r];
			int yy = qr.front().y + dy[r];
			if (xx >= widthOfWindow || xx < 0 || yy >= heightOfWindow || yy < 0)
			{
				continue;
			}

			if (targetColor == GetPixel(threadHDC, xx, yy))
			{
				if (threadsStopped)
				{
					InvalidateRect(hWnd, &statusRect, TRUE);
					ReleaseDC(hWnd, threadHDC);
					return 0;
				}

				point.x = xx;
				point.y = yy;

				qr.push(point);

				double coef = (double)PythagoreanDistance(point.x, point.y, start.x, start.y) / totalDistance;
				int intPart = (int)floor(coef);
				coef -= intPart;
				if (intPart % 2 == 1)
				{
					coef = 1 - coef;
				}
				COLORREF newColor = RGB(R - coef * deltaR, G - coef * deltaG, B - coef * deltaB);
				SetPixel(threadHDC, point.x, point.y, newColor == targetColor ? newColor - 1 : newColor);
			}
		}
		qr.pop();
	}
	InvalidateRect(hWnd, &statusRect, TRUE);
	ReleaseDC(hWnd, threadHDC);

	return 0;
}

void OnLButtonDown(int x, int y)
{
	prevX = x;
	prevY = y;
}

void OnLButtonUp(int x, int y)
{
	switch (currentTool)
	{
	case TOOLS::RECT:
	case TOOLS::RECT2:
	case TOOLS::ELLIPSE:
	case TOOLS::ELLIPSE2:
	{
		HDC hdc = GetDC(hWnd);
		HPEN pen = NULL;
		HBRUSH brush = NULL;
		if (currentTool == TOOLS::RECT || currentTool == TOOLS::ELLIPSE)
		{
			pen = CreatePen(PS_SOLID, SizeNum, RGB(RedNum, GreenNum, BlueNum));
			SelectObject(hdc, pen);
			brush = CreateSolidBrush(WhiteColor);
			SelectObject(hdc, brush);
		}
		if (currentTool == TOOLS::RECT2 || currentTool == TOOLS::ELLIPSE2)
		{
			pen = CreatePen(PS_SOLID, SizeNum, RGB(RedNum, GreenNum, BlueNum));
			SelectObject(hdc, pen);
			brush = CreateSolidBrush(RGB(RedNum, GreenNum, BlueNum));
			SelectObject(hdc, brush);
		}
		if (currentTool == TOOLS::RECT || currentTool == TOOLS::RECT2)
		{
			Rectangle(hdc, prevX, prevY, x, y);
		}
		if (currentTool == TOOLS::ELLIPSE || currentTool == TOOLS::ELLIPSE2)
		{
			Ellipse(hdc, prevX, prevY, x, y);
		}
		DeleteObject(pen);
		DeleteObject(brush);
		ReleaseDC(hWnd, hdc);
	}
	break;
	case TOOLS::FILL:
	{
		StandartFill(x, y, RGB(RedNum, GreenNum, BlueNum));
	}
	break;
	case TOOLS::GRADIENT:
	{
		THREAD_PARAMS params;
		params.startX = prevX;
		params.startY = prevY;
		params.endX = x;
		params.endY = y;
		params.RColor1 = RedNum;
		params.GColor1 = GreenNum;
		params.BColor1 = BlueNum;
		params.RColor2 = 255 - RedNum;
		params.GColor2 = 255 - GreenNum;
		params.BColor2 = 255 - BlueNum;
		DWORD dwThreadId;
		threadsStopped = false;
		HANDLE hThread = CreateThread(NULL, 0, fillGradient, &params, 0, &dwThreadId);
		if (!hThread) {
			MessageBox(hWnd, _T("Can not open thread"), _T("Error"), MB_OK);
		}
	}
	break;
	default:
		break;
	}
}

void OnMouseMove(int x, int y)
{
	if (GetAsyncKeyState(VK_LBUTTON))
	{
		countLeftMouse++;
	}
	else
	{
		countLeftMouse = 0;
	}

	if (countLeftMouse > 1)
	{
		if (currentTool == TOOLS::BRUSH || currentTool == TOOLS::ERASER)
		{
			HDC hdc = GetDC(hWnd);
			HPEN pen = NULL;
			if (currentTool == TOOLS::BRUSH)
			{
				pen = CreatePen(PS_SOLID, SizeNum, RGB(RedNum, GreenNum, BlueNum));
			}

			if (currentTool == TOOLS::ERASER)
			{
				pen = CreatePen(PS_SOLID, SizeNum, WhiteColor);
			}

			SelectObject(hdc, pen);
			MoveToEx(hdc, prevX, prevY, NULL);
			LineTo(hdc, x, y);
			prevX = x;
			prevY = y;
			DeleteObject(pen);
			ReleaseDC(hWnd, hdc);
		}
	}
}

void SaveData()
{
	char data[255] = { 0 };
	snprintf(data, 255, "%d %d %d %d", RedNum, GreenNum, BlueNum, SizeNum);
	FILE* outFile;
	fopen_s(&outFile, "ApplicationContext.txt", "w");
	fwrite(data, sizeof(char), strlen(data), outFile);
	fclose(outFile);
}

void CloneMyself()
{	
	SaveData();
	char szFileName[MAX_PATH];
	GetModuleFileNameA(NULL, szFileName, MAX_PATH);
	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	CreateProcessA(szFileName, NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CREATE:
	{
		FILE* inFile;
		errno_t err = fopen_s(&inFile, "ApplicationContext.txt", "r");
		if (!err) {
			fseek(inFile, 0, SEEK_END);
			int fsize = ftell(inFile);
			fseek(inFile, 0, SEEK_SET);
			char data[255] = {0};
			fread(data, sizeof(char), fsize, inFile);
			fclose(inFile);
			char* context = NULL;
			char* elem = strtok_s(data, " ", &context);
			char* res[4] = {0};
			int i = 0;
			while (elem) {
				res[i] = elem;
				i++;
				elem = strtok_s(NULL, " ", &context);				
			}
			
			
			StrToIntExA(res[0], NULL, &RedNum);
			StrToIntExA(res[1], NULL, &GreenNum);
			StrToIntExA(res[2], NULL, &BlueNum);				
			StrToIntExA(res[3], NULL, &SizeNum);


			
			//RedNum = 
		}
		

		CreateWindow(_T("STATIC"), _T("Size of line"), SS_CENTER | WS_CHILD | WS_VISIBLE,
			shiftBetween, shiftBetween, 150, standartHeight, hWnd, (HMENU)ID_LABEL_SIZE, hInst, NULL);
		hSizeScroll = CreateWindow(_T("SCROLLBAR"), _T(""), WS_CHILD | WS_VISIBLE | SBS_HORZ,
			shiftBetween, shiftBetween + standartHeight, scrollBarLength, standartHeight, hWnd, (HMENU)ID_SCROLLBAR_SIZE, hInst, NULL);
		SetScrollRange(hSizeScroll, SB_CTL, 1, maxSizeNum, TRUE);
		SetScrollPos(hSizeScroll, SB_CTL, SizeNum, TRUE);

		CreateWindow(_T("STATIC"), _T("Color"), SS_CENTER | WS_CHILD | WS_VISIBLE,
			shiftBetween, shiftBetween + (shiftBetween + standartHeight) * 2, (shiftBetween + standartHeight) * 3 - shiftBetween, standartHeight, hWnd, (HMENU)ID_LABEL_COLOR, hInst, NULL);

		CreateWindow(_T("STATIC"), _T("R"), SS_CENTER | WS_CHILD | WS_VISIBLE,
			shiftBetween, (shiftBetween + standartHeight) * 3, standartHeight, standartHeight, hWnd, NULL, hInst, NULL);
		CreateWindow(_T("STATIC"), _T("G"), SS_CENTER | WS_CHILD | WS_VISIBLE,
			shiftBetween + shiftBetween + standartHeight, (shiftBetween + standartHeight) * 3, standartHeight, standartHeight, hWnd, NULL, hInst, NULL);
		CreateWindow(_T("STATIC"), _T("B"), SS_CENTER | WS_CHILD | WS_VISIBLE,
			shiftBetween + (shiftBetween + standartHeight) * 2, (shiftBetween + standartHeight) * 3, standartHeight, standartHeight, hWnd, NULL, hInst, NULL);

		hRedScroll = CreateWindow(_T("SCROLLBAR"), _T(""), WS_CHILD | WS_VISIBLE | SBS_VERT,
			shiftBetween, 20 + (shiftBetween + standartHeight) * 3, standartHeight, scrollBarLength, hWnd, (HMENU)ID_SCROLLBAR_RED_COLOR, hInst, NULL);
		SetScrollRange(hRedScroll, SB_CTL, 0, maxColorNum, TRUE);
		SetScrollPos(hRedScroll, SB_CTL, RedNum, TRUE);		
		hGreenScroll = CreateWindow(_T("SCROLLBAR"), _T(""), WS_CHILD | WS_VISIBLE | SBS_VERT,
			shiftBetween + shiftBetween + standartHeight, 20 + (shiftBetween + standartHeight) * 3, standartHeight, scrollBarLength, hWnd, (HMENU)ID_SCROLLBAR_GREEN_COLOR, hInst, NULL);
		SetScrollRange(hGreenScroll, SB_CTL, 0, maxColorNum, TRUE);
		SetScrollPos(hGreenScroll, SB_CTL, GreenNum, TRUE);

		hBlueScroll = CreateWindow(_T("SCROLLBAR"), _T(""), WS_CHILD | WS_VISIBLE | SBS_VERT,
			shiftBetween + (shiftBetween + standartHeight) * 2, 20 + (shiftBetween + standartHeight) * 3, standartHeight, scrollBarLength, hWnd, (HMENU)ID_SCROLLBAR_BLUE_COLOR, hInst, NULL);
		SetScrollRange(hBlueScroll, SB_CTL, 0, maxColorNum, TRUE);
		SetScrollPos(hBlueScroll, SB_CTL, BlueNum, TRUE);

		scrollsDictionary[ID_SCROLLBAR_SIZE] = SCROLLS::SIZE;
		scrollsDictionary[ID_SCROLLBAR_RED_COLOR] = SCROLLS::RED;
		scrollsDictionary[ID_SCROLLBAR_GREEN_COLOR] = SCROLLS::GREEN;
		scrollsDictionary[ID_SCROLLBAR_BLUE_COLOR] = SCROLLS::BLUE;

		CreateWindow(_T("BUTTON"), _T("clear"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			shiftBetween, 20 + (shiftBetween + standartHeight) * 3 + 160, 70, standartHeight, hWnd, (HMENU)ID_BUTTON_CLEAR, hInst, NULL);

		CreateWindow(_T("STATIC"), _T("Tool"), SS_CENTER | WS_CHILD | WS_VISIBLE,
			shiftBetween, 30 + (shiftBetween + standartHeight) * 4 + 160, 150, standartHeight, hWnd, NULL, hInst, NULL);

		hWndComboBoxTool = CreateWindow(_T("COMBOBOX"), _T(""), WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			shiftBetween, 20 + (shiftBetween + standartHeight) * 5 + 160, 150, 400, hWnd, (HMENU)ID_COMBOBOX_TOOL, hInst, NULL);
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Brush");
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Rectangle");
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Solid rectangle");
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Ellipse");
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Solid ellipse");
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Eraser");
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Fill color");
		SendMessage(GetDlgItem(hWnd, ID_COMBOBOX_TOOL), CB_ADDSTRING, 0, (LPARAM)L"Gradient");

		SendMessage(hWndComboBoxTool, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		//Задаём границы области перерисовки
		ColorLineRect.left = (shiftBetween + standartHeight) * 3;
		ColorLineRect.top = (shiftBetween + standartHeight) * 2;
		ColorLineRect.right = ColorLineRect.left + colorStatusRectW + shiftBetween;
		ColorLineRect.bottom = ColorLineRect.top + colorStatusRectH + 20;

		RECT Rect;
		GetWindowRect(hWnd, &Rect);
		widthOfWindow = Rect.right - Rect.left;
		heightOfWindow = Rect.bottom - Rect.top;
	}
	break;
	case WM_COMMAND:
	{
		wmId = LOWORD(wParam);
		// Разобрать выбор в меню:
		switch (wmId)
		{
		case ID_COMBOBOX_TOOL:
			currentTool = toolsDictionary[SendMessage(hWndComboBoxTool, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0)];
			break;
		case ID_BUTTON_CLEAR:
			threadsStopped = true;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			SaveData();
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_LBUTTONDOWN:
		OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_HSCROLL:
	case WM_VSCROLL:
	{
		SCROLLS currentScroll = scrollsDictionary[GetWindowLong((HWND)lParam, GWL_ID)];
		int* correspondingVar;
		HWND currentScrollWindow;
		int maxValue = 0;
		switch (currentScroll)
		{
		case SCROLLS::RED:
			correspondingVar = &RedNum;
			currentScrollWindow = hRedScroll;
			maxValue = maxColorNum;
			break;
		case SCROLLS::GREEN:
			correspondingVar = &GreenNum;
			currentScrollWindow = hGreenScroll;
			maxValue = maxColorNum;
			break;
		case SCROLLS::BLUE:
			correspondingVar = &BlueNum;
			currentScrollWindow = hBlueScroll;
			maxValue = maxColorNum;
			break;
		case SCROLLS::SIZE:
			correspondingVar = &SizeNum;
			currentScrollWindow = hSizeScroll;
			maxValue = maxSizeNum;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		//Проверяем на какой полосе было перемещение
		switch (LOWORD(wParam)) {
		case SB_PAGERIGHT: //На страницу вправо
			*correspondingVar += 10;
			break;

		case SB_LINERIGHT: //На одну линию вправо
			*correspondingVar += 1;
			break;

		case SB_PAGELEFT: //На страницу влево
			*correspondingVar -= 10;
			break;

		case SB_LINELEFT: //На линию влево
			*correspondingVar -= 1;
			break;

		case SB_THUMBPOSITION: //Любое перемещение
		case SB_THUMBTRACK:
			*correspondingVar = HIWORD(wParam);
			break;
		default:
			break;
		}

		if (*correspondingVar > maxValue)
			*correspondingVar = maxValue;
		if (*correspondingVar < 0)
			*correspondingVar = 0;

		SetScrollPos(currentScrollWindow, SB_CTL, *correspondingVar, TRUE);

		//Вызываем сообщение WM_PAINT по перерисовке области 		
		InvalidateRect(hWnd, &ColorLineRect, TRUE);
	}
	break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		HPEN pen = CreatePen(PS_SOLID, SizeNum, RGB(RedNum, GreenNum, BlueNum));
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		SelectObject(hdc, pen);

		MoveToEx(hdc, (shiftBetween + standartHeight) * 4, (shiftBetween + standartHeight) * 3, NULL);
		LineTo(hdc, (shiftBetween + standartHeight) * 4, (shiftBetween + standartHeight) * 3 + scrollBarLength);

		SelectObject(hdc, oldPen);
		DeleteObject(pen);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CLOSE:
		CloneMyself();		
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

// Обработчик сообщений для окна "О программе".
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
