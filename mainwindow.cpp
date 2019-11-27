#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString cur_Path = QDir::currentPath();
    FileName = cur_Path + "/data.json";
    UserFileName = cur_Path + "/user.json";

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    RstTbv();
    ReadJson();

    ReadUserJson();
    if(ui->listWidget->count()>0)
        ui->listWidget->item(0)->setSelected(true);
    ui->tabWidget->setCurrentIndex(0);

    this->setAcceptDrops(true);

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_contextMenu = new QMenu;
    m_delAction = new QAction(tr("删除"), ui->listWidget);

    m_contextMenu->addAction(m_delAction);

    connect(ui->Btn_New,&QPushButton::clicked,this,&MainWindow::AddNew);
    connect(ui->Btn_Del,&QPushButton::clicked,this,&MainWindow::Delete);
    connect(ui->Btn_Up,&QPushButton::clicked,this,&MainWindow::ItemUp);
    connect(ui->Btn_Down,&QPushButton::clicked,this,&MainWindow::ItemDown);
    connect(ui->Btn_Run,&QPushButton::clicked,this,&MainWindow::AppRun);
    connect(ui->Btn_AddUS,&QPushButton::clicked,this,&MainWindow::AddUserSet);
    connect(myHeader,SIGNAL(checkStausChange(bool)),this,SLOT(ChangeCheckStatus(bool)));
    connect(myUserHeader,SIGNAL(checkStausChange(bool)),this,SLOT(ChangeCheckStatus(bool)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(showListWidgetMenuSlot(QPoint)));
    connect(m_delAction, &QAction::triggered,this,&MainWindow::DeleteUserSet);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CreateJson()
{
    json.insert("Name", QJsonValue(NameArr));
    json.insert("Path", QJsonValue(PathArr));
    json.insert("Set", QJsonValue(SetArr));
    json.insert("Time", QJsonValue(TimeArr));

    QFile srcFile(FileName);

    if(!srcFile.open(QFile::WriteOnly))
    {
        qDebug() << "can't open the file!";
        return;
    }

    doc.setObject(json);
    QByteArray byteArray = doc.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    QTextStream in(&srcFile);
    in<<strJson;
    in.flush();

    srcFile.close();
}

void MainWindow::JsonInsert(QString Name,QString Path,QString Set,int Time)
{
    NameArr.append(Name);
    PathArr.append(Path);
    SetArr.append(Set);
    TimeArr.append(Time);
}

void MainWindow::RstTbv()
{
    ui->tableWidget->verticalHeader()->setVisible(false);

    ui->tableWidget->setSelectionBehavior ( QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode ( QAbstractItemView::SingleSelection);

    ui->tableWidget->setColumnCount(5); //设置列数

    myHeader = new CCheckBoxHeaderView(0, Qt::Horizontal, ui->tableWidget);
    ui->tableWidget->setHorizontalHeader(myHeader);

    ui->tableWidget->setColumnWidth(0, 30);  //0 设置列宽
    ui->tableWidget->setColumnWidth(1, 100); //1 设置列宽
    ui->tableWidget->setColumnWidth(2, 230); //2 设置列宽
    ui->tableWidget->setColumnWidth(3, 80);  //3 设置列宽
    ui->tableWidget->setColumnWidth(4, 75);  //3 设置列宽

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("")<<tr("名称")<<tr("路径")<<tr("启动项")<<tr("启动间隔"));

    ui->userWidget->verticalHeader()->setVisible(false);

    ui->userWidget->setSelectionBehavior ( QAbstractItemView::SelectRows);
    ui->userWidget->setSelectionMode ( QAbstractItemView::SingleSelection);

    ui->userWidget->setColumnCount(5); //设置列数

    myUserHeader = new CCheckBoxHeaderView(0, Qt::Horizontal, ui->userWidget);
    ui->userWidget->setHorizontalHeader(myUserHeader);

    ui->userWidget->setColumnWidth(0, 30);  //0 设置列宽
    ui->userWidget->setColumnWidth(1, 100); //1 设置列宽
    ui->userWidget->setColumnWidth(2, 230); //2 设置列宽
    ui->userWidget->setColumnWidth(3, 80);  //3 设置列宽
    ui->userWidget->setColumnWidth(4, 75);  //3 设置列宽

    ui->userWidget->setHorizontalHeaderLabels(QStringList() << tr("")<<tr("名称")<<tr("路径")<<tr("启动项")<<tr("启动间隔"));
}

void MainWindow::Tbvaddline(QTableWidget *twg,QString Name,QString Path,QString Set,int Time)
{
    if(Line_exist(twg,Name,Path)==0)
    {
        QFileInfo finfo(Path);
        QFileIconProvider icon_pri;
        QIcon icon = icon_pri.icon(finfo);

        QTableWidgetItem *check=new QTableWidgetItem;
        check->setCheckState (Qt::Checked);
        int rowcount = twg->rowCount();
        twg->insertRow(rowcount);
        twg->setItem(rowcount,0,check);
        twg->setItem(rowcount,1,new QTableWidgetItem(icon,Name));
        twg->setItem(rowcount,2,new QTableWidgetItem(Path));
        twg->setItem(rowcount,3,new QTableWidgetItem(Set));
        twg->setItem(rowcount,4,new QTableWidgetItem(QString::number(Time)));
    }
    else
    {
        QMessageBox::warning(this, tr("注意"),tr("请不要重复添加"),QMessageBox::Yes);
    }

}

void MainWindow::ReadJson()
{
    QFile srcFile(FileName);
    if(!srcFile.open(QFile::ReadOnly))
    {
        qDebug() << "can't open the file!";
        return;
    }

    QJsonParseError error;
    doc = QJsonDocument::fromJson(srcFile.readAll(), &error);
    srcFile.close();
    if (!doc.isNull() && (error.error == QJsonParseError::NoError))
    {
        if (doc.isObject())
        {
            QJsonObject object = doc.object();
            QJsonValue Name = object.value("Name");
            QJsonValue Path = object.value("Path");
            QJsonValue Set = object.value("Set");
            QJsonValue Time = object.value("Time");
            QJsonArray NameArray = Name.toArray();
            QJsonArray PathArray = Path.toArray();
            QJsonArray SetArray = Set.toArray();
            QJsonArray TimeArray = Time.toArray();
            int nSize = NameArray.size();
            for (int i = 0; i < nSize; ++i)
            {
                Tbvaddline(ui->tableWidget,NameArray.at(i).toString(),PathArray.at(i).toString(),SetArray.at(i).toString(),TimeArray.at(i).toInt());
            }
        }
    }
}

void MainWindow::AddNew()
{
    QString curPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);  //获取应用当前目录
    QString dlgTitle = "选择一个文件";
    QString filter = "程序文件(*.lnk *.exe)";
    QString aFileName = QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    if(aFileName!="")
    {
        QFileInfo fileinfo(aFileName);
        if(fileinfo.isSymLink())
        {
            fileinfo = fileinfo.symLinkTarget();

        }
        if(ui->tabWidget->currentIndex()==0)
        {
            Tbvaddline(ui->tableWidget,fileinfo.baseName(),fileinfo.absoluteFilePath(),"",100);
        }
        else
        {
            Tbvaddline(ui->userWidget,fileinfo.baseName(),fileinfo.absoluteFilePath(),"",100);
        }

    }
    SaveAll();
}

