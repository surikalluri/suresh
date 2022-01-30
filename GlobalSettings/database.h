
#ifndef DATABASE_H
#define DATABASE_H

#include "globalmanager.h"

#define RES_DATABASE    "Database Servers"
//Columns
#define DB_COL_CNAME     "Connection Name"
#define DB_COL_NAME     "Database Name"
#define DB_COL_TYPE     "Database Type"

//Actions
#define DB_OP_ADD   "Add Database"
#define DB_OP_EDIT  "Edit Database"
#define DB_OP_EXECUTE  "Execute Query"
#define DB_OP_DEL   "Delete Database"

class CDatabaseManager;

/* DB Resource */
class CDBServer : public QObject
{
	Q_OBJECT

public:
    CDBServer(CDatabaseManager *manager);
	~CDBServer();
public:
	/* Resource Properties */
	QString m_strConn;
	QString m_dbType;
	QString m_dbName;
	QString m_strUser;
	QString m_strPwd;

	QString m_strHost;
	QString m_strPort;
	
	QString m_strError;

private:
	CDatabaseManager *m_pManager;
};

/* DB Resource Manager */
class CDatabaseManager : public IResource
{
public:
    CDatabaseManager();
	~CDatabaseManager();

public:
	virtual QString GetName();
	
	virtual bool Load(QDomElement &element);
	virtual bool Save(QDomDocument doc, QDomElement &element);

	virtual void SetResourceManager(CGlobalManager *manager);

	virtual QString GetIcon();
	virtual QString GetIcon(const QString &actionName);
	virtual QString GetIcon(const QString &restName, const QString &resColName);

	virtual QStringList GetColumnNames();
	virtual QStringList GetActionNames();
	virtual QStringList GetAvailableResources();

	virtual QHash<QString, QString> GetColumnValues(const QString &resName);
	virtual void SetAction(const QString &strActionName, const QStringList &lst);

	inline CGlobalManager* GetResourceManager() { return m_pGlobalManager; }
	
	inline QString GetKey(const QString &dbName, const QString &User) 
	{
		QString key = dbName + " [" + User + "]";

		return key;
	}

private:
	QString m_strName;
	QHash<QString, CDBServer*> m_DBServers;

	CGlobalManager *m_pGlobalManager;
};
#endif //DATABASE_H

