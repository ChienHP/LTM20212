// Win32Project2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Project2.h"
#include "windows.h"
#include "string"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "string.h"

using namespace std;
#define BACK 100
#define ANSWER_A 'A'
#define ANSWER_B 'B'
#define ANSWER_C 'C'
#define VIEW_LOGIN 1
#define VIEW_REGISTER 2
#define VIEW_LIST_ROOM 3
#define LOGIN 4
#define REGISTER 5
#define LOGOUT 6
#define VIEW_QUESTION 7
#define SUBMIT 8



#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define ENDING_DELEMETER "#"
#pragma comment(lib, "Ws2_32.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK View1Proc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LoginProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK RegisterProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK View2Proc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ListRoomProc(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK QuestionProc(HWND, UINT, WPARAM, LPARAM);

void CreateView1(HWND);
void CreateViewLogin(HWND);
void CreateViewRegister(HWND);
void CreateView2(HWND);
void CreateViewListRoom(HWND);

void CreateViewQuestion(HWND);

HWND hWnd;
HWND hWndNow;
HWND hUsername;
HWND hPassword;
HWND hConfirmPass;

HWND hQuestion;
HWND hAnswerA;
HWND hAnswerB;
HWND hAnswerC;
HWND hSubmit;
char questions[BUFF_SIZE];
wchar_t questionsW[BUFF_SIZE];
char result[BUFF_SIZE];
int indexQuestions = 0;
int flag = 0;


int Send(SOCKET s, char *sBuff, int size, int flags);
int Receive(SOCKET s, char *rBuff, int size, int flags);

int login(char *username, char *password);
int Register(char *username, char *password, char *confirmPass);

SOCKET client;

int isLogin = true;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	strcat(questions, "Cau1");
	strcat(questions, "/");
	strcat(questions, "dapanA");
	strcat(questions, "/");
	strcat(questions, "dapanB");
	strcat(questions, "/");
	strcat(questions, "dapanC");
	strcat(questions, "/");
	strcat(questions, "Cau2");
	strcat(questions, "/");
	strcat(questions, "dapanA");
	strcat(questions, "/");
	strcat(questions, "dapanB");
	strcat(questions, "/");
	strcat(questions, "dapanC");
	strcat(questions, "/");


	//Registering the Window Class
	WNDCLASSW wc = { 0 };
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpszClassName = L"myWindowClass";
	wc.lpfnWndProc = WndProc;
	if (!RegisterClassW(&wc)) {
		return -1;
	}
	//Create the window
	hWnd = CreateWindowW(L"myWindowClass", L"My Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 800, 400, NULL, NULL, NULL, NULL);

	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		MessageBox(hWnd, L"Winsock 2.2 is not supported.", L"Error!", MB_OK);
		return 0;
	}

	//Construct socket
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		MessageBox(hWnd, L"Error: Cannot construct client socket.", L"Error!", MB_OK);
		return 0;
	}

	//Specify server socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	//Step 4: Request to connect server
	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		MessageBox(hWnd, L"Error: Cannot connect server.", L"Error!", MB_OK);
		return 0;
	}
	MessageBox(hWnd, L"Connected server!", L"Error!", MB_OK);

	// Main message loop:
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		}
		break;
	case WM_CREATE:
		if (isLogin) {
			CreateView2(hwnd);
		}
		else {
			CreateView1(hwnd);
		}
		break;
	case WM_DESTROY:
		//Close socket
		closesocket(client);
		// Terminate winsock
		WSACleanup();

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

