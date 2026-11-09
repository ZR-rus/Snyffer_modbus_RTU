// Snyffer_modbus_RTU.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Snyffer_modbus_RTU.h"
#include <Windows.h>
#include <string>
#include <tchar.h>
//#include <iostream>
//#include <fstream>
#include <ctime>
#include <time.h>

#define MAX_LOADSTRING 100
#define fon RGB(0,250,250)

// Глобальные переменные:
unsigned int crc = 0xFFFF;
bool port, command_03, command_04, command_06, command_10;
unsigned char BUF_OBMENA_CHAR[512], master[512], slave[512], READ_registr[8] = { 0x10,0x03,0x02,0x00,0,0x1e,0,0 };// 0xc7, 0x3b };
char  BUF_OBMENA_DEC_MASTER[512], BUF_OBMENA_DEC[512], ENCODER_SLAVE[512], BUF_OBMENA_HEX_MASTER[512],
BUF_OBMENA_HEX[512], device_char[3], command_char[3], registr_char[5], byte_char[3], data_char[5], crc_char[5],
test_char[5];
int BUF_OBMENA_INT[5], sizebuf;
unsigned int device_int, command_int, registr_int, byte_int, data_int, crc_int, number_register = 1;
HMENU BTN1 = (HMENU)101, BTN2 = (HMENU)102, BTN3 = (HMENU)103, BTN4 = (HMENU)104, BTN5 = (HMENU)105, BTN6 = (HMENU)106;
HWND OPEN, LISTEN, READ, WRITE, CLEAR, CLOSE_PORT, WINDOW_master, WINDOW_slave, hex_master, hex_slave, dec_master,
dec_slave, test,test2,test3,test4,device_master, device_slave, command_master, command_slave, registr_master, registr_slave,
data_master, data_slave, crc_master, crc_slave, com_port, baud_port, edit;
HFONT hFont1, hFont2, hFont3;

// объявления для COM порта
HANDLE initCOM;
char buff[5],baud[7];
//DWORD LenBUF_OBMENA;
DCB DCB1;
LPCSTR Setting_COM;
COMMTIMEOUTS timeout{ 0,0,100,0,0 };
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void RESET();
unsigned int CRC16_Modbus(unsigned char* buf, int len);

