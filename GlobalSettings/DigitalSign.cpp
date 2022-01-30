
#include "DigitalSign.h"


#include "cmsecurity.h"

#include <QTime>
#include <math.h>
#include <QMessageBox>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CDSData::CDSData(CDigitalSignatureManager *manager)
{	
	m_pManager = manager;
}


CDSData::~CDSData()
{
}

CDigitalSignatureManager::CDigitalSignatureManager()
{
	m_strName = RES_DIGITALSIGNATURE;
}

CDigitalSignatureManager::~CDigitalSignatureManager()
{
}

QString CDigitalSignatureManager::GetName()
{
	return m_strName;
}

bool CDigitalSignatureManager::Load(QDomElement &element)
{

	QDomNodeList lstNodes = element.childNodes();
	QDomElement  oChildEle;
	CDSData *pDSData = NULL;

	for (int i = 0; i < lstNodes.size(); i++)
	{
		oChildEle = lstNodes.at(i).toElement();

		pDSData = new CDSData(this);
        
		pDSData->m_strDSSignName =  oChildEle.attribute("DSSignName");
		pDSData->m_strPfxPathName = oChildEle.attribute("pfxPathName");
		pDSData->m_strPassword = oChildEle.attribute("password");
		pDSData->m_strLocation = oChildEle.attribute("location");
		pDSData->m_strReason = oChildEle.attribute("reason");
		pDSData->m_strTimeStamp = oChildEle.attribute("timestamp");
		pDSData->m_strPageNo = oChildEle.attribute("pageNo");
		pDSData->m_strLeft = oChildEle.attribute("left");
		pDSData->m_strTop = oChildEle.attribute("top");
		pDSData->m_strWidth = oChildEle.attribute("width");
		pDSData->m_strHeight = oChildEle.attribute("height");
		pDSData->m_strCert = oChildEle.attribute("cert");
		pDSData->m_strPem = oChildEle.attribute("pem");
		pDSData->m_strUsrName = oChildEle.attribute("UserName");
		pDSData->m_strIssuerName = oChildEle.attribute("IssuerName");
		pDSData->m_strPageId = oChildEle.attribute("PageId");
		QString strVal = oChildEle.attribute("LastPage");
		if(strVal.toInt() == 1)
			pDSData->m_bLast = true;
		else
			pDSData->m_bLast = false;
		strVal = oChildEle.attribute("RSA2048Bit");
		if(strVal.toInt() == 1)
			pDSData->m_b2048Bit = true;
		else
			pDSData->m_b2048Bit = false;

		strVal = oChildEle.attribute("TmstmpChk");
		if(strVal.toInt() == 1)
			pDSData->m_bTimestamp = true;
		else
			pDSData->m_bTimestamp = false;

		strVal = oChildEle.attribute("DSImg");
		if(strVal.toInt() == 1)
			pDSData->m_bSignImages = true;
		else
			pDSData->m_bSignImages = false;

		

        m_DigitalSignatures.insert(pDSData->m_strDSSignName,pDSData);

		/*pDBServer->m_strConn = res.attribute("conn");
		pDBServer->m_dbType = res.attribute("dbtype");
		pDBServer->m_dbName = res.attribute("dbname");
		pDBServer->m_strUser = res.attribute("user");
		pDBServer->m_strHost = res.attribute("host");
		pDBServer->m_strPort = res.attribute("port");

		SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
		pDBServer->m_strPwd = crypto.decryptToString(res.attribute("pwd"));

		m_DBServers.insert(pDBServer->m_strConn, pDBServer);*/
	}

	return true;
}

