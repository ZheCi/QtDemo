#include "dialog.h"
#include <QThread>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <qpushbutton.h> 
#include <QtConcurrent>
void UserInfo::initUI(void)
{
    // 连接/断开
    pushButton_conncetORbreak = new QPushButton("连接/断开");

    // 连接信息Label
    label_ip = new QLabel("Ip:");
    label_port = new QLabel("Port:");
    label_user = new QLabel("Username:");
    label_password = new QLabel("Password:");

    // 连接信息lineEdit
    lineEdit_ip = new QLineEdit("127.0.0.1");
    lineEdit_port = new QLineEdit("55125");
    lineEdit_user = new QLineEdit("ZheCi");
    lineEdit_password = new QLineEdit("Dly000919.!");

    // 默认部件状态
    componentsStatus = true;
}

bool UserInfo::getComponentsStatus(void)
{
    return componentsStatus;
}

bool UserInfo::setComponentsStatus(bool status)
{
    bool oldstatus = componentsStatus;
    componentsStatus = status;
    
    lineEdit_ip->setReadOnly(status);
    lineEdit_port->setReadOnly(status);
    lineEdit_user->setReadOnly(status);
    lineEdit_password->setReadOnly(status);

    return oldstatus;
}

void CmdInfo::initUI(void)
{
    // 执行的命令和正则表达式
    lineEdit_execCmd = new QLineEdit();
    lineEdit_execRegular = new QLineEdit();

    // 执行和停止按钮
    pushButton_startCmd = new QPushButton("Start");
    pushButton_stopCmd = new QPushButton("Stop");
}

bool CmdInfo::getComponentsStatus(void)
{
    return componentsStatus;
}

bool CmdInfo::setComponentsStatus(bool status)
{
    bool oldstatus = componentsStatus;
    componentsStatus = status;
    
    lineEdit_execCmd->setReadOnly(status);
    lineEdit_execRegular->setReadOnly(status);
    pushButton_startCmd->setEnabled(status);
    pushButton_stopCmd->setEnabled(!status);

    return oldstatus;
}

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    initUI();
    initSignalSlots();
}

Dialog::~Dialog()
{
    delete sshObject;
}

void Dialog::initUI(void) 
{
    // 用户信息
    userinfo = new UserInfo;
    userinfo->initUI();

    // 命令信息
    cmdinfo = new CmdInfo;
    cmdinfo->initUI();

    // 连接成功提示
    label_connectSuccse = new QLabel("no connect");
    label_connectSuccse_color = R"(
        QLabel { 
        border: 1px solid #A9A9A9;  
        border-radius: 5px;           
        padding: 10px;               
        font-size: 13px;
        background-color: white;    
        }
    )";
    label_connectSuccse->setAlignment(Qt::AlignCenter);
    label_connectSuccse->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: red;}"));

    // log显示窗口
    textEdit_logWind = new QTextEdit();
    textEdit_logWind->setReadOnly(true);

    // 布局
    QVBoxLayout *VLayout = new QVBoxLayout();
    QHBoxLayout *HLayout_1 = new QHBoxLayout; 
    QHBoxLayout *HLayout_2 = new QHBoxLayout; 
    QHBoxLayout *HLayout_3 = new QHBoxLayout; 
    QHBoxLayout *HLayout_4 = new QHBoxLayout; 

    userinfo->setLayout(HLayout_1);
    HLayout_2->addWidget(label_connectSuccse);
    HLayout_3->addWidget(textEdit_logWind);
    cmdinfo->setLayout(HLayout_4);

    // tes.getElement<QPushButton>("pushButton_1")->setText("按钮");
    // tes.getElement<QLabel>("QLabel_1")->setText("你好呀");
    tes.setLayout(HLayout_3);
    // tes.setComponentsStatus(true);

    VLayout->addLayout(HLayout_1);
    VLayout->addLayout(HLayout_2);
    VLayout->addLayout(HLayout_3);
    VLayout->addLayout(HLayout_4);

    setLayout(VLayout);

    sshObject = new SSHClient(); 

}

void Dialog::initSignalSlots(void)
{
    connect(userinfo->pushButton_conncetORbreak, &QPushButton::clicked, [&](){
        // 设置连接信息不能编辑
        if(userinfo->lineEdit_ip->isReadOnly())
        {
            // 关闭当前的会话, 通道, socket
            sshObject->destory();
            userinfo->setComponentsStatus(false);
            label_connectSuccse->setText("on connect");
            label_connectSuccse->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: red;}"));
        }
        else
        {
            if(userinfo->lineEdit_ip->text().isEmpty() || userinfo->lineEdit_port->text().isEmpty() || userinfo->lineEdit_user->text().isEmpty() || userinfo->lineEdit_password->text().isEmpty())
                return;
        // 按照新的信息初始化新的会话
            if(sshObject->init(userinfo->lineEdit_ip->text().toStdString(), userinfo->lineEdit_port->text().toInt(), userinfo->lineEdit_user->text().toStdString(), userinfo->lineEdit_password->text().toStdString()))
            {
                label_connectSuccse->setText("connect");
                label_connectSuccse->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: green;}"));
                userinfo->setComponentsStatus(true);
            }
        }
        });
    connect(cmdinfo->pushButton_startCmd, &QPushButton::clicked, [&](){
           QThreadPool::globalInstance()->start([=]() {
               sshObject->executeCommand(cmdinfo->lineEdit_execCmd->text().toStdString(), [&](const std::string &str){
                 // 确保 UI 操作在主线程中进行
                 QMetaObject::invokeMethod(textEdit_logWind, [=]() {
                     textEdit_logWind->append(QString::fromStdString("========= Cmd(" + cmdinfo->lineEdit_execCmd->text().toStdString() + ") ========="));
                     textEdit_logWind->append(QString::fromStdString(str));
                     textEdit_logWind->repaint();
                 }, Qt::QueuedConnection);
               });
           });
    });
}