void CreateView1(HWND hwnd) {
	WNDCLASSW swc = { 0 };
	swc.lpszClassName = L"mySubWindow";
	swc.lpfnWndProc = View1Proc;
	RegisterClassW(&swc);
	hWndNow = CreateWindowW(L"mySubWindow", L"", WS_VISIBLE | WS_CHILD, 0, 0, 800, 800, hwnd, NULL, NULL, NULL);
	CreateWindowW(L"button", L"Login", WS_VISIBLE | WS_CHILD, 0, 0, 100, 50, hWndNow, (HMENU)VIEW_LOGIN, NULL, NULL);
	CreateWindowW(L"button", L"Register", WS_VISIBLE | WS_CHILD, 0, 50, 100, 50, hWndNow, (HMENU)VIEW_REGISTER, NULL, NULL);
}

LRESULT CALLBACK View1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case VIEW_LOGIN:
			DestroyWindow(hwnd);
			CreateViewLogin(hWnd);
			break;
		case VIEW_REGISTER:
			DestroyWindow(hwnd);
			CreateViewRegister(hWnd);
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

void CreateViewLogin(HWND hwnd) {
	WNDCLASSW swc = { 0 };
	swc.lpszClassName = L"ViewLogin";
	swc.lpfnWndProc = LoginProc;
	RegisterClassW(&swc);
	hWndNow = CreateWindowW(L"ViewLogin", L"", WS_VISIBLE | WS_CHILD, 0, 0, 800, 800, hwnd, NULL, NULL, NULL);
	hUsername = CreateWindowW(L"edit", L"username", WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 0, 100, 50, hWndNow, NULL, NULL, NULL);
	hPassword = CreateWindowW(L"edit", L"password", WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 50, 100, 50, hWndNow, NULL, NULL, NULL);
	CreateWindowW(L"button", L"Login", WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 100, 50, 50, hWndNow, (HMENU) LOGIN, NULL, NULL);
	CreateWindowW(L"button", L"Back", WS_VISIBLE | WS_CHILD | WS_BORDER, 52, 100, 50, 50, hWndNow, (HMENU)BACK, NULL, NULL);
}

LRESULT CALLBACK LoginProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case LOGIN:
		{
			wchar_t usernameW[100];
			GetWindowTextW(hUsername, usernameW, 100);
			char username[100];
			wcstombs(username, usernameW, 100);
			if (strcmp(username, "") == 0) {
				MessageBox(hWnd, L"Please enter your username.", L"Error!", MB_OK);
				break;
			}
			wchar_t passwordW[100];
			GetWindowTextW(hPassword, passwordW, 100);
			char password[100];
			wcstombs(password, passwordW, 100);
			if (strcmp(password, "") == 0) {
				MessageBox(hWnd, L"Please enter your password.", L"Error!", MB_OK);
				break;
			}

			if (login(username, password) == 1) {
				DestroyWindow(hWndNow);
				CreateView2(hWnd);
			}
			

			/*char rBuff[BUFF_SIZE];
			int ret = Receive(client, rBuff, BUFF_SIZE, 0);*/

			/*wchar_t rBuffW[BUFF_SIZE];
			mbstowcs(rBuffW, rBuff, BUFF_SIZE);
			MessageBox(hWnd, rBuffW, L"Error!", MB_OK);*/
			break;
		}
			
		case BACK:
			DestroyWindow(hwnd);
			CreateView1(hWnd);
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

void CreateViewRegister(HWND hwnd) {
	WNDCLASSW swc = { 0 };
	swc.lpszClassName = L"ViewRegister";
	swc.lpfnWndProc = RegisterProc;
	RegisterClassW(&swc);
	hWndNow = CreateWindowW(L"ViewRegister", L"", WS_VISIBLE | WS_CHILD, 0, 0, 800, 800, hwnd, NULL, NULL, NULL);
	hUsername = CreateWindowW(L"edit", L"username", WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 0, 100, 50, hWndNow, NULL, NULL, NULL);
	hPassword = CreateWindowW(L"edit", L"password", WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 50, 100, 50, hWndNow, NULL, NULL, NULL);
	hConfirmPass = CreateWindowW(L"edit", L"confirmPassword", WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 100, 100, 50, hWndNow, NULL, NULL, NULL);
	CreateWindowW(L"button", L"Register", WS_VISIBLE | WS_CHILD | WS_BORDER, 0, 150, 50, 50, hWndNow, (HMENU) REGISTER, NULL, NULL);
	CreateWindowW(L"button", L"Back", WS_VISIBLE | WS_CHILD | WS_BORDER, 52, 150, 50, 50, hWndNow, (HMENU)BACK, NULL, NULL);
}

