#include"Util.h"
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<string>
using namespace std;

int main() {
	int fd = open("test.txt", O_RDWR);
	string buf = "hello world";
	int n = writen(fd, buf);
	close(fd);
	cout << "写入了" << n << "个字符" << endl;
	return 0;
}
