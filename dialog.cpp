#include "dialog.h"
#include <QThread>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qcontainerinfo.h>
#include <qcoreevent.h>
#include <qlineedit.h>
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
    // 连接成功提示
    label_connectSuccse_color = R"(
        QLabel { 
        border: 1px solid #A9A9A9;  
        border-radius: 5px;           
        padding: 10px;               
        font-size: 13px;
        background-color: white;    
        }
    )";

    connectSuccseinfo.getElement<QLabel>("Label_connectSuccse")->setAlignment(Qt::AlignCenter);
    connectSuccseinfo.getElement<QLabel>("Label_connectSuccse")->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: red;}"));

    // log显示窗口
    logwindinfo.setComponentsStatus(false);

    // 布局
    QVBoxLayout *VLayout = new QVBoxLayout();
    QHBoxLayout *HLayout_1 = new QHBoxLayout; 
    QHBoxLayout *HLayout_2 = new QHBoxLayout; 
    QHBoxLayout *HLayout_3 = new QHBoxLayout; 
    QHBoxLayout *HLayout_4 = new QHBoxLayout; 

    userinfo.setLayout(HLayout_1);
    connectSuccseinfo.setLayout(HLayout_2);
    logwindinfo.setLayout(HLayout_3);
    cmdinfo.setLayout(HLayout_4);

    VLayout->addLayout(HLayout_1);
    VLayout->addLayout(HLayout_2);
    VLayout->addLayout(HLayout_3);
    VLayout->addLayout(HLayout_4);

    setLayout(VLayout);

    sshObject = new SSHClient(); 

}

void Dialog::initSignalSlots(void)
{
    connect(userinfo.getElement<QPushButton>("pushButton_conncetORbreak"), &QPushButton::clicked, [&](){
        // 设置连接信息不能编辑
        if(userinfo.getElement<QLineEdit>("LineEdit_ip")->isReadOnly())
        {
            // 关闭当前的会话, 通道, socket
            sshObject->destory();
            userinfo.setComponentsStatus(true);

            connectSuccseinfo.getElement<QLabel>("Label_connectSuccse")->setText("no connect");
            connectSuccseinfo.getElement<QLabel>("Label_connectSuccse")->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: red;}"));
        }
        else
        {
            std::cout << userinfo.getElement<QLineEdit>("LineEdit_ip")->text().toStdString() << std::endl;
            std::cout << userinfo.getElement<QLineEdit>("LineEdit_port")->text().toStdString() << std::endl;
            std::cout << userinfo.getElement<QLineEdit>("LineEdit_username")->text().toStdString() << std::endl;
            std::cout << userinfo.getElement<QLineEdit>("LineEdit_password")->text().toStdString() << std::endl;
            if(userinfo.getElement<QLineEdit>("LineEdit_ip")->text().isEmpty() || userinfo.getElement<QLineEdit>("LineEdit_port")->text().isEmpty() || userinfo.getElement<QLineEdit>("LineEdit_username")->text().isEmpty() || userinfo.getElement<QLineEdit>("LineEdit_password")->text().isEmpty())
                return;
        // 按照新的信息初始化新的会话
            if(sshObject->init(userinfo.getElement<QLineEdit>("LineEdit_ip")->text().toStdString(), userinfo.getElement<QLineEdit>("LineEdit_port")->text().toInt(), userinfo.getElement<QLineEdit>("LineEdit_username")->text().toStdString(), userinfo.getElement<QLineEdit>("LineEdit_password")->text().toStdString()))
            {
                connectSuccseinfo.getElement<QLabel>("Label_connectSuccse")->setText("connect");
                connectSuccseinfo.getElement<QLabel>("Label_connectSuccse")->setStyleSheet(QString::fromStdString(label_connectSuccse_color + "QLabel{color: green;}"));
                userinfo.setComponentsStatus(false);
            }
        }
        });

    connect(cmdinfo.getElement<QPushButton>("Pushbutton_Start"), &QPushButton::clicked, [&](){
           QThreadPool::globalInstance()->start([=]() {
               sshObject->executeCommand(cmdinfo.getElement<QLineEdit>("LineEdit_execCmd")->text().toStdString(), [&](const std::string &str){
                    QTextEdit* textEdit_logWind = logwindinfo.getElement<QTextEdit>("TextEdit_logWind");
                 // 确保 UI 操作在主线程中进行
                 QMetaObject::invokeMethod(textEdit_logWind, [=]() {
                     textEdit_logWind->append(QString::fromStdString("========= Cmd(" + cmdinfo.getElement<QLineEdit>("LineEdit_execCmd")->text().toStdString() + ") ========="));
                     textEdit_logWind->append(QString::fromStdString(str));
                     textEdit_logWind->repaint();
                 }, Qt::QueuedConnection);
               });
           });
    });
}
