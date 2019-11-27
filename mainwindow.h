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
    void JsonInsert(QString Name,QString Path,QString Set,int Time);
    void UserJsonInsert(QString ObjName);
    void CreateJson();
    void RstTbv();
    void Tbvaddline(QTableWidget *twg,QString Name,QString Path,QString Set,int Time);
    void ReadJson();
    void RemoveSet();
    void SaveUserSet(QListWidgetItem *previous);
    void ReadUserJson(QListWidgetItem *current = nullptr);
    void SaveAll();
    void ItemUp();
    void ItemDown();
    void AppRun();
    int Line_exist(QString Name);
    int Line_exist(QTableWidget *twg,QString Name,QString Path);

public slots:
    void AddNew();
    void AddUserSet();
    void Delete();
    void DeleteUserSet();
    void ChangeCheckStatus(bool);
    void showListWidgetMenuSlot(QPoint pos);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::MainWindow *ui;
    QJsonObject json;
    QJsonObject Userjson;
    QJsonArray NameArr;
    QJsonArray PathArr;
    QJsonArray SetArr;
    QJsonArray TimeArr;
    QJsonDocument doc;
    CCheckBoxHeaderView *myHeader;
    CCheckBoxHeaderView *myUserHeader;
    QString FileName;
    QString UserFileName;
    QMenu *m_contextMenu;
    QAction *m_delAction;
};
#endif // MAINWINDOW_H