bool CDigitalSignatureManager::Save(QDomDocument doc, QDomElement &element)
{
	element.setAttribute("type", m_strName);
	QHash<QString, CDSData*>::iterator itr;
	int nCount;

	for (itr = m_DigitalSignatures.begin(); itr != m_DBServers.end(); ++itr)
	{
		CDSData *pDSData = itr.value();
		
		QDomElement DS = doc.createElement("sign");
		QString strPemHex,strCerHex,strTemp,strHex;
		 strCerHex = pDSData->m_strCert;
		 strPemHex = pDSData->m_strPem;
		QString strPfxPth = pDSData->m_strPfxPathName;
		QString strPasswd = pDSData->m_strPassword;

		if(!strPfxPth.isEmpty() && !strPasswd.isEmpty())
		{
	              bRes = GenerateCertificate(strPfxPth,strPasswd,strPemHex,strCerHex);

			    
			    SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
		         strCerHex = crypto.encryptToString(strCerHex);

                SimpleCrypt crypto(Q_UINT64_C(0x0c2ad4a4acb9f023));
		        strPemHex= crypto.encryptToString(strPemHex);



				
			}
		}


        if(bRes)
		{
		oChildEle.setAttribute("DSSignName", pData->m_strDSSignName);	
		oChildEle.setAttribute("cert", strCerHex);
		oChildEle.setAttribute("pem", strPemHex);
		oChildEle.setAttribute("UserName", pData->m_strUsrName);
		oChildEle.setAttribute("IssuerName", pData->m_strIssuerName);
		oChildEle.setAttribute("location", pData->m_strLocation);
		oChildEle.setAttribute("reason", pData->m_strReason);
		oChildEle.setAttribute("timestamp", pData->m_strTimeStamp);
		oChildEle.setAttribute("pageNo", pData->m_strPageNo);
		oChildEle.setAttribute("left", pData->m_strLeft);
		oChildEle.setAttribute("top", pData->m_strTop);
		oChildEle.setAttribute("width", pData->m_strWidth);
		oChildEle.setAttribute("height", pData->m_strHeight);
		oChildEle.setAttribute("PageId", pData->m_strPageId);

		if(ui.chckBx_PageLast->isChecked())
		{
			pData->m_bLast = true;
			oChildEle.setAttribute("LastPage", QString("1"));
		}
		else
		{
			pData->m_bLast = false;
			oChildEle.setAttribute("LastPage", QString("0"));
		}

		if(ui.Timestamp_checkBox->isChecked())
		{
			pData->m_bTimestamp = true;
			oChildEle.setAttribute("TmstmpChk", QString("1"));
		}
		else
		{
			pData->m_bTimestamp = false;
			oChildEle.setAttribute("TmstmpChk", QString("0"));
		}

		if(ui.chckBx_SignImages->isChecked())
		{
			pData->m_bSignImages = true;
			oChildEle.setAttribute("DSImg", QString("1"));
		}
		else
		{
			pData->m_bSignImages = false;
			oChildEle.setAttribute("DSImg", QString("0"));
		}
		if(ui.PubKey_chckBx->isChecked())
		{
			pData->m_b2048Bit = true;
			oChildEle.setAttribute("RSA2048Bit", QString("1"));
		}
		else
		{
			pData->m_b2048Bit = false;
			oChildEle.setAttribute("RSA2048Bit", QString("0"));
		}						

		element.appendChild(DS);
		}
	}

	return true;
}

void CDigitalSignatureManager::SetResourceManager(CGlobalManager *manager)
{
	m_pGlobalManager = manager;
}

QStringList CDigitalSignatureManager::GetColumnNames()
{
	QStringList columneNames;

	columneNames.append(DIGITALSIGNATURE_COL_NAME);
	

	return columneNames;
}

QStringList CDigitalSignatureManager::GetActionNames()
{
	QStringList actionNames;
	actionNames.append("");
	actionNames.append(DIGITALSIGNATURE_ADD);
	actionNames.append(DIGITALSIGNATURE_EDIT);
	actionNames.append(DIGITALSIGNATURE_DELETE);	

	return actionNames;
}

QStringList CDigitalSignatureManager::GetAvailableResources()
{
	return m_DBServers.keys();
}

QString CDigitalSignatureManager::GetIcon()
{
	return QString(":/res/digitalsignature.png");
}

QString CDigitalSignatureManager::GetIcon(const QString &actionName)
{
	QString icon;

	if (actionName == DIGITALSIGNATURE_ADD)
	{
		icon = QString(":/res/dsadd.png");
	}
	else if (actionName == DIGITALSIGNATURE_EDIT)
	{
		icon = QString(":/res/dsedit.png");
	}
	else if (actionName == DIGITALSIGNATURE_DELETE)
	{
		icon = QString(":/res/dsdel.png");
	}
	
	return icon;
}

QString CDigitalSignatureManager::GetIcon(const QString &restName, const QString &resColName)
{
	QString icon;

	CDBServer *pDBServer = m_DBServers.value(restName);

	if (pDBServer != NULL)
	{
		if (resColName == DIGITALSIGNATURE_COL_NAME)
		{
			icon = QString(":/res/digitalsignature.png");
		}
	}

	return icon;
}

