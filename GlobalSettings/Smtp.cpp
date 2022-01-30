
#include "Smtp.h"
#include "RSSMTPinfodlg.h"

#include "cmsecurity.h"

#include <QTime>
//#include <QSqlError>
//#include <QSqlDatabase>
#include <QMessageBox>

CSMTPServer::CSMTPServer(CSMTPManager *manager)
{	
	m_pManager = manager;
}

CSMTPServer::~CSMTPServer()
{
}

CSMTPManager::CSMTPManager()
{
	m_strName = RES_SMTP;
}

CSMTPManager::~CSMTPManager()
{
}

QString CSMTPManager::GetName()
{
	return m_strName;
}

bool CSMTPManager::Load(QDomElement &element)
{

	QDomNodeList lstNodes = element.childNodes();
	QDomElement  res;
	CSMTPServer *pSMTPServer = NULL;

	for (int i = 0; i < lstNodes.size(); i++)
	{
		res = lstNodes.at(i).toElement();

		pSMTPServer = new CSMTPServer(this);

		pSMTPServer->m_strHost = res.attribute("host");
		pSMTPServer->m_strPort = res.attribute("port");
		pSMTPServer->m_strUser = res.attribute("user");

		SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
		pSMTPServer->m_strPwd = crypto.decryptToString(res.attribute("pwd"));

		//m_DBServers.insert(pDBServer->m_strConn, pDBServer);
	}

	return true;
}

bool CSMTPManager::Save(QDomDocument doc, QDomElement &element)
{
	element.setAttribute("type", m_strName);
	QHash<QString, CSMTPServer*>::iterator itr;

	for (itr = m_SMTPServers.begin(); itr != m_SMTPServers.end(); ++itr)
	{
		CSMTPServer *pSMTPServer = itr.value();

		QDomElement SMTP = doc.createElement("SMTP");

		SMTP.setAttribute("host",pSMTPServer->m_strHost);
		SMTP.setAttribute("port",pSMTPServer->m_strPort);
		SMTP.setAttribute("user", pSMTPServer->m_strUser);

		SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
		QString strPwd = crypto.encryptToString(pSMTPServer->m_strPwd);
		SMTP.setAttribute("pwd", strPwd);

		element.appendChild(SMTP);
	}

	return true;
}

void CSMTPManager::SetResourceManager(CGlobalManager *manager)
{
	m_pGlobalManager = manager;
}

QStringList CSMTPManager::GetColumnNames()
{
	QStringList columneNames;

	columneNames.append(SMTP_HOST_NAME);
	columneNames.append(SMTP_PORT);

	return columneNames;
}

QStringList CSMTPManager::GetActionNames()
{
	QStringList actionNames;
	actionNames.append("");
	actionNames.append(SMTP_ADD);
	actionNames.append(SMTP_EDIT);
//	//actionNames.append(DB_OP_DEL);	
//
return actionNames;
}

QStringList CSMTPManager::GetAvailableResources()
{
	return m_SMTPServers.keys();
}

QString CSMTPManager::GetIcon()
{
	return QString(":/res/database.png");
}

//QString CSMTPManager::GetIcon(const QString &actionName)
//{
//	//QString icon;
//
//	//if (actionName == DB_OP_ADD)
//	//{
//	//	icon = QString(":/res/dbadd.png");
//	//}
//	//else if (actionName == DB_OP_EDIT)
//	//{
//	//	icon = QString(":/res/dbedit.png");
//	//}
//	//else if (actionName == DB_OP_DEL)
//	//{
//	//	icon = QString(":/res/dbdel.png");
//	//}
//	//else if(actionName == DB_OP_EXECUTE)
//	//{
//	//	icon = QString(":/res/edit-sql-query.png");
//	//}
//	//return icon;
//}

