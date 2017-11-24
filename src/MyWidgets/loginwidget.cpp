﻿#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "CallDLL/callunifyloginsrv.h"
#include <QDebug>
#include "basemainwidget.h"
#include <QDomNodeList>
#include "settings.h"
#include <QMessageBox>
#include <QDir>
#include "videostudiolog.h"
LoginWidget::LoginWidget(QWidget *parent) :
    BaseMainWidget(parent),
    ui(new Ui::LoginWidget),
    m_bIsworking(false),
    m_nSearch(0)
{
    ui->setupUi(this);
    // this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏

    m_strSysID = ("00000000-0000-0000-0000-000000000003");
    QString info;
    info.append("系统ID为：");
    info.append(m_strSysID);
    LOG(info,"INFO");
    m_strJoincode = ("75c6c9bb17432454edf676f233800bb2");
    info = "安全码为：";
    info.append(m_strJoincode);
    LOG(info,"INFO");
    m_strUrl = ("http://192.1.2.202:3000/ConfigCenter/ConfigurationCenterService.asmx");
    info = "配置中心地址：";
    info.append(m_strUrl);
    LOG(info,"INFO");
    //初始化
    Init();
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::Init()
{
    //检查配置文件
    GetInitFileInfo();
    if(m_nSearch == 0)
    {
        ui->widget_15->hide();
        this->resize(112,360);
    }
    //删除临时文件
    QString strLoadPath = QCoreApplication::applicationDirPath()+"/Temp";
    DelDir(strLoadPath);
    //初始化
    CallUnifyLoginSrv::instance()->Init(m_strSysID,m_strJoincode,m_strUrl);

    //设置日志类型
    CallUnifyLoginSrv::instance()->SetLogType(ELT_ALL);
    //自动登录
    QString strMoudle("VideoStudio");
    wchar_t pszUserToken[256];
    memset(pszUserToken,0,256 * sizeof(wchar_t));
    int nSize =256;
    wchar_t pUserID[256];
    memset(pUserID,0,256 * sizeof(wchar_t));
    int nIDSize =256;
    int ret = CallUnifyLoginSrv::instance()->AvoidLoginV2(strMoudle,pszUserToken,&nSize,pUserID,&nIDSize);
    if(ret == ERT_TRUE)
    {
        m_loginType = ERT_TRUE;
        qDebug() << "自动登录成功";
        LOG("自动登录成功","INFO");
        wchar_t pszTicket[256];
        memset(pszTicket,0,256 * sizeof(wchar_t));
        int nsize =256;
        ret = CallUnifyLoginSrv::instance()->GetTicket(pszTicket,&nsize);
        if(ret == ERT_TRUE)
        {
            qDebug() << QStringLiteral("GetTicket succeed");
            LOG("获取票据成功","INFO");
            m_Ticket = QString::fromWCharArray(pszTicket);
        }
        SetLoginSucceed(pUserID,pszUserToken);
    }
    if(ret == ERT_NewTGT)
    {
        m_loginType = ERT_NewTGT;//需要注销
        qDebug() << "自动登录成功";
        LOG("自动登录成功","INFO");
        wchar_t pszTicket[256];
        memset(pszTicket,0,256 * sizeof(wchar_t));
        int nsize =256;
        ret = CallUnifyLoginSrv::instance()->GetTicket(pszTicket,&nsize);
        if(ret == ERT_TRUE)
        {
            qDebug() << QStringLiteral("GetTicket succeed");
            LOG("获取票据成功","INFO");
            m_Ticket = QString::fromWCharArray(pszTicket);
        }
        SetLoginSucceed(pUserID,pszUserToken);
    }

}

void LoginWidget::SetLoginSucceed(wchar_t *pUserID, wchar_t *pszUserToken)
{
    m_userID= QString::fromWCharArray(pUserID);
    m_UserToken= QString::fromWCharArray(pszUserToken);
    ui->pushButton_login->setEnabled(false);
    ui->pushButton_exit->setEnabled(true);
    ui->pushButton_open->setEnabled(true);
    ui->pushButton_save->setEnabled(true);
    ui->pushButton_send->setEnabled(true);
    ui->pushButton_saveoth->setEnabled(true);
    ui->pushButton_openvideo->setEnabled(true);
    ui->pushButton_savevideo->setEnabled(true);
    ui->pushButton_sendvideo->setEnabled(true);
    ui->pushButton_sendOthvideo->setEnabled(true);
    ui->pushButton_search->setEnabled(true);

    //初始化CloudDiskInterface
    CloudDiskInterface::instance()->Init(m_strSysID,m_strUrl,m_userID,m_strJoincode);
    CloudDiskInterface::instance()->SetLogType(ELT_ALL);
    CloudDiskInterface::instance()->SetToken(m_Ticket,m_UserToken);
}

bool LoginWidget::GetListInfo(QString strBuffer,ResourceInfo &InfoList)
{
    QDomDocument doc;
    doc.setContent(strBuffer);
    QDomNode node = doc.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement(); // try to convert the node to an element.
        if(!element.isNull())
        {
            qDebug()<<element.tagName() << ":" << element.text();
            QDomNode nodeson = element.firstChild();
            while(!nodeson.isNull())
            {
                QDomElement elementson = nodeson.toElement();
                if(!elementson.isNull())
                {
                    qDebug()<< "---" <<elementson.tagName();
                    QDomNode n = elementson.firstChild();
                    while(!n.isNull())
                    {
                        QDomElement e = n.toElement();
                        if(!e.isNull())
                        {
                            qDebug()<< "---" <<e.tagName();
                            if (e.tagName() == "GUID")
                            {
                                InfoList.m_strResourceID = e.text();
                            }
                            if (e.tagName() =="ParentID")
                            {
                                InfoList.m_strParentID = e.text();
                            }
                            if (e.tagName() =="Name")
                            {
                                InfoList.m_strResourceName = e.text();
                            }
                            if (e.tagName() =="Owner")
                            {
                                InfoList.m_strOwnerID = e.text();
                            }
                            if (e.tagName() =="OwnerType")
                            {
                                InfoList.m_strOwnerType = e.text();
                            }
                        }
                        n = n.nextSibling();
                    }
                }
                nodeson = nodeson.nextSibling();
            }
        }
        node = node.nextSibling();
    }
    if(InfoList.m_strResourceID == "")
    {
        return false;
    }
    return true;
}

