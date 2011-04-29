#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractItemModel>
#include <QFile>

//QAbstractItemModel
class CHexFileModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CHexFileModel(QObject *parent = 0);
	virtual ~CHexFileModel();

	virtual bool setSourceFile(const QString &szFile);
	void close();
	
	bool isModelOpen() const
	{
		if (m_pCurrentFile != NULL
			&& m_pView != NULL
			&& m_iSize > 0)
		{
			return true;
		}
		return false;
	}
	
	qint64 fileSize() const
	{
		return m_iSize;
	}

    //void setHorizontalHeaderLabels(const QStringList &labels);
    //void setVerticalHeaderLabels(const QStringList &labels);
	
	// virtual overloads
public:	

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                               int role = Qt::EditRole);
	
	// implementations for pure abstract methods (QAbstractItemModel)
public:
    //virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    //virtual QModelIndex parent(const QModelIndex &child) const;	
	
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	
signals:
	// entire data model changed
	// (backing file different)
	// -> should reset view ?
	// note: check signals from base for suitable..
	//void BackingModelChanged();
	
public slots:

protected:
	inline void Encode(uchar *pDigest, const size_t nLen, QString &szOutput) const;
	
private:
	QFile *m_pCurrentFile;
	qint64 m_iSize;
	uchar *m_pView;
};

#endif // FILEMODEL_H
