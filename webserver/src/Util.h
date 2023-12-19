#pragma once
#include <string>

//从文件描述符fd读取n个字节到buff缓冲区
ssize_t readn(int fd, void* buff, size_t n);
//从fd读取，并判断是否是零字节
ssize_t readn(int fd, std::string &in_buffer, bool &zero);
ssize_t readn(int fd, std::string &in_buffer);
ssize_t writen(int fd, void* buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
//处理SigPipe信号
void handleForSigPipe();
//将fd设置为非阻塞
int setSocketNonBlocking(int fd);
//禁用Nagle算法，允许小数据包立即发送
void setSocketNodelay(int fd);
//设置关闭前等待的时间
void setSocketNoLinger(int fd);
//关闭fd的写入端
void shutDownWR(int fd);
//创建一个套接字，并绑定到port端口
int socketBindListen(int port);