// функции
void RESET() 
{
    for (int i = 0; i < 512; i++)BUF_OBMENA_DEC[i] = ' ';
    for (int i = 0; i < 512; i++)BUF_OBMENA_HEX[i] = ' ';
    SendMessageA(WINDOW_master, WM_SETTEXT, sizeof BUF_OBMENA_DEC, (LPARAM)&BUF_OBMENA_DEC);
    SendMessageA(device_master, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(command_master, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(registr_master, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(data_master, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(crc_master, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(hex_master, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(dec_master, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(WINDOW_slave, WM_SETTEXT, sizeof BUF_OBMENA_DEC, (LPARAM)&BUF_OBMENA_DEC);
    SendMessageA(device_slave, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(command_slave, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(registr_slave, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(data_slave, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(crc_slave, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(hex_slave, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(dec_slave, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(edit, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(com_port, WM_SETTEXT, 0, (LPARAM)" ");
    SendMessageA(baud_port, WM_SETTEXT, 0, (LPARAM)" ");
    PurgeComm(initCOM, PURGE_RXCLEAR);// очистить com порт
}
unsigned int CRC16_Modbus(unsigned char* buf, int len) {
   
    for (int pos = 0; pos < len; pos++) {
        crc ^= (unsigned int)buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else {
                crc >>= 1;
            }
        }
    }
    // Преобразование байтов: crc = ((crc & 0x00ff) << 8) | ((crc & 0xff00) >> 8);  
    return crc;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SNYFFERMODBUSRTU, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNYFFERMODBUSRTU));

    MSG msg;

    // Цикл основного сообщения:
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
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNYFFERMODBUSRTU));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(fon);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SNYFFERMODBUSRTU);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       0, 100, 1280, 600, nullptr, nullptr, hInstance, nullptr);
      if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
    int Value;
    switch (message)
    {
    case WM_CREATE:
    {
        //SYSTEMTIME sm;
        //GetSystemTime(&sm);
        //wsprintfA(BUF_OBMENA_CHAR, "%lu", sm.wYear);//преобразуем число в символ
        hFont1 = CreateFont(30, 10, 0, 0, 700, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("Times New Roman"));
        hFont2 = CreateFont(14, 5, 0, 0, 500, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("Arial"));
        hFont3 = CreateFont(18, 7, 0, 0, 1500, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("GOST type A"));
        OPEN = CreateWindowEx(WS_EX_DLGMODALFRAME, _T("BUTTON"), _T("Подключиться"), WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE | BS_PUSHLIKE, 20, 100, 120, 30, hWnd, BTN1, hInst, 0);// кнопка
        LISTEN = CreateWindowEx(WS_EX_DLGMODALFRAME, _T("BUTTON"), _T("Слушать"), WS_CHILD | BS_DEFPUSHBUTTON | BS_PUSHLIKE, 20, 140, 120, 30, hWnd, BTN2, hInst, 0);// кнопка
        READ = CreateWindowEx(WS_EX_DLGMODALFRAME, _T("BUTTON"), _T("Считать"), WS_CHILD | BS_DEFPUSHBUTTON | BS_PUSHLIKE, 20, 180, 120, 30, hWnd, BTN3, hInst, 0);// кнопка
        WRITE = CreateWindowEx(WS_EX_DLGMODALFRAME, _T("BUTTON"), _T("Записать"), WS_CHILD | BS_DEFPUSHBUTTON | BS_PUSHLIKE, 20, 220, 120, 30, hWnd, BTN4, hInst, 0);// кнопка
        CLEAR = CreateWindowEx(WS_EX_DLGMODALFRAME, _T("BUTTON"), _T("Очистить"), WS_CHILD | BS_DEFPUSHBUTTON | BS_PUSHLIKE, 20, 260, 120, 30, hWnd, BTN5, hInst, 0);// кнопка 
        CLOSE_PORT = CreateWindowEx(WS_EX_DLGMODALFRAME, _T("BUTTON"), _T("Закрыть порт"), WS_CHILD | BS_DEFPUSHBUTTON | BS_PUSHLIKE, 20, 300, 120, 30, hWnd, BTN6, hInst, 0);// кнопка
        edit = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT | ES_MULTILINE | ES_AUTOVSCROLL , 750, 40, 500, 380, hWnd, 0, hInst, 0);
        device_master = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 100, 100, 20, hWnd, 0, hInst, 0);
        command_master = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 300, 100, 100, 20, hWnd, 0, hInst, 0);
        registr_master = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 400, 100, 100, 20, hWnd, 0, hInst, 0);
        data_master = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 500, 100, 100, 20, hWnd, 0, hInst, 0);
        crc_master = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 600, 100, 100, 20, hWnd, 0, hInst, 0);
        WINDOW_master = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 140, 500, 20, hWnd, 0, hInst, 0);
        dec_master = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 160, 500, 20, hWnd, 0, hInst, 0);        
        hex_master = CreateWindow(_T("EDIT"), _T(""), ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 180, 500, 20, hWnd, 0, hInst, 0);
        device_slave = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 300, 100, 20, hWnd, 0, hInst, 0);
        command_slave = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 300, 300, 100, 20, hWnd, 0, hInst, 0);
        registr_slave = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 400, 300, 100, 20, hWnd, 0, hInst, 0);
        data_slave = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 500, 300, 100, 20, hWnd, 0, hInst, 0);
        crc_slave = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 600, 300, 100, 20, hWnd, 0, hInst, 0);
        WINDOW_slave = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 340, 500, 20, hWnd, 0, hInst, 0);
        hex_slave = CreateWindow(_T("EDIT"), _T(""), ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 380, 500, 20, hWnd, 0, hInst, 0);
        dec_slave = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 360, 500, 20, hWnd, 0, hInst, 0);
        com_port = CreateWindow(_T("COMBOBOX"), NULL, CBS_DROPDOWN | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 20, 20, 120, 150, hWnd, 0, hInst, 0);
        baud_port = CreateWindow(_T("COMBOBOX"), NULL, CBS_DROPDOWN | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 20, 60, 120, 150, hWnd, 0, hInst, 0);
        for (int i = 0; i < 10; i++)//заполняем список для выбора com порта
        {
            char NumberCOM[5] = "COM";
            wsprintfA(&NumberCOM[3], "%lu", i);//преобразуем число в символ
            SendMessageA(com_port, CB_ADDSTRING, 0, (LPARAM)&NumberCOM[0]);
        }
        SendMessage(com_port, WM_SETTEXT, 0, (LPARAM)_T("COM-порт:"));      
        char baudCOM[7]="";
        wsprintfA(&baudCOM[0], "%lu", 9600);//преобразуем число в символ
        SendMessageA(baud_port, CB_ADDSTRING, 0, (LPARAM)&baudCOM[0]);
        wsprintfA(&baudCOM[0], "%lu", 19200);//преобразуем число в символ
        SendMessageA(baud_port, CB_ADDSTRING, 0, (LPARAM)&baudCOM[0]);
        wsprintfA(&baudCOM[0], "%lu", 115200);//преобразуем число в символ
        SendMessageA(baud_port, CB_ADDSTRING, 0, (LPARAM)&baudCOM[0]);
        SendMessage(baud_port, WM_SETTEXT, 0, (LPARAM)_T("baud:"));

        test = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 10, 500, 100, 20, hWnd, 0, hInst, 0);
        test2 = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 10, 520, 100, 20, hWnd, 0, hInst, 0);
        test3 = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 500, 200, 20, hWnd, 0, hInst, 0);
        test4 = CreateWindow(_T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 220, 520, 200, 20, hWnd, 0, hInst, 0);

        SendMessageA(WINDOW_master, WM_SETTEXT, 0, LPARAM("no connect..."));
        for (int i = 0; i < 512; i++)BUF_OBMENA_CHAR[i] = 0;
        
    }
    break;
    case WM_COMMAND:
    {
        if (wParam == 101)// подключиться
        {
            KillTimer(hWnd, 1);//отключить таймер
            KillTimer(hWnd, 2);//отключить таймер
            SendMessageA(com_port, WM_GETTEXT, sizeof buff, (LPARAM)&buff);// считываем введеный номер порта
            if (buff[3] == '-')
            {
                MessageBox(0, _T("Укажите номер COM порта! "), L"Сообщение", 0);
                return 0;
            }
            SendMessageA(baud_port, WM_GETTEXT, sizeof baud, (LPARAM)&baud);// считываем введеный номер порта
            if (baud[0] == 'b')
            {
                MessageBox(0, _T("Укажите скорость COM порта! "), L"Сообщение", 0);
                return 0;
            }
            ///////////////открытие порта///////////////////////////////////////////////////////////////////
            initCOM = CreateFileA(buff, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (initCOM == INVALID_HANDLE_VALUE)//проверка доступности порта
            {
                MessageBox(0, _T("COM порт не доступен"), L"Сообщение об ошибке", 0);
                SendMessage(com_port, WM_SETTEXT, 0, (LPARAM)_T("COM-порт:"));
                return 0;//выход
            }
            else
            {
                std::string setting = " parity=N data=8 stop=1";
                std::string baudrate = ": baud=";
                std::string com = buff + baudrate + baud + setting;
                Setting_COM = (com).c_str(); // преобразование string в LPCSTR
                Value = BuildCommDCBA(Setting_COM, &DCB1);//присвоить настройки порту
                Value = SetCommState(initCOM, &DCB1);//инициализация порта
                //MessageBox(0, _T("COM порт отрыт"), L"Сообщение", 0);
                port = true;
                RESET();
                SendMessageA(WINDOW_master, WM_SETTEXT, 0, (LPARAM)"ready...");
                ShowWindow(LISTEN, 1);
                ShowWindow(READ, 1);
                ShowWindow(WRITE, 1);
                ShowWindow(CLEAR, 1);
                ShowWindow(CLOSE_PORT, 1);
                ShowWindow(com_port, 0);
                ShowWindow(baud_port, 0);
                ShowWindow(OPEN, 0);
                
            }
        }
        if (wParam == 102)//слушать порт
        {
            KillTimer(hWnd, 1);//отключить таймер
            KillTimer(hWnd, 2);//отключить таймер
            SendMessageA(test, WM_SETTEXT, 0, (LPARAM)"0");
            SetTimer(hWnd, 1, 100, NULL);//включить первый таймер на 1000 миллисек   Условная задержка 
        }
        if (wParam == 103)//считать регистры
        {    
            //SendMessageA(test3, WM_SETTEXT, 0, (LPARAM));
            //SendMessageA(test4, WM_SETTEXT, 0, (LPARAM)crc_L);
            //READ_registr[2] = crc_H-48;
            //READ_registr[3] = crc_L-48;
            crc = 0xffff;
            CRC16_Modbus(READ_registr, 6);
            SendMessageA(test2, WM_SETTEXT, sizeof crc, (LPARAM)&crc);
            READ_registr[6] = crc;
            READ_registr[7] = crc>>8;


            registr_int = 48;
            
            Value = WriteFile(initCOM, &READ_registr, 8, 0, NULL);//записываем в порт из буфер
            Value = ReadFile(initCOM, &BUF_OBMENA_CHAR, 512, 0, NULL);// считываем порт в буфер
            
            for (int i = 0; i < 512; i++)slave[i] = BUF_OBMENA_CHAR[i];
            for (int i = 0; i < 507; i = i + 5)
            {
                wsprintfA(&BUF_OBMENA_DEC[i], "%lu", slave[i / 5] / 100 % 10);//преобразуем число в символ
                wsprintfA(&BUF_OBMENA_DEC[i + 1], "%lu", slave[i / 5] / 10 % 10);//преобразуем число в символ
                wsprintfA(&BUF_OBMENA_DEC[i + 2], "%lu", slave[i / 5] % 10);//преобразуем число в символ
                BUF_OBMENA_DEC[i + 3] = 32;
                BUF_OBMENA_DEC[i + 4] = 32;

                BUF_OBMENA_HEX[i] = '0';
                BUF_OBMENA_HEX[i + 1] = 'x';
                if (slave[i / 5] / 16 > 9)BUF_OBMENA_HEX[i + 2] = slave[i / 5] / 16 + 55;
                else BUF_OBMENA_HEX[i + 2] = slave[i / 5] / 16 + 48;
                if (slave[i / 5] % 16 > 9)BUF_OBMENA_HEX[i + 3] = slave[i / 5] % 16 + 55;
                else BUF_OBMENA_HEX[i + 3] = slave[i / 5] % 16 + 48;
                BUF_OBMENA_HEX[i + 4] = ' ';
            }
            SendMessageA(dec_slave, WM_SETTEXT, sizeof BUF_OBMENA_DEC, (LPARAM)&BUF_OBMENA_DEC);
            SendMessageA(hex_slave, WM_SETTEXT, sizeof BUF_OBMENA_HEX, (LPARAM)&BUF_OBMENA_HEX);
            SendMessageA(edit, WM_SETTEXT, 10, (LPARAM)BUF_OBMENA_HEX);
            SendMessageA(edit, WM_SETTEXT, 0, (LPARAM)'\r\n');

            wsprintfA(&device_char[0], "%lu", slave[0] / 100 % 10);//преобразуем число в символ
            wsprintfA(&device_char[1], "%lu", slave[0] / 10 % 10);
            wsprintfA(&device_char[2], "%lu", slave[0] % 10);
            wsprintfA(&command_char[0], "%lu", slave[1] / 100 % 10);
            wsprintfA(&command_char[1], "%lu", slave[1] / 10 % 10);
            wsprintfA(&command_char[2], "%lu", slave[1] % 10);
            wsprintfA(&byte_char[0], "%lu", slave[2] / 100 % 10);
            wsprintfA(&byte_char[1], "%lu", slave[2] / 10 % 10);
            wsprintfA(&byte_char[2], "%lu", slave[2] % 10);
            wsprintfA(&data_char[0], "%lu", (slave[3 + (registr_int - 48) * 2] * 256 + slave[4 + (registr_int - 48) * 2]) / 10000 % 10);
            wsprintfA(&data_char[1], "%lu", (slave[3 + (registr_int - 48) * 2] * 256 + slave[4 + (registr_int - 48) * 2]) / 1000 % 10);
            wsprintfA(&data_char[2], "%lu", (slave[3 + (registr_int - 48) * 2] * 256 + slave[4 + (registr_int - 48) * 2]) / 100 % 10);
            wsprintfA(&data_char[3], "%lu", (slave[3 + (registr_int - 48) * 2] * 256 + slave[4 + (registr_int - 48) * 2]) / 10 % 10);
            wsprintfA(&data_char[4], "%lu", (slave[3 + (registr_int - 48) * 2] * 256 + slave[4 + (registr_int - 48) * 2]) % 10);
            wsprintfA(&crc_char[0], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 10000 % 10);
            wsprintfA(&crc_char[1], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 1000 % 10);
            wsprintfA(&crc_char[2], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 100 % 10);
            wsprintfA(&crc_char[3], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 10 % 10);
            wsprintfA(&crc_char[4], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) % 10);

            //for (int i = 0; i < 5; i++)data_char[i] = data_char[i]+;

            SendMessageA(WINDOW_slave, WM_SETTEXT, sizeof slave, (LPARAM)&slave);
            SendMessageA(device_slave, WM_SETTEXT, sizeof device_char, (LPARAM)&device_char);
            SendMessageA(command_slave, WM_SETTEXT, sizeof command_char, (LPARAM)&command_char);
            SendMessageA(registr_slave, WM_SETTEXT, sizeof byte_char, (LPARAM)&byte_char);
            SendMessageA(data_slave, WM_SETTEXT, sizeof data_char, (LPARAM)&data_char);
            //SetTimer(hWnd, 2, 1, NULL);//включить первый таймер на 1 миллисек   Условная задержка
        }
        /*if (wParam == 104)//записать регистры
        {
            int sizebuf;
            SendMessageA(WINDOW_master, WM_GETTEXT, sizeof BUF_OBMENA_CHAR, (LPARAM)&BUF_OBMENA_CHAR);
            for (int i = 0; i < 256; i++)
            {
                if (BUF_OBMENA_CHAR[i] ='/n/r')// ищем конец строки
                {
                    sizebuf = i;
                    i = 256;
                }
            }
            Value = WriteFile(initCOM, BUF_OBMENA_CHAR, sizebuf, 0, NULL);//записываем в порт из буфера
        }*/
        if (wParam == 105)//стереть
        {
            RESET();
        }
        if (wParam == 106)//закрыть порт
        {
            Value = CloseHandle(initCOM);
            SendMessage(com_port, WM_SETTEXT, 0, (LPARAM)_T("COM-порт:"));
            SendMessage(baud_port, WM_SETTEXT, 0, (LPARAM)_T("baud:"));
            KillTimer(hWnd, 1);//отключить таймер
            KillTimer(hWnd, 2);//отключить таймер
            ShowWindow(LISTEN, 0);
            ShowWindow(READ, 0);
            ShowWindow(WRITE, 0);
            ShowWindow(CLEAR, 0);
            ShowWindow(CLOSE_PORT, 0);
            ShowWindow(com_port, 1);
            ShowWindow(baud_port, 1);
            ShowWindow(OPEN, 1);
        }
    }
    break;
    case WM_USER + 100:
    {
       SetCommTimeouts(initCOM, &timeout);
       Value = ReadFile(initCOM, BUF_OBMENA_CHAR, 512, 0, NULL);// считываем порт в буфер

        //if (BUF_OBMENA_CHAR[0] == BUF_OBMENA_CHAR[8] && BUF_OBMENA_CHAR[1] == BUF_OBMENA_CHAR[9]&&
           // BUF_OBMENA_CHAR[0] == 16 && BUF_OBMENA_CHAR[1] == 0x03)
        //{
            //if (BUF_OBMENA_CHAR[1] == 0x03)command_03=true;
            //if (BUF_OBMENA_CHAR[1] == 0x04)command_04=true;
            //if (BUF_OBMENA_CHAR[1] == 0x06)command_06=true;
            //if (BUF_OBMENA_CHAR[1] == 0x10)command_10 = true;
        //}
       
        //command_03 = true;
        //if (command_03 == true)//чтение аналогового вывода
        if(BUF_OBMENA_CHAR[0] == 0x10 && BUF_OBMENA_CHAR[1] == 0x03 && BUF_OBMENA_CHAR[2] == 0x02 && BUF_OBMENA_CHAR[3] == 0x00)
        //
        {
            for (int i = 0; i < 8; i++)master[i] = BUF_OBMENA_CHAR[i];
            for (int i = 0; i < 3 + ((BUF_OBMENA_CHAR[4] * 256 + BUF_OBMENA_CHAR[5]) * 2) + 2; i++)slave[i] = BUF_OBMENA_CHAR[8 + i];   
            //for (int i = 0; i < 256; i++)BUF_OBMENA_CHAR[i]=0x00; 
            //PurgeComm(initCOM, PURGE_RXCLEAR);// очистить com порт
            //command_03 = false;

           
            SendMessageA(test, WM_GETTEXT, sizeof registr_int, (LPARAM)&registr_int);
            sizebuf = BUF_OBMENA_CHAR[2] * 256 + BUF_OBMENA_CHAR[3]+registr_int-48;// номер запрошеннщгщ регистра
            wsprintfA(&test_char[0], "%lu", sizebuf / 10000 % 10);//преобразуем число в символ
            wsprintfA(&test_char[1], "%lu", sizebuf / 1000 % 10);//преобразуем число в символ
            wsprintfA(&test_char[2], "%lu", sizebuf / 100 % 10);//преобразуем число в символ
            wsprintfA(&test_char[3], "%lu", sizebuf / 10 % 10);//преобразуем число в символ
            wsprintfA(&test_char[4], "%lu", sizebuf % 10);//преобразуем число в символ
            SendMessageA(test2, WM_SETTEXT, sizeof test_char, (LPARAM)&test_char);
            //for (int i = 0; i < 5; i++)test_char[i] = 0;
            //SendMessageA(test, WM_GETTEXT, sizeof registr_int, (LPARAM)&registr_int);
            //number_register = ((test_char[0]-48)*10)+ (test_char[1]-48);
            //SendMessageA(test2, WM_SETTEXT, sizeof number_register, (LPARAM)&number_register);

            /////////////////////////////master///////////////////////////////////////////

            for (int i = 0; i < 507; i = i + 5)
            {
                wsprintfA(&BUF_OBMENA_DEC_MASTER[i], "%lu", master[i / 5] / 100 % 10);//преобразуем число в символ
                wsprintfA(&BUF_OBMENA_DEC_MASTER[i + 1], "%lu", master[i / 5] / 10 % 10);//преобразуем число в символ
                wsprintfA(&BUF_OBMENA_DEC_MASTER[i + 2], "%lu", master[i / 5] % 10);//преобразуем число в символ
                BUF_OBMENA_DEC_MASTER[i + 3] = 32;
                BUF_OBMENA_DEC_MASTER[i + 4] = 32;
                
                BUF_OBMENA_HEX_MASTER[i] = '0';
                BUF_OBMENA_HEX_MASTER[i + 1] = 'x';
                if (master[i / 5] / 16 > 9)BUF_OBMENA_HEX_MASTER[i + 2] = master[i / 5] / 16 + 55;
                else BUF_OBMENA_HEX_MASTER[i + 2] = master[i / 5] / 16 + 48;
                if (master[i / 5] % 16 > 9)BUF_OBMENA_HEX_MASTER[i + 3] = master[i / 5] % 16 + 55;
                else BUF_OBMENA_HEX_MASTER[i + 3] = master[i / 5] % 16 + 48;
                BUF_OBMENA_HEX_MASTER[i + 4] = ' ';
            }
            SendMessageA(dec_master, WM_SETTEXT, sizeof BUF_OBMENA_DEC_MASTER, (LPARAM)&BUF_OBMENA_DEC_MASTER);
            SendMessageA(hex_master, WM_SETTEXT, sizeof BUF_OBMENA_HEX_MASTER, (LPARAM)&BUF_OBMENA_HEX_MASTER);
           
            wsprintfA(&device_char[0], "%lu", master[0] / 100 % 10);//преобразуем число в символ
            wsprintfA(&device_char[1], "%lu", master[0] / 10 % 10);//преобразуем число в символ
            wsprintfA(&device_char[2], "%lu", master[0] % 10);//преобразуем число в символ
            wsprintfA(&command_char[0], "%lu", master[1] / 100 % 10);//преобразуем число в символ
            wsprintfA(&command_char[1], "%lu", master[1] / 10 % 10);//преобразуем число в символ
            wsprintfA(&command_char[2], "%lu", master[1] % 10);//преобразуем число в символ
            wsprintfA(&registr_char[0], "%lu", (master[2] * 256 + master[3]) / 10000 % 10);//преобразуем число в символ
            wsprintfA(&registr_char[1], "%lu", (master[2] * 256 + master[3]) / 1000 % 10);//преобразуем число в символ
            wsprintfA(&registr_char[2], "%lu", (master[2] * 256 + master[3]) / 100 % 10);//преобразуем число в символ
            wsprintfA(&registr_char[3], "%lu", (master[2] * 256 + master[3]) / 10 % 10);//преобразуем число в символ
            wsprintfA(&registr_char[4], "%lu", (master[2] * 256 + master[3]) % 10);//преобразуем число в символ
            wsprintfA(&data_char[0], "%lu", (master[4] * 256 + master[5]) / 10000 % 10);//преобразуем число в символ
            wsprintfA(&data_char[1], "%lu", (master[4] * 256 + master[5]) / 1000 % 10);//преобразуем число в символ
            wsprintfA(&data_char[2], "%lu", (master[4] * 256 + master[5]) / 100 % 10);//преобразуем число в символ
            wsprintfA(&data_char[3], "%lu", (master[4] * 256 + master[5]) / 10 % 10);//преобразуем число в символ
            wsprintfA(&data_char[4], "%lu", (master[4] * 256 + master[5]) % 10);//преобразуем число в символ
            wsprintfA(&crc_char[0], "%lu", (master[6] * 256 + master[7]) / 10000 % 10);//преобразуем число в символ
            wsprintfA(&crc_char[1], "%lu", (master[6] * 256 + master[7]) / 1000 % 10);//преобразуем число в символ
            wsprintfA(&crc_char[2], "%lu", (master[6] * 256 + master[7]) / 100 % 10);//преобразуем число в символ
            wsprintfA(&crc_char[3], "%lu", (master[6] * 256 + master[7]) / 10 % 10);//преобразуем число в символ
            wsprintfA(&crc_char[4], "%lu", (master[6] * 256 + master[7]) % 10);//преобразуем число в символ
            
            SendMessageA(WINDOW_master, WM_SETTEXT, sizeof master, (LPARAM)&master);
            SendMessageA(device_master, WM_SETTEXT, sizeof device_char, (LPARAM)&device_char);
            SendMessageA(command_master, WM_SETTEXT, sizeof command_char, (LPARAM)&command_char);
            SendMessageA(registr_master, WM_SETTEXT, sizeof registr_char, (LPARAM)&registr_char);
            SendMessageA(data_master, WM_SETTEXT, sizeof data_char, (LPARAM)&data_char);
            SendMessageA(crc_master, WM_SETTEXT, sizeof crc_char, (LPARAM)&crc_char);

            ///////////////////////////////slave///////////////////////////////////////

            for (int i = 0; i < 507; i = i + 5)
            {
                wsprintfA(&BUF_OBMENA_DEC[i], "%lu", slave[i / 5] / 100 % 10);//преобразуем число в символ
                wsprintfA(&BUF_OBMENA_DEC[i + 1], "%lu", slave[i / 5] / 10 % 10);//преобразуем число в символ
                wsprintfA(&BUF_OBMENA_DEC[i + 2], "%lu", slave[i / 5] % 10);//преобразуем число в символ
                BUF_OBMENA_DEC[i + 3] = 32;
                BUF_OBMENA_DEC[i + 4] = 32;
               
                BUF_OBMENA_HEX[i] = '0';
                BUF_OBMENA_HEX[i + 1] = 'x';
                if (slave[i / 5] / 16 > 9)BUF_OBMENA_HEX[i + 2] = slave[i / 5] / 16 + 55;
                else BUF_OBMENA_HEX[i + 2] = slave[i / 5] / 16 + 48;
                if (slave[i / 5] % 16 > 9)BUF_OBMENA_HEX[i + 3] = slave[i / 5] % 16 + 55;
                else BUF_OBMENA_HEX[i + 3] = slave[i / 5] % 16 + 48;
                BUF_OBMENA_HEX[i + 4] = ' ';
            }
            SendMessageA(dec_slave, WM_SETTEXT, sizeof BUF_OBMENA_DEC, (LPARAM)&BUF_OBMENA_DEC);
            SendMessageA(hex_slave, WM_SETTEXT, sizeof BUF_OBMENA_HEX, (LPARAM)&BUF_OBMENA_HEX);
            SendMessageA(edit, WM_SETTEXT, 10, (LPARAM)BUF_OBMENA_HEX);
            SendMessageA(edit, WM_SETTEXT, 0, (LPARAM)'\r\n');
                        
            wsprintfA(&device_char[0], "%lu", slave[0] / 100 % 10);//преобразуем число в символ
            wsprintfA(&device_char[1], "%lu", slave[0] / 10 % 10);
            wsprintfA(&device_char[2], "%lu", slave[0] % 10);
            wsprintfA(&command_char[0], "%lu", slave[1] / 100 % 10);
            wsprintfA(&command_char[1], "%lu", slave[1] / 10 % 10);
            wsprintfA(&command_char[2], "%lu", slave[1] % 10);
            wsprintfA(&byte_char[0], "%lu", slave[2] / 100 % 10);
            wsprintfA(&byte_char[1], "%lu", slave[2] / 10 % 10);
            wsprintfA(&byte_char[2], "%lu", slave[2] % 10);
            wsprintfA(&data_char[0], "%lu", (slave[3 + (registr_int - 48)*2] * 256 + slave[4 + (registr_int - 48) * 2]) / 10000 % 10);
            wsprintfA(&data_char[1], "%lu", (slave[3 + (registr_int - 48)*2] * 256 + slave[4 + (registr_int - 48)*2]) / 1000 % 10);
            wsprintfA(&data_char[2], "%lu", (slave[3 + (registr_int - 48)*2] * 256 + slave[4 + (registr_int - 48)*2]) / 100 % 10);
            wsprintfA(&data_char[3], "%lu", (slave[3 + (registr_int - 48)*2] * 256 + slave[4 + (registr_int - 48)*2]) / 10 % 10);
            wsprintfA(&data_char[4], "%lu", (slave[3 + (registr_int - 48)*2] * 256 + slave[4 + (registr_int - 48)*2]) % 10);
            wsprintfA(&crc_char[0], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 10000 % 10);
            wsprintfA(&crc_char[1], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 1000 % 10);
            wsprintfA(&crc_char[2], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 100 % 10);
            wsprintfA(&crc_char[3], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) / 10 % 10);
            wsprintfA(&crc_char[4], "%lu", (slave[3 + slave[2]] * 256 + slave[4 + slave[2]]) % 10);
           
            //for (int i = 0; i < 5; i++)data_char[i] = data_char[i]+;

            SendMessageA(WINDOW_slave, WM_SETTEXT, sizeof slave, (LPARAM)&slave);
            SendMessageA(device_slave, WM_SETTEXT, sizeof device_char, (LPARAM)&device_char);
            SendMessageA(command_slave, WM_SETTEXT, sizeof command_char, (LPARAM)&command_char);
            SendMessageA(registr_slave, WM_SETTEXT, sizeof byte_char, (LPARAM)&byte_char);
            SendMessageA(data_slave, WM_SETTEXT, sizeof data_char, (LPARAM)&data_char);
            SendMessageA(crc_slave, WM_SETTEXT, sizeof crc_char, (LPARAM)&crc_char);
        }
        SetTimer(hWnd, 1, 1, NULL);//включить первый таймер на 1 миллисек   Условная задержка
        
    }
    break;
    case WM_TIMER:// по окочанию таймера выполнить... 
        if (wParam == 1)
        {
            KillTimer(hWnd, 1);//отключить таймер
            SendMessage(hWnd, WM_USER + 100, 0, (LPARAM)lParam); // отправить сообщение в очередь о повторном запуске цикла
        }
        if (wParam == 2)
        {
            KillTimer(hWnd, 2);//отключить таймер
            SendMessage(hWnd, WM_COMMAND, 103, (LPARAM)lParam); // отправить сообщение в очередь о повторном запуске цикла
        }
    break;
    case WM_CTLCOLOREDIT:// изменение цветов в статических окнах
        if (port == true && edit != (HWND)lParam)
        {
            SetTextColor((HDC)wParam, RGB(0, 0, 0));
            SetBkColor((HDC)wParam, RGB(0, 255, 0));// свой цвет фона
        }
        else
        {
            if (edit != (HWND)lParam)
            {
                SetTextColor((HDC)wParam, RGB(0, 0, 0));
                SetBkColor((HDC)wParam, RGB(255, 0, 0));// свой цвет фона
            }          
        }
        //return (LRESULT)RGB(0, 255, 0);// возвращаем цвет
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        SetTextColor(hdc, RGB(150, 0, 150));// цвет текста
        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, hFont1);
        Rectangle(hdc, 150, 40, 720, 220);
        Rectangle(hdc, 150, 240, 720, 420);
        TextOut(hdc, 250, 0, _T("Данные по протоколу ModBus (8N1)"), 32);// текст по координатам(дискриптор окна,x,y,текст,размер)
        
        SetTextColor(hdc, RGB(0, 0, 0));// цвет текста
        SelectObject(hdc, hFont3);
        TextOut(hdc, 160, 50, _T("ЗАПРОС"), 6);
        TextOut(hdc, 160, 250, _T("ОТВЕТ"), 5);
        TextOut(hdc, 40, 480, _T("тест"), 4);
        SetTextColor(hdc, RGB(0, 0, 150));// цвет текста
        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, hFont2);
        TextOut(hdc, 205, 85, _T("адрес устройства"), 16);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 325, 85, _T("команда"), 7);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 420, 85, _T("1-й регистр"), 11);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 505, 85, _T("кол-во регистров"), 16);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 640, 85, _T("crc"), 3);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 160, 140, _T("ASCII"), 6);
        TextOut(hdc, 160, 160, _T("DEC"), 3);
        TextOut(hdc, 160, 180, _T("HEX"), 3);
        TextOut(hdc, 205, 285, _T("устройство"), 10);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 325, 285, _T("команда"), 7);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 420, 285, _T("кол-во байт"), 11);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 525, 285, _T("значениe"), 8);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 640, 285, _T("crc"), 3);// текст по координатам(дискриптор окна,x,y,текст,размер)
        TextOut(hdc, 160, 340, _T("ASCII"), 6);
        TextOut(hdc, 160, 360, _T("DEC"), 3);
        TextOut(hdc, 160, 380, _T("HEX"), 3);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        Value = CloseHandle(initCOM);
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