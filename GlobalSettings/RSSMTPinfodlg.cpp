
#include <rssessionmanager.h>

#include <QList>
#include <QSettings>
#include <QStringList>
#include <QHeaderView>
#include <QPluginLoader>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>
#include <QDirModel>

#include <RSCoreHelper.h>
#include <RSComponentManager.h>
#include <rssessionmanager.h>
#include "RSSMTPinfodlg.h"

CRSSMTPInfoDlg::CRSSMTPInfoDlg(const QString strIcon, QWidget* parent, Qt::WFlags flags) : QDialog(parent, flags )
{
	m_oDlg.setupUi(this);

	connect(m_oDlg.leHostIP,		 SIGNAL(textEdited(const QString&)), this, SLOT(onSettingsChanged(const QString&)));
	connect(m_oDlg.cmbAuth,			 SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onAuthTypeChanged(const QString&)));
	connect(m_oDlg.leUserName,		 SIGNAL(textEdited(const QString&)), this, SLOT(onSettingsChanged(const QString&)));
	connect(m_oDlg.lePassword,		 SIGNAL(textEdited(const QString&)), this, SLOT(onSettingsChanged(const QString&)));
	connect(m_oDlg.sbPortNumber,	 SIGNAL(valueChanged(const QString&)), this, SLOT(onSettingsChanged(const QString&)));

	connect(m_oDlg.cmbEncType, SIGNAL(currentIndexChanged(const int)), this, SLOT(onCmbEncTypeChanged(const int)));
	m_oDlg.cmbAuth->addItems(QStringList()<<tr("None")<<tr("Auth Login"));
	m_oDlg.cmbEncType->addItems(QStringList() << tr("None") << tr("STARTTLS") << tr("TLS") << tr("SSL"));

}

CRSSMTPInfoDlg::~CRSSMTPInfoDlg()
{

}

void CRSSMTPInfoDlg::CleanMembers()
{
//	m_strListDBs.clear();

	m_strUserName.clear();
	m_strUserPwd.clear();
	m_strAuthType.clear();
	m_strEncryptionType.clear();


	m_strPort.clear();
	m_strHost.clear();
}

//Validator for Host Name/IP Address
void CRSSMTPInfoDlg::setIPAddressValidator()
{
	QRegExp regExp("^[A-Za-z0-9]+([A-Za-z0-9-.]{1,})"); 
	QRegExpValidator *pValidator = NULL;
	pValidator = new QRegExpValidator(regExp, m_oDlg.leHostIP);
	m_oDlg.leHostIP->setValidator(pValidator);
}

void CRSSMTPInfoDlg::InitDialog(const QString& strProfileName,const QString& strUserName, const QString& strUserPwd,const QString &strHost,const QString &strPort,const QString& strAuthType, const QString& strEncryptionType)
{
	m_strProfileName = strProfileName;
	m_strUserName	= strUserName;
	m_strUserPwd	= strUserPwd;
	m_strAuthType = strAuthType;
	m_strEncryptionType= strEncryptionType;
	m_strPort = strPort;
	m_strHost = strHost;

	m_oDlg.leHostIP->setText( strHost );
	int nIndex = m_oDlg.cmbAuth->findText(m_strAuthType);
	if( nIndex<0 )
		nIndex=0;
	m_oDlg.cmbAuth->setCurrentIndex(nIndex);

	if(QString::compare(m_oDlg.cmbAuth->currentText(), tr("None"), Qt::CaseInsensitive) != 0)
	{
		m_oDlg.leUserName->setText(strUserName );
		m_oDlg.lePassword->setText(strUserPwd );
	}

	//QString strPort = QString::number(m_oDlg.leHostIP);
//	m_oDlg.sbPortNumber->setValue(strPort);

	if(!m_strEncryptionType.isEmpty())
	{
		int nEncIndex = m_oDlg.cmbEncType->findText(m_strEncryptionType);
		m_oDlg.cmbEncType->setCurrentIndex(nEncIndex);
	}
	else
		m_oDlg.cmbEncType->setCurrentIndex(0);		// Keeping None

	//if( QString::compare(m_oDlg.cmbAuth->currentText(), tr("None"), Qt::CaseInsensitive)!= 0)
	//{
	//	QString strUsrName, strPassword;
	//	strUsrName  = m_oDlg.leUserName->text();
	//	strPassword = m_oDlg.lePassword->text();
	//	if(strUsrName.isEmpty() || strPassword.isEmpty())
	//	{
	//		QMessageBox::critical(this, tr("Blank Username/Password"), tr("Username/Password field(s) left blank."));
	//		if(strUsrName.isEmpty())
	//			m_oDlg.leUserName->setFocus();
	//		else
	//			m_oDlg.lePassword->setFocus();
	//		return false;
	//	}
	//}
	//pProfile->m_strHostIP = m_oDlg.leHostIP->text();
	//pProfile->m_strAuthType = m_oDlg.cmbAuth->currentText();

	//long lPort = m_oDlg.sbPortNumber->text().toLong();
	//pProfile->setPortNumber(lPort);

	//if(pProfile->m_strAuthType.compare(tr("None"),Qt::CaseInsensitive) != 0)
	//{
	//	pProfile->m_strUserName = m_oDlg.leUserName->text();
	//	pProfile->m_strPassword = m_oDlg.lePassword->text();
	//}
	//pProfile->m_strEncryptionType = m_oDlg.cmbEncType->currentText();		// Getting EncType..

}

void CRSSMTPInfoDlg::onSettingsChanged(const QString&)
{
	//m_bChanged = true;
	//emit settingsChanged(true);
}

void CRSSMTPInfoDlg::onAuthTypeChanged(const QString& strType)
{
	bool bEnable = true;

	if (strType.compare(tr("None"), Qt::CaseInsensitive)==0)
	{
		bEnable = false;
	}

	m_oDlg.leUserName->setEnabled(bEnable);
	m_oDlg.lePassword->setEnabled(bEnable);
	onSettingsChanged("");
}

void CRSSMTPInfoDlg::onCmbEncTypeChanged(const int nIndex)
{
	//if(nIndex == 1)					// 1 = STARTTLS
	//	m_oDlg.sbPortNumber->setValue(587);
	//else if(nIndex == 2)			// 2 = TLS
	//	m_oDlg.sbPortNumber->setValue(25);
	//else if(nIndex == 3)			// 3 == SSL
	//	m_oDlg.sbPortNumber->setValue(465);
	//else
	//	m_oDlg.sbPortNumber->setValue(0);


	onSettingsChanged("");
}   