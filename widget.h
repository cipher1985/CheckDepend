#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>
#include <QFileInfo>
#include <QStandardItemModel>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QString detectBits(const QString& pathFile);
    QStringList detectDepend(const QString& pathFile);
public:
    static bool saveResFile(const QString& resFile, const QString& saveFile);
private slots:
    void on_pushButton_brower_file_clicked();
    void on_pushButton_brower_folder_clicked();
    void on_pushButton_detect_clicked();
    void on_radioButton_all_clicked();
    void on_radioButton_x86_clicked();
    void on_radioButton_x64_clicked();
private:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void uploadFileList();
    void detectFile(const QString& pathFile,
        bool isDetectBits = true, bool isDetectDepend = true);
    void detectFileList();
    void updateShowFilter();
    Ui::Widget *ui;
    QProcess* m_process{};
    QStandardItemModel *m_model{};
    QStandardItemModel *m_modelX86{};
    QStandardItemModel *m_modelX64{};
    QList<QFileInfo> m_listFiles;
    int m_showMode = 0;
};
#endif // WIDGET_H
