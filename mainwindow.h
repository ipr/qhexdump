#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QFile>

#include "HexFileModel.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void FileSelected(QString szFile);
	void ResizeColumns();

public slots:
    void FileSelection(QString szNewFile);

private:
    Ui::MainWindow *m_pUi;
	CHexFileModel *m_pModel;
	QString m_szBaseTitle;

private slots:
    void on_actionAbout_triggered();
    void on_actionChooseFile_triggered();
};

#endif // MAINWINDOW_H