bool LoginWidget::SaveProject(QString strFilePath,int ntype,int projectType)
{
    //获取资源选中窗口的信息
    if(m_ProResourceInfo.m_strResourceID == "" || projectType ==EV_ShotCut)
    {
        return SaveProjectOther(strFilePath);
    }
    wchar_t pBuff[1024];
    memset(pBuff,0,1024 * sizeof(wchar_t));
    int npoint= strFilePath.lastIndexOf(QRegExp("/"));
    int n= strFilePath.lastIndexOf('.');
    QString filePath = strFilePath.mid(npoint+1,strFilePath.length()-npoint);
    QString fileType =strFilePath.mid(n+1,strFilePath.length()-n);
    //预保存资源到云盘
    int nJobID = CloudDiskInterface::instance()->PreUploadResource(m_ProResourceInfo.m_strResourceName,
                                                                   m_ProResourceInfo.m_strResourceID,
                                                                   strFilePath,
                                                                   filePath,
                                                                   EREST_Other,
                                                                   fileType,
                                                                   NULL,
                                                                   EWF_WEB_SAVE_COVER_PROMPT);
    if(nJobID <= 0)
    {
        qDebug() <<"save failed";
        LOG("预保存工程失败","ERROR");
        return false;
    }
    AddFile(strFilePath,nJobID,"",1);
    int ret = CloudDiskInterface::instance()->UploadResource(nJobID,m_strBuffInfo);
    if(ret == ERT_TRUE)
    {
        qDebug() <<"save succeed";
        LOG("保存工程成功","INFO");
        if(ntype != SF_SaveSend)
        {
            QMessageBox::about(NULL, QStringLiteral("成功"), QStringLiteral("     工程保存成功      "));
        }
        return true;
    }else{
        qDebug() <<"save failed";
        LOG("保存工程失败","ERROR");
        return false;
    }
}

bool LoginWidget::SaveProjectOther(QString strFilePath)
{

    int npoint= strFilePath.lastIndexOf(QRegExp("/"));
    int tpoint = strFilePath.lastIndexOf('.');
    QString filePath = strFilePath.mid(npoint+1,strFilePath.length()-npoint);
    QString fileName = strFilePath.mid(npoint+1,tpoint-npoint-1);
    //打开选择窗口
    int nsize = CloudDiskInterface::instance()->ResourceDialog(0,
                                                               "视频编辑站",
                                                               fileName,
                                                               ERMT_Other,
                                                               "mlt",
                                                               NULL,EWF_WEB_SAVE_COVER_PROMPT);
    if(nsize < ERT_TRUE)
    {
        qDebug() <<QStringLiteral("CloudDiskInterface failed");
        LOG("工程另存为 打开选择窗口失败","ERROR");
        return false;
    }
    if(nsize == ERT_CANCEL)
    {
        qDebug() <<QStringLiteral("cancle");
        LOG("工程另存为 打开选择窗口取消了操作","INFO");
        return false;
    }
    if(nsize > ERT_CANCEL)
    {
        LOG("工程另存为 打开选择窗口成功","INFO");
        //获取资源选中窗口的信息
        wchar_t pBuff[1024];
        memset(pBuff,0,1024 * sizeof(wchar_t));
        CloudDiskInterface::instance()->GetResourceList(pBuff, 1024);
        m_strBuffInfo = QString::fromWCharArray(pBuff);
        GetListInfo(m_strBuffInfo,m_ProResourceInfo);
        //预保存资源到云盘
        int nJobID = CloudDiskInterface::instance()->PreUploadResource(m_ProResourceInfo.m_strResourceName,
                                                                       m_ProResourceInfo.m_strResourceID,
                                                                       strFilePath,
                                                                       filePath,
                                                                       EREST_Other,
                                                                       "mlt",
                                                                       NULL,
                                                                       EWF_WEB_SAVE_COVER_PROMPT);
        if(nJobID <= 0)
        {
            qDebug() <<"save failed";
            LOG("工程另存为 预保存工程失败","ERROR");
            return false;;
        }
        AddFile(strFilePath,nJobID,"",1);
        int ret = CloudDiskInterface::instance()->UploadResource(nJobID,m_strBuffInfo);
        if(ret == ERT_TRUE)
        {
            qDebug() <<"save succeed";
            LOG("工程另存为 保存工程成功","INFO");
            QMessageBox::about(NULL, QStringLiteral("成功"), QStringLiteral("     工程保存成功      "));
            return true;
        }
    }
}

