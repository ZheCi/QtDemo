#pragma once

#include <string>
#include <cstring>
#include <type_traits>
#include <unistd.h>
#include <iostream>
#include <libssh2.h>
#include <arpa/inet.h>

class SSHClient
{
public:
    SSHClient();
    ~SSHClient();

public:
    // 初始化
    bool init(const std::string &ip, int port, const std::string &username, const std::string &password);
    // 销毁
    void destory();
    // 执行命令
    template<typename FunType, typename = std::enable_if<std::is_invocable_v<FunType, std::string>>>
    std::string executeCommand(const std::string &command, FunType funobje) {
        openChannel();
        sendCommand(command);
        return readResponse_noback(funobje);
    }
private:
    // 初始化套接字
    bool initializeSocker();
    // 初始化ssh会话
    bool initializeSession();
    // ssh认证
    bool authenticate();
    // 打开通道
    bool openChannel();
    // 发送命令
    bool sendCommand(const std::string &command);
    // 非阻塞读取响应
    template<typename FuncType, typename = std::enable_if_t<std::is_invocable_v<FuncType, std::string>>>
        std::string readResponse_noback(FuncType funobje) {
        std::string response;
        char buffer[1024];
        ssize_t n;

        // 设置非阻塞模式
        libssh2_channel_set_blocking(channel_, 0);

        while (true) {
            // 读取数据
            n = libssh2_channel_read(channel_, buffer, sizeof(buffer));
            if (n > 0) {
                funobje(std::string(buffer, n-1));  // 调用回调函数处理数据
            } else if (n == LIBSSH2_ERROR_EAGAIN) {
                usleep(50000); // 减少等待时间，提高响应速度
            } else if (n < 0) {
                throw std::runtime_error("Error reading from SSH channel");
            }

            // 检查通道是否已关闭
            if (libssh2_channel_eof(channel_)) {
                channel_ = nullptr;
                break;
            }
        }
        return response;
    }

private:
    // 远程IP
    std::string ip_;
    // 远程端口
    int port_;
    // 远程用户名
    std::string username_;
    //  远程密码
    std::string password_;
    // 会话
    LIBSSH2_SESSION *session_;
    // 通道
    LIBSSH2_CHANNEL *channel_;
    // socke
    int sock_;
};
