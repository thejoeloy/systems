#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void readFile(string fileName);
void writeFile(string fileName);

int main(int argc, char* argv[]) {
	readFile("hamlet.txt");
	writeFile("BEASTMODE.txt");
	return 0;
}

void readFile(string fileName) {
	ifstream f(fileName);
	if (f.is_open()) {
		string line;
		while (getline(f, line)) {
			cout << line << "\n";
		}
		f.close();	
	} else {
		cout << "Unable to open file" << endl;
	}
}

void writeFile(string fileName) {
	ofstream out(fileName);
	if (out.is_open()) {
		out << "Here is some text" << endl;
		out << "Onfonem" << endl;
		out.close();
		cout << fileName << " was created and written to" << endl;
	} else {
		cout << "Unable to create" << fileName << endl;
	}
}
 
