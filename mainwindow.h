#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTableWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "myheader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void ReadFile(QString t_FileName);
    void WriteFile(QString t_FileName,QJsonObject t_Json);

    //Json操作函数
    void JsonInsert(QString Name,QString Path,QString Set,int Time);
    void CreateJson(QTableWidget *Cur_Twg);
    void UserJsonInsert(QString ObjName);

    void SaveAll();
    void SaveUserSet(QListWidgetItem *current, QListWidgetItem *previous);

    void ResJson(QJsonObject object,QTableWidget *twg);
    void ReadJson();
    void ReadUserJson(QListWidgetItem *current = nullptr);

    //tablewidget操作函数
    void RstTbv();
    void Tbvaddline(QTableWidget *twg,QString Name,QString Path,QString Set,int Time);
    void ItemSwap(int differ);
    void ItemUp();
    void ItemDown();

    //应用启动函数
    void AppRun();

    //唯一性判断函数
    int Line_exist(QString Name,QString Path = "",QTableWidget *twg = nullptr);

    //拖拽事件对应函数
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;

    CCheckBoxHeaderView *myHeader;
    CCheckBoxHeaderView *myUserHeader;

    QTableWidget *Cur_Twg;

    QMenu *m_contextMenu;
    QAction *m_delAction;

    QJsonDocument Doc;
    QJsonObject Json;
    QJsonObject UserJson;
    QJsonArray NameArr;
    QJsonArray PathArr;
    QJsonArray SetArr;
    QJsonArray TimeArr;

    QString FileName;
    QString UserFileName;

private slots:
    void AddNew();
    void AddUserSet();
    void Delete();
    void DeleteUserSet();
    void ChangeCheckStatus(bool);
    void showListWidgetMenuSlot(QPoint pos);
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

};
#endif // MAINWINDOW_H