void LoginWidget::SaveVideo(QString strFilePath)
{
    bool bIsUpload = false;
    int npoint= strFilePath.lastIndexOf(QRegExp("/"));
    int tpoint = strFilePath.lastIndexOf('.');
    //   QString filePath = strFilePath.mid(npoint+1,strFilePath.length()-npoint);
    QString fileName = strFilePath.mid(npoint+1,tpoint-npoint-1);
    QString fileType =strFilePath.mid(tpoint+1,strFilePath.length()-tpoint);
    //打开选择窗口
    int nsize = CloudDiskInterface::instance()->ResourceDialog(0,
                                                               "视频编辑",
                                                               fileName,
                                                               ERMT_ALL,
                                                               fileType,
                                                               NULL,
                                                               EWF_WEB_OPEN_MUTI_SELECT);
    if(nsize < ERT_TRUE)
    {
        qDebug() <<QStringLiteral("CloudDiskInterface failed");
        LOG("保存资源 打开选择窗口失败","ERROR");
    }
    if(nsize == ERT_CANCEL)
    {
        qDebug() <<QStringLiteral("cancle");
        LOG("保存资源 打开选择窗口取消了操作","INFO");
    }
    if(nsize > ERT_CANCEL)
    {
        m_bIsworking = true;
        bIsUpload = true;
        LOG("保存资源 打开选择窗口成功","INFO");
    }
    emit Signal_UploadVideo(bIsUpload);
}

void LoginWidget::SendVideoNoDlg(QString strFilePath)
{
    bool bIsUpload = false;
    wchar_t pBuff[1024];
    memset(pBuff,0,1024 * sizeof(wchar_t));
    int npoint= strFilePath.lastIndexOf(QRegExp("/"));
    int n= strFilePath.lastIndexOf('.');
    //    QString filePath = strFilePath.mid(npoint+1,strFilePath.length()-npoint);
    QString fileType =strFilePath.mid(n+1,strFilePath.length()-n);
    QString fileName = strFilePath.mid(npoint+1,n-npoint-1);
    qDebug() <<" filename =" << fileName;
    qDebug() <<"fileType ="<<fileType;
    //打开选择窗口
    int nsize = CloudDiskInterface::instance()->ResourceNoDialog("VideoStudio",
                                                                 "视频编辑站临时目录",
                                                                 fileName,
                                                                 fileType);

    if(nsize < ERT_TRUE)
    {
        qDebug() <<QStringLiteral("CloudDiskInterface failed");
        LOG("发送资源 打开选择窗口失败","ERROR");
    }
    if(nsize == ERT_CANCEL)
    {
        qDebug() <<QStringLiteral("cancle");
        LOG("发送资源 打开选择窗口取消了操作","INFO");
    }
    if(nsize > ERT_CANCEL)
    {
        m_bIsworking = true;
        bIsUpload = true;
        LOG("发送资源 打开选择窗口成功","INFO");
    }
    emit Signal_UploadVideo(bIsUpload);
}

