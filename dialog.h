#ifndef DIALOG_H
#define DIALOG_H

#include "ssh_object.h"
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    // 连接/断开
    QPushButton *pushButton_conncetORbreak;

    // 连接信息Label
    QLabel *label_ip;
    QLabel *label_port;
    QLabel *label_user;
    QLabel *label_password;

    // 连接信息lineEdit
    QLineEdit *lineEdit_ip;
    QLineEdit *lineEdit_port;
    QLineEdit *lineEdit_user;
    QLineEdit *lineEdit_password;

    // 连接成功提示
    QLabel *label_connectSuccse;
    std::string label_connectSuccse_color;

    // log显示窗口
    QTextEdit *textEdit_logWind;

    // 执行的命令和正则表达式
    QLineEdit *lineEdit_execCmd;
    QLineEdit *lineEdit_execRegular;

    // 执行和停止按钮
    QPushButton *pushButton_startCmd;
    QPushButton *pushButton_stopCmd;

    SSHClient *sshObject;
public:
    void initUI(void);
    void initSignalSlots(void);

};


#endif // DIALOG_H
