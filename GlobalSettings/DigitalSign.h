
#ifndef DIGITALSIGNATURE_H
#define DIGITALSIGNATURE_H

#include "globalmanager.h"
#include "rsencrypt.h"

#define RES_DIGITALSIGNATURE    "Digital Signature"
//Columns
#define DIGITALSIGNATURE_COL_NAME "DIGITALSIGNATURE"

//Actions
#define DIGITALSIGNATURE_ADD   "Add Digital Signature"
#define DIGITALSIGNATURE_EDIT  "Edit Digital Signature"
#define DIGITALSIGNATURE_DELETE "Delete Digital Signature"
 


class CDigitalSignatureManager;

/* SMTP Resource */
class CDSData :public QObject
{
	Q_OBJECT
public:
	CDSData(CDigitalSignatureManager *manager);
	//CDSData(QString& strPfxPathName, QString& strPassword, QString& strLocation, QString& strReason, QString& strTimeStamp, int& nPageNo, int& nLeft, int& nTop, int& nWidth, int& nHeight);
	~CDSData();

public:
	QString m_strDSSignName ;
	QString m_strPfxPathName;
	QString m_strPassword;
	QString m_strLocation;
	QString m_strReason;
	QString m_strTimeStamp;
	QString m_strPageNo;
	QString m_strLeft;
	QString m_strTop;
	QString m_strWidth;
	QString m_strHeight;
	QString m_strUsrName;
	QString m_strIssuerName;
	QString m_strValidity;
	QString m_strCert;
	QString m_strPem;
	bool m_bLast;
	QString m_strPageId;
	bool m_bTimestamp;
	bool m_bSignImages;
	bool m_b2048Bit;
	QString m_strOpenSSLPath;
	private:
	CDigitalSignatureManager *m_pManager;
};

/* SMTP Resource Manager */
class CDigitalSignatureManager : public IResource
{
public:
    CDigitalSignatureManager();
	~CDigitalSignatureManager();

	
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
	
	

private:
	QString m_strName;
	QHash<QString, QString> m_hNotifications;
	QHash<QString, CDSData*> m_DigitalSignatures;

	CGlobalManager *m_pGlobalManager;
};
#endif //SMTP_H