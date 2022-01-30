
#include "database.h"
#include "rsdbinfodlg.h"

#include "cmsecurity.h"

#include <QTime>
#include <QSqlError>
#include <QSqlDatabase>
#include <QMessageBox>

CDBServer::CDBServer(CDatabaseManager *manager)
{	
	m_pManager = manager;
}

CDBServer::~CDBServer()
{
}

CDatabaseManager::CDatabaseManager()
{
	m_strName = RES_DATABASE;
}

CDatabaseManager::~CDatabaseManager()
{
}

QString CDatabaseManager::GetName()
{
	return m_strName;
}

bool CDatabaseManager::Load(QDomElement &element)
{

	QDomNodeList lstNodes = element.childNodes();
	QDomElement  res;
	CDBServer *pDBServer = NULL;

	for (int i = 0; i < lstNodes.size(); i++)
	{
		res = lstNodes.at(i).toElement();

		pDBServer = new CDBServer(this);

		pDBServer->m_strConn = res.attribute("conn");
		pDBServer->m_dbType = res.attribute("dbtype");
		pDBServer->m_dbName = res.attribute("dbname");
		pDBServer->m_strUser = res.attribute("user");
		pDBServer->m_strHost = res.attribute("host");
		pDBServer->m_strPort = res.attribute("port");

		SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
		pDBServer->m_strPwd = crypto.decryptToString(res.attribute("pwd"));

		m_DBServers.insert(pDBServer->m_strConn, pDBServer);
	}

	return true;
}

bool CDatabaseManager::Save(QDomDocument doc, QDomElement &element)
{
	element.setAttribute("type", m_strName);
	QHash<QString, CDBServer*>::iterator itr;

	for (itr = m_DBServers.begin(); itr != m_DBServers.end(); ++itr)
	{
		CDBServer *pDBServer = itr.value();

		QDomElement db = doc.createElement("db");

		db.setAttribute("conn", pDBServer->m_strConn);
		db.setAttribute("dbtype", pDBServer->m_dbType);
		db.setAttribute("dbname", pDBServer->m_dbName);
		db.setAttribute("user", pDBServer->m_strUser);
		db.setAttribute("host",pDBServer->m_strHost);
		db.setAttribute("port",pDBServer->m_strPort);

		SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
		QString strPwd = crypto.encryptToString(pDBServer->m_strPwd);
		db.setAttribute("pwd", strPwd);

		element.appendChild(db);
	}

	return true;
}

void CDatabaseManager::SetResourceManager(CGlobalManager *manager)
{
	m_pGlobalManager = manager;
}

QStringList CDatabaseManager::GetColumnNames()
{
	QStringList columneNames;

	columneNames.append(DB_COL_CNAME);
	columneNames.append(DB_COL_NAME);
	columneNames.append(DB_COL_TYPE);

	return columneNames;
}

QStringList CDatabaseManager::GetActionNames()
{
	QStringList actionNames;
	actionNames.append("");
	actionNames.append(DB_OP_ADD);
	actionNames.append(DB_OP_EDIT);
	actionNames.append(DB_OP_DEL);	

	return actionNames;
}

QStringList CDatabaseManager::GetAvailableResources()
{
	return m_DBServers.keys();
}

QString CDatabaseManager::GetIcon()
{
	return QString(":/res/database.png");
}

QString CDatabaseManager::GetIcon(const QString &actionName)
{
	QString icon;

	if (actionName == DB_OP_ADD)
	{
		icon = QString(":/res/dbadd.png");
	}
	else if (actionName == DB_OP_EDIT)
	{
		icon = QString(":/res/dbedit.png");
	}
	else if (actionName == DB_OP_DEL)
	{
		icon = QString(":/res/dbdel.png");
	}
	else if(actionName == DB_OP_EXECUTE)
	{
		icon = QString(":/res/edit-sql-query.png");
	}
	return icon;
}

QString CDatabaseManager::GetIcon(const QString &restName, const QString &resColName)
{
	QString icon;

	CDBServer *pDBServer = m_DBServers.value(restName);

	if (pDBServer != NULL)
	{
		if (resColName == DB_COL_CNAME)
		{
			icon = QString(":/res/database.png");
		}
	}

	return icon;
}

