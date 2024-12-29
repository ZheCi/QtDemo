#ifndef DIALOG_H
#define DIALOG_H

#include "ssh_object.h"
#include <QLayout>
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

template<typename ...ElementType>
class MyComponents : public QObject
{
public:
    bool componentsStatus;
    using ComponentType = std::variant<QTextEdit*, QLineEdit*, QPushButton*, QLabel*>;
    std::vector<std::tuple<std::string, ComponentType>> elements;

    template<typename ...ElementName, 
        typename = std::enable_if_t<(std::disjunction_v<std::is_same<ElementName, std::string>, std::is_convertible<ElementName, const char*>> && ...)>,
        typename = std::enable_if_t<sizeof...(ElementType) == sizeof...(ElementName)>>
    MyComponents(ElementName ...elementNames)
    {
        // 展开参数包并初始化 map
        ([&](){
            std::tuple<std::string, ElementType*>element = std::make_tuple(elementNames, new ElementType{});
            if(std::is_same_v<ElementType, QLabel>)
            {
            std::get<1>(element)->setText(elementNames);
            }
            elements.emplace_back( element); }(),...);

        componentsStatus = true;
    }

    ~MyComponents()
    {
        for (auto& [name, element] : elements) {
            std::visit([](auto* ptr) { delete ptr; }, element);
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

    bool getComponentsStatus(void)
    {
        return componentsStatus;
    }

    template<typename T>
    T* getElement(const std::string& name)
    {
        for(auto it : elements)
        {
            if(std::get<0>(it) == name)
            {
                return std::get<T*>(std::get<1>(it));
            }
        }

        return nullptr;
    }

    template<typename T>
    void addElement(const std::string elementName, T *element)
    {
        elements.push_back({elementName, element});
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
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    // 用户信息
    MyComponents<QLabel, QLineEdit, QLabel, QLineEdit, QLabel, QLineEdit, QLabel, QLineEdit, QPushButton> userinfo{"Label_ip", "LineEdit_ip", "Label_port", "LineEdit_port", "Label_username", "LineEdit_username", "Label_password", "LineEdit_password", "pushButton_conncetORbreak"};

    // 命令信息
    MyComponents<QLineEdit, QLineEdit, QPushButton, QPushButton> cmdinfo{"LineEdit_execCmd", "LineEdit_execReg", "Pushbutton_Start", "Pushbutton_Stop"};

    // 连接成功提示
    std::string label_connectSuccse_color;
    MyComponents<QLabel> connectSuccseinfo{"Label_connectSuccse"};

    // log显示窗口
    MyComponents<QTextEdit> logwindinfo{"TextEdit_logWind"};

    // ssh对象
    SSHClient *sshObject;

public:
    void initUI(void);
    void initSignalSlots(void);
};


#endif // DIALOG_H
