#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_pUi(new Ui::MainWindow),
    m_pModel(NULL),
	m_szBaseTitle()
{
    m_pUi->setupUi(this);
	m_szBaseTitle = windowTitle();
	
    connect(this, SIGNAL(FileSelected(QString)), SLOT(FileSelection(QString)));
	//connect(m_pUi->tableView, SIGNAL(activated(QModelIndex)), SLOT(ColActivated(QModelIndex)));
	connect(this, SIGNAL(ResizeColumns()), m_pUi->tableView, SLOT(resizeColumnsToContents()));
	//connect(this, SIGNAL(ShowRow(int)), m_pUi->tableView, SLOT(showRow(int)));
	
	// if file given in command line
	QStringList vCmdLine = QApplication::arguments();
	if (vCmdLine.size() > 1)
	{
		emit FileSelected(vCmdLine[1]);
	}
}

MainWindow::~MainWindow()
{
	if (m_pModel != NULL)
	{
		m_pModel->close();
		delete m_pModel;
		m_pModel = NULL;
	}
    delete m_pUi;
}

void MainWindow::FileSelection(QString szNewFile)
{
	// we are using destroying to make view "forget" the model
	if (m_pModel != NULL)
	{
		m_pModel->close();
		delete m_pModel;
		m_pModel = NULL;
	}
	
	m_pModel = new CHexFileModel(this);
	if (m_pModel->setSourceFile(szNewFile) == false)
	{
		delete m_pModel;
		m_pModel = NULL;
		
		setWindowTitle(m_szBaseTitle);
		return;
	}
	
	// use this model
	m_pUi->tableView->setModel(m_pModel);
	
	// TODO:
	// scroll to first row on change..

	// resize view to contents
	emit ResizeColumns();
	
	// set width of column to fit all chars with current font
	// (ASCII-col)
	//
	/*
	QFontInfo fi = m_pUi->tableView->fontInfo();
	m_pUi->tableView->setColumnWidth(4, fi.pixelSize()*16);
	*/
	
	/*
	// resize window to fit contents
	QSize vSize = m_pUi->tableView->size();
	QSize wndFrame = (frameSize() - size());
	resize(vSize + wndFrame);
	*/
	
	// show current file in title bar
	//setWindowFilePath(szNewFile);
	setWindowTitle(m_szBaseTitle + " - " + szNewFile);
	
	// show entire file size in status bar
	QString szSize;
	szSize.setNum(m_pModel->fileSize());
	m_pUi->statusBar->showMessage("Size: " + szSize);
}

/*
void MainWindow::ColActivated(QModelIndex index)
{
	if (index.column() < 4)
	{
		// data selected
		// -> highlight byte in ASCII
		//m_pUi->tableView->se
	}
	else if (index.column() == 4)
	{
		// ASCII selected
		// -> highlight all bytes?
	}
}
*/

void MainWindow::on_actionChooseFile_triggered()
{
    // open dialog for user to select file
    QString szFileName = QFileDialog::getOpenFileName(this, tr("Open file"));
    if (szFileName != NULL)
    {
		emit FileSelected(szFileName);
    }
}

void MainWindow::on_actionAbout_triggered()
{
	QTextEdit *pTxt = new QTextEdit(this);
	pTxt->setWindowFlags(Qt::Window); //or Qt::Tool, Qt::Dialog if you like
	pTxt->setReadOnly(true);
	pTxt->append("qHexDump by Ilkka Prusi 2011");
	pTxt->append("");
	pTxt->append("This program is free to use and distribute. No warranties of any kind.");
	pTxt->append("Program uses Qt 4.7.1 under LGPL v. 2.1");
	pTxt->append("");
	pTxt->append("Keyboard shortcuts:");
	pTxt->append("");
	pTxt->append("F = open file");
	pTxt->append("Esc = close");
	//pTxt->append("G = go to offset");
	//pTxt->append("S = search string");
	pTxt->append("");
	pTxt->show();
}
