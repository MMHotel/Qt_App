#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
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

    //设置主界面大小无法改变
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedSize(this->width(), this->height());

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
    connect(ui->lineEdit,&QLineEdit::returnPressed,this,&MainWindow::AddUserSet);
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

//打开对应配置文件，并写入Doc中
void MainWindow::ReadFile(QString t_FileName)
{
    QFile srcFile(t_FileName);
    if(!srcFile.open(QFile::ReadWrite))
    {
        qDebug() << "can't open the file!";
        return;
    }

    QJsonParseError error;
    Doc = QJsonDocument::fromJson(srcFile.readAll(), &error);

    srcFile.close();
}

//将Json对象写入对应配置文件中
void MainWindow::WriteFile(QString t_FileName,QJsonObject t_Json)
{
    QFile srcFile(t_FileName);
    srcFile.remove();

    if(!srcFile.open(QFile::ReadWrite))
    {
        qDebug() << "can't open the file!";
        return;
    }

    Doc.setObject(t_Json);
    QByteArray byteArray = Doc.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    QTextStream in(&srcFile);
    in<<strJson;
    in.flush();

    srcFile.close();
}

//构建基础Json数组
void MainWindow::JsonInsert(QString Name,QString Path,QString Set,int Time)
{
    NameArr.append(Name);
    PathArr.append(Path);
    SetArr.append(Set);
    TimeArr.append(Time);
}

//创建JsonObject对象
void MainWindow::CreateJson(QTableWidget *Twg)
{
    //数组清空
    NameArr = {};
    PathArr = {};
    SetArr = {};
    TimeArr = {};

    Json = {};

    //获取Cur_Twg里的信息，放进对应Array
    for(int i=0;i<Twg->rowCount();++i)
    {
        JsonInsert(Twg->item(i,1)->text(),Twg->item(i,2)->text(),Twg->item(i,3)->text(),Twg->item(i,4)->text().toInt());
    }
    Json.insert("Name",QJsonValue(NameArr));
    Json.insert("Path",QJsonValue(PathArr));
    Json.insert("Set",QJsonValue(SetArr));
    Json.insert("Time",QJsonValue(TimeArr));
}

//创建个人配置Json对象
void MainWindow::UserJsonInsert(QString ObjName)
{
    CreateJson(ui->userWidget);
    UserJson.insert(ObjName,QJsonValue(Json));
}

void MainWindow::SaveAll()
{
    CreateJson(ui->tableWidget);
    WriteFile(FileName,Json);
}

//保存个人配置Json
void MainWindow::SaveUserSet(QListWidgetItem *current,QListWidgetItem *previous)
{
    QString ObjName;
    if(previous == nullptr)
    {
        ObjName = current->text();
    }
    else
    {
        ObjName = previous->text();
    }

    ReadFile(UserFileName);

    if (!Doc.isNull())
    {
        if (Doc.isObject())
        {
            UserJson = {};
            QJsonObject object = Doc.object();
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
                        UserJson.insert(keyList.at(i),QJsonValue(object.value(keyList.at(i)).toObject()));
                    }
                }
            }
            else
            {
                UserJson = object;
                UserJsonInsert(ObjName);
            }
        }
    }
    else if(Doc.isNull())
    {
        UserJson = {};
        UserJsonInsert(ObjName);
    }
    WriteFile(UserFileName,UserJson);
}

void MainWindow::ResJson(QJsonObject object,QTableWidget *twg)
{
    QJsonArray NameArray = object.value("Name").toArray();
    QJsonArray PathArray = object.value("Path").toArray();
    QJsonArray SetArray = object.value("Set").toArray();
    QJsonArray TimeArray = object.value("Time").toArray();
    int nSize = NameArray.size();
    for (int i = 0; i < nSize; ++i)
    {
        Tbvaddline(twg,NameArray.at(i).toString(),PathArray.at(i).toString(),SetArray.at(i).toString(),TimeArray.at(i).toInt());
    }
}

//读取主列表配置
void MainWindow::ReadJson()
{
    ReadFile(FileName);
    if (!Doc.isNull())
    {
        if (Doc.isObject())
        {
            QJsonObject object = Doc.object();
            ResJson(object,ui->tableWidget);
        }
    }
}

//读取用户配置文件
void MainWindow::ReadUserJson(QListWidgetItem *current)
{
    ReadFile(UserFileName);

    if (!Doc.isNull())
    {
        if(Doc.isObject())
        {
            QJsonObject object = Doc.object();
            QStringList strlist = object.keys();

            if(current != nullptr)
            {
                QString ObjName = current->text();
                if(object.contains(ObjName))
                {
                    QJsonObject userobj = object.value(ObjName).toObject();
                    ResJson(userobj,ui->userWidget);
                }
            }
            else
            {
                ui->listWidget->addItems(strlist);
                QString ObjName = ui->listWidget->item(0)->text();
                if(object.contains(ObjName))
                {
                    QJsonObject userobj = object.value(ObjName).toObject();
                    ResJson(userobj,ui->userWidget);
                }
            }
        }
    }
}

//初始化tableWidget
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
    ui->tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignHCenter);

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
    ui->userWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignHCenter);
}

//TableWidget插入行
void MainWindow::Tbvaddline(QTableWidget *twg,QString Name,QString Path,QString Set,int Time)
{
    if(Line_exist(Name,Path,twg)==0)
    {
        QFileInfo finfo(Path);
        QFileIconProvider icon_pri;
        QIcon icon = icon_pri.icon(finfo);

        QTableWidgetItem *check=new QTableWidgetItem;

        QTableWidgetItem *N,*P,*S,*T;

        N = new QTableWidgetItem(icon,Name);
        P = new QTableWidgetItem(Path);
        S = new QTableWidgetItem(Set);
        T = new QTableWidgetItem(QString::number(Time));

        check->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        check->setCheckState (Qt::Checked);
        int rowcount = twg->rowCount();
        twg->insertRow(rowcount);
        twg->setItem(rowcount,0,check);
        twg->setItem(rowcount,1,N);
        twg->setItem(rowcount,2,P);
        twg->setItem(rowcount,3,S);
        twg->setItem(rowcount,4,T);
    }
    else
    {
        QMessageBox::warning(this, tr("注意"),tr("请不要重复添加"),QMessageBox::Yes);
    }

}

