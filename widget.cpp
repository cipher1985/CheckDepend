#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QProcess>
#include <QFileDialog>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QSplitter>

#include <QMenu>
#include <QClipboard>
#include <QDebug>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //设置执行程序
    m_process = new QProcess(this);
    m_process->setReadChannel(QProcess::StandardOutput);
    m_process->setProgram(
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("dumpbin.exe"));
    //设置数据模型
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList()<<u8"检测文件");
    ui->treeView->setModel(m_model);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &pos){
        Q_UNUSED(pos);
        QModelIndex index = ui->treeView->currentIndex();
        QString fileName = ui->treeView->model()->data(index).toString();
        if(fileName.isEmpty())
            return;
        QMenu menu;
        menu.addAction(QString(u8"复制 \"%1\" 文件名").arg(fileName));
        connect(&menu, &QMenu::triggered, this, [fileName] {
            QApplication::clipboard()->setText(fileName);
        });
        menu.exec(QCursor::pos());
    });
    //设置拖拽
    this->setAcceptDrops(true);
    //设置分割
    QSplitter* splitter = new QSplitter;
    ui->verticalLayout->insertWidget(1, splitter);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(ui->plainTextEdit);
    splitter->addWidget(ui->treeView);
    splitter->setSizes(QList<int>()<<25000<<50000);
    //过滤用数据模型
    m_modelX86 = new QStandardItemModel(this);
    m_modelX86->setHorizontalHeaderLabels(QStringList()<<u8"检测文件");
    m_modelX64 = new QStandardItemModel(this);
    m_modelX64->setHorizontalHeaderLabels(QStringList()<<u8"检测文件");
}

Widget::~Widget()
{
    delete ui;
}

bool Widget::saveResFile(const QString &resFile, const QString &saveFile)
{
    QFile file(resFile);
    if(!file.open(QFile::ReadOnly))
        return false;
    QByteArray btyes = file.readAll();
    file.close();
    file.setFileName(saveFile);
    if(!file.open(QFile::WriteOnly))
        return false;
    file.write(btyes);
    file.close();
    return true;
}

QString Widget::detectBits(const QString &pathFile)
{
    m_process->setArguments(QStringList()<<"/headers"<<pathFile);
    m_process->start();
    m_process->waitForFinished();
    QString data = m_process->readAll();
    //查找表头数据
    QString findData = "FILE HEADER VALUES";
    int loc = data.indexOf(findData);
    if(loc == -1)
        return QString();
    data = data.remove(0, loc + findData.count());
    data = data.trimmed();
    //获得首行数据
    loc = data.indexOf("\r\n");
    data = data.left(loc);
    data = data.trimmed();
    data = data.simplified();
    //移除首空格前数据
    data = data.remove(0, data.lastIndexOf(' ') + 1);
    return data;
}

QStringList Widget::detectDepend(const QString &pathFile)
{
    m_process->setArguments(QStringList()<<"/dependents"<<pathFile);
    m_process->start();
    m_process->waitForFinished();
    QString data = m_process->readAll();
    //查找依赖数据
    QString findData = "Image has the following dependencies:";
    int loc = data.indexOf(findData);
    if(loc == -1)
        return QStringList();
    data = data.remove(0, loc + findData.count());
    data = data.trimmed();
    //截取所有依赖Dll数据
    loc = data.indexOf("Summary\r\n\r\n");
    data = data.left(loc);
    data = data.trimmed();
    data = data.simplified();
    //分割所有dll
    return data.split(' ');
}
void Widget::on_pushButton_brower_file_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(u8"EXE或DLL文件 (*.exe;*.dll)");
    dialog.setWindowTitle(u8"选择文件");
    if (dialog.exec() == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if(fileNames.count() == 0)
            return;
        m_listFiles.clear();
        for (auto& i : fileNames) {
            QFileInfo info(i);
            m_listFiles.append(info);
        }
        uploadFileList();
    }
}

