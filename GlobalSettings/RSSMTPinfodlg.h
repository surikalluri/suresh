#ifndef RSSMTPINFODLG_H
#define RSSMTPINFODLG_H

//#include <QSqlDatabase>

#include "ui_SMTPinfodlg.h"
//#include "ui_executequery.h"

using namespace Ui;

class CRSSMTPInfoDlg : public QDialog
{
    Q_OBJECT

public:
	CRSSMTPInfoDlg(const QString strIcon, QWidget* parent, Qt::WFlags flags = 0);
	~CRSSMTPInfoDlg();

public:
	Ui::SMTPInfoDlg m_oDlg;
	
public slots:

	void setIPAddressValidator();
	void InitDialog( const QString& strProfileName,const QString& strUserName, const QString& strUserPwd,const QString &strHost,const QString &strPort,const QString& strAuthType, const QString& strEncryptionType);
	void CleanMembers();
	void onAuthTypeChanged(const QString& strType);
	void onCmbEncTypeChanged(const int nIndex);
	void onSettingsChanged(const QString&);


//	bool IsInputValid(QString& strError);


public:
	QString m_strSMTPServer;

	QString m_strProfileName;
	QString m_strUserName;
	QString m_strUserPwd;
	QString m_strEncryptionType;
	QString m_strAuthType;

	QString m_strPort;
	QString m_strHost;

private:
	QStringList m_strListDBs;
	QString m_strServiceName;
};


#endif	//RSSMTPINFODLG_H