LRESULT CALLBACK RegisterProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case REGISTER: {
			wchar_t usernameW[100];
			GetWindowTextW(hUsername, usernameW, 100);
			char username[100];
			wcstombs(username, usernameW, 100);
			if (strcmp(username, "") == 0) {
				MessageBox(hWnd, L"Please enter your username.", L"Error!", MB_OK);
				break;
			}
			wchar_t passwordW[100];
			GetWindowTextW(hPassword, passwordW, 100);
			char password[100];
			wcstombs(password, passwordW, 100);
			if (strcmp(password, "") == 0) {
				MessageBox(hWnd, L"Please enter your password.", L"Error!", MB_OK);
				break;
			}
			wchar_t confirmPassW[100];
			GetWindowTextW(hConfirmPass, confirmPassW, 100);
			char confirmPass[100];
			wcstombs(confirmPass, confirmPassW, 100);
			if (strcmp(confirmPass, "") == 0) {
				MessageBox(hWnd, L"Please confirm your password.", L"Error!", MB_OK);
				break;
			}
			if (strcmp(password, confirmPass) != 0) {
				int result = MessageBoxW(NULL, L"Password and confirm password does not match", L"Error!", MB_OK | MB_ICONERROR);
				SetWindowText(hConfirmPass, L"");
			}
			else
			{
				if (Register(username, password, confirmPass) == 1) {
					DestroyWindow(hWndNow);
					CreateView1(hWnd);
				}
			}
			break;
		}

		case BACK:
			DestroyWindow(hwnd);
			CreateView1(hWnd);
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

void CreateView2(HWND hwnd) {
	WNDCLASSW swc = { 0 };
	swc.lpszClassName = L"View2";
	swc.lpfnWndProc = View2Proc;
	RegisterClassW(&swc);
	hWndNow = CreateWindowW(L"View2", L"", WS_VISIBLE | WS_CHILD, 0, 0, 800, 800, hwnd, NULL, NULL, NULL);
	CreateWindowW(L"button", L"List Room", WS_VISIBLE | WS_CHILD, 0, 0, 100, 50, hWndNow, (HMENU)VIEW_LIST_ROOM, NULL, NULL);
	CreateWindowW(L"button", L"Practice", WS_VISIBLE | WS_CHILD, 0, 50, 100, 50, hWndNow, (HMENU) VIEW_QUESTION, NULL, NULL);
	CreateWindowW(L"button", L"Logout", WS_VISIBLE | WS_CHILD, 0, 100, 100, 50, hWndNow, (HMENU)LOGOUT, NULL, NULL);
}

LRESULT CALLBACK View2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case VIEW_LIST_ROOM:
			DestroyWindow(hwnd);
			CreateViewListRoom(hWnd);
			break;
		case VIEW_QUESTION:
			DestroyWindow(hwnd);
			CreateViewQuestion(hWnd);
			break;
		case LOGOUT:
			DestroyWindow(hwnd);
			CreateView1(hWnd);
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

