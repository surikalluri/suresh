
#ifndef SMTP_H
#define SMTP_H

#include "globalmanager.h"

#define RES_SMTP    "SMTP Servers"
//Columns
#define SMTP_HOST_NAME     "Host Name"
#define SMTP_PORT     "Port Number"

//Actions
#define SMTP_ADD   "Add SMTP"
#define SMTP_EDIT  "Edit SMTP"
//#define DB_OP_EXECUTE  "Execute Query"
//#define DB_OP_DEL   "Delete Database"

class CSMTPManager;

/* SMTP Resource */
class CSMTPServer : public QObject
{
	Q_OBJECT

public:
    CSMTPServer(CSMTPManager *manager);
	~CSMTPServer();
public:
	/* Resource Properties */
	QString m_strProfileName;

	QString m_strHost;
	QString m_strPort;
	QString m_strUser;
	QString m_strPwd;
	QString m_strError;

private:
	CSMTPManager *m_pManager;
};

/* SMTP Resource Manager */
class CSMTPManager : public IResource
{
public:
    CSMTPManager();
	~CSMTPManager();

public:
	virtual QString GetName();
	
	virtual bool Load(QDomElement &element);
	virtual bool Save(QDomDocument doc, QDomElement &element);

	virtual void SetResourceManager(CGlobalManager *manager);

	virtual QString GetIcon();
	//virtual QString GetIcon(const QString &actionName);
	//virtual QString GetIcon(const QString &restName, const QString &resColName);

	virtual QStringList GetColumnNames();
	virtual QStringList GetActionNames();
	virtual QStringList GetAvailableResources();

//	virtual QHash<QString, QString> GetSelResources();
//	virtual void SetSelResources(const QHash<QString, QString> &hLst);

	virtual QHash<QString, QString> GetColumnValues(const QString &resName);
	virtual void SetAction(const QString &strActionName, const QStringList &lst);

	inline CGlobalManager* GetResourceManager() { return m_pGlobalManager; }
	
	inline QString GetKey(const QString &Name, const QString &User) 
	{
		QString key = Name + " [" + User + "]";

		return key;
	}

private:
	QString m_strName;
	QHash<QString, QString> m_hNotifications;
	QHash<QString, CSMTPServer*> m_SMTPServers;

	CGlobalManager *m_pGlobalManager;
};
#endif //SMTP_H