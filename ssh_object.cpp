#include "ssh_object.h"

SSHClient::SSHClient() : session_(nullptr), channel_(nullptr), sock_(-1)
{
}

SSHClient::~SSHClient(void)
{
    destory();
}

bool SSHClient::init(const std::string &ip, int port, const std::string &username, const std::string &password) 
{
    ip_ = ip;
    port_ = port;
    username_ = username;
    password_ = password;

    return initializeSocker() && initializeSession() && authenticate();
}

void SSHClient::destory(void)
{
    if (channel_) {
        libssh2_channel_close(channel_);
        libssh2_channel_free(channel_);
        channel_ = nullptr;
    }

    if (session_) {
        libssh2_session_disconnect(session_, "Normal Shutdown");
        libssh2_session_free(session_);
        session_ = nullptr;
    }

    if (sock_ != -1) {
        close(sock_);
        sock_ = -1;
    }

    libssh2_exit();
}

bool SSHClient::initializeSocker(void)
{
    // 创建套接字
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ == -1) {
        return false;
    }

    // IP结构提
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_pton(AF_INET, ip_.c_str(), &server_addr.sin_addr);

    // 连接套接字
    if (::connect(sock_, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) { // 使用 ::connect 调用全局函数
        return false;
    }

    return  true;
}

bool SSHClient::initializeSession(void)
{
    // 调用会话初始化函数
    session_ = libssh2_session_init();
    if (!session_) {
        return false;
    }

    // 执行SSH握手
    if (libssh2_session_handshake(session_, sock_)) {
        return false;
    }

    return true;
}

bool SSHClient::authenticate(void)
{
    if (libssh2_userauth_password(session_, username_.c_str(), password_.c_str())) {
        return false;
    }
    return true;
}

bool SSHClient::openChannel(void)
{
    channel_ = libssh2_channel_open_session(session_);
    while (!channel_) {
        channel_ = libssh2_channel_open_session(session_);
        if (!channel_) {
            int err = libssh2_session_last_error(session_, NULL, NULL, 0);
            if (err == LIBSSH2_ERROR_EAGAIN) {
                usleep(5000);
                continue;
            } else {
                return false;
            }
        }
    }
    return true;
}

bool SSHClient::sendCommand(const std::string &command) {
    if (libssh2_channel_exec(channel_, command.c_str())) {
        return false;
    }
    return true;
}
