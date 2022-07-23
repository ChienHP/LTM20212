#include <iostream>
#include <string>
using namespace std;

typedef struct Room
{
	string id;
	string status;
} Room;
Room rooms[100];

int main() {
	string data = "12 1/23 2/34 3/45 1/";
	int flag = 0;
	string statusTemp = "", idTemp = "";
	int indexRoom = 0, i = 0;
	while (data[i]) {
		if (data[i] == '/') {
			flag = 0;
			rooms[indexRoom].status = statusTemp;
			statusTemp = "";
			indexRoom++;
			i++;
			continue;
		}
		else
			if (data[i] == ' ') {
				flag = 1;
				rooms[indexRoom].id = idTemp;
				idTemp = "";
				i++;
				continue;
			}
		if (flag == 1) {
			statusTemp += data[i];

		}
		else if (flag == 0) {
			idTemp += data[i];
		}
		i++;
	}

	for (int indexRoom = 0; indexRoom<4; indexRoom++) {
		cout << rooms[indexRoom].id << " " << rooms[indexRoom].status << endl;
	}
	return 0;
}