//QString CSMTPManager::GetIcon(const QString &restName, const QString &resColName)
//{
//	//QString icon;
//
//	//CSMTPServer *pSMTPServer = m_SMTPServers.value(restName);
//
//	//if (pSMTPServer != NULL)
//	//{
//	//	if (resColName == DB_COL_CNAME)
//	//	{
//	//		icon = QString(":/res/database.png");
//	//	}
//	//}
//
//	//return icon;
//}
//
QHash<QString, QString> CSMTPManager::GetColumnValues(const QString &resName)
{
	QHash<QString, QString> columnValues;

	CSMTPServer *pSMTPServer = m_SMTPServers.value(resName);

	if (pSMTPServer != NULL)
	{
		QString key = GetKey(pSMTPServer->m_strProfileName, pSMTPServer->m_strUser);

		//columnValues.insert(SMTP_HOST_NAME, key);
		//columnValues.insert(DB_COL_TYPE, pDBServer->m_dbType);
	}

	return columnValues;
}

void CSMTPManager::SetAction(const QString &strActionName, const QStringList &lst)
{	
	if(strActionName == SMTP_ADD || strActionName == SMTP_EDIT)
	{
		CRSSMTPInfoDlg oSMTPInfoDlg(GetIcon(), NULL);

		if(strActionName == SMTP_EDIT)
		{
			CSMTPServer *pSMTPServer = m_SMTPServers.value(lst.at(0));

			if(pSMTPServer)
			{
				oSMTPInfoDlg.InitDialog(pSMTPServer->m_strUser, pSMTPServer->m_strPwd, "", "",pSMTPServer->m_strHost,pSMTPServer->m_strPort,"");
			//void CRSSMTPInfoDlg::InitDialog(const QString& strUserName, const QString& strUserPwd,const QString &strHost,const QString &strPort,const QString& strAuthType, const QString& strEncryptionType)

			}
		}
		else 
		{
			foreach(QString str, m_SMTPServers.keys())
			{
				//oDBInfoDlg.m_strExtConnList.append(str);
			}
		}

		if(oSMTPInfoDlg.exec() == QDialog::Accepted)
		{
//			QString strServiceName = oSMTPInfoDlg.GetServiceName();

			QHash<QString, QString> columnValues;
			
			QString key = GetKey(oSMTPInfoDlg.m_strSMTPServer, oSMTPInfoDlg.m_strUserName);

//			CSMTPServer *pDBServer = m_SMTPServers.value(strServiceName);

			//if(!pDBServer)
			//{
			//	pDBServer = new CSMTPServer(this);				
			//}		

			//pDBServer->m_strUser = oDBInfoDlg.m_strUserName;
			//pDBServer->m_strPwd = oDBInfoDlg.m_strUserPwd;
			//pDBServer->m_strHost = oDBInfoDlg.m_strHost;
			//pDBServer->m_strPort = oDBInfoDlg.m_strPort;
			//m_DBServers.insert(strServiceName, pDBServer);

//			m_pGlobalManager->ResourceUpdated(RES_SMTP, pDBServer->m_strConn);
		}
	}
	//else if(strActionName == DB_OP_DEL)
	//{
	//	if(lst.size() > 0 && !lst.at(0).isEmpty())
	//	{
	//		if(QMessageBox::question(NULL, "Databse", "Are you sure want to free the databse from monitoring?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	//		{
	//			CSMTPServer *pDBServer = m_DBServers.take(lst.at(0));

	//			if(pDBServer) delete pDBServer;

	//			m_pGlobalManager->ResourceUpdated(RES_DATABASE, lst.at(0));
	//		}
	//	}
	//}
	//else if(strActionName == DB_OP_EXECUTE)
	//{
	//	if(lst.size() > 0 && !lst.at(0).isEmpty())
	//	{
	//		CExecuteQuery oExecQuery(NULL);

	//		CSMTPServer *pDBServer = m_DBServers.value(lst.at(0));
	//		if(pDBServer)
	//		{
	//			QString strError;
	//			if(oExecQuery.InitExecQueryDlg(pDBServer->m_dbType, pDBServer->m_dbName, pDBServer->m_strUser, pDBServer->m_strPwd, "", "", strError,pDBServer->m_strHost,pDBServer->m_strPort))
	//			{
	//				if(oExecQuery.exec() == QDialog::Accepted)
	//				{

	//				}
	//			}
	//		}					
	//	}
	//}

	//if (strActionName == DB_OP_ADD  || 
	//	strActionName == DB_OP_EDIT || 
	//	strActionName == DB_OP_DEL )
	//{
	//	m_pGlobalManager->Save();
	//}
}