void CreateViewListRoom(HWND hwnd) {
	WNDCLASSW swc = { 0 };
	swc.lpszClassName = L"ViewListRoom";
	swc.lpfnWndProc = ListRoomProc;
	RegisterClassW(&swc);
	hWndNow = CreateWindowW(L"ViewListRoom", L"", WS_VISIBLE | WS_CHILD, 0, 0, 800, 800, hwnd, NULL, NULL, NULL);

	int x = 0;
	for (int i = 1; i < 5; i++) {
		HWND temp = CreateWindowW(L"static", L"", WS_VISIBLE | WS_CHILD, 0, x, 100, 50, hWndNow, NULL, NULL, NULL);
		SetWindowTextW(temp, L"Phong thi");
		CreateWindowW(L"button", L"Join", WS_VISIBLE | WS_CHILD, 100, x, 100, 50, hWndNow, NULL, NULL, NULL);
		x = x + 50;
	}
	CreateWindowW(L"button", L"Create New Room", WS_VISIBLE | WS_CHILD, 0, x, 150, 50, hWndNow, NULL, NULL, NULL);
	CreateWindowW(L"button", L"Back", WS_VISIBLE | WS_CHILD, 0, x += 50, 100, 50, hWndNow, (HMENU)BACK, NULL, NULL);
}

LRESULT CALLBACK ListRoomProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case BACK:
			DestroyWindow(hwnd);
			CreateView2(hWnd);
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

void CreateViewQuestion(HWND hwnd) {
	WNDCLASSW swc = { 0 };
	swc.lpszClassName = L"ViewQuestion";
	swc.lpfnWndProc = QuestionProc;
	RegisterClassW(&swc);
	hWndNow = CreateWindowW(L"ViewQuestion", L"", WS_VISIBLE | WS_CHILD, 0, 0, 800, 800, hwnd, NULL, NULL, NULL);

	char question[BUFF_SIZE];
	int indexQuestion = 0;
	int flag = 0;
	while (true) {
		if (questions[indexQuestions] == '\0') {
			indexQuestions = 0;
			wchar_t resultW[BUFF_SIZE];
			mbstowcs(resultW, result, BUFF_SIZE);
			MessageBox(hWnd,resultW,L"Result", MB_OK);
			DestroyWindow(hWndNow);
			CreateViewListRoom(hWnd);
			break;
		}

		if (questions[indexQuestions] == '/') {

			question[indexQuestion] = '\0';
			wchar_t questionW[BUFF_SIZE];
			mbstowcs(questionW, question, BUFF_SIZE);

			switch (flag++)
			{
			case 0: {
				hQuestion = CreateWindowW(L"static", questionW, WS_VISIBLE | WS_CHILD, 0, 0, 100, 50, hWndNow, NULL, NULL, NULL);
				break;
			}
			case 1: {
				hAnswerA = CreateWindowW(L"button", questionW, WS_VISIBLE | WS_CHILD, 0, 50, 150, 50, hWndNow, (HMENU) ANSWER_A, NULL, NULL);
				break;
			}
			case 2: {
				hAnswerB = CreateWindowW(L"button", questionW, WS_VISIBLE | WS_CHILD, 0, 100, 150, 50, hWndNow, (HMENU)ANSWER_B, NULL, NULL);
				break;
			}
			case 3: {
				hAnswerC = CreateWindowW(L"button", questionW, WS_VISIBLE | WS_CHILD, 0, 150, 150, 50, hWndNow, (HMENU)ANSWER_C, NULL, NULL);
				break;
			}
			default:
				break;
			}

			indexQuestions++;
			indexQuestion = 0;
			question[0] = '\0';
		}
		else
		{
			question[indexQuestion++] = questions[indexQuestions++];
		}
		if (flag == 4) {
			break;
		}
		hSubmit = CreateWindowW(L"button", L"Submit", WS_VISIBLE | WS_CHILD, 0, 200, 150, 50, hWndNow, (HMENU)SUBMIT, NULL, NULL);

	}
	/*int x = 0;
	for (int i = 1; i < 5; i++) {
		HWND temp = CreateWindowW(L"static", L"", WS_VISIBLE | WS_CHILD, 0, x, 100, 50, hWndNow, NULL, NULL, NULL);
		SetWindowTextW(temp, L"Phong thi");
		CreateWindowW(L"button", L"Join", WS_VISIBLE | WS_CHILD, 100, x, 100, 50, hWndNow, NULL, NULL, NULL);
		x = x + 50;
	}
	CreateWindowW(L"button", L"Create New Room", WS_VISIBLE | WS_CHILD, 0, x, 150, 50, hWndNow, NULL, NULL, NULL);
	CreateWindowW(L"button", L"Back", WS_VISIBLE | WS_CHILD, 0, x += 50, 100, 50, hWndNow, (HMENU)BACK, NULL, NULL);*/
}