QHash<QString, QString> CDatabaseManager::GetColumnValues(const QString &resName)
{
	QHash<QString, QString> columnValues;

	CDBServer *pDBServer = m_DBServers.value(resName);

	if (pDBServer != NULL)
	{
		QString key = GetKey(pDBServer->m_dbName, pDBServer->m_strUser);

		columnValues.insert(DB_COL_CNAME, pDBServer->m_strConn);
		columnValues.insert(DB_COL_NAME, key);
		columnValues.insert(DB_COL_TYPE, pDBServer->m_dbType);
	}

	return columnValues;
}

void CDatabaseManager::SetAction(const QString &strActionName, const QStringList &lst)
{	
	if(strActionName == DB_OP_ADD || strActionName == DB_OP_EDIT)
	{
		CRSDBInfoDlg oDBInfoDlg(GetIcon(), NULL);

		if(strActionName == DB_OP_EDIT)
		{
			CDBServer *pDBServer = m_DBServers.value(lst.at(0));

			if(pDBServer)
			{
				oDBInfoDlg.InitDialog(pDBServer->m_strConn, pDBServer->m_dbType, pDBServer->m_dbName, pDBServer->m_strUser, pDBServer->m_strPwd, "", "",pDBServer->m_strHost,pDBServer->m_strPort);
			}
		}
		else 
		{
			foreach(QString str, m_DBServers.keys())
			{
				oDBInfoDlg.m_strExtConnList.append(str);
			}
		}

		if(oDBInfoDlg.exec() == QDialog::Accepted)
		{
			QString strServiceName = oDBInfoDlg.GetServiceName();

			QHash<QString, QString> columnValues;
			
			QString key = GetKey(oDBInfoDlg.m_strDatabase, oDBInfoDlg.m_strUserName);

			CDBServer *pDBServer = m_DBServers.value(strServiceName);

			if(!pDBServer)
			{
				pDBServer = new CDBServer(this);				
			}		

			pDBServer->m_strConn = oDBInfoDlg.m_strConnName;
			pDBServer->m_dbType = oDBInfoDlg.m_strDBType;
			pDBServer->m_dbName = oDBInfoDlg.m_strDatabase;
			pDBServer->m_strUser = oDBInfoDlg.m_strUserName;
			pDBServer->m_strPwd = oDBInfoDlg.m_strUserPwd;
			pDBServer->m_strHost = oDBInfoDlg.m_strHost;
			pDBServer->m_strPort = oDBInfoDlg.m_strPort;
			m_DBServers.insert(strServiceName, pDBServer);

			m_pGlobalManager->ResourceUpdated(RES_DATABASE, pDBServer->m_strConn);
		}
	}
	else if(strActionName == DB_OP_DEL)
	{
		if(lst.size() > 0 && !lst.at(0).isEmpty())
		{
			if(QMessageBox::question(NULL, "Databse", "Are you sure want to free the databse from monitoring?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
			{
				CDBServer *pDBServer = m_DBServers.take(lst.at(0));

				if(pDBServer) delete pDBServer;

				m_pGlobalManager->ResourceUpdated(RES_DATABASE, lst.at(0));
			}
		}
	}
	else if(strActionName == DB_OP_EXECUTE)
	{
		if(lst.size() > 0 && !lst.at(0).isEmpty())
		{
			CExecuteQuery oExecQuery(NULL);

			CDBServer *pDBServer = m_DBServers.value(lst.at(0));
			if(pDBServer)
			{
				QString strError;
				if(oExecQuery.InitExecQueryDlg(pDBServer->m_dbType, pDBServer->m_dbName, pDBServer->m_strUser, pDBServer->m_strPwd, "", "", strError,pDBServer->m_strHost,pDBServer->m_strPort))
				{
					if(oExecQuery.exec() == QDialog::Accepted)
					{

					}
				}
			}					
		}
	}

	if (strActionName == DB_OP_ADD  || 
		strActionName == DB_OP_EDIT || 
		strActionName == DB_OP_DEL )
	{
		m_pGlobalManager->Save();
	}
}