//tableWidget列交换
void MainWindow::ItemSwap(int differ)
{
    QString up_t,bottom_t;
    QIcon icon;
    QTableWidget *twg;
    if(ui->tabWidget->currentIndex()==0)
    {
        twg = ui->tableWidget;
    }
    else
    {
        twg = ui->userWidget;
    }
    int row = twg->currentRow();
    if(row == 0 && differ == -1)
    {
        return;
    }
    else if(row == twg->rowCount() && differ == 1)
    {
        return;
    }
    else
    {
        for(int i=1;i<twg->columnCount();i++)
        {
            icon = twg->item(row+differ,i)->icon();
            up_t = twg->item(row+differ,i)->text();
            bottom_t = twg->item(row,i)->text();
            twg->setItem(row+differ,i,new QTableWidgetItem(twg->item(row,i)->icon(),bottom_t));
            twg->setItem(row,i,new QTableWidgetItem(icon,up_t));
        }
    }
}

void MainWindow::ItemUp()
{
    ItemSwap(-1);
}

void MainWindow::ItemDown()
{
    ItemSwap(1);
}

//列表程序启动函数
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
            QTimer::singleShot(time*100, &eventloop, SLOT(quit()));
            eventloop.exec();
        }
    }

}

int MainWindow::Line_exist(QString Name,QString Path,QTableWidget *twg)
{
    if(Path==""&&twg==nullptr)
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
    else
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
}

//拖拽事件响应函数
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //如果为文件，则支持拖放
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

//拖拽事件处理函数
void MainWindow::dropEvent(QDropEvent *event)
{
    int count = ui->listWidget->count();
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;
    //多个文件取第一个来进行后面的操作
    for(int i=0;i<urls.size();i++)
    {
        QString file = urls.at(i).toLocalFile();
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
                if(fileinfo.completeSuffix() != "exe" && fileinfo.completeSuffix() != "lnk")
                {
                    qDebug() << fileinfo.completeSuffix();
                    continue;
                }
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
    SaveUserSet(ui->listWidget->item(count-1),nullptr);
    SaveAll();
}

//添加一个程序或者快捷方式进入列表
void MainWindow::AddNew()
{
    QString curPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);  //获取桌面目录
    QString dlgTitle = "选择一个文件";
    QString filter = "程序文件(*.lnk *.exe)";
    QString aFileName = QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    int count = ui->listWidget->count();
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
    SaveUserSet(ui->listWidget->item(count-1),nullptr);
    SaveAll();
}

void MainWindow::AddUserSet()
{
    QString Name;

    //清空userWidget中的内容
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
            SaveUserSet(ui->listWidget->item(count-1),nullptr);
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

void MainWindow::Delete()
{
    QTableWidget *twg;
    QMessageBox::StandardButton ch=QMessageBox::question(this,tr("注意"),tr("确认需要删除选中条目"),QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
    if(ch == QMessageBox::Yes)
    {
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
                twg->removeRow(i);
                i--;
            }
        }
    }
    SaveAll();
}

//用户配置的删除函数
void MainWindow::DeleteUserSet()
{
    int cur = ui->listWidget->currentRow()-1;
    QString key = ui->listWidget->currentItem()->text();

    QListWidgetItem *item = ui->listWidget->currentItem();

    if(cur>0)
        ui->listWidget->item(cur)->setSelected(true);

    ui->listWidget->removeItemWidget(item);
    delete item;

    ReadFile(UserFileName);
    if (!Doc.isNull())
    {
        if (Doc.isObject())
        {
            UserJson = {};
            QJsonObject object = Doc.object();
            QStringList keyList = object.keys();
            if(keyList.contains(key))
            {
                object.remove(key);
                UserJson = object;
            }
        }

        WriteFile(UserFileName,UserJson);
    }
}

//复选框控制函数
void MainWindow::ChangeCheckStatus(bool status)
{
    QTableWidget *twg;
    if(ui->tabWidget->currentIndex()==0)
    {
        twg = ui->tableWidget;
    }
    else
    {
        twg = ui->userWidget;
    }
    if(status==true)
    {
        for(int i=0;i<twg->rowCount();++i)
        {
            twg->item(i,0)->setCheckState(Qt::Checked);
        }
    }
    else
    {
        for(int i=0;i<twg->rowCount();++i)
        {
            twg->item(i,0)->setCheckState(Qt::Unchecked);
        }
    }
}

//ListWidget 右键菜单
void MainWindow::showListWidgetMenuSlot(QPoint)
{
    m_contextMenu->exec(QCursor::pos());
}

//ListWidget选择状态切换控制
void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current == nullptr)
    {
        return;
    }
    else if(previous==nullptr&&current!=nullptr)
    {
        for(int i=0;i<ui->userWidget->rowCount();++i)
        {
            ui->userWidget->removeRow(i);
            i--;
        }
        ReadUserJson(current);
        ui->tabWidget->setCurrentIndex(1);
    }
    else
    {
        SaveUserSet(current,previous);
        for(int i=0;i<ui->userWidget->rowCount();++i)
        {
            ui->userWidget->removeRow(i);
            i--;
        }
        ReadUserJson(current);
        ui->tabWidget->setCurrentIndex(1);
    }
}
