
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
#include "RSDigitalSignInfoDlg.h"

CRSDigitalSignatureInfoDlg::CRSDigitalSignatureInfoDlg(const QString strIcon, QWidget* parent, Qt::WFlags flags) : QDialog(parent, flags )
m_strDSSignName(""),
m_strPfxPathName(""),
m_strPassword(""),
m_strLocation(""),
m_strReason(""),
m_strTimeStamp(""),
m_strPageNo("1"),
m_strLeft(""),
m_strTop(""),
m_strWidth(""),
m_strHeight(""),
m_bLast(false),
m_strPageId(""),
m_bTimestamp(false),
m_bSignImages(true),
m_b2048Bit(false)
{
	ui.setupUi(this);
	  
	  
	connect(ui.m_browseBtn, SIGNAL(clicked()), this, SLOT(onBrowse()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(onCancel()));
	connect(ui.ok,SIGNAL(clicked()),this,SLOT(onOk()));
	connect(ui.pageNmeCombobox, SIGNAL(activated(int)), this, SLOT(onComboBoxItemChanged(int)));
	connect(ui.pfxFilePathEdt, SIGNAL(editingFinished()), this, SLOT(onPFXFilePath()));
	connect(ui.passwordEdt, SIGNAL(editingFinished()), this, SLOT(onPasswordEdit()));
	connect(ui.locationEdt, SIGNAL(editingFinished()), this, SLOT(onLocationEdit()));
	connect(ui.reasonEdt, SIGNAL(editingFinished()), this, SLOT(onReasonEdit()));
	connect(ui.timeStampEdt, SIGNAL(editingFinished()), this, SLOT(onTimeStampEdit()));
	connect(ui.leftEdt, SIGNAL(editingFinished()), this, SLOT(onLeftEdit()));
	connect(ui.topEdt, SIGNAL(editingFinished()), this, SLOT(onTopEdit()));
	connect(ui.widthEdt, SIGNAL(editingFinished()), this, SLOT(onWidthEdit()));
	connect(ui.heightEdit, SIGNAL(editingFinished()), this, SLOT(onHeightEdit()));  
	connect(ui.cmbBox_UserName, SIGNAL(activated(int)), this, SLOT(onCmbBoxUserNameChanged(int)));
	connect(ui.Timestamp_checkBox, SIGNAL(stateChanged(int)), this, SLOT(onTimestampStateChanged(int)));
	
	
	

	

}

CRSDigitalSignatureInfoDlg::~CRSDigitalSignatureInfoDlg()
{
m_strDSSignName.clear();
m_strPfxPathName..clear();
m_strPassword..clear();
m_strLocation.clear();
m_strReason.clear();
m_strTimeStamp.clear();
m_strPageNo.clear();
m_strLeft.clear();
m_strTop.clear();
m_strWidth.clear();
m_strHeight.clear();
m_bLast.clear();
m_strPageId.clear();
m_bTimestamp.clear();
m_bSignImages.clear();
m_b2048Bit.clear();
}

void CRSDigitalSignatureInfoDlg::CRSDigitalSignatureInfoDlg()
{


	
}

//Validator for Host Name/IP Address
void CRSSMTPInfoDlg::setIPAddressValidator()
{
	QRegExp regExp("^[A-Za-z0-9]+([A-Za-z0-9-.]{1,})"); 
	QRegExpValidator *pValidator = NULL;
	pValidator = new QRegExpValidator(regExp, m_oDlg.leHostIP);
	m_oDlg.leHostIP->setValidator(pValidator);
}

void CRSDigitalSignatureInfoDlg::InitDialog(QString& strLocation, QString& strReason, QString& strTimeStamp, QString& strPageNo, QString& strLeft,QString& strTop ,QString& strWidth , QString& strHeight, QString& strPageId,
bool& bTimestamp,bool& bSignImages,bool& b2048Bit );
		
{
	    
		m_strLocation = strLocation;
		m_strReason = strReason;
		m_strTimeStamp = strTimeStamp;
		m_strPageNo = strPageNo;
		m_strLeft = strLeft;
		m_strTop = strTop;
		m_strWidth = strWidth;
		m_strHeight = strHeight;
		m_strCert = strCert;
		m_strPem = strPem;
		m_strUsrName = strUsrName;
		m_strIssuerName = strIssuerName;
		m_strPageId = strPageId;
		m_bLast = bLast;
		m_bTimestamp = bTimestamp;
		 m_bSignImages = bSignImages;
		m_b2048Bit =  b2048Bit;
		//m_strValidity  = strValidity;

	
		ui.pfxFilePathEdt->setDisabled(true);
		ui.passwordEdt->setDisabled(true);
		ui.m_browseBtn->setDisabled(true);
	
	ui.locationEdt->setText(strLocation); /* location */
	ui.reasonEdt->setText(strReason); /* reason */
	ui.timeStampEdt->setText(strTimeStamp);
	if(pData->m_bTimestamp) /* timestamp checking */
		ui.Timestamp_checkBox->setChecked(true);
	else
		ui.Timestamp_checkBox->setChecked(false);

	if(pData->m_bSignImages) /* Digital sign images checking */
		ui.chckBx_SignImages->setChecked(true);
	else
		ui.chckBx_SignImages->setChecked(false);
	
	if(!pData->m_strUsrName.isEmpty())/* user name */
		ui.cmbBox_UserName->setCurrentIndex(ui.cmbBox_UserName->findText(m_pGlblObjMapPtr->value(pData->m_strUsrName),Qt::MatchExactly));
	
	if(!pData->m_strPageId.isEmpty()) /* Page name */
		ui.pageNmeCombobox->setCurrentIndex(ui.pageNmeCombobox->findText(m_PageInfoMap.value(pData->m_strPageId),Qt::MatchExactly));
	
	if(pData->m_bLast) /* last repeated page checking */
		ui.chckBx_PageLast->setChecked(true);
	else
		ui.chckBx_PageLast->setChecked(false);

	if(pData->m_b2048Bit)
		ui.PubKey_chckBx->setChecked(true);
	else
		ui.PubKey_chckBx->setChecked(false);

	ui.leftEdt->setText(strLeft);
	ui.topEdt->setText(strTop);
	ui.widthEdt->setText(strWidth);
	ui.heightEdit->setText(strHeight);
	

}

void CRSDigitalSignatureInfoDlg::onSettingsChanged(const QString&)
{
	//m_bChanged = true;
	//emit settingsChanged(true);
}




void CRSDigitalSignatureInfoDlg::onCmbEncTypeChanged(const int nIndex)
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
void CRSDigitalSignatureInfoDlg::convertCharToHex(char *strTmp, int nLen, QString &strHex)
{
	unsigned char chTemp;
	char *pHexString = NULL;
	int ni=0, nindex = 0, nValue = 0, nCount = 0;
	strHex.clear();

	if (pHexString = (char*) malloc( sizeof(char)*(nLen*2)+1 ))
	{
		memset( pHexString, 0, sizeof(char)*(nLen*2)+1 );
		for(; nindex < nLen ; nindex++)
		{
			chTemp = strTmp[nindex];
			for( ni = 0; ni<2 ; ni++ )
			{
				if(ni == 0)
					nValue = chTemp/16;
				else
					nValue = chTemp%16;

				if( nValue >= 0 && nValue < 10 )		// HEX DIGITS 0 to 9
					nValue += 48;
				else if ( nValue > 9 && nValue < 16)	//HEX DIGITS A to F
					nValue += 55;
				pHexString[ nCount++ ] = nValue;

			}
		}
		strHex.append( pHexString );
		free(pHexString);
	}
}
bool CRSDigitalSignatureInfoDlg::validateInput()
{
	QString strError("");

	bool ok = true;
	/*QString strPgNo = ui.pageNoCombobox->currentText();
	int nPgNo;
	nPgNo = strPgNo.toInt(&ok);

	for(int nIndex = 0; nIndex < m_pListGrpInfo->count(); nIndex++)
	{
	QMap <QString,QString> pgMap;
	CGroupInfo *pGrpInfo;
	pGrpInfo = m_pListGrpInfo->at(nIndex);
	QList<CRSPageInfo*> pLstPgInfo;
	int i = 0,nCount;
	pLstPgInfo = pGrpInfo->pages();
	nCount  = pLstPgInfo.count();
	CRSPageInfo *pRSPageInfo;
	for( i = 0; i<nCount; i++)
	{
	if(i == nPgNo-1)
	{
	pRSPageInfo = pLstPgInfo.at(i);
	qreal rlPgWdth = pRSPageInfo->m_PageWidth;
	qreal rlPgHght = pRSPageInfo->m_PageHeight;
	}
	}
	}
	*/
	float fLft  = ui.leftEdt->text().toDouble(&ok);
	float fWdth = ui.widthEdt->text().toDouble(&ok);
	float fTop  = ui.topEdt->text().toDouble(&ok);
	float fHght = ui.heightEdit->text().toDouble(&ok);
	float fVal = 0.0;
	if(!m_bLoadValues)
	{
		if(ui.pfxFilePathEdt->text().isEmpty())
		{
			strError = tr("pfx File path should not be empty");
		}
		else if(ui.passwordEdt->text().isEmpty())
		{
			strError = tr("Password should not be empty");
		}
		if(!strError.isEmpty())
		{
			onCurrentTabChanged(1);
			QMessageBox::critical(0, tr("Digital Signature"), strError);
			return false;
		}
	}
	if(ui.pageNmeCombobox->currentIndex() <= 0)
	{
		strError = tr("Page Name should not be empty");
	}
	else if(ui.leftEdt->text().isEmpty())
	{
		strError = tr("Left field should not be empty");
	}
	else if(fLft < 0 || fLft > 8.27)
	{
		strError = tr("Left value should be less than 8");
	}
	else if(ui.widthEdt->text().isEmpty())
	{
		strError = tr("Width field should not be empty");
	}
	else if(fWdth < 0 || (fLft+fWdth) > 8.27)
	{
		fVal = 8.27 -(fLft);
		strError = tr("Width should be less than %1").arg(fVal);
	}
	else if(ui.topEdt->text().isEmpty())
	{
		strError = tr("Top field should not be empty");
	}
	else if(fTop < 0 || fTop > 11.69)
	{
		strError = tr("Top should be less than 11");
	}
	else if(ui.heightEdit->text().isEmpty())
	{
		strError = tr("Height field should not be empty");
	}
	else if(fHght < 0 || (fTop+fHght) > 11.69)
	{
		fVal = 11.69-(fTop);
		strError = tr("Height should be less than %1").arg(fVal);;
	}

	if(!strError.isEmpty())
	{
		onCurrentTabChanged(1);
		QMessageBox::critical(0, tr("Digital Signature"), strError);
		return false;
	}

	return true;
}
 void CRSDigitalSignatureInfoDlg::onBrowse()
{
	QString strPfxFilePath("");
	strPfxFilePath = QFileDialog::getOpenFileName(this, tr("Open pfx file"), QDir::currentPath(), "*.pfx");
	ui.pfxFilePathEdt->setText(strPfxFilePath);
	pDataptr->m_strPfxPathName = ui.pfxFilePathEdt->text();
		
}

void CRSDigitalSignatureInfoDlg::onComboBoxItemChanged(int nIndex)
{
	pDataptr->m_strPageId = m_PageInfoMap.key(ui.pageNmeCombobox->itemText(nIndex));
			
}

void CRSDigitalSignatureInfoDlg::onCmbBoxUserNameChanged(int nIndex)
{
	pDataptr->m_strUsrName = m_pGlblObjMapPtr->key(ui.cmbBox_UserName->itemText(nIndex));

}

void CRSDigitalSignatureInfoDlg::onTimestampStateChanged(int nState)
{
	if(nState == 0)
		ui.timeStampEdt->setDisabled(true);
	else
		ui.timeStampEdt->setDisabled(false);
}
void CRSDigitalSignatureInfoDlg::onCancel()
{
	reject();
}
void CRSDigitalSignatureInfoDlg::onPasswordEdit()
{
	pDataptr->m_strPassword = ui.passwordEdt->text();
}

void CRSDigitalSignatureInfoDlg::onLocationEdit()
{
	pDataptr->m_strLocation = ui.locationEdt->text();
}

void CRSDigitalSignatureInfoDlg::onReasonEdit()
{
	pDataptr->m_strReason = ui.reasonEdt->text();
}

void CRSDigitalSignatureInfoDlg::onTimeStampEdit()
{
	pDataptr->m_strTimeStamp = ui.timeStampEdt->text();
	
}

void CRSDigitalSignatureInfoDlg::onLeftEdit()
{
	
			pDataptr->m_strLeft = ui.leftEdt->text();
		
	
}

void CRSDigitalSignatureInfoDlg::onTopEdit()
{
	pDataptr->m_strTop = ui.topEdt->text();
		
}

void CRSDigitalSignatureInfoDlg::onWidthEdit()
{
	pDataptr->m_strWidth = ui.widthEdt->text();
		
}

void CRSDigitalSignatureInfoDlg::onHeightEdit()
{
	pDataptr->m_strHeight = ui.heightEdit->text();
	
}
void CRSDigitalSignatureInfoDlg::onOk()
{
    bValidate = validateInput();
    if(bValidate)
	{
		accept();

     }
}
oid CRSDigitalSignatureInfoDlg::onPFXFilePath()
{
	
			pDataptr->m_strPfxPathName = ui.pfxFilePathEdt->text();
	
}