void Widget::on_pushButton_brower_folder_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle(u8"选择文件夹");
    if (dialog.exec() == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if(fileNames.count() == 0)
            return;
        m_listFiles.clear();
        for (auto& i : fileNames) {
            QFileInfo info(i);
            m_listFiles.append(info);
        }
        uploadFileList();
    }
}

void Widget::detectFile(const QString &pathFile, bool isDetectBits, bool isDetectDepend)
{
    if(!isDetectBits && !isDetectDepend)
        return;
    QFileInfo fileInfo(pathFile);
    QStandardItem* curItem;
    QStandardItem* filterItem;
    bool isX86 = false;
    if(isDetectBits) {
        QString bits = detectBits(fileInfo.absoluteFilePath());
            curItem = new QStandardItem(QString("%1 %2").arg(fileInfo.fileName(), bits));
            filterItem = new QStandardItem(QString("%1 %2").arg(fileInfo.fileName(), bits));
            isX86 = bits.contains("x86");
        } else {
            curItem = new QStandardItem(fileInfo.fileName());
            filterItem = new QStandardItem(fileInfo.fileName());
        }
        if(isDetectDepend) {
            QStringList dependList = detectDepend(fileInfo.absoluteFilePath());
            curItem->setToolTip(fileInfo.absoluteFilePath());
            filterItem->setToolTip(fileInfo.absoluteFilePath());
        for (auto& i : dependList) {
            curItem->appendRow(new QStandardItem(i));
            filterItem->appendRow(new QStandardItem(i));
        }
    }
    m_model->appendRow(curItem);

    if(isX86) {
        m_modelX86->appendRow(filterItem);
    } else {
        m_modelX64->appendRow(filterItem);
    }
}

void Widget::detectFileList()
{
    m_model->clear();
    m_modelX86->clear();
    m_modelX64->clear();
    for (auto& i : m_listFiles) {
        if(i.isDir()) {
            // 获取所有文件名
            QDir dir = QDir(i.absoluteFilePath());
            dir.setFilter(QDir::Files);
            dir.setSorting(QDir::Name);
            dir.setNameFilters(QStringList()<<"*.exe"<<"*.dll");
            QFileInfoList fileList = dir.entryInfoList();
            for (auto& itor : fileList) {
                if(itor.isFile()) {
                    detectFile(itor.absoluteFilePath());
                }
            }
        } else {
            detectFile(i.absoluteFilePath());
        }
    }
}

void Widget::on_pushButton_detect_clicked()
{
    detectFileList();
}

void Widget::on_radioButton_all_clicked()
{
    m_showMode = 0;
    updateShowFilter();
}
void Widget::on_radioButton_x64_clicked()
{
    m_showMode = 1;
    updateShowFilter();
}

void Widget::on_radioButton_x86_clicked()
{
    m_showMode = 2;
    updateShowFilter();
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasUrls())
    {
        QList<QUrl> urls = mimeData->urls();
        for (auto& i : urls) {
            QFileInfo info(i.toLocalFile());
            QString ext = info.suffix().toLower();
            if(!(ext == "exe" || ext == "dll" || info.isDir())) {
                event->ignore();
                return;
            }
        }
        event->acceptProposedAction();
        event->setDropAction(Qt::LinkAction);
        event->accept();
        return;
    }
    event->ignore();
}

void Widget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasUrls())
    {
        m_listFiles.clear();
        QList<QUrl> urls = mimeData->urls();
        for (auto& i : urls) {
            QFileInfo info(i.toLocalFile());
            QString ext = info.suffix().toLower();
            if(ext == "exe" || ext == "dll" || info.isDir())
                m_listFiles.append(info);
        }
        uploadFileList();
    }
}

void Widget::uploadFileList()
{
    ui->plainTextEdit->clear();
    for(auto& i : m_listFiles) {
        ui->plainTextEdit->appendPlainText(i.absoluteFilePath());
    }
}

void Widget::updateShowFilter()
{
    switch (m_showMode) {
    case 1://x64
        ui->treeView->setModel(m_modelX64);
        break;
    case 2://x86
        ui->treeView->setModel(m_modelX86);
        break;
    default:
        ui->treeView->setModel(m_model);
        break;
    }
}
