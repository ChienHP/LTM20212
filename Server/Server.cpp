// WSAEventSelectServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <process.h>
#include <fstream>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
CRITICAL_SECTION criticalSection;
typedef struct session {
	SOCKET sock;
	sockaddr_in clientAddr;
	string account;
	int status;
} session;

typedef struct room {
	string status; // 1: Chua bat dau, 2: Dang dien ra, 3: Da ket thuc
	string time;
	string numberOfQuestion;
	string idOfExam;
	session * admin;
	vector<pair<string, int>> resultOfExam;
} room;
// thông tin của 1 câu hỏi
typedef struct questionInfo {
	string question;  // câu hỏi và 3 lựa chọn trắc nghiệm
	char result; // kết quả
} questionInfo;

typedef struct exam {
	int numberOfQuestion; // số lượng câu hỏi của đề thi
	vector<questionInfo> questions; // số thứ tự của câu hỏi trong file lưu trữ
} exam;

#define BUFF_SIZE 2048
#define SERVER_ADDR "127.0.0.1"
#define MAX_NUM_THREAD 1000
session clientSession[1000];
vector<pair<string, string> > userAccount;
vector<pair<string, string> >::iterator iterAccount; 
vector<room> rooms; // danh sách phòng thi
vector<questionInfo> questions; // mảng lưu trữ question từ file
vector<exam> exams;
int isCreated[MAX_NUM_THREAD] = { 0 };

void sendMessage(SOCKET, char *);

void registerAccount(string, session *);

// handle when user logs in
void login(string ,session *);

// handle when user posts
void practice(string ,session *);

// handle when user logs out
void logout(session *);

// defines how the server should process when a user makes a message
void handle(char *, session *);

// chon de
void choose(string, session *);

void createRoom(string data, session *);

void getListRoom(session *);

void submit(string data, session *);

void join(string data, session *);

// read file into userAccount
int readFileAccount();

int readFileQuestion();

/* procThread - Thread to receive the message from client and process*/
unsigned __stdcall procThread(void *);


int main(int argc, char* argv[])
{
	DWORD		nEvents = 0;
	DWORD		index;
	SOCKET		listenSock;
	WSAEVENT	eventListen[1];
	WSANETWORKEVENTS sockEvent;
	int a;
	readFileAccount();
	readFileQuestion();
	cin >> a;
	return 0;
	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}
	//Step 2: Construct LISTEN socket	
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	eventListen[0] = WSACreateEvent(); //create new events
	nEvents++;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(listenSock, eventListen[0], FD_ACCEPT | FD_CLOSE);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	//InitializeCriticalSection(&criticalSection);
	while (1) {
		//wait for network events on all socket
		index = WSAWaitForMultipleEvents(1, eventListen, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			break;
		}

		WSAEnumNetworkEvents(listenSock, eventListen[0], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
				printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
				break;
			}
			//Add new socket into socks array
			int i;
			for (i = 0; i < 1000; i++)
				if (clientSession[i].sock == 0) {
					session *tempSession = new session;
					// set session parameters
					tempSession->clientAddr = clientAddr;
					tempSession->sock = connSock;
					clientSession[i].sock = connSock;
					// add to session array
					clientSession[i] = *tempSession;
					break;
				}
			// if i is a multiple of 64 and the thread has not been created then create a new thread
			int numThread = i / 64;

			if (i % 64 == 0 && !isCreated[numThread]) {
				_beginthreadex(0, 0, procThread, (void *)&i, 0, 0);
			}

			//reset event
			WSAResetEvent(eventListen);
		}
	}
	//DeleteCriticalSection(&criticalSection);
	closesocket(listenSock);
	WSACleanup();
	return 0;
}

void sendMessage(SOCKET sock, char *sendBuff) {
	int ret = send(sock, sendBuff, strlen(sendBuff), 0);
	if (ret == SOCKET_ERROR) {
		printf("Error %d: Cannot send data.\n", WSAGetLastError());
		return;
	}
}

void registerAccount(string data, session *userSession) {
	int temp = data.find(' ');
	string user = data.substr(0, temp);
	string password = data.substr(temp + 1);
	for (iterAccount = userAccount.begin(); iterAccount != userAccount.end(); iterAccount++) {
		if (iterAccount->first == user) {
			sendMessage(userSession->sock, "20#");
			return;
		}
	}
	pair<string, string> newAccount;
	newAccount.first = user;
	newAccount.second = password;
	userAccount.push_back(newAccount);
	ofstream fileOutput("account.txt", ios::app);
	if (fileOutput.fail()) {
		printf("Failed to open this file!\n");
		return;
	}
	fileOutput << user << " " << password << endl;
	fileOutput.close();
	sendMessage(userSession->sock, "10#");
	return;
}


