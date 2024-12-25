#ifndef DIALOG_H
#define DIALOG_H

#include "ssh_object.h"
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <qcoreevent.h>
#include <qlabel.h>
#include <qlayoutitem.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qtmetamacros.h>
#include <type_traits>
#include <unordered_map>

template<typename ...ElementType>
class Components : public QObject
{
public:
    bool componentsStatus;
    std::unordered_map<std::string, std::variant<QTextEdit*, QLineEdit*, QPushButton*, QLabel*>> elements;

    template<typename ...ElementName, 
        typename = std::enable_if_t<(std::disjunction_v<std::is_same<ElementName, std::string>, std::is_convertible<ElementName, const char*>> && ...)>,
        typename = std::enable_if_t<sizeof...(ElementType) == sizeof...(ElementName)>>
    Components(ElementName ...elementNames)
    {
        // 展开参数包并初始化 map
        ([&](){
            auto tem = new ElementType();
            if(std::is_same_v<ElementType, QLabel>)
            {
                tem->setText(elementNames);
            }
            elements.emplace(elementNames, tem);
        }(),...);

        componentsStatus = true;
    }

    ~Components()
    {
        for (auto& [name, element] : elements) {
            std::visit([](auto* ptr) { delete ptr; }, element);
        }
    }
    template<typename T>
    T* getElement(const std::string& name)
    {
        auto it = elements.find(name);
        if (it != elements.end()) {
            // 确保类型匹配
            if (auto* ptr = std::get<T*>(it->second)) {
                return ptr;
            } else {
                qWarning() << "Type mismatch for element: " << QString::fromStdString(name);
                return nullptr;
            }
        } else {
            qWarning() << "Element not found: " << QString::fromStdString(name);
            return nullptr;
        }
    }

public:
    template<typename LayoutType, typename = decltype(std::declval<LayoutType>().addWidget(nullptr))>
    void setLayout(LayoutType* layout)
    {
        for (auto& [name, element] : elements) {
            std::visit([&](auto* ptr) { 
                layout->addWidget(ptr);
            }, element);
        }
    }

    // 检查是否存在成员函数 A 的 trait
    template <typename T, typename = void>
    struct hasMemberReadOnly : std::false_type {};
    template <typename T>
    struct hasMemberReadOnly<T, std::void_t<decltype(std::declval<T>().setReadOnly(false))>> : std::true_type {};

    // 执行器，用于调用 A 或者什么都不做
    template <typename T>
    struct CallMemberReadOnly
    {
        bool status;
        bool operator()(T& obj) const
        {
            if constexpr (hasMemberReadOnly<T>::value) {
                obj.setReadOnly(status); // 如果有成员函数 A，调用它
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    // 检查是否存在成员函数 A 的 trait
    template <typename T, typename = void>
    struct hasMemberEnabled : std::false_type {};
    template <typename T>
    struct hasMemberEnabled<T, std::void_t<decltype(std::declval<T>().setEnabled(false))>> : std::true_type {};

    // 执行器，用于调用 A 或者什么都不做
    template <typename T>
    struct CallMemberEnable
    {
        bool status;
        bool operator()(T& obj) const
        {
            if constexpr (hasMemberEnabled<T>::value) {
                obj.setEnabled(status); // 如果有成员函数 A，调用它
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    template<typename T, typename ...TT>
    void setComponentsStatus_(std::variant<QTextEdit*, QLineEdit*, QPushButton*, QLabel*> tem, bool status) {
        auto pp_element = std::get_if<T>(&tem);
        if (pp_element) {
            auto p_element = *pp_element;
            CallMemberReadOnly<typename std::remove_pointer<decltype(p_element)>::type> set_ReadOnly{!status};
            CallMemberEnable<typename std::remove_pointer<decltype(p_element)>::type> set_Enabled{status};
            !set_ReadOnly(*p_element) && set_Enabled(*p_element);
        } else if constexpr (sizeof...(TT) > 0) {
            // 如果类型不匹配，递归尝试下一个类型
            return setComponentsStatus_<TT...>(tem, status);
        }
    }

    bool setComponentsStatus(bool status)
    {
        bool oldstatus = componentsStatus;
        componentsStatus = status;

        QPushButton *a;

        for (auto& [name, element] : elements) {
            setComponentsStatus_<ElementType* ...>(element, status);
        }

        return oldstatus;
    }

    bool getComponentsStatus(void)
    {
        return componentsStatus;
    }
};

class UserInfo : public QObject
{
    Q_OBJECT

public:
    UserInfo() = default;
    ~UserInfo() = default;

public:
    void initUI(void);
    bool setComponentsStatus(bool status);
    bool getComponentsStatus(void);
    template<typename LayoutType, typename = decltype(std::declval<LayoutType>().addWidget(nullptr))>
    void setLayout(LayoutType* layout)
    {
        layout->addWidget(label_ip);
        layout->addWidget(lineEdit_ip);
        layout->addStretch();
        layout->addWidget(label_port);
        layout->addWidget(lineEdit_port);
        layout->addStretch();
        layout->addWidget(label_user);
        layout->addWidget(lineEdit_user);
        layout->addStretch();
        layout->addWidget(label_password);
        layout->addWidget(lineEdit_password);
        layout->addStretch();
        layout->addWidget(pushButton_conncetORbreak);
    }

public:
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

    // 部件状态
    bool componentsStatus;
};

class CmdInfo : public QObject
{
    Q_OBJECT

public:
    CmdInfo() = default;
    ~CmdInfo() = default;

public:
    void initUI(void);
    bool setComponentsStatus(bool status);
    bool getComponentsStatus(void);
    template<typename LayoutType, typename = decltype(std::declval<LayoutType>().addWidget(nullptr))>
    void setLayout(LayoutType* layout)
    {
        layout->addWidget(lineEdit_execCmd);
        layout->addWidget(lineEdit_execRegular);
        layout->addWidget(pushButton_startCmd);
        layout->addWidget(pushButton_stopCmd);
    }

public:
    // 执行的命令和正则表达式
    QLineEdit *lineEdit_execCmd;
    QLineEdit *lineEdit_execRegular;

    // 执行和停止按钮
    QPushButton *pushButton_startCmd;
    QPushButton *pushButton_stopCmd;

    // 部件状态
    bool componentsStatus;
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    Components<QLabel, QLineEdit, QLabel, QLineEdit, QLabel, QLineEdit, QLabel, QLineEdit, QPushButton> tes{"Label_ip", "LineEdit_ip", "Label_port", "LineEdit_port", "Label_username", "LineEdit_username", "Label_password", "LineEdit_password", "pushButton_conncetORbreak"};

    // 用户信息
    UserInfo *userinfo;

    // 命令信息
    CmdInfo *cmdinfo;

    // 连接成功提示
    QLabel *label_connectSuccse;
    std::string label_connectSuccse_color;

    // log显示窗口
    QTextEdit *textEdit_logWind;

    // ssh对象
    SSHClient *sshObject;

public:
    void initUI(void);
    void initSignalSlots(void);
};


#endif // DIALOG_H