QHash<QString, QString> CDigitalSignatureManager::GetColumnValues(const QString &resName)
{
	QHash<QString, QString> columnValues;

	CDSData *pDSData = m_DigitalSignatures.value(resName);

	if (pDSData != NULL)
	{
		columnValues.insert(DIGITALSIGNATURE_COL_NAME, pDSData->m_strDSSignName);
	}

	return columnValues;
}

void CDatabaseManager::SetAction(const QString &strActionName, const QStringList &lst)
{	
	if(strActionName = DIGITALSIGNATURE_ADD || strActionName == DIGITALSIGNATURE_EDIT)
	{
		CRSDigitalSignatureInfoDlg oDSInfoDlg(GetIcon(), NULL);

		if(strActionName == DIGITALSIGNATURE_EDIT)
		{
			CDSData *pDSData = m_DigitalSignatures.value(lst.at(0));

			if(pDSData)
			{
				oDSInfoDlg.InitDialog(pDSData->m_strLocation, pDSData->m_strReason, pDSData->m_strTimeStamp, pDSData->m_strPageNo, pDSData->m_strLeft,pDSData->m_strTop ,pDSData->m_strWidth ,pDSData->m_strHeight,pDBServer->m_strPageId,
				pDSData-> m_bTimestamp,pDSData-> m_bSignImages,pDSData-> m_b2048Bit);
			}
		}
		else 
		{
			foreach(QString str, m_DigitalSignatures.keys())
			{
				oDSInfoDlg.m_strExtConnList.append(str);
			}
		}

		if(oDSInfoDlg.exec() == QDialog::Accepted)
		{
			QString strServiceName = oDSInfoDlg.GetServiceName();

			QHash<QString, QString> columnValues;
			
			

			CDSData *pDSData = m_DigitalSignatures.value(strServiceName);

			if(!pDSData)
			{
				pDSData = new pDSData(this);				
			}		

		pDSData->m_strDSSignName =   oDSInfoDlg.m_strDSSignName;
		pDSData->m_strPfxPathName = oDSInfoDlg.m_strPfxPathName;
		pDSData->m_strPassword = oDSInfoDlg.m_strPassword;
		pDSData->m_strLocation = oDSInfoDlg.m_strLocation;
		pDSData->m_strReason = oDSInfoDlg.m_strReason;
		pDSData->m_strTimeStamp = oDSInfoDlg.m_strTimeStamp;
		pDSData->m_strPageNo = oDSInfoDlg.m_strPageNo;
		pDSData->m_strLeft = oDSInfoDlg.m_strLeft;
		pDSData->m_strTop = oDSInfoDlg.m_strTop;
		pDSData->m_strWidth = oDSInfoDlg.m_strWidth;
		pDSData->m_strHeight = oDSInfoDlg.m_strHeight;
		pDSData->m_strCert = oDSInfoDlg.m_strCert;
		pDSData->m_strPem = oDSInfoDlg.m_strPem;
		pDSData->m_strUsrName = oDSInfoDlg.m_strUsrName;
		pDSData->m_strIssuerName = oDSInfoDlg.m_strIssuerName;
		pDSData->m_strPageId = oDSInfoDlg.m_strPageId;
		pDSData->m_bLast = oDSInfoDlg.m_bLast;
		pDSData->m_bTimestamp = oDSInfoDlg.m_bTimestamp;
		pDSData-> m_bSignImages = oDSInfoDlg.m_bSignImages;
		pDSData->m_b2048Bit =  oDSInfoDlg.m_b2048Bit;
		pDSData->m_strValidity  = oDSInfoDlg.m_strValidity;
		m_DBServers.insert(strServiceName, pDBServer);

		m_pGlobalManager->ResourceUpdated(RES_DIGITALSIGNATURE, pDSData->m_strDSSignName);
		}
	}
	else if(strActionName == DIGITALSIGNATURE_DELETE)
	{
		if(lst.size() > 0 && !lst.at(0).isEmpty())
		{
			if(QMessageBox::question(NULL, "Databse", "Are you sure want to free the databse from monitoring?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
			{
				CDSData *pDSData = m_DigitalSignatures.take(lst.at(0));

				if(pDSData) delete pDSData;

				m_pGlobalManager->ResourceUpdated(RES_DIGITALSIGNATURE, lst.at(0));
			}
		}
	}
	

	if (strActionName == DIGITALSIGNATURE_ADD  || 
		strActionName == DIGITALSIGNATURE_EDIT || 
		strActionName == DIGITALSIGNATURE_DELETE )
	{
		m_pGlobalManager->Save();
	}
}

