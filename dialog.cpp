#include "dialog.h"
#include <QThread>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <QtConcurrent>

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
    label_connectSuccse->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: red;}"));

    label_connectSuccse->setAlignment(Qt::AlignCenter);

    // log显示窗口
    textEdit_logWind = new QTextEdit();
    textEdit_logWind->setReadOnly(true);

    // 执行的命令和正则表达式
    lineEdit_execCmd = new QLineEdit();
    lineEdit_execRegular = new QLineEdit();

    // 执行和停止按钮
    pushButton_startCmd = new QPushButton("Start");
    pushButton_stopCmd = new QPushButton("Stop");

    // 布局
    QVBoxLayout *VLayout = new QVBoxLayout();
    QHBoxLayout *HLayout_1 = new QHBoxLayout; 
    QHBoxLayout *HLayout_2 = new QHBoxLayout; 
    QHBoxLayout *HLayout_3 = new QHBoxLayout; 
    QHBoxLayout *HLayout_4 = new QHBoxLayout; 

    HLayout_1->addWidget(label_ip);
    HLayout_1->addWidget(lineEdit_ip);
    HLayout_1->addStretch();
    HLayout_1->addWidget(label_port);
    HLayout_1->addWidget(lineEdit_port);
    HLayout_1->addStretch();
    HLayout_1->addWidget(label_user);
    HLayout_1->addWidget(lineEdit_user);
    HLayout_1->addStretch();
    HLayout_1->addWidget(label_password);
    HLayout_1->addWidget(lineEdit_password);
    HLayout_1->addStretch();
    HLayout_1->addWidget(pushButton_conncetORbreak);

    HLayout_2->addWidget(label_connectSuccse);
    HLayout_3->addWidget(textEdit_logWind);

    HLayout_4->addWidget(lineEdit_execCmd);
    HLayout_4->addWidget(lineEdit_execRegular);
    HLayout_4->addWidget(pushButton_startCmd);
    HLayout_4->addWidget(pushButton_stopCmd);

    VLayout->addLayout(HLayout_1);
    VLayout->addLayout(HLayout_2);
    VLayout->addLayout(HLayout_3);
    VLayout->addLayout(HLayout_4);

    setLayout(VLayout);

    sshObject = new SSHClient(); 
}

void Dialog::initSignalSlots(void)
{
    connect(pushButton_conncetORbreak, &QPushButton::clicked, [&](){
        // 设置连接信息不能编辑
        if(lineEdit_ip->isReadOnly())
        {
            // 关闭当前的会话, 通道, socket
            sshObject->destory();
            lineEdit_ip->setReadOnly(false);
            lineEdit_port->setReadOnly(false);
            lineEdit_user->setReadOnly(false);
            lineEdit_password->setReadOnly(false);
            label_connectSuccse->setText("on connect");
            label_connectSuccse->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: red;}"));
        }
        else
        {
            if(lineEdit_ip->text().isEmpty() || lineEdit_port->text().isEmpty() || lineEdit_user->text().isEmpty() || lineEdit_password->text().isEmpty())
                return;
        // 按照新的信息初始化新的会话
            if(sshObject->init(lineEdit_ip->text().toStdString(), lineEdit_port->text().toInt(), lineEdit_user->text().toStdString(), lineEdit_password->text().toStdString()))
            {
                label_connectSuccse->setText("connect");
                label_connectSuccse->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: green;}"));
                lineEdit_ip->setReadOnly(true);
                lineEdit_port->setReadOnly(true);
                lineEdit_user->setReadOnly(true);
                lineEdit_password->setReadOnly(true);
            }
        }
        });
    connect(pushButton_startCmd, &QPushButton::clicked, [&](){
           QThreadPool::globalInstance()->start([=]() {
               sshObject->executeCommand(lineEdit_execCmd->text().toStdString(), [&](const std::string &str){
                 // 确保 UI 操作在主线程中进行
                 QMetaObject::invokeMethod(textEdit_logWind, [=]() {
                     textEdit_logWind->append(QString::fromStdString("========= Cmd(" + lineEdit_execCmd->text().toStdString() + ") ========="));
                     textEdit_logWind->append(QString::fromStdString(str));
                     textEdit_logWind->repaint();
                 }, Qt::QueuedConnection);
               });
           });
    });
}