void login(string data, session *userSession) {
	char *result = "";
	int temp = data.find(' ');
	string user = data.substr(0, temp);
	string password = data.substr(temp + 1);
	// The current session is session in
	//EnterCriticalSection(&criticalSection);
	if (userSession->account != "") {
		result = "24#";
	}
	//LeaveCriticalSection(&criticalSection);
	if (result != "") {
		sendMessage(userSession->sock, result);
		return;
	}
	// check accounts that are already session in elsewhere
	//EnterCriticalSection(&criticalSection);
	for (int i = 0; i < 1000; i++) {
		if (clientSession[i].account == user) {
			result = "22#";
			break;
		}
	}
	//LeaveCriticalSection(&criticalSection);
	if (result != "") {
		sendMessage(userSession->sock, result);
		return;
	}

	// Validate loggin
	for (iterAccount = userAccount.begin(); iterAccount != userAccount.end(); iterAccount++) {
		if (iterAccount->first == user) {
			// active account
			if (iterAccount->second == password) {// dung password
				//EnterCriticalSection(&criticalSection);
				userSession->account = iterAccount->first;
				userSession->status = 1;
				//LeaveCriticalSection(&criticalSection);
				sendMessage(userSession->sock, "11#");
				return;
			}
			// sai pass
			if (iterAccount->second != password) { 
				sendMessage(userSession->sock, "21#");
				return;
			}
		}
	}
	sendMessage(userSession->sock, "23#");
	return;
}

// handle when user post
void practice(session *userSession) {
	return; // Tra ve danh sach
}

void createRoom(string data, session *userSession) {
	char *result = "";
	int temp = data.find(' ');
	string time = data.substr(0, temp);
	string numberOfQuestion = data.substr(temp + 1);
	// Thiet lap thong tin cua phong thi va them vao mang PhongThi
	room *newRoom = new room;
	newRoom->numberOfQuestion = stoi(numberOfQuestion);
	newRoom->status = 1;
	newRoom->time = time;
	newRoom->admin = userSession;
	rooms.push_back(*newRoom);
	sendMessage(userSession->sock, "#15");
	return;
}

void getListRoom(session *userSession){
	string result = "13 "; 
	for (int i = 0; i < rooms.size(); i++) {
		result = result + to_string(i) + " " + rooms[i].status + "/";
	}
	int sentByte = 0, length = result.length();
	char sendBuff[2048];

	while (sentByte + 2048 < length) {
		strncpy_s(sendBuff, result.c_str(), 2048);
		sentByte += 2048;
		sendMessage(userSession->sock, sendBuff);
	}
	strncpy_s(sendBuff, result.c_str(), length - sentByte);
	strcat_s(sendBuff, "#");
	sendMessage(userSession->sock, sendBuff);
}

void submit(string data, session * userSession) {
	int temp = data.find(' ');
	int idOfRoom = stoi(data.substr(0, temp));
	string result = data.substr(temp + 1);
	int idOfExam = stoi(rooms[idOfRoom].idOfExam);
	int indexOfResult = 0;
	int correct = 0;
	while (result[indexOfResult]) {
		if (result[indexOfResult] == exams[idOfExam].questions[indexOfResult].result) {
			correct++;
		}
	}
	int numberOfQuestion = stoi(rooms[idOfRoom].numberOfQuestion);
	float point = correct / numberOfQuestion * 10;
	rooms[idOfRoom].resultOfExam.push_back(make_pair(userSession->account, point));
	// Gửi điểm về client và lưu vào phòng thi
}

void join(string data, session *userSession) {
	int idOfRoom = stoi(data);
	rooms[idOfRoom].resultOfExam.push_back(make_pair(userSession->account, -1));
	sendMessage(userSession->sock, "14#");
}
// handle when user logs out
void logout(session *userSession) {
	char *result = "";
	// The current session is not session in
	//EnterCriticalSection(&criticalSection);
	if (userSession->account == "") {
		result = "21#";
	}
	else {
		// Log out of the current session
		userSession->account = "";
		userSession->status = 0;
		result = "12#";
	}
	//LeaveCriticalSection(&criticalSection);
	if (result != "") {
		sendMessage(userSession->sock, result);
		return;
	}

	sendMessage(userSession->sock, "99#");
	return;
}

// defines how the server should process when a user makes a message
void handle(char* sBuff, session *userSession) {
	int temp = string(sBuff).find(' ');
	string requestMessageType = string(sBuff).substr(0, temp);
	string data = string(sBuff).substr(temp + 1);
	if (requestMessageType == "LOGIN") {
		login(data, userSession);
	} else
	if (requestMessageType == "REGISTER") {
		registerAccount(data, userSession);
	} else
	if (requestMessageType == "PRACTICE") {
		practice(userSession);
	} else
	if (requestMessageType == "CREATEROOM") {
		createRoom(data, userSession);
	} else
	if (requestMessageType == "LISTROOM") {
		getListRoom(userSession);
	} else
	if (requestMessageType == "JOIN") {
		join(data, userSession);
	} else
	if (requestMessageType == "RESULT") {

	} else
	if (requestMessageType == "SUBMIT") {
		submit(data, userSession);
	}
	

	sendMessage(userSession->sock, "99#");
	return;
}