void LoginWidget::SendProjectNoDlg(QString strFilePath,int projectType)
{
    wchar_t pBuff[1024];
    memset(pBuff,0,1024 * sizeof(wchar_t));
    int npoint= strFilePath.lastIndexOf(QRegExp("/"));
    int n= strFilePath.lastIndexOf('.');
    QString filePath = strFilePath.mid(npoint+1,strFilePath.length()-npoint);
    QString fileType =strFilePath.mid(n+1,strFilePath.length()-n);
    QString fileName = strFilePath.mid(npoint+1,n-npoint-1);
    qDebug() <<" filename =" << fileName;
    qDebug() <<"fileType ="<<fileType;
    //获取资源选中窗口的信息
    if(m_ProResourceInfo.m_strResourceID == "" || projectType ==EV_ShotCut)
    {
        //打开选择窗口
        int nsize = CloudDiskInterface::instance()->ResourceNoDialog("VideoStudio",
                                                                     "视频编辑站临时目录",
                                                                     fileName,
                                                                     fileType);
        if(nsize < ERT_TRUE)
        {
            qDebug() <<QStringLiteral("CloudDiskInterface failed");
            LOG("发送工程 打开选择窗口失败","ERROR");
            return;
        }
        if(nsize == ERT_CANCEL)
        {
            qDebug() <<QStringLiteral("cancle");
            LOG("发送工程 打开选择窗口取消了操作","INFO");
            return;
        }
        if(nsize > ERT_CANCEL)
        {
            LOG("发送工程 打开选择窗口成功","INFO");
            //获取资源选中窗口的信息
            CloudDiskInterface::instance()->GetResourceList(pBuff, 1024);
            qDebug() <<QString::fromWCharArray(pBuff);
            //获取资源选中窗口的信息

            CloudDiskInterface::instance()->GetResourceList(pBuff, 1024);
            QString buffInfo = QString::fromWCharArray(pBuff);
            ResourceInfo InfoList;
            GetListInfo(buffInfo,InfoList);
            //预保存资源到云盘
            int nJobID = CloudDiskInterface::instance()->PreUploadResource(InfoList.m_strResourceName,
                                                                           InfoList.m_strResourceID,
                                                                           strFilePath,
                                                                           filePath,
                                                                           EREST_Other,
                                                                           "mlt",
                                                                           NULL,
                                                                           EWF_WEB_SAVE_COVER_PROMPT);
            if(nJobID <= 0)
            {
                qDebug() <<"save failed";
                LOG("发送工程 预保存工程失败","ERROR");
                return;
            }
            AddFile(strFilePath,nJobID,"",1);
            int ret = CloudDiskInterface::instance()->UploadResource(nJobID,buffInfo);
            if(ret == ERT_TRUE)
            {
                qDebug() <<"save succeed";
                LOG("发送工程 预保存工程成功","INFO");
                //云盘内资源发送
                int t = CloudDiskInterface::instance()->LocalSend(InfoList.m_strResourceID);
                if(t == ERT_TRUE)
                {
                    qDebug()<<"LocalSend successful";
                    LOG("发送工程 发送成功","INFO");
                    QMessageBox::about(NULL, QStringLiteral("成功"), QStringLiteral("     工程发送成功      "));
                }
                else if(t < ERT_TRUE)
                {
                    LOG("发送工程 发送失败","INFO");
                    QMessageBox::critical(NULL, QStringLiteral("失败"), QStringLiteral("      工程发送失败      "));
                }
            }
        }
    }else{
        //预保存资源到云盘
        int nJobID = CloudDiskInterface::instance()->PreUploadResource(m_ProResourceInfo.m_strResourceName,
                                                                       m_ProResourceInfo.m_strResourceID,
                                                                       strFilePath,
                                                                       filePath,
                                                                       EREST_Other,
                                                                       "mlt",
                                                                       NULL,
                                                                       EWF_WEB_SAVE_COVER_PROMPT);
        if(nJobID <= 0)
        {
            qDebug() <<"save failed";
            LOG("发送工程 预保存工程失败","ERROR");
            return;
        }
        AddFile(strFilePath,nJobID,"",1);
        int ret = CloudDiskInterface::instance()->UploadResource(nJobID,m_strBuffInfo);
        if(ret == ERT_TRUE)
        {
            qDebug() <<"save succeed";
            LOG("发送工程 预保存工程成功","INFO");
            //云盘内资源发送
            int t = CloudDiskInterface::instance()->LocalSend(m_ProResourceInfo.m_strResourceID);
            if(t == ERT_TRUE)
            {
                qDebug()<<"LocalSend successful";
                LOG("发送工程 发送成功","INFO");
                QMessageBox::about(NULL, QStringLiteral("成功"), QStringLiteral("     工程发送成功      "));
            }
            else if(t < ERT_TRUE)
            {
                LOG("发送工程 发送失败","ERROR");
                QMessageBox::critical(NULL, QStringLiteral("失败"), QStringLiteral("      工程发送失败      "));
            }
        }
    }

}