void MainWindow::Delete()
{
    QMessageBox::StandardButton ch=QMessageBox::question(this,tr("注意"),tr("确认需要删除选中条目"),QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
    if(ch == QMessageBox::Yes)
    {
        for(int i=0;i<ui->tableWidget->rowCount();++i)
        {
            if(ui->tableWidget->item(i,0)->checkState() == Qt::Checked)
            {
                ui->tableWidget->removeRow(i);
                i--;
            }
        }
    }
    SaveAll();
}

int MainWindow::Line_exist(QTableWidget *twg,QString Name,QString Path)
{
    for(int i=0;i<twg->rowCount();i++)
    {
        if(twg->item(i,1)->text()==Name && twg->item(i,2)->text()==Path)
        {
            return -1;
        }
    }
    return 0;
}

int MainWindow::Line_exist(QString Name)
{
    for(int i=0;i<ui->listWidget->count();i++)
    {
        if(ui->listWidget->item(i)->text()==Name)
        {
            return -1;
        }
    }
    return 0;
}

void MainWindow::ItemUp()
{
    QString up,bottom;
    QTableWidget *twg;
    if(ui->tabWidget->currentIndex()==0)
    {
        twg = ui->tableWidget;
    }
    else
    {
        twg = ui->userWidget;
    }
    int row = ui->tableWidget->currentRow();
    if(row > 0)
    {
        for(int i=1;i<twg->columnCount();i++)
        {
            up = twg->item(row-1,i)->text();
            bottom = twg->item(row,i)->text();
            twg->setItem(row-1,i,new QTableWidgetItem(bottom));
            twg->setItem(row,i,new QTableWidgetItem(up));
        }
    }
}

void MainWindow::ItemDown()
{
    QString up,bottom;
    QTableWidget *twg;
    if(ui->tabWidget->currentIndex()==0)
    {
        twg = ui->tableWidget;
    }
    else
    {
        twg = ui->userWidget;
    }
    int row = ui->tableWidget->currentRow();
    if(row > 0)
    {
        for(int i=1;i<twg->columnCount();i++)
        {
            up = twg->item(row,i)->text();
            bottom = twg->item(row+1,i)->text();
            twg->setItem(row,i,new QTableWidgetItem(bottom));
            twg->setItem(row+1,i,new QTableWidgetItem(up));
        }
    }
}

void MainWindow::AppRun()
{
    QProcess process;
    QString str;
    QTableWidget *twg;
    int time;
    if(ui->tabWidget->currentIndex()==0)
    {
        twg = ui->tableWidget;
    }
    else
    {
        twg = ui->userWidget;
    }
    for(int i=0;i<twg->rowCount();++i)
    {
        if(twg->item(i,0)->checkState() == Qt::Checked)
        {
            str = twg->item(i,2)->text();
            if(twg->item(i,3)->text()!="")
            {
                str += " ";
                str += twg->item(i,3)->text();
            }
            str.replace(" ","\" \"");
            process.startDetached(str);
            QEventLoop eventloop;
            time = twg->item(i,4)->text().toInt();
            QTimer::singleShot(time*30, &eventloop, SLOT(quit()));
            eventloop.exec();
        }
    }

}

void MainWindow::ChangeCheckStatus(bool status)
{
    if(status==true)
    {
        for(int i=0;i<ui->tableWidget->rowCount();++i)
        {
            ui->tableWidget->item(i,0)->setCheckState(Qt::Checked);
        }
    }
    else
    {
        for(int i=0;i<ui->tableWidget->rowCount();++i)
        {
            ui->tableWidget->item(i,0)->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::AddUserSet()
{
    QString Name;

    for(int i=0;i<ui->userWidget->rowCount();++i)
    {
        ui->userWidget->removeRow(i);
        i--;
    }

    if(ui->lineEdit->text().remove(QRegExp("\\s")) != "")
    {
        Name = ui->lineEdit->text().trimmed();
        if(Line_exist(Name)==0)
        {
            int count;
            ui->listWidget->addItem(new QListWidgetItem(Name));
            for(int i=0;i<ui->tableWidget->rowCount();++i)
            {
                if(ui->tableWidget->item(i,0)->checkState() == Qt::Checked)
                {
                    Tbvaddline(ui->userWidget,ui->tableWidget->item(i,1)->text(),ui->tableWidget->item(i,2)->text(),ui->tableWidget->item(i,3)->text(),ui->tableWidget->item(i,4)->text().toInt());
                }
            }


            count = ui->listWidget->count();
            //qDebug()<<ui->listWidget->item(count-1)->text();
            SaveUserSet(ui->listWidget->item(count-1));
            ui->tabWidget->setCurrentIndex(1);
            ui->listWidget->item(count-1)->setSelected(true);
        }
        else
        {
            QMessageBox::warning(this,tr("注意"),tr("同名配置文件已存在！"),QMessageBox::Yes);
        }
    }
    else
    {
        QMessageBox::warning(this,tr("注意"),tr("请先输入配置名称！"),QMessageBox::Yes);
    }

}

void MainWindow::SaveUserSet(QListWidgetItem *previous)
{
    QString ObjName;
    if(previous==nullptr)
    {
        return;
    }
    ObjName = previous->text();

    QFile srcFile(UserFileName);
    if(!srcFile.open(QFile::ReadWrite))
    {
        qDebug() << "can't open the file!";
        return;
    }

    QJsonParseError error;
    doc = QJsonDocument::fromJson(srcFile.readAll(), &error);

    if (!doc.isNull() && (error.error == QJsonParseError::NoError))
    {
        if (doc.isObject())
        {
            Userjson = {};
            QJsonObject object = doc.object();
            QStringList keyList = object.keys();
            if(keyList.contains(ObjName))
            {
                for(int i=0;i<keyList.size();i++)
                {
                    if(keyList.at(i)==ObjName)
                    {
                        UserJsonInsert(ObjName);
                    }
                    else
                    {
                        Userjson.insert(keyList.at(i),QJsonValue(object.value(keyList.at(i)).toObject()));
                    }
                }
            }
            else
            {
                Userjson = object;
                UserJsonInsert(ObjName);
            }
        }
        srcFile.close();
        srcFile.remove();
        srcFile.open(QFile::ReadWrite);

        doc.setObject(Userjson);
        QByteArray byteArray = doc.toJson(QJsonDocument::Compact);
        QString strJson(byteArray);
        QTextStream in(&srcFile);
        in<<strJson;
        in.flush();

    }
    else if(doc.isNull())
    {
        Userjson = {};
        UserJsonInsert(ObjName);

        doc.setObject(Userjson);
        QByteArray byteArray = doc.toJson(QJsonDocument::Compact);
        QString strJson(byteArray);
        qDebug() << strJson;
        QTextStream in(&srcFile);
        in<<strJson;
        in.flush();
    }
    srcFile.close();

}

void MainWindow::ReadUserJson(QListWidgetItem *current)
{
    QFile srcFile(UserFileName);
    if(!srcFile.open(QFile::ReadOnly))
    {
        qDebug() << "can't open the file!";
        return;
    }

    QJsonParseError error;
    doc = QJsonDocument::fromJson(srcFile.readAll(), &error);



    if (!doc.isNull() && (error.error == QJsonParseError::NoError))
    {
        if(doc.isObject())
        {
            QJsonObject object = doc.object();
            QStringList strlist = object.keys();

            if(current != nullptr)
            {
                QString ObjName = current->text();
                if(object.contains(ObjName))
                {
                    QJsonObject userobj = object.value(ObjName).toObject();
                    QJsonArray NameArray = userobj.value("Name").toArray();
                    QJsonArray PathArray = userobj.value("Path").toArray();
                    QJsonArray SetArray = userobj.value("Set").toArray();
                    QJsonArray TimeArray = userobj.value("Time").toArray();
                    int nSize = NameArray.size();
                    for (int i = 0; i < nSize; ++i)
                    {
                        Tbvaddline(ui->userWidget,NameArray.at(i).toString(),PathArray.at(i).toString(),SetArray.at(i).toString(),TimeArray.at(i).toInt());
                    }
                }
            }
            else
            {
                ui->listWidget->addItems(strlist);
            }
        }
    }
    srcFile.close();
}

void MainWindow::UserJsonInsert(QString ObjName)
{
    //数组清空
    NameArr = {};
    PathArr = {};
    SetArr = {};
    TimeArr = {};

    QJsonObject tmpobject;

    //获取userWidget里的信息，放进对应Array
    for(int i=0;i<ui->userWidget->rowCount();++i)
    {
        JsonInsert(ui->userWidget->item(i,1)->text(),ui->userWidget->item(i,2)->text(),ui->userWidget->item(i,3)->text(),ui->userWidget->item(i,4)->text().toInt());
    }
    tmpobject = {};
    tmpobject.insert("Name",QJsonValue(NameArr));
    tmpobject.insert("Path",QJsonValue(PathArr));
    tmpobject.insert("Set",QJsonValue(SetArr));
    tmpobject.insert("Time",QJsonValue(TimeArr));
    Userjson.insert(ObjName,QJsonValue(tmpobject));
}

void MainWindow::SaveAll()
{

    NameArr = {};
    PathArr = {};
    SetArr = {};
    TimeArr = {};

    for(int i=0;i<ui->tableWidget->rowCount();++i)
    {
        JsonInsert(ui->tableWidget->item(i,1)->text(),ui->tableWidget->item(i,2)->text(),ui->tableWidget->item(i,3)->text(),ui->tableWidget->item(i,4)->text().toInt());
    }
    CreateJson();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //如果为文件，则支持拖放
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{

    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;
    //多个文件取第一个来进行后面的操作
    QString file = urls.first().toLocalFile();
    if (file.isEmpty())
    {
        return;
    }
    else
    {
        if(file!="")
        {
            QFileInfo fileinfo(file);
            QTableWidget *twg;
            if(fileinfo.isSymLink())
            {
                fileinfo = fileinfo.symLinkTarget();

            }
            if(ui->tabWidget->currentIndex()==0)
            {
                twg = ui->tableWidget;
            }
            else
            {
                twg = ui->userWidget;
            }
            Tbvaddline(twg,fileinfo.baseName(),fileinfo.absoluteFilePath(),"",100);

        }
    }
}

void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current == nullptr)
    {
        return;
    }
    SaveUserSet(previous);
    for(int i=0;i<ui->userWidget->rowCount();++i)
    {
        ui->userWidget->removeRow(i);
        i--;
    }
    ReadUserJson(current);
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::showListWidgetMenuSlot(QPoint)
{
    m_contextMenu->exec(QCursor::pos());
}

void MainWindow::DeleteUserSet()
{
    int cur = ui->listWidget->currentRow()-1;
    QString key = ui->listWidget->currentItem()->text();

    QListWidgetItem *item = ui->listWidget->currentItem();

    ui->listWidget->removeItemWidget(item);
    delete item;

    if(cur>1&&cur<ui->listWidget->count()-1)
        ui->listWidget->item(cur)->setSelected(true);

    QFile srcFile(UserFileName);
    if(!srcFile.open(QFile::ReadWrite))
    {
        qDebug() << "can't open the file!";
        return;
    }

    QJsonParseError error;
    doc = QJsonDocument::fromJson(srcFile.readAll(), &error);

    if (!doc.isNull() && (error.error == QJsonParseError::NoError))
    {
        if (doc.isObject())
        {
            Userjson = {};
            QJsonObject object = doc.object();
            QStringList keyList = object.keys();
            if(keyList.contains(key))
            {
                object.remove(key);
                Userjson = object;
            }
        }
        srcFile.close();
        srcFile.remove();
        srcFile.open(QFile::ReadWrite);

        doc.setObject(Userjson);
        QByteArray byteArray = doc.toJson(QJsonDocument::Compact);
        QString strJson(byteArray);
        QTextStream in(&srcFile);
        in<<strJson;
        in.flush();

    }
    srcFile.close();

    for(int i=0;i<ui->userWidget->rowCount();++i)
    {
        ui->userWidget->removeRow(i);
        i--;
    }
}