// read file into userAccount
int readFileAccount() {
	string account;
	string statusAccount;
	ifstream fileInput("account.txt");
	if (fileInput.fail()) {
		printf("Failed to open this file!\n");
		return 0;
	}
	while (!fileInput.eof())
	{
		char temp[255];
		fileInput.getline(temp, 255);
		string line = temp;
		account = line.substr(0, line.find(" "));
		statusAccount = line.substr(line.find(" ") + 1, line.length());
		userAccount.push_back(make_pair(account, statusAccount));
	}

	fileInput.close();
	return 1;
}

int readFileQuestion() {
	ifstream fileInput("Question.txt");
	if (fileInput.fail()) {
		printf("Failed to open this file!\n");
		return 0;
	}
	questionInfo * tempQuestion = new questionInfo();
	int i = 0;
	while (!fileInput.eof())
	{
		char temp[2048];
		fileInput.getline(temp, 2048);
		if (i == 5) {
			i = 0;
			continue;
		} else
		if (i == 4) {
			char result = temp[0];
			tempQuestion->result = (char)result;
			questions.push_back(*tempQuestion);
			tempQuestion = new questionInfo();
			i++;
		} else
		if (i == 3) {
			string line = temp;
			tempQuestion->question += line;
			i++;
			continue;
		}
		else {
			string line = temp;
			tempQuestion->question += line;
			tempQuestion->question += "/";
			i++;
		}
	}

	fileInput.close();
	for (int i = 0; i < questions.size(); i++) {
		cout << questions[i].question << endl;
		cout << "Result: " <<questions[i].result << endl;
	}
}

unsigned __stdcall procThread(void *param) {
	DWORD		nEvents = 0;
	DWORD		index;
	SOCKET		socks[WSA_MAXIMUM_WAIT_EVENTS] = { 0 };
	WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS sockEvent;

	char *sendBuff, message[BUFF_SIZE], rcvBuff[BUFF_SIZE];
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int ret, i, indexMess = 0;
	int indexClient = *(int *)param;
	int numThread = indexClient / 64;

	while (1) {
		// //Add client socket into socks array
		for (i = indexClient; i < indexClient + 64; i++) {
			int indexSock = i % 64;
			if (clientSession[i].sock != 0 && socks[indexSock] == 0) {
				socks[indexSock] = clientSession[i].sock;
				events[indexSock] = WSACreateEvent();
				WSAEventSelect(socks[indexSock], events[indexSock], FD_READ | FD_CLOSE);
				nEvents++;
			}
		}

		if (socks[0] == 0) {
			continue;
		}

		//wait for network events
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, 500, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			break;
		}

		if (index == WSA_WAIT_TIMEOUT) {
			continue;
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(socks[index], events[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			ret = recv(socks[index], rcvBuff, BUFF_SIZE, 0);
			if (ret <= 0) {
				closesocket(socks[index]);
				socks[index] = 0;
				WSACloseEvent(events[index]);
				nEvents--;
				printf("Error %d: Cannot receive data.", WSAGetLastError());
				break;
			}

			//Release socket and event if an error occurs
			else {
				rcvBuff[ret] = 0;
				int indexBuff = 0;
				while (rcvBuff[indexBuff] != '\0') {
					// ENDING_DELIMITER, handle the message
					if (rcvBuff[indexBuff] == '#') {
						message[indexMess] = 0;
						// handle and send message to client
						handle(message, &clientSession[index]);
						indexMess = 0;
						indexBuff++;
						continue;
					}
					// not ENDING_DELIMITER, copy from rBuff to message
					message[indexMess] = rcvBuff[indexBuff];
					indexMess++;
					indexBuff++;
				}

				//reset event
				WSAResetEvent(events[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			int indexClientClose = numThread * 64 + index; // close client position in clientSession array
			int indexLastClient = numThread * 64 + nEvents - 1; // last client of thread in clientSession array
			int i = 0;
			// is the last element, then remove from the array
			if (index == nEvents - 1) {
				// close socket
				closesocket(socks[index]);
				WSACloseEvent(events[index]);
				socks[index] = 0;
				// reset session of client
				//EnterCriticalSection(&criticalSection);
				clientSession[indexClientClose].account = "";
				clientSession[indexClientClose].sock = 0;
				clientSession[indexClientClose].clientAddr = {};
				//LeaveCriticalSection(&criticalSection);
				nEvents--;
			}
			// is the middle element then remove from the array and swap the last element
			else {
				// close event and swap
				WSACloseEvent(events[index]);
				events[index] = events[nEvents - 1];
				// close socket and swap
				closesocket(socks[index]);
				socks[index] = socks[nEvents - 1];
				socks[nEvents - 1] = 0;
				// swap the last session and reset session
				//EnterCriticalSection(&criticalSection);
				clientSession[indexClientClose] = clientSession[indexLastClient];
				clientSession[indexLastClient].account = "";
				clientSession[indexLastClient].sock = 0;
				clientSession[indexLastClient].clientAddr = {};
				//LeaveCriticalSection(&criticalSection);

				nEvents--;
			}
		}
	}
	return 0;
}
