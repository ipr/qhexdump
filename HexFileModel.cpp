#include "HexFileModel.h"

#include <QAbstractItemModel>

#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>

#include <QFile>

//#include <limits.h>
//#include <stdint.h>


CHexFileModel::CHexFileModel(QObject *parent) :
    //QAbstractItemModel(parent),
    QAbstractTableModel(parent),
	m_pCurrentFile(NULL),
	m_iSize(0),
	m_pView(NULL)
{
}

CHexFileModel::~CHexFileModel()
{
	close();
}

bool CHexFileModel::setSourceFile(const QString &szFile)
{
	// clear previous (if any)
	close();
	
	m_pCurrentFile = new QFile(szFile, this);
	if (m_pCurrentFile == NULL)
	{
		return false;
	}
	
	// must open file now
	if (m_pCurrentFile->open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	/*
	// TODO: do we need to open or is open already?
	if (m_pCurrentFile->isOpen() == false)
	{
		return false;
	}
	*/
	
	m_iSize = m_pCurrentFile->size();
	m_pView = m_pCurrentFile->map(0, m_iSize);
	if (m_pView == NULL)
	{
		return false;
	}
	
	// note: check signals from base for suitable..
	//emit BackingModelChanged();
	return true;
}

void CHexFileModel::close()
{
	if (m_pCurrentFile != NULL)
	{
		if (m_pCurrentFile->isOpen() == true)
		{
			m_pCurrentFile->close();
		}
		delete m_pCurrentFile;
		m_pCurrentFile = NULL;
	}
	
	// reset for easier detection of being closed
	m_pView = NULL;
	m_iSize = 0;
}

/*
void CHexFileModel::setHorizontalHeaderLabels(const QStringList &labels)
{
	//this->d_ptr->
}
*/

// TODO: offset as vertical labels?
// (generate from file)
/*
void CHexFileModel::setVerticalHeaderLabels(const QStringList &labels)
{
}
*/

QVariant CHexFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole
		&& role != Qt::DisplayPropertyRole)
	{
		return QVariant();
	}
	
	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < 4)
		{
			QString szValue;
			szValue.sprintf("0x%.2d", 4*(section+1));
			return QVariant(szValue);
		}
		else if (section == 4)
		{
			return QVariant("ASCII");
		}
	}
	else if (orientation == Qt::Vertical)
	{
		// vertical "row" headers: set as offset
		qint64 iOffset = 4*4*section;
		QString szValue;
		if (m_iSize < USHRT_MAX)
		{
			szValue.sprintf("0x%.4d", iOffset);
		}
		else if (m_iSize < ULONG_MAX)
		{
			szValue.sprintf("0x%.8d", iOffset);
		}
		else
		{
			szValue.sprintf("0x%.16d", iOffset);
		}
		return QVariant(szValue);
	}
	return QVariant();
}

bool CHexFileModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	//this->d_ptr->
	return true;
}

/*
QModelIndex CHexFileModel::index(int row, int column, const QModelIndex &parent) const
{
	return QModelIndex();
}
*/

/*
QModelIndex CHexFileModel::parent(const QModelIndex &child) const
{
	return QModelIndex();
}
*/

int CHexFileModel::rowCount(const QModelIndex &parent) const
{
	if (m_pCurrentFile == NULL
		|| m_iSize == 0
		|| m_pView == NULL)
	{
		return 0;
	}

	/*
	// should return 0 when parent is valid?
	if (parent.isValid() == false)
	{
		return 0;
	}
	*/
	
	// count offset to file position,
	// for now, have 4*4 bytes per row
	// and possibly one partial row
	
	qint64 iRowCount = m_iSize/(4*4);
	qint64 iRem = m_iSize%(4*4);
	if (iRem > 0)
	{
		iRowCount++;
	}
	
	return (int)iRowCount;
}

int CHexFileModel::columnCount(const QModelIndex &parent) const
{
	if (m_pCurrentFile == NULL
		|| m_iSize == 0
		|| m_pView == NULL)
	{
		return 0;
	}

	/*	
	// should return 0 when parent is valid?
	if (parent.isValid() == false)
	{
		return 0;
	}
	*/
	
	// fixed column count for simplicity:
	// 4 byte-cols, ASCII-col (offset at row-headers)
	return 5;
}

QVariant CHexFileModel::data(const QModelIndex &index, int role) const
{
	if (m_pCurrentFile == NULL
		|| m_iSize == 0
		|| m_pView == NULL)
	{
		return QVariant();
	}

	/*	
	if (index.isValid() == false)
	{
		return QVariant();
	}
	*/

	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QString szValue;
	
	// count offset to file position,
	// for now, have 4*4 bytes per row
	// -> wanted row at offset 4*4*rowindex
	qint64 iOffset = 4*4*index.row();

	// ASCII-column of the row
	if (index.column() == 4)
	{
		// ascii-column
		qint64 iBytesRem = (m_iSize - iOffset);
		
		// size of entire row
		qint64 iCharsToCode = 4*4;
		if (iBytesRem < iCharsToCode)
		{
			iCharsToCode = (int)iBytesRem;
		}
		
		uchar *pData = m_pView + iOffset;
		for (qint64 i = 0; i < iCharsToCode; i++)
		{
			if (isalnum(pData[i]) == 0)
			{
				// use dot where non-displayable character
				szValue += '.';
			}
			else
			{
				// use actual character
				szValue += (char)pData[i];
			}
		}
		return QVariant(szValue);
	}
	
	// raw hex columns
	// correct column
	iOffset += index.column()*4;
	if (iOffset >= m_iSize)
	{
		// out of range
		return QVariant();
	}
	
	// if odd-sized file, check for last part near end..
	int iBytes = (m_iSize - iOffset);
	if (iBytes > 4)
	{
		iBytes = 4;
	}

	uchar *pData = m_pView + iOffset;
	Encode(pData, iBytes, szValue);
	
	return QVariant(szValue);
}

void CHexFileModel::Encode(uchar *pDigest, const size_t nLen, QString &szOutput) const
{
	char hextable[] = "0123456789ABCDEF";

	// reserve 2*iLen space in output 
	// (should improve efficiency slightly..)
	//
	size_t nSize = (szOutput.size() + (nLen*2));
	size_t nCapacity = szOutput.capacity();
	if (nCapacity < nSize)
	{
		szOutput.reserve(nSize);
	}
	
	// determine half-bytes of each byte 
	// and appropriate character representing value of it
	// for hex-encoded string
	for ( size_t y = 0; y < nLen; y++ )
	{
		unsigned char upper;
		unsigned char lower;

		upper = lower = pDigest[y];

		lower = lower & 0xF;

		upper = upper >> 4;
		upper = upper & 0xF;

		// STL string grows automatically so we just push new
		// characters at the end, same way with reserve().
		//
		szOutput += hextable[upper]; szOutput += hextable[lower];
	}
}