bool LoginWidget::DelDir(const QString &path)
{
    if (path.isEmpty()){
        return false;
    }
    QDir dir(path);
    if(!dir.exists()){
        return true;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    foreach (QFileInfo file, fileList){ //遍历文件信息
        if (file.isFile()){ // 是文件，删除
            file.dir().remove(file.fileName());
        }else{ // 递归删除
            DelDir(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath()); // 删除文件夹
}

void LoginWidget::AddFile(QString filePath, int JobID,QString fatherPath,int type)
{
    QString path = filePath;
    QString ProFileName ="";
    if(type == 1)
    {
        int npoint= filePath.lastIndexOf(QRegExp("/"));
        path = filePath.mid(0,npoint+1);
        ProFileName = filePath.mid(npoint+1,filePath.length()-npoint);
    }
    if (path.isEmpty()){
        return ;
    }
    QDir dir(path);
    if(!dir.exists()){
        return ;
    }

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    foreach (QFileInfo file, fileList)
    {
        //遍历文件信息
        if (file.isFile())
        {
            // 是文件，添加
            QString fileName =file.fileName();
            if(fileName == ProFileName)
            {
                continue;
            }
            QString fpath = path+"/"+fileName;
            QString fName = fatherPath+fileName;
            enResourceType nfileType = GetFileType(fileName);
            int ret =  CloudDiskInterface::instance()->AddUploadFile(JobID,fpath,fName,nfileType,"");
            if(ret == 1)
            {
                qDebug()<<"AddUploadFile successful";
                QString info;
                info.append("添加工程文件 文件添加成功:%1").arg(fileName);
                LOG(info,"INFO");
            }else{
                qDebug()<<"AddUploadFile failed";
                QString info;
                info.append("添加工程文件 文件添加失败:%1").arg(fileName);
                LOG(info,"ERROR");

            }

        }
        else
        {
            // 递归
            QString fpath = fatherPath +file.fileName()+"/";
            AddFile(file.absoluteFilePath(),JobID,fpath,0);
        }
    }
    return;
}

enResourceType LoginWidget::GetFileType(QString fileName)
{
    int n= fileName.lastIndexOf('.');
    QString fileType =fileName.mid(n+1,fileName.length()-n);
    if(fileType.contains("mp3",Qt::CaseInsensitive)
            ||fileType.contains("s48",Qt::CaseInsensitive)
            ||fileType.contains("wav",Qt::CaseInsensitive))
    {
        return EREST_Audio;
    }
    if(fileType.contains("mp4",Qt::CaseInsensitive)
            ||fileType.contains("rm",Qt::CaseInsensitive)
            ||fileType.contains("flv",Qt::CaseInsensitive)
            ||fileType.contains("avi",Qt::CaseInsensitive)
            ||fileType.contains("wmv",Qt::CaseInsensitive) )
    {
        return EREST_Video;

    }else{
        return EREST_Other;
    }
}

void LoginWidget::UploadVideo(QString strFilePath)
{
    m_bIsworking = false;
    int npoint= strFilePath.lastIndexOf(QRegExp("/"));
    int tpoint = strFilePath.lastIndexOf('.');
    QString filePath = strFilePath.mid(npoint+1,strFilePath.length()-npoint);
    QString fileType =strFilePath.mid(tpoint+1,strFilePath.length()-tpoint);
    //获取资源选中窗口的信息
    wchar_t pBuff[1024];
    memset(pBuff,0,1024 * sizeof(wchar_t));
    CloudDiskInterface::instance()->GetResourceList(pBuff, 1024);
    qDebug() <<QString::fromWCharArray(pBuff);
    ResourceInfo infoList;
    GetListInfo(QString::fromWCharArray(pBuff),infoList);

    enResourceType  nType = GetFileType(fileType);
    //预保存资源到云盘
    int nJobID = CloudDiskInterface::instance()->PreUploadResource(infoList.m_strResourceName,
                                                                   infoList.m_strResourceID,
                                                                   strFilePath,
                                                                   filePath,
                                                                   nType,
                                                                   fileType,
                                                                   NULL,
                                                                   EWF_WEB_SAVE_COVER_PROMPT);
    if(nJobID <= 0)
    {
        qDebug() <<"save failed";
        LOG("上传资源 预保存失败","ERROR");
        return;
    }
    int ret = CloudDiskInterface::instance()->UploadResource(nJobID,QString::fromWCharArray(pBuff));
    if(ret == ERT_TRUE)
    {
        qDebug() <<"save succeed";
        LOG("上传资源 保存成功","INFO");
        QMessageBox::about(NULL, QStringLiteral("成功"), QStringLiteral("     视频保存成功      "));
    }
    if(ret < ERT_TRUE)
    {
        LOG("上传资源 保存失败","ERROR");
        QMessageBox::critical(NULL, QStringLiteral("失败"), QStringLiteral("      视频保存失败      "));
    }

}

void LoginWidget::UploadSendVideo(QString strFilePath)
{
    m_bIsworking = false;
    wchar_t pBuff[1024];
    memset(pBuff,0,1024 * sizeof(wchar_t));
    int npoint= strFilePath.lastIndexOf(QRegExp("/"));
    int n= strFilePath.lastIndexOf('.');
    QString filePath = strFilePath.mid(npoint+1,strFilePath.length()-npoint);
    QString fileType =strFilePath.mid(n+1,strFilePath.length()-n);
    QString fileName = strFilePath.mid(npoint+1,n-npoint-1);
    qDebug() <<" filename =" << fileName;
    qDebug() <<"fileType ="<<fileType;
    //获取资源选中窗口的信息
    CloudDiskInterface::instance()->GetResourceList(pBuff, 1024);
    qDebug() <<QString::fromWCharArray(pBuff);
    ResourceInfo infoList;
    GetListInfo(QString::fromWCharArray(pBuff),infoList);
    //预保存资源到云盘
    enResourceType  nType = GetFileType(fileType);
    int nJobID = CloudDiskInterface::instance()->PreUploadResource(infoList.m_strResourceName,
                                                                   infoList.m_strResourceID,
                                                                   strFilePath,
                                                                   filePath,
                                                                   nType,
                                                                   fileType,
                                                                   NULL,
                                                                   EWF_WEB_SAVE_COVER_PROMPT);
    if(nJobID <= 0)
    {
        qDebug() <<"save failed";
        LOG("发送资源 预保存失败","ERROR");
        return;
    }
    int ret = CloudDiskInterface::instance()->UploadResource(nJobID,QString::fromWCharArray(pBuff));
    if(ret == ERT_TRUE)
    {
        qDebug() <<"save succeed";
        LOG("发送资源 保存成功","INFO");
        //云盘内资源发送
        int t = CloudDiskInterface::instance()->LocalSend(infoList.m_strResourceID);
        if(t == ERT_TRUE)
        {
            LOG("发送资源 发送成功","INFO");
            qDebug()<<"LocalSend successful";
            QMessageBox::about(NULL, QStringLiteral("成功"), QStringLiteral("     视频发送成功      "));
        }
        if(t < ERT_TRUE)
        {
            LOG("发送资源 发送失败","ERROR");
            QMessageBox::critical(NULL, QStringLiteral("失败"), QStringLiteral("      视频发送失败      "));
        }
    }else{
         LOG("发送资源 保存失败","ERROR");
    }

}

void LoginWidget::GetInitFileInfo()
{
    QSettings *configIniRead = NULL;
    QString path = QCoreApplication::applicationDirPath();
    path.append("/VideoStudio.ini");
    configIniRead = new QSettings(path, QSettings::IniFormat);
    //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
    if(!configIniRead)
    {
        qDebug() <<"read ServerStatus.ini false";
        return ;
    }
    m_nSearch = configIniRead->value("/SETTING/search").toInt();

    //打印得到的结果
    qDebug() << "search ="<< m_nSearch;

    //读入入完成后删除指针
    delete configIniRead;
}

void LoginWidget::SetIsWorking(bool bIsWorking)
{
    m_bIsworking = bIsWorking;
}

bool LoginWidget::IsWoking()
{
    return m_bIsworking;
}

void LoginWidget::on_pushButton_login_clicked()
{
     LOG("点击登录按钮","CLICK");
    //初始化
    CallUnifyLoginSrv::instance()->Init(m_strSysID,m_strJoincode,m_strUrl);
    //登录有界面
    QString strMoudle("VideoStudio");
    wchar_t pszUserToken[256];
    memset(pszUserToken,0,256 * sizeof(wchar_t));
    int nSize =256;
    wchar_t pUserID[256];
    memset(pUserID,0,256 * sizeof(wchar_t));
    int nIDSize =256;
    int ret = CallUnifyLoginSrv::instance()->LoginV2(strMoudle,pszUserToken,&nSize,pUserID,&nIDSize);
    if(ret != ERT_TRUE)
    {
        qDebug() << ret <<"登录失败";
         LOG("登录 登录失败","ERROR");
        wchar_t pError[256];
        memset(pError,0,256 * sizeof(wchar_t));
        int nSize =256;
        ret = CallUnifyLoginSrv::instance()->GetLastError(pError,&nSize);
        qDebug() << pError << nSize;
        QString info;
        info.append("登录 获取错误信息:%1").arg(QString::fromWCharArray(pError));
        LOG(info,"ERROR");
        return;
    }
    wchar_t pszTicket[256];
    memset(pszTicket,0,256 * sizeof(wchar_t));
    int nsize =256;
    ret = CallUnifyLoginSrv::instance()->GetTicket(pszTicket,&nsize);
    if(ret == ERT_TRUE)
    {
        LOG("登录 获取票据成功","INFO");
        qDebug() << QStringLiteral("GetTicket succeed");
        m_Ticket = QString::fromWCharArray(pszTicket);
        m_loginType = ERT_NewTGT;
        SetLoginSucceed(pUserID,pszUserToken);
    }else{
        LOG("登录 获取票据失败","ERROR");
    }

}

void LoginWidget::on_pushButton_close_clicked()
{
    LOG("点击关闭按钮","CLICK");
    this->hide();
}

void LoginWidget::on_pushButton_exit_clicked()
{
    LOG("点击注销按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    //调用注销接口
    if(m_loginType == ERT_NewTGT)
    {
        CallUnifyLoginSrv::instance()->Logout();
    }
    ui->pushButton_login->setEnabled(true);
    ui->pushButton_exit->setEnabled(false);
    ui->pushButton_open->setEnabled(false);
    ui->pushButton_save->setEnabled(false);
    ui->pushButton_send->setEnabled(false);
    ui->pushButton_saveoth->setEnabled(false);
    ui->pushButton_openvideo->setEnabled(false);
    ui->pushButton_savevideo->setEnabled(false);
    ui->pushButton_sendvideo->setEnabled(false);
    ui->pushButton_sendOthvideo->setEnabled(false);
    ui->pushButton_search->setEnabled(false);
    m_userID = "";
    m_UserToken = "";
    m_Ticket = "";
    m_loginType = ERT_FALSE;
}

void LoginWidget::on_pushButton_open_clicked()
{
    LOG("点击打开工程按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    //打开选择窗口
    int nsize = CloudDiskInterface::instance()->ResourceDialog(1,
                                                               "视频编辑站",
                                                               "",
                                                               ERMT_Other,
                                                               "mlt",
                                                               NULL,
                                                               EWF_WEB_OPEN_FLAG_DEFAULT);
    if(nsize < ERT_TRUE)
    {
        qDebug() <<QStringLiteral("CloudDiskInterface failed");
        LOG("打开工程 打开选择窗口失败","ERROR");
        return;
    }
    if(nsize == ERT_CANCEL)
    {
        qDebug() <<QStringLiteral("cancle");
        LOG("打开工程 打开选择窗口取消了操作","INFO");
        return;
    }
    if(nsize > ERT_CANCEL)
    {
        //获取资源选中窗口的信息
        wchar_t pBuff[1024];
        memset(pBuff,0,1024 * sizeof(wchar_t));
        CloudDiskInterface::instance()->GetResourceList(pBuff,nsize);
        m_strBuffInfo = QString::fromWCharArray(pBuff);
        qDebug() << m_strBuffInfo;
        if(!GetListInfo(m_strBuffInfo,m_ProResourceInfo))
        {
            qDebug()<<"获取工程信息失败";
            LOG("打开工程 获取工程信息失败","ERROR");
            return;
        }

        //下载资源
        QString strLoadPath = QCoreApplication::applicationDirPath()+"/DownLoad";
        wchar_t FileListBuffer[1024 *5];
        memset(FileListBuffer,0,1024 *5 * sizeof(wchar_t));
        int nbufferSize = 1024 *5;
        int t = CloudDiskInterface::instance()->DownloadResource(m_ProResourceInfo.m_strResourceID,
                                                                 strLoadPath,
                                                                 FileListBuffer,
                                                                 nbufferSize,
                                                                 NULL,
                                                                 EDFalg_FORCE_COVER);
        if(t == ERT_TRUE)
        {
            qDebug()<<"DownloadResource successful";
            LOG("打开工程 下载工程成功","INFO");
            //加载到工程
            //获取主工程路径
            wchar_t FilePathBuffer[512];
            memset(FilePathBuffer,0,512 * sizeof(wchar_t));
            int rt =  CloudDiskInterface::instance()->GetMainFilePath(FilePathBuffer,512);
            if(rt != ERT_TRUE)
            {
                qDebug()<<"GetMainFilePath failed";
                LOG("打开工程 获取主工程文件名失败","ERROR");
            }
            QString strMainFilePath = QString::fromWCharArray(FilePathBuffer);
            QString strProjectPath = strLoadPath + "/"+m_ProResourceInfo.m_strResourceID+"/"+strMainFilePath;
            emit signal_OpenProject(strProjectPath);
        }else{
            LOG("打开工程 下载工程失败","ERROR");
        }
    }
}

void LoginWidget::on_pushButton_save_clicked()
{
    LOG("点击保存工程按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    emit signal_SaveProject(SF_Save);//代表默认保存
}


void LoginWidget::on_pushButton_saveoth_clicked()
{
    LOG("点击工程另存为按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    emit signal_SaveProject(SF_SaveOther);//代表另存为
}

void LoginWidget::on_pushButton_openvideo_clicked()
{
    LOG("点击打开资源按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    //打开选择窗口
    int nsize = CloudDiskInterface::instance()->ResourceDialog(1,
                                                               "视频编辑站",
                                                               "",
                                                               ERMT_ALL,
                                                               "",
                                                               NULL,EWF_WEB_OPEN_FLAG_DEFAULT);
    if(nsize < ERT_TRUE)
    {
        qDebug() <<QStringLiteral("CloudDiskInterface failed");
        LOG("打开资源 打开选择窗口失败","ERROR");
        return;
    }
    if(nsize == ERT_CANCEL)
    {
        qDebug() <<QStringLiteral("cancle");
        LOG("打开资源 打开选择窗口取消了操作","INFO");
        return;
    }
    if(nsize > ERT_CANCEL)
    {
        LOG("打开资源 打开选择窗口成功","INFO");
        //获取资源选中窗口的信息
        wchar_t pBuff[1024];
        memset(pBuff,0,1024 * sizeof(wchar_t));
        CloudDiskInterface::instance()->GetResourceList(pBuff,nsize);
        qDebug() <<QString::fromWCharArray(pBuff);
        ResourceInfo infoList;
        if(!GetListInfo(QString::fromWCharArray(pBuff),infoList))
        {
            qDebug()<<"获取资源信息失败";
            LOG("打开资源 获取资源信息失败","ERROR");
            return;
        }

        //下载资源
        QString strLoadPath = QCoreApplication::applicationDirPath()+"/DownLoad";
        wchar_t FileListBuffer[512];
        memset(FileListBuffer,0,512 * sizeof(wchar_t));
        int nbufferSize = 512;
        int t = CloudDiskInterface::instance()->DownloadResource(infoList.m_strResourceID,
                                                                 strLoadPath,
                                                                 FileListBuffer,
                                                                 nbufferSize,
                                                                 NULL,
                                                                 EDFalg_FORCE_COVER);
        if(t == ERT_TRUE)
        {
            LOG("打开资源 下载资源成功","INFO");
            qDebug()<<"DownloadResource successful";
            QString strVideoPath = strLoadPath + QString::fromWCharArray(FileListBuffer);
            emit signal_OpenVideo(strVideoPath);
        }else{
            LOG("打开资源 下载资源失败","ERROR");
            QMessageBox::critical(NULL, QStringLiteral("失败"), QStringLiteral("     下载资源失败      "));
        }
    }
}



void LoginWidget::on_pushButton_savevideo_clicked()
{
    LOG("点击保存资源按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    emit signal_SaveVideo(SF_SaveOther);//代表自己选择路径保存
}

void LoginWidget::on_pushButton_sendvideo_clicked()
{
    LOG("点击发送资源按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    //发送视频前先默认保存视频
    emit signal_SaveVideo(SF_SaveSend);//代表默认路径保存并发送
}

void LoginWidget::on_pushButton_sendOthvideo_clicked()
{
    LOG("点击发送其他资源按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    //打开选择窗口
    int nsize = CloudDiskInterface::instance()->ResourceDialog(3,
                                                               "视频编辑站",
                                                               "",
                                                               ERMT_ALL,
                                                               "",
                                                               NULL,
                                                               EWF_WEB_OPEN_MUTI_SELECT);
    if(nsize < ERT_TRUE)
    {
        qDebug() <<QStringLiteral("ResourceDialog failed");
        LOG("发送其他资源 打开选择窗口失败","ERROR");
        return;
    }
    if(nsize == ERT_CANCEL)
    {
        qDebug() <<QStringLiteral("cancle");
        LOG("发送其他资源 打开选择窗口取消了操作","INFO");
        return;
    }
    if(nsize > ERT_CANCEL)
    {
        LOG("发送其他资源 打开选择窗口成功","INFO");
        //获取资源选中窗口的信息
        wchar_t pBuff[1024];
        memset(pBuff,0,1024 * sizeof(wchar_t));
        CloudDiskInterface::instance()->GetResourceList(pBuff,nsize);
        qDebug() <<QString::fromWCharArray(pBuff);
        ResourceInfo infoList;
        if(!GetListInfo(QString::fromWCharArray(pBuff),infoList))
        {
            qDebug()<<"获取资源信息失败";
            LOG("发送其他资源 获取资源信息失败","ERROR");
            return;
        }

        //云盘内资源发送
        int t = CloudDiskInterface::instance()->LocalSend(infoList.m_strResourceID);
        if(t == ERT_TRUE)
        {
            qDebug()<<"LocalSend successful";
            LOG("发送其他资源 发送成功","INFO");
            QMessageBox::about(NULL, QStringLiteral("成功"), QStringLiteral("     视频发送成功      "));
        }
        if(t < ERT_TRUE)
        {
            LOG("发送其他资源 发送失败","ERROR");
            QMessageBox::critical(NULL, QStringLiteral("失败"), QStringLiteral("      视频发送失败      "));
        }
    }
}

void LoginWidget::on_pushButton_send_clicked()
{
    LOG("点击发送工程按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    emit signal_SaveProject(SF_SaveSend);//代表默认保存并发送
}

void LoginWidget::on_pushButton_search_clicked()
{
    LOG("点击搜索按钮","CLICK");
    if(m_bIsworking)
    {
        QMessageBox::warning(NULL, QStringLiteral("提示"), QStringLiteral("有任务正在进行请等待..."));
        return;
    }
    int nsize = CloudDiskInterface::instance()->SearchOpenEx(ERMT_ALL,
                                                             "",
                                                             NULL,
                                                             EWF_WEB_OPEN_FLAG_DEFAULT,
                                                             "");
    if(nsize < ERT_TRUE)
    {
        qDebug() <<QStringLiteral("ResourceDialog failed");
        LOG("搜索 打开窗口失败","ERROR");
        return;
    }
    if(nsize == ERT_CANCEL)
    {
        qDebug() <<QStringLiteral("cancle");
        LOG("搜索 打开窗口取消了操作","INFO");
        return;
    }
    if(nsize > ERT_CANCEL)
    {
        LOG("搜索 打开窗口成功","INFO");
        //获取资源选中窗口的信息
        wchar_t pBuff[1024];
        memset(pBuff,0,1024 * sizeof(wchar_t));
        CloudDiskInterface::instance()->GetResourceList(pBuff,nsize);
        qDebug() <<QString::fromWCharArray(pBuff);
        ResourceInfo infoList;
        if(!GetListInfo(QString::fromWCharArray(pBuff),infoList))
        {
            qDebug()<<"获取资源信息失败";
            LOG("搜索 获取资源信息失败","ERROR");
            return;
        }

        //下载资源
        QString strLoadPath = QCoreApplication::applicationDirPath()+"/DownLoad";
        wchar_t FileListBuffer[512];
        memset(FileListBuffer,0,512 * sizeof(wchar_t));
        int nbufferSize = 512;
        int t = CloudDiskInterface::instance()->DownloadResource(infoList.m_strResourceID,
                                                                 strLoadPath,
                                                                 FileListBuffer,
                                                                 nbufferSize,
                                                                 NULL,
                                                                 EDFalg_FORCE_COVER);
        if(t == ERT_TRUE)
        {
             LOG("搜索 下载资源成功","INFO");
            qDebug()<<"DownloadResource successful";
            QString strVideoPath = strLoadPath + QString::fromWCharArray(FileListBuffer);
            emit signal_OpenVideo(strVideoPath);
        }else{
             LOG("搜索 下载资源失败","ERROR");
            QMessageBox::critical(NULL, QStringLiteral("失败"), QStringLiteral("      下载资源失败      "));
        }
    }
}