LRESULT CALLBACK QuestionProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case ANSWER_A: {
			strcat(result, "A");
			DestroyWindow(hWndNow);
			CreateViewQuestion(hWnd);
			break;
		}
		case ANSWER_B: {
			strcat(result, "B");
			DestroyWindow(hWndNow);
			CreateViewQuestion(hWnd); 
			break;
		}
		case ANSWER_C: {
			strcat(result, "C");
			DestroyWindow(hWndNow);
			CreateViewQuestion(hWnd); 
			break;
		}
		case SUBMIT: {
			
			break;
		}
		case BACK:
			DestroyWindow(hwnd);
			CreateView2(hWnd);
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

/* The send() wrapper function*/
int Send(SOCKET s, char *sBuff, int size, int flags) {
	strcat(sBuff, ENDING_DELEMETER);
	int n = send(s, sBuff, size, flags);
	if (n == SOCKET_ERROR)
		MessageBox(hWnd, L"Error: Cannot send data.", L"Error!", MB_OK);
	return n;
}
/* The recv() wrapper function */
int Receive(SOCKET s, char *rBuff, int size, int flags) {
	int n = recv(s, rBuff, size, flags);
	if (n == SOCKET_ERROR)
		MessageBox(hWnd, L"Error: Cannot receive data.", L"Error!", MB_OK);
	else if (n == 0)
		MessageBox(hWnd, L"Client disconnects.", L"Error!", MB_OK);
	else if (n > 0){
		rBuff[n] = '\0';
	}
	return n;
} 

int login(char *username, char *password) {
	char sBuff[BUFF_SIZE] = "LOGIN";
	strcat(sBuff, " ");
	strcat(sBuff, username);
	strcat(sBuff, " ");
	strcat(sBuff, password);
	int ret = Send(client, sBuff, BUFF_SIZE, 0);

	char rBuff[BUFF_SIZE];
	ret = Receive(client, rBuff, BUFF_SIZE, 0);

	string rBuffStr = string(rBuff);

	if (rBuffStr == "11") {
		MessageBox(NULL, L"Login successfully.", L"Error!", MB_OK);
		return 1;
	}
	else if (rBuffStr == "21") {
		MessageBox(NULL, L"Password does not correct.", L"Error!", MB_OK);
	}
	else if (rBuffStr == "22") {
		MessageBox(NULL, L"This account is already logged in another client.", L"Error!", MB_OK);
	}
	else if (rBuffStr == "23") {
		MessageBox(NULL, L"This username does not exists.", L"Error!", MB_OK);
	}
	return 0;
}

int Register(char *username, char *password, char *confirmPass) {
	char sBuff[BUFF_SIZE] = "REGISTER";
	strcat(sBuff, " ");
	strcat(sBuff, username);
	strcat(sBuff, " ");
	strcat(sBuff, password);
	strcat(sBuff, " ");
	strcat(sBuff, confirmPass);
	int ret = Send(client, sBuff, BUFF_SIZE, 0);

	char rBuff[BUFF_SIZE];
	ret = Receive(client, rBuff, BUFF_SIZE, 0);

	string rBuffStr = string(rBuff);

	if (rBuffStr == "10") {
		MessageBox(NULL, L"Register successfully.", L"Error!", MB_OK);
		return 1;
	}
	else if (rBuffStr == "20") {
		MessageBox(NULL, L"This username already exists.", L"Error!", MB_OK);
	}
	return 0;

}
