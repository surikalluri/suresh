#include "AdvSettDlgPDF.h"
#include <QDomDocument>
#include <QDomNodeList>
#include <QMessageBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QTime>
#include <QComboBox>
#include <QMatrix>
#include <QTextCursor>
#include <QProcess>

#include <QApplication>
#include <QDir>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//#include <openssl/pkcs12.h>
//#include <openssl/x509.h>
//#include <openssl/pem.h>

#include <PrintPlugin.h>
#include <rssessionmanager.h>
Q_DECLARE_METATYPE(CGroupInfo*);
// Constructor
CAdvSettDlgPDF::CAdvSettDlgPDF( const  QMap<QString,QString>& Map,QList<CGroupInfo*>& lstGrpInfo, QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags),
m_pCurrentItem(0),
m_pRenameItem(0),
m_bLoadValues(false),
m_bEditMargin(false),
m_bAddNew(false),
m_strTitle(""),
m_strAuthor(""),
m_strSubject(""),
m_strKeywords(""),
m_strImgLocPath(""),
m_strImgUrlPath(""),
m_bPgWatermark(false),
m_strTemplatePath(""),
m_nIndex(-1),
m_bPgImageWM(false),
m_strOpenSSLPath("")
{
	ui.setupUi(this);
	ui.lnEdit_MasterPwd->setMaxLength(31);
	ui.Title_lineEdit->setMaxLength(60);
	ui.Author_lineEdit->setMaxLength(60);
	ui.Subject_lineEdit->setMaxLength(60);
	ui.Keywords_lineEdit->setMaxLength(120);
	ui.m_RscLclPath->setMaxLength(500);
	ui.m_UrlAddr->setMaxLength(500);
	//ui.tabWidget->setTabEnabled(2, false);
	m_strLstSignNames.clear();
	m_strPgLst.clear();
	initialize(Map, lstGrpInfo);
}

//Destructor
CAdvSettDlgPDF::~CAdvSettDlgPDF()
{
	if(m_pEncrObj)
	{
		delete m_pEncrObj;
		m_pEncrObj = NULL;
	}
	if(m_pPlainTextObj)
	{
		delete m_pPlainTextObj;
		m_pPlainTextObj = NULL;
	}
	if(m_pFrame)
	{
		delete m_pFrame;
		m_pFrame = NULL;
	}
}

void CAdvSettDlgPDF::initialize(const QMap<QString,QString>& Map, QList<CGroupInfo*>& lstGrpInfo)
{
	ui.tabWidget->setCurrentIndex(0);
	ui.Title_lineEdit->setText("");
	ui.Author_lineEdit->setText("");
	ui.Subject_lineEdit->setText("");
	ui.Keywords_lineEdit->setText("");
	
	m_pEncrObj = new CRSEncrypt ( "FORTUNE" );
	//Adding Items to 'EncryptLevel' Combo Box
	ui.cmbBox_EncrLevel->addItem(tr("None"));
	ui.cmbBox_EncrLevel->addItem(tr("40 bit Encryption"));
	ui.cmbBox_EncrLevel->addItem(tr("128 bit Encryption"));

	//Adding editing options to 'Allow Changes' Combo Box
	m_strLstCmbBoxItems	<<tr("None")
		<<tr("inserting, deleting, and rotation of Pages")
		<<tr("Fill in of form fields and signing")
		<<tr("Commenting, filling in of form files, and signing")
		<<tr("Any except extracting of pages");

	//Adding Print Options to  'Allow Printing' Combo Box
	ui.cmbBox_Print->addItem(tr("None"));
	ui.cmbBox_Print->addItem(tr("Low Resolution(150 dpi)"));
	ui.cmbBox_Print->addItem(tr("High Resolution"));

	m_pGlblObjMapPtr = &Map;
	ui.cmbBox_UserPwd->addItems(m_pGlblObjMapPtr->values());
	ui.cmbBox_UserPwd->setDisabled(true);

	//CAdvSettDlgPDF odigsign(this,this);
	m_pTreeWidget = ui.treeWidget;

	m_pTreeWidget->header()->hide();

	ui.passwordEdt->setEchoMode(QLineEdit::Password);
	m_pListGrpInfo = &lstGrpInfo;
	m_pTreeWidget->setColumnCount(1);
	ui.groupBox_4->setDisabled(true);

	QString strTxt, strGroupName ,strKey, strVal;
	CGroupInfo *pGrpInfo = 0;
	int i = 0, nCount = 0;
	m_strPgLst.clear();
	m_PageInfoMap.clear();
	for(int nIndex = 0; nIndex < m_pListGrpInfo->count(); nIndex++)
	{
		pGrpInfo = m_pListGrpInfo->at(nIndex);
		if(pGrpInfo)
		{
			strGroupName = pGrpInfo->m_strGroupName;
			nCount = pGrpInfo->m_PageInfoMap.count();
			QMap<QString, QString> ::const_iterator itr =	pGrpInfo->m_PageInfoMap.constBegin();

			while(itr != pGrpInfo->m_PageInfoMap.constEnd())
			{
				strVal = strGroupName +  "." + itr.value();
				strKey = itr.key();
				m_PageInfoMap.insert(strKey, strVal);
				++itr;
			}
		}
	}
	m_strPgLst = m_PageInfoMap.values();
	ui.pageNmeCombobox->addItem("");
	ui.pageNmeCombobox->addItems(m_strPgLst);
	setValidation();

	ui.cmbBox_UserName->addItem("");
	ui.cmbBox_UserName->addItems(m_pGlblObjMapPtr->values());
	ui.timeStampEdt->setDisabled(true);
	ui.chckBx_SignImages->setChecked(true);
 
	m_pTreeWidgetItem = new QTreeWidgetItem(m_pTreeWidget);
	m_pTreeWidgetItem->setText(0, tr("Digital Signature"));

	connect(m_pTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onItemClicked(QTreeWidgetItem *, int)));
	connect(m_pTreeWidget, SIGNAL(RightMousePressed(QTreeWidgetItem*, QPoint, const int& )), this, SLOT(onItemRightClicked(QTreeWidgetItem *, QPoint, const int&)));
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
	connect(m_pTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)),
		this, SLOT(onTreeCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)));
	connect(ui.tabWidget, SIGNAL(currentChanged(int)),this,SLOT(onCurrentTabChanged(int)));
	connect(ui.cmbBox_UserName, SIGNAL(activated(int)), this, SLOT(onCmbBoxUserNameChanged(int)));
	connect(ui.Timestamp_checkBox, SIGNAL(stateChanged(int)), this, SLOT(onTimestampStateChanged(int)));


    strTxt = "";
	ui.m_cmbBox_DSItems->addItem(tr("None"));
	ui.m_cmbBox_DSItems->addItem(tr("Page Watermark"));

	CDSWatermarkProp oPgWMProp;
	m_mapDSProp.clear();
	/* Default values*/

	strTxt = "Sample";
	oPgWMProp.setValues("Arial", strTxt, 0, QRect(100,100, 400, 100), QRect(100, 100, 400, 100), 0, 35, QColor(1,1,1), false);
	m_mapDSProp.insert(1, oPgWMProp);

	m_bPgWatermark = false;

	ui.m_cmbBox_DSItems->setCurrentIndex(0);
	ui.m_cmbBox_Data->addItems(m_pGlblObjMapPtr->values());
	ui.m_cmbBox_Data->setDisabled(false);
	ui.m_cmbBox_Data->setCurrentIndex(0);
	//ui.m_groupBx_Prop->setDisabled(false);

	m_pFrame = NULL;
	m_pPlainTextObj = NULL;
	m_pPlainTextObj = new CPlainTextObj;
	m_pTextLabel = NULL;
	ui.m_Preview_frame->hide();
	
	m_strEditText = "";
	m_nXPos = 0;
	m_nYPos = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nAngle = 0;

	m_strPgWMText = "";
	m_strPgWMFont = "";
	m_nPgWMFntStyle = 0;
	m_nPgWMFntSize = 0;
	m_nPgWMAngle = 0;
	
	ui.m_spinBox_XPos->setRange(0,756);
	ui.m_spinBox_YPos->setRange(0,1070);
	ui.m_spinBox_Width->setRange(0,756);
	ui.m_spinBox_Height->setRange(0,1070);
	ui.m_spinBox_Rotation->setRange(0, 360);
	ui.m_lineEdit_font->setReadOnly(true);
	ui.m_lineEdit_color->setReadOnly(true);
	//ui.tabWidget->setTabEnabled(2, false);

	connect(ui.m_optImage,SIGNAL(toggled(bool)),this,SLOT(onImgOptionSelected(bool)));
	connect( ui.m_cmbBox_Data,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(onDataObjClicked(const QString&)));
	connect(ui.m_pushBtn_data,SIGNAL(clicked()),this,SLOT(buttonInsertClicked()));
	connect(ui.m_btnBrowseImg,SIGNAL(clicked()),this,SLOT(buttonBrowseImgClicked()));
	connect( ui.m_textEdit,SIGNAL(textChanged()),this,SLOT(setChanged()));

	connect(ui.m_cmbBox_DSItems,   SIGNAL(activated(int)), this, SLOT(on_cmbBox_DS_currentIndexChanged(int)));
	connect(ui.m_pushBtn_font,     SIGNAL(clicked()), this, SLOT(onDSFontClicked()));
	connect(ui.m_pushBtn_Colr,     SIGNAL(clicked()), this, SLOT(onDSColorClicked()));	
	connect(ui.m_Ds_Reset,         SIGNAL(clicked()), this, SLOT(onDSRestBtnClicked()));
	connect(ui.m_spinBox_Rotation, SIGNAL(valueChanged(int)), this, SLOT(onAngleValChanged(int)));
	connect(ui.m_spinBox_XPos,     SIGNAL(valueChanged(int)), this, SLOT(onXValEntered(int)));
	connect(ui.m_spinBox_YPos,     SIGNAL(valueChanged(int)), this, SLOT(onYValEntered(int)));
	connect(ui.m_spinBox_Width,    SIGNAL(valueChanged(int)), this, SLOT(onWidthValEntered(int)));
	connect(ui.m_spinBox_Height,   SIGNAL(valueChanged(int)), this, SLOT(onHeightValEntered(int)));
	connect(ui.m_DS_Save,          SIGNAL(clicked()), this, SLOT(onDSSaveClicked()));

	m_nIndex = 0;
	ui.m_cmbBox_DSItems->setCurrentIndex(m_nIndex);
	displayDSProperties(m_nIndex);


	//////////////Resource Settings Tab/////////////////
	ui.m_ImgResStGBx->setChecked(false);
	ui.m_optLocal->setChecked(false);
	ui.m_optURL->setChecked(false);
	ui.m_lblURLAdd->setEnabled(false);
	ui.m_UrlAddr->setEnabled(false);
	ui.m_MultFDScheckBox->setChecked(false);
	connect(ui.m_FileBrowseBtn , SIGNAL(clicked()), this, SLOT(onFileBrowseButtonClicked()));
	connect(ui.m_optURL , SIGNAL(clicked()) , this , SLOT(OnURLRdBtnClick()));
	connect(ui.m_optLocal , SIGNAL(clicked()) , this , SLOT(OnLocalRdBtnClick()));
	connect(ui.m_UrlAddr ,SIGNAL(textEdited(QString &)), this, SLOT(OnSetUrlPath(QString&)));
	connect(ui.m_RscLclPath ,SIGNAL(textEdited(const QString &)), this, SLOT(OnSetLocalPath(const QString&)));

	//Bookmark Settings Tab
	m_strPageList.clear();
	m_pBMCurrentItem = NULL;
	ui.m_BookmarksTreeWdgt->header()->setDefaultAlignment(Qt::AlignCenter);
	ui.m_BookmarksTreeWdgt->header()->setResizeMode(QHeaderView::ResizeToContents);
	ui.m_BookmarksTreeWdgt->setHeaderLabels(QStringList() <<tr("Bookmark Label") << tr("Destination Page"));

	m_pDocTreeWidget = ui.m_BookmarksTreeWdgt;
	m_pDocTreeWidget->setColumnCount(2);
	m_pDocTreeWidget->SetRowHasLines(true);

	m_pPageTreeWidgetItem = new QTreeWidgetItem(m_pDocTreeWidget);
	m_pPageTreeWidgetItem->setText(0, tr("Page_Level"));
	m_pPageTreeWidgetItem->setToolTip(0, tr("Right click to add Bookmarks"));

	connect(m_pDocTreeWidget, SIGNAL(RightMousePressed(QTreeWidgetItem*, QPoint, const int& )), this, SLOT(onBMItemRightClicked(QTreeWidgetItem *, QPoint, const int&)));
	connect(m_pDocTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onBMItemClicked(QTreeWidgetItem *, int)));
	connect(m_pDocTreeWidget, SIGNAL(RSDBTreeComboSelChange(const QString&)), this, SLOT(onBMComboSelectionChange(const QString&)));

}

void CAdvSettDlgPDF::setAdvSettNode(QDomElement oAdvSettNode)
{
	if(!m_pEncrObj)
		return;
	QString strHex(""), strChar(""), strTagName("");
	QDomElement oChildEle;
	m_oAdvSettNode = oAdvSettNode.cloneNode().toElement();

	/* Document watermark Properties */
	oChildEle = m_oAdvSettNode.firstChildElement("Watermark_Prop");
	strTagName = oChildEle.tagName();
	if(strTagName.compare("Watermark_Prop") == 0)
	{
		int nVal = oChildEle.attribute("PWM").toInt();
		m_mapDSProp.clear();
		CDSWatermarkProp oPgWMProp;
		QString strTmp;
		if(nVal == 1)
		{
			int nX,nY,nW,nH, nImgStat;
			m_bPgWatermark = true;

			m_strPgWMText = oChildEle.attribute("Txt"); 
			m_strPgWMFont = oChildEle.attribute("FN"); 
			m_nPgWMFntSize = oChildEle.attribute("FS").toInt();
			m_nPgWMFntStyle = oChildEle.attribute("ST").toInt(); 
			m_nPgWMAngle = oChildEle.attribute("A").toInt();
			nX = oChildEle.attribute("X").toInt(); 
			nY = oChildEle.attribute("Y").toInt();
			nW = oChildEle.attribute("W").toInt();
			nH = oChildEle.attribute("H").toInt();
			m_PgWMRect.setRect(nX, nY, nW, nH);
			nX = oChildEle.attribute("TX").toInt(); 
			nY = oChildEle.attribute("TY").toInt();
			nW = oChildEle.attribute("TW").toInt();
			nH = oChildEle.attribute("TH").toInt();
			m_PgWMTmpRect.setRect(nX, nY, nW, nH);
			m_PgWMColor = QColor(oChildEle.attribute("C"));
			nImgStat = oChildEle.attribute("Img").toInt();
			if(nImgStat == 1)
				m_bPgImageWM = true;
			else
				m_bPgImageWM = false;

			replaceKeysWithNames(m_strPgWMText, strTmp);

			if(m_bPgImageWM && strTmp.at(0) != '^')
			{
				QString strBasePath(""), strImgNm(""), strTemp("");
				int nIndex = 0;
				if(!m_strTemplatePath.isEmpty())
				{
					nIndex = m_strTemplatePath.lastIndexOf("/");
					if(nIndex == -1)
						nIndex = m_strTemplatePath.lastIndexOf("\\");

					if(nIndex > -1)
					{
						strBasePath = m_strTemplatePath.mid(0,nIndex);
						
						nIndex = 0;
						nIndex = strTmp.lastIndexOf("/");
						if(nIndex == -1)
							nIndex = strTmp.lastIndexOf("\\");

						if(nIndex > -1)
							strImgNm = strTmp.mid(nIndex+1, (strTmp.length() - nIndex));

						strTemp = QDir::toNativeSeparators(QString("%1/images/%2").arg(strBasePath).arg(strImgNm));
						strTmp = strTemp;
					}
				}
			}
			else
				m_strPgWMText = strTmp;
			oPgWMProp.setValues(m_strPgWMFont,strTmp,m_nPgWMAngle,m_PgWMRect,m_PgWMTmpRect,m_nPgWMFntStyle,m_nPgWMFntSize,m_PgWMColor,m_bPgImageWM);
			m_mapDSProp.insert(1, oPgWMProp);
			
			if(m_pPlainTextObj && !m_bPgImageWM)
				m_pPlainTextObj->setData(m_strPgWMFont,m_strPgWMText,m_nPgWMAngle,m_PgWMRect,m_PgWMTmpRect,m_nPgWMFntStyle,m_nPgWMFntSize,m_PgWMColor);
			else if(m_pPlainTextObj && m_bPgImageWM)
				m_pPlainTextObj->setData(m_strPgWMText,m_nPgWMAngle,m_PgWMRect,m_PgWMTmpRect);

			m_nIndex = 1;
			ui.m_cmbBox_DSItems->setCurrentIndex(m_nIndex);
		}
		else
		{
			m_bPgWatermark = false;

			strTmp = "Sample";
			oPgWMProp.setValues("Arial",strTmp,0,QRect(100,100,400,100),QRect(100,100,400,100),0,35,QColor(1,1,1),false);
			m_mapDSProp.insert(1, oPgWMProp);

			if(m_pPlainTextObj)
				m_pPlainTextObj->setData("Arial",strTmp,0,QRect(100,100,400,100),QRect(100,100,400,100),0,35,QColor(1,1,1));

			m_nIndex = 0;
			ui.m_cmbBox_DSItems->setCurrentIndex(m_nIndex);
		}
		displayDSProperties(m_nIndex);
	}

	/* Document Properties */
	oChildEle = m_oAdvSettNode.firstChildElement("Document_Prop");
	strTagName = oChildEle.tagName();
	if(strTagName.compare("Document_Prop") == 0)
	{
		m_strTitle = oChildEle.attribute("Title");
		m_strAuthor = oChildEle.attribute("Author");
		m_strSubject = oChildEle.attribute("Subject");
		m_strKeywords = oChildEle.attribute("Keywords");

		ui.Title_lineEdit->setText(m_strTitle);
		ui.Author_lineEdit->setText(m_strAuthor);
		ui.Subject_lineEdit->setText(m_strSubject);
		ui.Keywords_lineEdit->setText(m_strKeywords);
	}
	
	oChildEle.clear();
	strTagName.clear();
	oChildEle = m_oAdvSettNode.firstChildElement("Security_Settings");
	strTagName = oChildEle.tagName();

	if(strTagName.compare("Security_Settings") == 0)
	{
		if(!m_oAdvSettNode.isNull())
		{
			convertHexToChar(oChildEle.attribute("Security_Level"), strHex);
			m_pEncrObj->Decrypt_New( strHex, strChar, TYPE_DATA, TYPE_DATA);

			if(strChar.compare(tr("40")) == 0)
				ui.cmbBox_EncrLevel->setCurrentIndex(1);
			else if( strChar.compare(tr("128")) == 0)
				ui.cmbBox_EncrLevel->setCurrentIndex(2);
			else
			{
				ui.cmbBox_EncrLevel->setCurrentIndex(0);
				//return;
			}

			strChar.clear();
			convertHexToChar( oChildEle.attribute("Mstr"), strHex);
			m_pEncrObj->Decrypt_New( strHex, strChar, TYPE_DATA, TYPE_DATA);
			if( !strChar.isEmpty() )
			{
				ui.chkBox_MasterPwd->setChecked(true);
				ui.lnEdit_MasterPwd->setText(strChar);
			}

			strChar.clear();
			convertHexToChar( oChildEle.attribute("Usr"), strHex);
			m_pEncrObj->Decrypt_New( strHex, strChar, TYPE_DATA, TYPE_DATA);

			if( !strChar.isEmpty() )
			{
				ui.chkBox_UserPwd->setChecked(true);
				ui.cmbBox_UserPwd->setCurrentIndex(ui.cmbBox_UserPwd->findText(m_pGlblObjMapPtr->value(strChar),Qt::MatchExactly));
			}

			strChar.clear();
			convertHexToChar( oChildEle.attribute("CmbChange"), strHex);
			m_pEncrObj->Decrypt_New( strHex, strChar, TYPE_DATA, TYPE_DATA);
			ui.cmbBox_Changes->setCurrentIndex(strChar.toInt());

			strChar.clear();
			convertHexToChar( oChildEle.attribute("CmbPrint"), strHex);
			m_pEncrObj->Decrypt_New( strHex, strChar, TYPE_DATA, TYPE_DATA);
			ui.cmbBox_Print->setCurrentIndex(strChar.toInt());

			strChar.clear();
			convertHexToChar( oChildEle.attribute("ChkCopy"), strHex);
			m_pEncrObj->Decrypt_New( strHex, strChar, TYPE_DATA, TYPE_DATA);
			ui.chkBox_Copy->setChecked(strChar.toInt());

			strChar.clear();
			convertHexToChar( oChildEle.attribute("ChkTxtAcc"), strHex);
			m_pEncrObj->Decrypt_New( strHex, strChar, TYPE_DATA, TYPE_DATA);
			ui.chkBox_txtAccess->setChecked(strChar.toInt());

		}
	}
	oChildEle.clear();
	oChildEle = m_oAdvSettNode.firstChildElement("Digital_Signature");
	strTagName = oChildEle.tagName();
	if(strTagName.compare("Digital_Signature")==0)
	{
		loadDigitalSignNode();
	}
	
	/* Resource Properties */
	oChildEle = m_oAdvSettNode.firstChildElement("PDFRES");
	strTagName = oChildEle.tagName();
	if(strTagName.compare("PDFRES") == 0)
	{
		m_strImgLocPath = oChildEle.attribute("L");
		m_strImgUrlPath = oChildEle.attribute("U");
		ui.m_ImgResStGBx->setChecked(true);

		if(!m_strImgUrlPath.isEmpty())
		{
			OnURLRdBtnClick();
			ui.m_RscLclPath->setText(m_strImgLocPath);
			ui.m_UrlAddr->setText(m_strImgUrlPath);
		}
		else if(!m_strImgLocPath.isEmpty())
		{
			ui.m_optLocal->setChecked(true);
			ui.m_optURL->setChecked(false);
			ui.m_RscLclPath->setText(m_strImgLocPath);
		}
		
		int nVal = oChildEle.attribute("M").toInt();
		if(nVal == 1)
			ui.m_MultFDScheckBox->setChecked(true);
		else
			ui.m_MultFDScheckBox->setChecked(false);
	}

	// Bookmark Settings	
	QString strAttributeName, strPageID, strPageName;
	QTreeWidgetItem * pTempTreeWidgetItem;

	if(m_pPageTreeWidgetItem)
	{
		for(int nIndex = 0; nIndex < m_pPageTreeWidgetItem->childCount(); nIndex++)
		{
			pTempTreeWidgetItem = m_pPageTreeWidgetItem->child(nIndex);
			if(pTempTreeWidgetItem)
			{
				m_pPageTreeWidgetItem->removeChild(pTempTreeWidgetItem);
				delete pTempTreeWidgetItem;
				pTempTreeWidgetItem = NULL;
			}
		}
	}

	m_strPageList.clear();
	oChildEle.clear();
	oChildEle =  m_oAdvSettNode.firstChildElement("Bookmarks_Prop");
	strTagName = oChildEle.tagName();
	if(strTagName.compare("Bookmarks_Prop") == 0)
	{
		ui.m_BMRootNode_LnEdt->setText(oChildEle.attribute("RootNd"));

		QDomNodeList nodesList = oChildEle.childNodes();
		QDomElement tempEle;
		if(m_pPageTreeWidgetItem)
		{
			for(int n = 0; n < nodesList.count(); n++)
			{
				tempEle  = nodesList.at(n).toElement();
				strAttributeName  = tempEle.attribute("name");
				strPageID = tempEle.attribute("value");

				pTempTreeWidgetItem = new QTreeWidgetItem(m_pPageTreeWidgetItem);
				if(pTempTreeWidgetItem)
				{
					pTempTreeWidgetItem->setText(0, strAttributeName);
					strPageName        = m_PageInfoMap.value(strPageID);
					pTempTreeWidgetItem->setText(1, strPageName);
					m_strPageList.append(strAttributeName);
				}
			}

			m_pPageTreeWidgetItem->setExpanded(true);
		}
	}
}
void CAdvSettDlgPDF::OnURLRdBtnClick()
{
	ui.m_lblURLAdd->setEnabled(true);
	ui.m_UrlAddr->setEnabled(true);
	ui.m_optURL->setChecked(true);
}

void CAdvSettDlgPDF::OnLocalRdBtnClick()
{
	ui.m_lblURLAdd->setEnabled(false);
	ui.m_UrlAddr->setEnabled(false);
	ui.m_UrlAddr->setText("");
}

void CAdvSettDlgPDF::OnSetUrlPath(const QString& strURLPath)
{
	ui.m_UrlAddr->setText(strURLPath);
	m_strImgUrlPath = ui.m_UrlAddr->text();
}

void CAdvSettDlgPDF::OnSetLocalPath(const QString& strLclPath)
{
	if(!ui.m_optURL->isChecked())
		ui.m_optLocal->setChecked(true);
	ui.m_RscLclPath->setText(strLclPath);
	m_strImgLocPath = ui.m_RscLclPath->text();
}

void CAdvSettDlgPDF::onFileBrowseButtonClicked()
{
	if(!ui.m_optURL->isChecked())
		ui.m_optLocal->setChecked(true);
	QString strDir(QFileDialog::getExistingDirectory());

	if( !strDir.isEmpty() )
		ui.m_RscLclPath->setText(strDir);
}

void CAdvSettDlgPDF::on_cmbBox_EncrLevel_currentIndexChanged(int nIndex)
{
	if( nIndex == 0 ) //No encryption.
	{
		ui.grpBox_Permissions->setDisabled(true);
		ui.grpBox_UserPwd->setDisabled(true);
	}
	else // 40 or 128 bit Encryption
	{
		ui.grpBox_Permissions->setDisabled(false);
		ui.grpBox_UserPwd->setDisabled(false);
		ui.cmbBox_Changes->clear();
		ui.cmbBox_Changes->addItems(m_strLstCmbBoxItems);

		if(nIndex == 1)// 40 bit Encryption.
		{
			ui.cmbBox_Changes->removeItem(1);
			ui.chkBox_txtAccess->hide();

			if(ui.cmbBox_Print->count() == 3) // remove "High resolution" Item.
				ui.cmbBox_Print->removeItem(2);
		}
		else if(nIndex == 2)//128 bit Encryption.
		{
			ui.chkBox_txtAccess->show();
			if(!ui.chkBox_Copy->isEnabled())
				ui.chkBox_txtAccess->setDisabled(true);
			else
				ui.chkBox_txtAccess->setDisabled(false);
			if(ui.cmbBox_Print->count() == 2)
				ui.cmbBox_Print->addItem(tr("High Resolution"));
		}
	}
}

void CAdvSettDlgPDF::on_chkBox_UserPwd_stateChanged(int nState)
{
	if(nState)
		ui.cmbBox_UserPwd->setDisabled(false);
	else
		ui.cmbBox_UserPwd->setDisabled(true);
}

void CAdvSettDlgPDF::on_chkBox_MasterPwd_stateChanged(int nState)
{
	bool bChecked = true;
	if(nState)
		bChecked = false;

	ui.lnEdit_MasterPwd->setDisabled(bChecked);
	ui.cmbBox_Changes->setDisabled(bChecked);
	ui.cmbBox_Print->setDisabled(bChecked);
	ui.chkBox_txtAccess->setDisabled(bChecked);
	ui.chkBox_Copy->setDisabled(bChecked);
}

void CAdvSettDlgPDF::on_chkBox_Copy_stateChanged(int nState)
{
	if(nState)
	{
		if(ui.chkBox_txtAccess->checkState() == Qt::Unchecked)
			ui.chkBox_txtAccess->setCheckState(Qt::PartiallyChecked);
		ui.chkBox_txtAccess->setDisabled(true);
	}
	else
	{
		if(ui.chkBox_txtAccess->checkState() == Qt::PartiallyChecked)
			ui.chkBox_txtAccess->setCheckState(Qt::Unchecked);
		ui.chkBox_txtAccess->setDisabled(false);
	}

}

// to convert a partial check to a full check (Tristate to dual state)
void CAdvSettDlgPDF::on_chkBox_txtAccess_clicked()
{
	if(ui.chkBox_txtAccess->checkState() == Qt::PartiallyChecked)
		ui.chkBox_txtAccess->setCheckState(Qt::Checked);
}

//void CAdvSettDlgPDF::on_ok_clicked()
void CAdvSettDlgPDF::onOk()
{
	QString strTemp, strEncrData, strData, strEnc;
	QDomElement oTempEle;
	bool bStatus = true;
	unsigned char chPerm[5] = "";
	int nSecurityIndex = ui.cmbBox_EncrLevel->currentIndex();
	QDomDocument pDoc("Sample");
	m_oAdvSettNode.clear();
	m_oAdvSettNode = pDoc.createElement("AdvTab");

	if( nSecurityIndex == 0 )
	{
		strEnc = tr("0");
		m_pEncrObj->Encrypt_New(strEnc, strTemp, TYPE_DATA, TYPE_DATA);
		convertCharToHex( strTemp, strEncrData );
		oTempEle = pDoc.createElement("Security_Settings");
		oTempEle.setAttribute("Security_Level", strEncrData);
		//accept();
	}
	else
	{
		if(ui.chkBox_MasterPwd->isChecked()  || ui.chkBox_UserPwd->isChecked())
		{
			if(ui.chkBox_MasterPwd->isChecked() && ui.lnEdit_MasterPwd->text().isEmpty())
			{
				onCurrentTabChanged(0);
				QMessageBox::warning( this, tr("Blank fields!!!"), tr("Permissions Password field is left blank\n Enter 'Permissions Password' to continue"),QMessageBox::Ok,QMessageBox::Ok);
				bStatus = false;
			}

			if(bStatus) //if everything is entered properly.. write to Dom Node.
			{
				oTempEle = pDoc.createElement("Security_Settings");

				// Setting the Security Level.
				if(nSecurityIndex == 1)
				{
					strEnc = tr("40");
					m_pEncrObj->Encrypt_New(strEnc, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("Security_Level", strEncrData);
				}
				else
				{
					strEnc = tr("128");
					m_pEncrObj->Encrypt_New(strEnc, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("Security_Level", strEncrData);
				}

				if(ui.chkBox_UserPwd->isChecked())
				{
					strEnc = ui.cmbBox_UserPwd->itemText(ui.cmbBox_UserPwd->currentIndex());
					m_pEncrObj->Encrypt_New(m_pGlblObjMapPtr->key(strEnc), strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("Usr"  , strEncrData);
				}

				if(ui.chkBox_MasterPwd->isChecked())
				{
					calculatePermissions(ui.cmbBox_EncrLevel->currentIndex(), chPerm);

					strEnc = QString((const char *)chPerm);
					m_pEncrObj->Encrypt_New(strEnc, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("Perm", strEncrData);


					strEnc = ui.lnEdit_MasterPwd->text();
					m_pEncrObj->Encrypt_New(strEnc, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("Mstr", strEncrData);

					strData = QString("%1").arg(ui.cmbBox_Changes->currentIndex());
					m_pEncrObj->Encrypt_New(strData, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("CmbChange", strEncrData);

					strData.clear();
					strData = QString("%1").arg(ui.cmbBox_Print->currentIndex());
					m_pEncrObj->Encrypt_New(strData, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("CmbPrint", strEncrData);

					strData.clear();
					strData = QString("%1").arg(ui.chkBox_Copy->checkState());
					m_pEncrObj->Encrypt_New(strData, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("ChkCopy", strEncrData);

					strData.clear();
					strData = QString("%1").arg(ui.chkBox_txtAccess->checkState());
					m_pEncrObj->Encrypt_New(strData, strTemp, TYPE_DATA, TYPE_DATA);
					convertCharToHex( strTemp, strEncrData );
					oTempEle.setAttribute("ChkTxtAcc", strEncrData);
				}
			}
		}
		else
		{
			onCurrentTabChanged(0);
			QMessageBox::warning( this, tr("No Password selected!!!"), tr("Select a password to continue"),QMessageBox::Ok,QMessageBox::Ok);
			bStatus = false;
		}
	}
	m_oAdvSettNode.appendChild(oTempEle);
	bool bValidate = true;
	if(m_pTreeWidgetItem != NULL)
	{
		if((m_bEditMargin || (m_bAddNew) || m_bLoadValues) && m_pTreeWidgetItem->childCount() != 0)
		{
			bValidate = validateInput();
			if(bValidate)
			{
				if(saveToXml())
				{
					bValidate = true;
				}
				else
				{
					bValidate = false;
				}
			}
		}
	}

	oTempEle.clear();
	oTempEle = pDoc.createElement("Watermark_Prop");
	if(oTempEle.isElement())
	{
		if(m_bPgWatermark)
			oTempEle.setAttribute("PWM", QString("1"));
		else
			oTempEle.setAttribute("PWM", QString("0"));
		if(m_bPgWatermark)
		{
			CDSWatermarkProp oPgWMProp;
			oPgWMProp = m_mapDSProp.value(1);
			m_strPgWMText = oPgWMProp.m_strText;
			m_strPgWMFont = oPgWMProp.m_strFont;
			m_nPgWMFntSize = oPgWMProp.m_nFontSize;
			m_nPgWMFntStyle = oPgWMProp.m_nFontStyle;
			m_nPgWMAngle = oPgWMProp.m_nAngle;
			m_PgWMColor = oPgWMProp.m_Color;
			m_PgWMRect = oPgWMProp.m_Rect;
			m_PgWMTmpRect = oPgWMProp.m_TmpRect;
			m_bPgImageWM = oPgWMProp.m_bImageWM;

			QString strTmp("");
			replaceNamesWithKeys(m_strPgWMText, strTmp);
			oTempEle.setAttribute("Txt",strTmp);
			oTempEle.setAttribute("FN", m_strPgWMFont);
			oTempEle.setAttribute("FS", m_nPgWMFntSize);
			oTempEle.setAttribute("ST", m_nPgWMFntStyle);
			oTempEle.setAttribute("A", m_nPgWMAngle);
			oTempEle.setAttribute("X", m_PgWMRect.topLeft().x());
			oTempEle.setAttribute("Y", m_PgWMRect.topLeft().y());
			oTempEle.setAttribute("W", m_PgWMRect.width());
			oTempEle.setAttribute("H", m_PgWMRect.height());
			oTempEle.setAttribute("TX", m_PgWMTmpRect.topLeft().x());
			oTempEle.setAttribute("TY", m_PgWMTmpRect.topLeft().y());
			oTempEle.setAttribute("TW", m_PgWMTmpRect.width());
			oTempEle.setAttribute("TH", m_PgWMTmpRect.height());
			oTempEle.setAttribute("C", (m_PgWMColor).name());
			if(m_bPgImageWM)
				oTempEle.setAttribute("Img",1);
			else
				oTempEle.setAttribute("Img",0);
		}

		m_oAdvSettNode.appendChild(oTempEle);
	}
//#endif	
	
	/* Document Properties */
	oTempEle.clear();
	oTempEle = pDoc.createElement("Document_Prop");
	if(oTempEle.isElement())
	{
		m_strTitle = ui.Title_lineEdit->text();
		m_strAuthor = ui.Author_lineEdit->text();
		m_strSubject = ui.Subject_lineEdit->text();
		m_strKeywords = ui.Keywords_lineEdit->text();

		oTempEle.setAttribute("Title", m_strTitle);
		oTempEle.setAttribute("Author", m_strAuthor);
		oTempEle.setAttribute("Subject", m_strSubject);
		oTempEle.setAttribute("Keywords", m_strKeywords);
		m_oAdvSettNode.appendChild(oTempEle);
	}
	
	/* Resource settings */
	oTempEle.clear();
	if(ui.m_ImgResStGBx->isChecked())
	{
		oTempEle = pDoc.createElement("PDFRES");
		if(oTempEle.isElement())
		{
			if(validateImgSett())
			{
				m_strImgLocPath = ui.m_RscLclPath->text();
				m_strImgUrlPath = ui.m_UrlAddr->text();
				oTempEle.setAttribute("L", m_strImgLocPath);
				oTempEle.setAttribute("U", m_strImgUrlPath);
				if(ui.m_MultFDScheckBox->isChecked())
					oTempEle.setAttribute("M", 1);
				else
					oTempEle.setAttribute("M", 0);
				m_oAdvSettNode.appendChild(oTempEle);
			}
			else
				bStatus = false;
		}
	}
	/* Bookmark Settings*/
	oTempEle.clear();
	oTempEle = pDoc.createElement("Bookmarks_Prop");	
	oTempEle.setAttribute("RootNd", ui.m_BMRootNode_LnEdt->text());
	m_oAdvSettNode.appendChild(oTempEle);

	QDomElement	oEle;	
	QTreeWidgetItem *pTempItem = NULL;
	QString strAttributeName, strValue, strPageID;
	int nRowCount = m_pPageTreeWidgetItem->childCount();
	for(int nIndex = 0; nIndex < nRowCount; nIndex++)
	{
		pTempItem = m_pPageTreeWidgetItem->child(nIndex);
		if(pTempItem)
		{
			oEle = pDoc.createElement("Tag");
			strAttributeName = pTempItem->text(0);
			strValue         = pTempItem->text(1);
			strPageID        = m_PageInfoMap.key(strValue);
			oEle.setAttribute("name", strAttributeName);
			oEle.setAttribute("value", strPageID);

			oTempEle.appendChild(oEle);

		}
	}
	
	if(bValidate == true && bStatus == true)
	{
		accept();
	}
}

bool CAdvSettDlgPDF::validateImgSett()
{
	if(ui.m_optLocal->isChecked() || ui.m_optURL->isChecked())
	{
		if(ui.m_optURL->isChecked())
		{
			m_strImgLocPath = ui.m_RscLclPath->text();
			m_strImgUrlPath = ui.m_UrlAddr->text();
			if(ui.m_RscLclPath->text().isEmpty())
			{
				onCurrentTabChanged(4);
				QMessageBox::warning( this, tr("Invalid Local Path!!!"), tr("Enter Local Path"),QMessageBox::Ok,QMessageBox::Ok);
				return false;
			}
			if(ui.m_UrlAddr->text().isEmpty())	
			{
				onCurrentTabChanged(4);
				QMessageBox::warning( this, tr("Invalid URL Path!!!"), tr("Enter URL Path"),QMessageBox::Ok,QMessageBox::Ok);
				return false;
			}
		}

		if(ui.m_optLocal->isChecked() && ui.m_RscLclPath->text().isEmpty())
		{
			onCurrentTabChanged(4);
			QMessageBox::warning( this, tr("Invalid Local Path!!!"), tr("Enter Local Path"),QMessageBox::Ok,QMessageBox::Ok);
			return false;
		}
	}
	else
	{
		onCurrentTabChanged(4);
		QMessageBox::warning( this, tr("No Path selected!!!"), tr("Select a path option to continue"),QMessageBox::Ok,QMessageBox::Ok);
		return false;
	}
	return true;
}

void CAdvSettDlgPDF::calculatePermissions(int nIndex, unsigned char *chPerm)
{
	// nPrint - Bit 3;			nModify - Bit 4;		nCopy - Bit 5;
	// nForms - Bit 6;			nFillForms - Bit 9;		nTxtAccess - Bit 10;
	// nQualityPrint - Bit 12;							nPageChanges - Bit 11;
	int nPrint = 0, nModify = 0, nCopy = 0, nForms = 0, nFillForms = 0, nTxtAccess = 0, nPageChanges = 0, nQualityPrint = 0;
	//------------------------------------------------
	//					40 bit encryption.
	//------------------------------------------------
	if(nIndex == 1)
	{
		nPrint = ui.cmbBox_Print->currentIndex();
		switch(ui.cmbBox_Changes->currentIndex())
		{
		case 1:
			nModify = 1;							//bit 4
			break;
		case 2:
			nForms  = 1;							//bit 6
			break;
		case 3:
			nForms  = 1;							//bits 6 n 4
			nModify = 1;
			break;
		}

		nCopy = ui.chkBox_Copy->isChecked();

		chPerm[0] = 192 + nForms*32 + nCopy*16 + nModify*8 + nPrint*4;
		chPerm[1] = 255;
		chPerm[2] = 255;
		chPerm[3] = 255;
	}
	//------------------------------------------------
	//					128bit encryption.
	//------------------------------------------------
	else if(nIndex == 2)
	{
		switch(ui.cmbBox_Print->currentIndex())
		{
		case 1:
			nPrint  = 1;							//bit 3
			break;
		case 2:
			nPrint  = 1;	
			nQualityPrint  = 1;						//bit 12
			break;
		}
		switch(ui.cmbBox_Changes->currentIndex())
		{
		case 1:
			nPageChanges = 1;						//bit 11
			break;
		case 2:
			nFillForms  = 1;						//bit 9
			break;
		case 3:
			nForms  = 1;							//bits 6 n 9
			nFillForms = 1;
			break;
		case 4:
			nFillForms = 1;							//bits 9 n 4
			nModify = 1;
			nForms = 1;
			break;
		}

		nCopy = ui.chkBox_Copy->isChecked();		//bit 5

		if(ui.chkBox_txtAccess->checkState() == Qt::PartiallyChecked || ui.chkBox_txtAccess->checkState() == Qt::Checked)
			nTxtAccess = 1;							//bit 10

		chPerm[0] = 192 + nForms*32 + nCopy*16 + nModify*8 + nPrint*4;
		chPerm[1] = 240 + nQualityPrint*8 + nPageChanges*4 + nTxtAccess*2 + nFillForms;
		chPerm[2] = 255;
		chPerm[3] = 255;
	}
}

void CAdvSettDlgPDF::convertCharToHex(const QString &strChar, QString &strHex)
{
	unsigned char chTemp;
	char *pHexString = NULL;
	int ni=0, nindex = 0, nValue = 0, nLen = strChar.length(), nCount = 0;
	strHex.clear();

	if (pHexString = (char*) malloc( sizeof(char)*(nLen*2)+1 ))
	{
		memset( pHexString, 0, sizeof(char)*(nLen*2)+1 );
		for(; nindex < nLen ; nindex++)
		{
			chTemp = strChar.at(nindex).toAscii();
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

void CAdvSettDlgPDF::convertHexToChar(const QString &strInput, QString &strDest)
{
	int nCtr  = 0;
	int nLen = strInput.length();
	unsigned char chHigher, chLower;
	strDest.clear();
	for (; nCtr < nLen-1; nCtr+=2)
	{
		chHigher = strInput.at(nCtr).toAscii();
		chLower  = strInput.at(nCtr+1).toAscii();

		if(chHigher < 58)
			chHigher -= 48;
		else if(chHigher > 64 && chHigher < 71)
			chHigher -= 55;
		else
			chHigher -= 87;

		if(chLower < 58)
			chLower -= 48;
		else if(chLower > 64 && chLower < 71)
			chLower -= 55;
		else
			chLower -= 87;

		strDest.append(QChar(chHigher * 16 + chLower));
	}
}

/*************************Digital signature dialog starts here***********************************************************/
void CAdvSettDlgPDF::onItemClicked(QTreeWidgetItem* pItem, int nCol)
{
	if(pItem->text(0) == tr("Digital Signature"))
	{
		ui.groupBox_4->setDisabled(true);
	}
	else
	{
		if(pItem)
		{
			m_pCurrentItem = pItem;
			ui.groupBox_4->setDisabled(false);
			CDSData *pData = NULL;
			pData = qvariant_cast<CDSData*>(pItem->data(nCol,Qt::UserRole));
			if(pData)
			{
				setData(pData);
			}
		}
	}
}

void CAdvSettDlgPDF::setValidation()
{
	QDoubleValidator *pValidator = NULL;
	pValidator = new QDoubleValidator(this);
	pValidator->setNotation(QDoubleValidator::StandardNotation);
	pValidator->setRange(0, 10, 2);
	ui.leftEdt->setValidator(pValidator);
//	ui.topEdt->setValidator(pValidator);
	ui.widthEdt->setValidator(pValidator);
	ui.heightEdit->setValidator(pValidator);
}

bool CAdvSettDlgPDF::validateInput()
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

//to set validation for TimeStamp
//void CAdvSettDlgPDF::setValidationToTimeStamp()
//{
//	QRegExpValidator *pTimeStampValidator = NULL;
//	QRegExp regExp("^[0-9]+(:{0,1}[0-9]+){1,2}[0-9]+$");
//	pTimeStampValidator = new QRegExpValidator(regExp, ui.timeStampEdt);
//	ui.timeStampEdt->setValidator(pTimeStampValidator);
//	pTimeStampValidator = NULL;
//}

//on browse button clicked
void CAdvSettDlgPDF::onBrowse()
{
	QString strPfxFilePath("");
	strPfxFilePath = QFileDialog::getOpenFileName(this, tr("Open pfx file"), QDir::currentPath(), "*.pfx");
	ui.pfxFilePathEdt->setText(strPfxFilePath);

	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		if(m_pCurrentItem)
		{
			pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
			if(pDataptr)
				pDataptr->m_strPfxPathName = ui.pfxFilePathEdt->text();
		}
	}
}

void CAdvSettDlgPDF::onComboBoxItemChanged(int nIndex)
{
	if(m_pCurrentItem)
	{
		if(m_pCurrentItem->text(0) != tr("Digital Signature"))
		{
			CDSData *pDataptr = NULL;
			pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
			if(pDataptr)
			{				
				pDataptr->m_strPageId = m_PageInfoMap.key(ui.pageNmeCombobox->itemText(nIndex));
			}
		}
	}
}

void CAdvSettDlgPDF::onCmbBoxUserNameChanged(int nIndex)
{
	if(m_pCurrentItem)
	{
		if(m_pCurrentItem->text(0) != tr("Digital Signature"))
		{
			CDSData *pDataptr = NULL;
			pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
			if(pDataptr)
				pDataptr->m_strUsrName = m_pGlblObjMapPtr->key(ui.cmbBox_UserName->itemText(nIndex));
		}
	}
}

void CAdvSettDlgPDF::onTimestampStateChanged(int nState)
{
	if(nState == 0)
		ui.timeStampEdt->setDisabled(true);
	else
		ui.timeStampEdt->setDisabled(false);
}

//on right chicking an item. creating a context menu
void CAdvSettDlgPDF::onItemRightClicked(QTreeWidgetItem* pItem, QPoint pos, const int& /*nCol*/)
{
	if(!pItem)
		return;
	
	QAction* pAction = NULL;
	
	QList<QAction*> menuList;
	menuList.clear();

	QAction* pAddAction(new QAction(QIcon(":/images/Add.png"), tr("&Add New"), this));
	QAction* pRenameAction(new QAction(QIcon(":/images/Rename.png"), tr("&Rename"), this));
	QAction* pDeleteAction(new QAction(QIcon(":/images/Delete.png"), tr("&Delete"), this));

	QString strActionText("");
	if(pItem->text(0) == tr("Digital Signature"))
	{
		menuList.insert(0, pAddAction);
		
		m_pTreeWidget->CreateSetRClkMenuData(menuList);
		pAction = m_pTreeWidget->ExecPopupMenu(pos);
		
		if(pAction!= NULL)
		{
			strActionText = pAction->text();
			if(strActionText.compare(menuList.value(0)->text()) == 0)
			{
				m_bLoadValues = false;
				m_bAddNew = true;/*To validate at ok button for digital signature inputs*/
				addNewItem();
			}
		}
	}
	else
	{
		m_bAddNew = false;

		menuList.insert(0, pRenameAction);
		menuList.insert(1, pDeleteAction);

		m_pTreeWidget->CreateSetRClkMenuData(menuList);
		pAction = m_pTreeWidget->ExecPopupMenu(pos);

		if(pAction!= NULL)
		{
			strActionText = pAction->text();
			if(strActionText.compare(menuList.value(0)->text()) == 0)
			{
				onRename(pItem);
			}
			else if(strActionText.compare(menuList.value(1)->text()) == 0)
			{
				onDelete();
			}
		}
	}
	
	foreach(QAction *pAction, menuList)
	{
		if(pAction)
		{
			delete pAction;
			pAction = NULL;
		}
	}

	menuList.clear();
}

//Adding new item to the tree
void CAdvSettDlgPDF::addNewItem()
{
	QTreeWidgetItem *pItem = NULL;
	int nCount = m_pTreeWidgetItem->childCount();
	//if(nCount <= 0)
	{
		pItem = new QTreeWidgetItem(m_pTreeWidgetItem);
		if(pItem)
		{
			QString strName("");
			strName = tr("Sign%1").arg(nCount+1);
			if(m_strLstSignNames.contains(strName, Qt::CaseSensitive))
			{
				strName = tr("Sign%1").arg(++nCount);
			}
			pItem->setText(0, strName);
			m_strLstSignNames.append(strName);
			CDSData *pDSData = new CDSData();
			if(pDSData)
			{
				pDSData->m_strSignName = strName;
				pItem->setData(0,Qt::UserRole, qVariantFromValue(pDSData));
				m_pTreeWidgetItem->setExpanded(true);
				QDateTime oTimeStamp;
				oTimeStamp = oTimeStamp.currentDateTime().toLocalTime();
				pDSData->m_strTimeStamp = oTimeStamp.toString("hh:mm:ss");
			}
			if(!m_bLoadValues)
			{
				ui.pfxFilePathEdt->setDisabled(false);
				ui.passwordEdt->setDisabled(false);
				ui.m_browseBtn->setDisabled(false);
			}
		}
	}
	/*else
	{
		m_bLoadValues = true;
		onCurrentTabChanged(1);
		QMessageBox::critical(this,"Error","Cannot Add more than one signature");
	}*/
}

//On renaming an item
void CAdvSettDlgPDF::onRename(QTreeWidgetItem *pItem)//On renaming an item
{
	if(!pItem)
		return;

	QString strOldName("");
	QString strNewName("");
	bool bOk = false;
	strOldName = pItem->text(0);
	strNewName = QInputDialog::getText(this, tr("Renaming Item"), tr("Enter Name"), QLineEdit::Normal, strOldName, &bOk);
	if(bOk)
	{
		QRegExp regExpName("^[A-Za-z]+(_{0,1}[a-zA-Z0-9]+){1,}$");
		if(regExpName.indexIn(strNewName) != -1)
		{
			if(m_strLstSignNames.contains(strNewName, Qt::CaseSensitive))
			{
				onCurrentTabChanged(1);
				QMessageBox::warning(0, tr("Renaming Item"), tr("Name already exists."));
				onRename(pItem);
			}
			else
			{
				pItem->setText(0, strNewName);
				CDSData *pData = NULL;
				pData = qvariant_cast<CDSData *>(pItem->data(0,Qt::UserRole));
				if(pData)
					pData->m_strSignName = strNewName;
			}
		}
		else
		{
			onCurrentTabChanged(1);
			QMessageBox::information(0, tr("Renaming Item"), tr("Invalid Item name"));
		}
	}
}

void CAdvSettDlgPDF::onEdit()
{}

//oOn deleting an item
void CAdvSettDlgPDF::onDelete()
{
	CDSData *pData = NULL;
	QTreeWidgetItem* pItem = m_pTreeWidget->currentItem();
	pData = qvariant_cast<CDSData *>(pItem->data(0,Qt::UserRole));
	if(pData)
	{
		m_strLstSignNames.removeAt(m_strLstSignNames.indexOf(pData->m_strSignName));
		delete pData;
		pData = NULL;
	}
	if(pItem)
	{
		delete pItem;
		pItem = NULL;
	}
	if(m_pTreeWidgetItem->childCount() == 0)
	{
		ui.groupBox_4->setDisabled(true);
		ui.locationEdt->clear();
		ui.pfxFilePathEdt->clear();
		ui.passwordEdt->clear();
		ui.leftEdt->clear();
		ui.reasonEdt->clear();
		ui.timeStampEdt->clear();
		ui.topEdt->clear();
		ui.widthEdt->clear();
		ui.heightEdit->clear();
		ui.cmbBox_UserName->setCurrentIndex(0);
		ui.pageNmeCombobox->setCurrentIndex(0);
		ui.chckBx_PageLast->setChecked(false);		
		ui.PubKey_chckBx->setChecked(false);
	}
}

bool CAdvSettDlgPDF::saveToXml()
{
	QDomDocument doc;
	QDomElement oRootElement;
	QDomElement oChildEle, oDummyEle;
	bool bRes = true;
	QTreeWidgetItem *pTreeItem = NULL;
	if(m_pTreeWidgetItem!= NULL)
	{
		pTreeItem = m_pTreeWidget->topLevelItem(0);
		oRootElement = doc.createElement("Digital_Signature");

		if(pTreeItem)
		{
			QString strRootName = pTreeItem->text(0);
			strRootName.replace(" ","_");

			int nSize = m_pTreeWidgetItem->childCount();
			QString strAppPath = RSAppPath;
			QString strPemHex,strCerHex,strTemp,strHex;

			QString strName = "", strPfxPth, strPasswd;
			QTreeWidgetItem *pItem = 0;
			CDSData *pData = 0;
			for(int nIndex = 0; nIndex < nSize; nIndex++)
			{
				pItem = m_pTreeWidgetItem->child(nIndex);
				pData = qvariant_cast<CDSData*>(pItem->data(0,Qt::UserRole));
				if(pData)
				{
					strName = pData->m_strSignName;
					oChildEle = doc.createElement(strName);

					strCerHex = pData->m_strCert;
					strPemHex = pData->m_strPem;
					strPfxPth = pData->m_strPfxPathName;
					strPasswd = pData->m_strPassword;

					if(!strPfxPth.isEmpty() && !strPasswd.isEmpty())
					{
						bRes = GenerateCertificate(strPfxPth,strPasswd,strPemHex,strCerHex);

						if(m_pEncrObj)
						{
							m_pEncrObj->Encrypt_New(strCerHex,strTemp,TYPE_DATA,TYPE_DATA);
							convertCharToHex(strTemp,strHex);
							strCerHex = strHex;

							m_pEncrObj->Encrypt_New(strPemHex,strTemp,TYPE_DATA,TYPE_DATA);
							convertCharToHex(strTemp,strHex);
							strPemHex = strHex;
						}
					}
					
					if(bRes == true)
					{
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
						oRootElement.appendChild(oChildEle);
					}
				}
			}
		}
		m_oAdvSettNode.appendChild(oRootElement);
	}
	return bRes;
}

void CAdvSettDlgPDF::onCancel()
{
	reject();
}

void CAdvSettDlgPDF::setData(CDSData* pData)
{
	if(!pData)
		return;

	int nSize = 0;
	int nIndex = 0;
	QString strTemp("");
	
	if(pData->m_strPfxPathName.isEmpty() && pData->m_strPassword.isEmpty() && m_bLoadValues)
	{
		ui.pfxFilePathEdt->setDisabled(true);
		ui.passwordEdt->setDisabled(true);
		ui.m_browseBtn->setDisabled(true);
	}
	else
	{
		ui.pfxFilePathEdt->setText(pData->m_strPfxPathName);
		ui.passwordEdt->setText(pData->m_strPassword);
	}
	ui.locationEdt->setText(pData->m_strLocation); /* location */
	ui.reasonEdt->setText(pData->m_strReason); /* reason */
	ui.timeStampEdt->setText(pData->m_strTimeStamp);
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

	ui.leftEdt->setText(pData->m_strLeft);
	ui.topEdt->setText(pData->m_strTop);
	ui.widthEdt->setText(pData->m_strWidth);
	ui.heightEdit->setText(pData->m_strHeight);
}

CDSData::CDSData()
:m_strSignName(""),
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

}

CDSData::~CDSData()
{}

void CAdvSettDlgPDF::onPasswordEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strPassword = ui.passwordEdt->text();
	}
}

void CAdvSettDlgPDF::onLocationEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strLocation = ui.locationEdt->text();
	}
}

void CAdvSettDlgPDF::onReasonEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strReason = ui.reasonEdt->text();
	}
}

void CAdvSettDlgPDF::onTimeStampEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strTimeStamp = ui.timeStampEdt->text();
	}
}

void CAdvSettDlgPDF::onLeftEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strLeft = ui.leftEdt->text();
		m_bEditMargin = true;
	}
}

void CAdvSettDlgPDF::onTopEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strTop = ui.topEdt->text();
		m_bEditMargin = true;
	}
}

void CAdvSettDlgPDF::onWidthEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strWidth = ui.widthEdt->text();
		m_bEditMargin = true;
	}
}

void CAdvSettDlgPDF::onHeightEdit()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strHeight = ui.heightEdit->text();
		m_bEditMargin = true;
	}
}

void CAdvSettDlgPDF::onTreeCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
	if(current)
	{
		CDSData *pDSData = NULL;
		pDSData = qvariant_cast<CDSData*>(current->data(0, Qt::UserRole));
		if(pDSData)
		{
			setData(pDSData);
		}
	}
}

void CAdvSettDlgPDF::loadDigitalSignNode()
{
	QDomElement oRootEle;
	QDomNode oAdvSettNode;
	oAdvSettNode = m_oAdvSettNode.firstChildElement("Digital_Signature");
	oRootEle = oAdvSettNode.toElement();
	QString strTemp = oRootEle.tagName();
	strTemp.replace("_"," ");
	if(strTemp.compare("Digital Signature") == 0 && !oRootEle.isNull())
	{
		m_pTreeWidgetItem->setText(0, strTemp);	//displaying the root element on the tree widget
		strTemp = oRootEle.tagName();
		QDomNodeList oNodeList;
		oNodeList = oRootEle.childNodes();
		int nSize = oNodeList.size();
		QDomElement oChildEle, oDummyEle;
		QTreeWidgetItem *pChild = 0;
		CDSData *pData = 0;
		int nVal = 0;
		for(int nIndex = 0; nIndex < nSize; nIndex++)
		{
			oChildEle = oNodeList.at(nIndex).toElement();
			strTemp = oChildEle.tagName();
			pChild = new QTreeWidgetItem(m_pTreeWidgetItem);
			m_pTreeWidgetItem->setExpanded(true);
			pChild->setText(0, strTemp);

			pData = new CDSData;
			if(pData)
			{
				pData->m_strSignName = strTemp;

				if(oChildEle.hasAttributes())
				{
					QString strVal = "";
					m_bLoadValues = true;
					pData->m_strPfxPathName = oChildEle.attribute("pfxPathName");
					pData->m_strPassword = oChildEle.attribute("password");
					pData->m_strLocation = oChildEle.attribute("location");
					pData->m_strReason = oChildEle.attribute("reason");
					pData->m_strTimeStamp = oChildEle.attribute("timestamp");
					pData->m_strPageNo = oChildEle.attribute("pageNo");
					pData->m_strLeft = oChildEle.attribute("left");
					pData->m_strTop = oChildEle.attribute("top");
					pData->m_strWidth = oChildEle.attribute("width");
					pData->m_strHeight = oChildEle.attribute("height");
					pData->m_strCert = oChildEle.attribute("cert");
					pData->m_strPem = oChildEle.attribute("pem");
					pData->m_strUsrName = oChildEle.attribute("UserName");
					pData->m_strIssuerName = oChildEle.attribute("IssuerName");
					pData->m_strPageId = oChildEle.attribute("PageId");
					strVal = oChildEle.attribute("LastPage");
					if(strVal.toInt() == 1)
						pData->m_bLast = true;
					else
						pData->m_bLast = false;
					strVal = oChildEle.attribute("RSA2048Bit");
					if(strVal.toInt() == 1)
						pData->m_b2048Bit = true;
					else
						pData->m_b2048Bit = false;

					strVal = oChildEle.attribute("TmstmpChk");
					if(strVal.toInt() == 1)
						pData->m_bTimestamp = true;
					else
						pData->m_bTimestamp = false;

					strVal = oChildEle.attribute("DSImg");
					if(strVal.toInt() == 1)
						pData->m_bSignImages = true;
					else
						pData->m_bSignImages = false;

					pChild->setData(0, Qt::UserRole, qVariantFromValue(pData));
					if(nIndex == 0 && pData)
						setData(pData);
				}
			}
		}
	}
	ui.groupBox_4->setDisabled(true);
}
bool CAdvSettDlgPDF::GenerateCertificate(QString strpfxPath,QString strPassword,QString &strpemHex,QString &strcertHex)
{ 
	//FILE *fp;
	//EVP_PKEY *pkey;
	//X509 *cert;
	//STACK_OF(X509) *ca = NULL;
	//PKCS12 *p12;
	//SSLeay_add_all_algorithms();
	QDir dir;
	bool bStatus = true;
	QByteArray oCertArr,oPemArr;
	QString strAppPath = RSAppPath; //QApplication::applicationDirPath();
	QString strpemPath = strAppPath+"/tama.pem";
	QString strcertPath = strAppPath+"/eata.d";
	QString strCetPath1 = strAppPath+"/tama1.cer";
	QFile oCert(strcertPath);
	QFile oPem(strpemPath);

	//ERR_load_crypto_strings();
	//if (!(fp = fopen(strpfxPath.toAscii().data(), "rb")))
	//{
	//	onCurrentTabChanged(1);
	//	QMessageBox::warning(this,tr("Error"),tr("File read permissions are not there"));
	//	bStatus = false;
	//	return bStatus;
	//	//fprintf(stderr, "Error opening file %s\n", argv[1]);
	//	//exit(1);
	//}
	//else
	//{
	//	p12 = d2i_PKCS12_fp(fp, NULL);
	//	fclose (fp);
	//}
	//if (!p12)
	//{
	//	onCurrentTabChanged(1);
	//	QMessageBox::warning(this,tr("Error"),tr("Wrong pfx or Wrong Certificate"));
	//	bStatus = false;
	//	return bStatus;
	//	//fprintf(stderr, "Error reading PKCS#12 file\n");
	//	//ERR_print_errors_fp(stderr);
	//	//exit (1);//
	//}
	//if (!PKCS12_parse(p12, strPassword.toAscii().data(), &pkey, &cert, &ca))
	//{
	//	onCurrentTabChanged(1);
	//	QMessageBox::warning(this,tr("Error"),tr("Wrong pfx or Wrong Password"));
	//	bStatus = false;
	//	return bStatus;
	//	//fprintf(stderr, "Error parsing PKCS#12 file\n");
	//	//ERR_print_errors_fp(stderr);
	//	//exit (1);
	//}
	//else
	//{
	//	BUF_MEM *chissuer = cert->cert_info->issuer->bytes;
	//	PKCS12_free(p12);
	//}
	
	//if(bStatus)
	//{
		//if (!(fp = fopen(strcertPath.toAscii().data(), "w+")))
		//{
		//	//exit(1);
		//}
		//else
		//{
		//	if (cert)
		//	{
		//		i2d_X509_fp(fp,cert);
		//	}
		//	fclose(fp);
		//}
		//if (!(fp = fopen(strpemPath.toAscii().data(), "w+")))
		//{
		//	//exit(1);
		//}
		//else
		//{
		//	if (pkey)
		//	{
		//		PEM_write_PrivateKey(fp, pkey, NULL, NULL, 0, NULL, NULL);
		//	}
		//	fclose(fp);
		//}
		
	//Delete the temp files before reading pem and cert
	dir.remove(strcertPath);
	dir.remove(strpemPath);
	dir.remove(strCetPath1);

	QString strSSLExePath = "";
	if(!m_strOpenSSLPath.isEmpty())
	{
		strSSLExePath = QString("%1").arg(m_strOpenSSLPath);

		strSSLExePath.replace(QRegExp("[\\\\|/]"), QDir::separator());
		strSSLExePath = QDir::toNativeSeparators(strSSLExePath);
	}
	else
		strSSLExePath = QString("openssl");

	QString strPwdArg = QString("pass:%1").arg(strPassword);
	QStringList strArguments;
	strArguments.clear();
	strArguments << "pkcs12" << "-in" << strpfxPath << "-passin" << strPwdArg << "-out"<< strCetPath1 <<"-nodes";
	QProcess::execute(strSSLExePath, strArguments);
	
	strArguments.clear();
	strArguments << "x509" << "-outform" <<"der" << "-in" <<strCetPath1<<"-out"<< strcertPath;
	QProcess::execute(strSSLExePath, strArguments);

	strArguments.clear();
	strArguments << "rsa" << "-in" << strCetPath1 <<"-out" << strpemPath;
	QProcess::execute(strSSLExePath, strArguments);


	if(oCert.open(QIODevice::ReadOnly))
	{
		oCertArr = oCert.readAll();
		oCert.close();
		bStatus = true;
	}
	else
	{
		onCurrentTabChanged(1);
		QMessageBox::warning(this,tr("Error"),tr("Wrong pfx or Wrong Password"));
		bStatus = false;
		return bStatus;
	}

	if(oPem.open(QIODevice::ReadOnly))
	{
		oPemArr = oPem.readAll();
		oPem.close();
		bStatus = true;
	}
	else
	{
		onCurrentTabChanged(1);
		QMessageBox::warning(this,tr("Error"),tr("Wrong pfx or Wrong Password"));
		bStatus = false;
		return bStatus;
	}

	convertCharToHex(oCertArr.data(),oCertArr.length(),strcertHex);
	convertCharToHex(QString(oPemArr),strpemHex);
	strcertHex.replace("0D0A","0A");
	//Delete the temp files after reading pem and cert
	dir.remove(strcertPath);
	dir.remove(strpemPath);
	dir.remove(strCetPath1);

	return bStatus;
}
void CAdvSettDlgPDF::onPFXFilePath()
{
	if(m_pCurrentItem->text(0) != tr("Digital Signature"))
	{
		CDSData *pDataptr = NULL;
		pDataptr = qvariant_cast<CDSData *>(m_pCurrentItem->data(0,Qt::UserRole));
		if(pDataptr)
			pDataptr->m_strPfxPathName = ui.pfxFilePathEdt->text();
	}
}
void CAdvSettDlgPDF::convertCharToHex(char *strTmp, int nLen, QString &strHex)
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
void CAdvSettDlgPDF::onCurrentTabChanged(int nIndex)
{
	ui.tabWidget->setCurrentIndex(nIndex);
}
void CAdvSettDlgPDF::onDSFontClicked()
{
	if(m_nIndex > 0)
	{
		bool ok;
		m_Font = QFontDialog::getFont(&ok, m_Font, this);
		ui.m_lineEdit_font->setText(m_Font.toString());
		ui.m_lineEdit_font->setToolTip(m_Font.toString());
		if(m_pPlainTextObj)
			m_pPlainTextObj->setFont(m_Font);
		displayPreView();
	}
}

void CAdvSettDlgPDF::onDSColorClicked()
{
	if(m_nIndex > 0)
	{
		QColor clr = QColorDialog::getColor(m_Color, this);
		if(!clr.isValid())
			return;
		m_Color = clr;
		QPalette palette = ui.m_lineEdit_color->palette();
		palette.setColor( QPalette::Normal, QPalette::Base, m_Color);
		ui.m_lineEdit_color->setPalette(palette);
		
		if(m_pPlainTextObj)
			m_pPlainTextObj->setColor(m_Color);
		displayPreView();
	}
}

void CAdvSettDlgPDF::onXValEntered(int nVal)
{
	if(m_nIndex > 0)
	{	
		m_nXPos = nVal;
		displayPreView();
	}
}

void CAdvSettDlgPDF::onYValEntered(int nVal)
{
	if(m_nIndex > 0)
	{	
		m_nYPos = nVal;
		displayPreView();
	}
}

void CAdvSettDlgPDF::setXPosValue(int nVal)
{
	ui.m_spinBox_XPos->setValue(nVal);
	m_nXPos = nVal;
}

void CAdvSettDlgPDF::setYPosValue(int nVal)
{
	ui.m_spinBox_YPos->setValue(nVal);
	m_nYPos = nVal;
}

void CAdvSettDlgPDF::onWidthValEntered(int nVal)
{
	if(m_nIndex > 0)
	{		
		m_nWidth = nVal;
		displayPreView();
		if(m_pPlainTextObj)
		{
			m_nXPos = m_pPlainTextObj->m_Rect.topLeft().x();
			m_nYPos = m_pPlainTextObj->m_Rect.topLeft().y();
			m_nWidth = m_pPlainTextObj->m_Rect.width();
			m_nHeight = m_pPlainTextObj->m_Rect.height();
		}
			ui.m_spinBox_XPos->setValue(m_nXPos);
			ui.m_spinBox_YPos->setValue(m_nYPos);
			ui.m_spinBox_Width->setValue(m_nWidth);
			ui.m_spinBox_Height->setValue(m_nHeight);
	}
}

void CAdvSettDlgPDF::onHeightValEntered(int nVal)
{
	if(m_nIndex > 0)
	{		
		m_nHeight = nVal;
		displayPreView();
		if(m_pPlainTextObj)
		{
			m_nXPos = m_pPlainTextObj->m_Rect.topLeft().x();
			m_nYPos = m_pPlainTextObj->m_Rect.topLeft().y();
			m_nWidth = m_pPlainTextObj->m_Rect.width();
			m_nHeight = m_pPlainTextObj->m_Rect.height();
		}
			ui.m_spinBox_XPos->setValue(m_nXPos);
			ui.m_spinBox_YPos->setValue(m_nYPos);
			ui.m_spinBox_Width->setValue(m_nWidth);
			ui.m_spinBox_Height->setValue(m_nHeight);
		
	}
}

void CAdvSettDlgPDF::onAngleValChanged(int nAngle)
{
	if(m_nIndex > 0)
	{
		m_nAngle = nAngle;
		ui.m_spinBox_Rotation->setToolTip(QString::number(m_nAngle));
		if(m_pPlainTextObj)
		{
			m_pPlainTextObj->setTextRotation(m_nAngle);
			m_nXPos = m_pPlainTextObj->m_Rect.topLeft().x();
			m_nYPos = m_pPlainTextObj->m_Rect.topLeft().y();
			m_nWidth = m_pPlainTextObj->m_Rect.width();
			m_nHeight = m_pPlainTextObj->m_Rect.height();
			ui.m_spinBox_XPos->setValue(m_nXPos);
			ui.m_spinBox_YPos->setValue(m_nYPos);
			ui.m_spinBox_Width->setValue(m_nWidth);
			ui.m_spinBox_Height->setValue(m_nHeight);
		}
		ui.m_spinBox_XPos->setValue(m_nXPos);
		ui.m_spinBox_YPos->setValue(m_nYPos);
		ui.m_spinBox_Width->setValue(m_nWidth);
		ui.m_spinBox_Height->setValue(m_nHeight);
		displayPreView();
	}
}

void CAdvSettDlgPDF::on_cmbBox_DS_currentIndexChanged(int nIndex)
{
	ui.m_optImage->setDisabled(true);
	
	if(nIndex == 0) 
	{	
		m_bPgWatermark = false;
		ui.m_groupBx_Prop->setDisabled(true);
	}
	else if(nIndex == 1) /*page watermark*/
	{
		ui.m_groupBx_Prop->setDisabled(false);
		ui.m_optImage->setDisabled(false);
		if(ui.m_optImage->isChecked())
			ui.m_btnBrowseImg->setVisible(true);
		else
			ui.m_btnBrowseImg->setVisible(false);
		m_bPgWatermark = true;		
	}
	displayDSProperties(nIndex);
}

void CAdvSettDlgPDF::onImgOptionSelected(bool bStatus)
{
	if(bStatus)
	{		
		m_bImageWM = true;	

		if(m_strPgWMText!=m_strEditText )
			ui.m_textEdit->clear();
		if(m_strPgWMText==ui.m_textEdit->toPlainText())
			ui.m_textEdit->clear();
	
		ui.m_btnBrowseImg->setVisible(true);
		ui.m_btnBrowseImg->setToolTip("Select Image from Local Disk");
		ui.m_lineEdit_font->setDisabled(true);
		ui.m_lineEdit_color->setDisabled(true);
		ui.m_pushBtn_Colr->setDisabled(true);
		ui.m_pushBtn_font->setDisabled(true);
		ui.m_spinBox_Rotation->setValue(0);
		ui.m_spinBox_Rotation->setDisabled(true);
	}
	else
	{
		if(!ui.m_textEdit->toPlainText().isEmpty())
		ui.m_textEdit->clear();
		m_bImageWM = false;		
		ui.m_btnBrowseImg->setVisible(false);
		ui.m_lineEdit_font->setDisabled(false);
		ui.m_lineEdit_color->setDisabled(false);
		ui.m_pushBtn_Colr->setDisabled(false);
		ui.m_pushBtn_font->setDisabled(false);
		ui.m_spinBox_Rotation->setDisabled(false);
	}
	displayPreView();
}

void CAdvSettDlgPDF::displayDSProperties(int nIndex)
{
	m_nIndex = nIndex;
	if(nIndex == 0) 
	{
		ui.m_groupBx_Prop->setDisabled(true);
	}
	else if(nIndex > 0 && m_mapDSProp.size() != 0)
	{
		if(nIndex == 1)
			ui.m_groupBx_Prop->setDisabled(false);

		CDSWatermarkProp oDSWMProp;
		oDSWMProp = m_mapDSProp.value(nIndex);
		m_strEditText = oDSWMProp.m_strText;
		m_Font = oDSWMProp.m_Font;
		m_Color = oDSWMProp.m_Color;
		m_nAngle = oDSWMProp.m_nAngle;
		m_Rect = oDSWMProp.m_Rect;
		m_TmpRect = oDSWMProp.m_TmpRect;
		m_nXPos = oDSWMProp.m_Rect.topLeft().x();
		m_nYPos = oDSWMProp.m_Rect.topLeft().y();
		m_nWidth = oDSWMProp.m_Rect.width();
		m_nHeight = oDSWMProp.m_Rect.height();
		m_bImageWM = oDSWMProp.m_bImageWM;
		
		if(m_bImageWM)
		{
			ui.m_optImage->setDisabled(false);
			ui.m_optImage->setChecked(true);
			ui.m_btnBrowseImg->setVisible(true);			
		}
		else
		{
			if(m_nIndex == 1)
				ui.m_optImage->setDisabled(false);
			else
				ui.m_optImage->setDisabled(true);
			ui.m_optText->setChecked(true);
			ui.m_btnBrowseImg->setVisible(false);			
		}

		if(m_pPlainTextObj && !m_bImageWM)
			m_pPlainTextObj->setData(m_strEditText,m_Font,m_Color,m_nAngle,m_Rect,m_TmpRect);
		else if(m_pPlainTextObj && m_bImageWM)
			m_pPlainTextObj->setData(m_strEditText,m_nAngle,m_Rect,m_TmpRect);

		ui.m_textEdit->setPlainText(m_strEditText);
		ui.m_textEdit->setToolTip(m_strEditText);
		
		QString strFont = m_Font.toString();
		ui.m_lineEdit_font->setText(strFont);
		ui.m_lineEdit_font->setToolTip(strFont);
		
		QPalette palette = ui.m_lineEdit_color->palette();
		palette.setColor(QPalette::Normal , QPalette::Base , m_Color);
		ui.m_lineEdit_color->setPalette(palette);
		ui.m_lineEdit_color->setToolTip(m_Color.name());
		
		ui.m_spinBox_Rotation->setValue(m_nAngle);
		ui.m_spinBox_Rotation->setToolTip(QString::number(m_nAngle));

		ui.m_spinBox_XPos->setValue(m_nXPos);
		ui.m_spinBox_YPos->setValue(m_nYPos);
		ui.m_spinBox_Width->setValue(m_nWidth);
		ui.m_spinBox_Height->setValue(m_nHeight);
			
		displayPreView();
	}
}

void CAdvSettDlgPDF::ResetDSProperties()
{
	CDSWatermarkProp oDSWMProp;
	QString strTxt;
	if(m_nIndex == 1)
	{
		strTxt = "Sample";
		oDSWMProp.setValues("Arial", strTxt, 0, QRect(100,100, 400, 100), QRect(100, 100, 400, 100), 0, 35, QColor(1,1,1),false);
		m_mapDSProp.insert(1, oDSWMProp);
	}	
	displayDSProperties(m_nIndex);
}

void CAdvSettDlgPDF::onDSRestBtnClicked()
{
	ResetDSProperties();
}

void CAdvSettDlgPDF::setChanged()
{
	if(m_nIndex > 0)
	{
		QString strText = ui.m_textEdit->toPlainText();
		ui.m_textEdit->setToolTip(strText);
		m_strEditText = strText;
		displayPreView();
	}
}

/* When a Data Object is clicked from Combo */
void CAdvSettDlgPDF::onDataObjClicked(const QString& strDataObjName)
{
	QString strText = ui.m_cmbBox_Data->currentText();
	ui.m_pushBtn_data->setEnabled(true);
	if(!strText.isEmpty())
	{
		strText = strDataObjName;
		ui.m_cmbBox_Data->setEditText(strText);
		ui.m_cmbBox_Data->setToolTip(strText);
	}
	
}

/*Insert Data Object */
void CAdvSettDlgPDF::buttonInsertClicked()
{
	QString strObject(""), strExistingText(""), strName("");
	strExistingText = ui.m_textEdit->toPlainText();
	strName = ui.m_cmbBox_Data->currentText();
	QTextCursor cursor(ui.m_textEdit->textCursor());
	int nCurPosition=cursor.position();

	if(!strName.isEmpty())
	{
		if(!m_bImageWM)
		{
			strObject = '^' + strName + '^';
			strExistingText.insert(nCurPosition,strObject);
			
			ui.m_textEdit->insertPlainText(strObject);
			ui.m_textEdit->setToolTip(strExistingText);
			m_strEditText = strExistingText;
		}
		else
		{
			ui.m_textEdit->clear();
			strObject = '^' + strName + '^';
			ui.m_textEdit->insertPlainText(strObject);
			ui.m_textEdit->setToolTip(strObject);
			m_strEditText = strObject;
		}
	}
	displayPreView();
}

void CAdvSettDlgPDF::buttonBrowseImgClicked()
{
	QString strFileName = QFileDialog::getOpenFileName(this, tr("Select Image"), QDir::currentPath(), tr("Images (*.png *.xpm *.jpg *.jpeg *.gif *.pbm *.pgm *.ppm *.xbm *.bmp *.tif)"));
	if(!strFileName.isEmpty())
	{
		ui.m_textEdit->clear();
		ui.m_textEdit->insertPlainText(strFileName);
		ui.m_textEdit->setToolTip(strFileName);
		m_strEditText = strFileName;
	}
	displayPreView();
}

void CAdvSettDlgPDF::onDSSaveClicked()
{
	if(ValidateInput())
	{
		if(m_nIndex == 1) /* watermark*/
		{
			QRect Rect, TmpRect;
			if(m_pPlainTextObj)
			{
				Rect = m_pPlainTextObj->m_Rect;
				TmpRect = m_pPlainTextObj->m_TemplateRect;
			}
			CDSWatermarkProp oDSWMProp;
			if(m_bImageWM && !m_strEditText.contains("^") && !m_strEditText.isEmpty())
			{
				QString strActPath(""), strBasePath(""), strTemp(""), strImgNm("");
				int nIndx = 0, nCols, nRows, clrBits;;
				strActPath = m_strEditText;
				strActPath.replace(QRegExp("[\\\\|/]"), QDir::separator());

				nIndx = strActPath.lastIndexOf("/");
				if(nIndx == -1)
					nIndx = strActPath.lastIndexOf("\\");

				if(nIndx > -1)
					strImgNm = strActPath.mid(nIndx+1, (strActPath.lastIndexOf(".") - nIndx) -1);

				if(!m_strTemplatePath.isEmpty())
				{
					nIndx = 0;
					nIndx = m_strTemplatePath.lastIndexOf("/");
					if(nIndx == -1)
						nIndx = m_strTemplatePath.lastIndexOf("\\");

					strTemp = m_strTemplatePath.mid(0,nIndx);
					QFile file(strActPath);
					if(file.exists())
					{
						nIndx = strActPath.lastIndexOf('.');
						if(nIndx != -1)
						{
							nIndx += 1;
							QString strFilExtension = strActPath.right(strActPath.size()- (nIndx));
							if(!strFilExtension.compare("jpeg", Qt::CaseInsensitive)  == 0)
							{
								strBasePath = QDir::toNativeSeparators(QString("%1/images/%2.jpeg").arg(strTemp).arg(strImgNm));
								strBasePath.replace(QRegExp("[\\\\|/]"), QDir::separator());

								QImage imgObj(strActPath);
								QFile fileNew(strBasePath);
								if(!fileNew.exists())//save only if the file doesnot exists 
								{
									imgObj.save(strBasePath, "jpeg");
								}
							}
							else
							{
								strBasePath = QDir::toNativeSeparators(QString("%1/images/%2.jpeg").arg(strTemp).arg(strImgNm));
								strBasePath.replace(QRegExp("[\\\\|/]"), QDir::separator());

								QFile fileNew(strBasePath);
								if(!fileNew.exists())//save only if the file doesnot exists 
								{
									file.copy(strBasePath);
								}
							}
							m_strEditText = strBasePath;
						}
					}
				}
			}
			oDSWMProp.setValues(m_strEditText, Rect, TmpRect, m_nAngle, m_Font, m_Color, m_bImageWM);

			m_mapDSProp.insert(1, oDSWMProp);
			m_bPgWatermark = true;
		}
		else
		{
			m_bPgWatermark = false;
		}
	}
}

bool CAdvSettDlgPDF::ValidateInput()
{
	if(ui.m_textEdit->toPlainText().isEmpty())
	{
		QMessageBox::critical(this, tr("Watermark Settings"), tr("Enter Text/Image."));
		return false;
	}
	if(ui.m_lineEdit_font->text().isEmpty())
	{
		QMessageBox::critical(this, tr("Watermark Settings"), tr("Select Font."));
		return false;
	}
	return true;
}

void CAdvSettDlgPDF::displayPreView()
{
	
	if(m_pFrame == NULL)
	{
		m_pFrame = new CPreviewFrame(this);
		if(QLayout* pLayout = ui.m_Preview_groupBox->layout())
			delete pLayout;

		QHBoxLayout* pHLayout = new QHBoxLayout();
		pHLayout->addWidget(m_pFrame);
		ui.m_Preview_groupBox->setLayout(pHLayout);
	}
	
	if(m_pFrame)
	{
		QRect objRect;
		
		if(m_pPlainTextObj == NULL)
			m_pPlainTextObj = new CPlainTextObj;

		if(!m_bImageWM)
		{
			m_pPlainTextObj->setValues(m_strEditText, m_Font, m_Color, m_nXPos, m_nYPos, m_nWidth, m_nHeight, m_nAngle);
			}
		else
		{
			m_pPlainTextObj->setValues(m_strEditText, m_nXPos, m_nYPos, m_nWidth, m_nHeight, m_nAngle);
		}
		objRect = m_pPlainTextObj->getBoundingRect();

		if(m_pTextLabel == NULL)
		{
			m_pTextLabel = new QLabel(m_pFrame);
			m_pTextLabel->setScaledContents(true);
		}
		m_pFrame->setFrameRect(QRect(0,0,260,362));
		QRect frameRect = m_pFrame->frameRect();

		
		int nFrmeLW = m_pFrame->lineWidth();
		int nFrmeW = frameRect.width() - 2*nFrmeLW;
		int nFrmeH = frameRect.height() - 2*nFrmeLW;

		QPixmap pm = QPixmap(objRect.size() + QSize(1, 1));
		QPainter *pPainter = new QPainter(&pm);
		pPainter->fillRect(pm.rect(), QBrush(Qt::white));
		pPainter->scale(1, 1);
		pPainter->translate(-objRect.x() + 1, -objRect.y() + 1);
		if(!m_bImageWM)
			m_pPlainTextObj->draw(pPainter);
		else
			m_pPlainTextObj->drawImg(pPainter);

		m_pTextLabel->setPixmap(pm);

		QSize size = pm.size();
		int w = size.width();
		int h = size.height();

		size.scale(w*nFrmeW/756, h*nFrmeH/1070, Qt::IgnoreAspectRatio);

		m_pTextLabel->resize(size);
		int nX = objRect.topLeft().x();
		int nY = objRect.topLeft().y();
		
		m_pTextLabel->move(nFrmeLW+nX*nFrmeW/756, nFrmeLW+nY*nFrmeH/1070);
		if(pPainter)
		{
			delete pPainter;
			pPainter = NULL;
		}
	}
	update();
}
/* Replaces Object Keys with Object names.*/
void CAdvSettDlgPDF::replaceKeysWithNames(const QString &strInput, QString &strOutput)
{
	strOutput.clear();
	QString strObjName;
	int nSize  = 0;
	QChar ch = '\0';
	bool bOpen = false;
	nSize = strInput.size();

	for(int nIndex=0; nIndex<nSize; nIndex++)
	{
		ch = strInput.at(nIndex);
		if( !bOpen && (ch == '^' ) )
		{
			bOpen = true;
			strOutput.append(ch);
			strObjName.clear();
			continue;
		}
		else if(ch == '^' )
		{
			bOpen = false;
			strObjName = strObjName.trimmed();

			if (!strObjName.isEmpty() && !strObjName.contains(" "))
			{
				if (m_pGlblObjMapPtr && m_pGlblObjMapPtr->contains(strObjName))
					strOutput.append(m_pGlblObjMapPtr->value(strObjName));
				else
					strOutput.append(strObjName);

			}
			else
				strOutput.append(strObjName);

			strOutput.append(ch);
			strObjName.clear();
			continue;
		}

		if(bOpen)
		{
			strObjName.append(ch);
			continue;
		}
		strOutput.append(ch);
	}

	if (bOpen && !strObjName.isEmpty())
	{
		strOutput.append(strObjName);
	}
}

/* Replaces Object names with Object keys. */
void CAdvSettDlgPDF::replaceNamesWithKeys(const QString& strInput,QString& strOutput)
{
	strOutput.clear();

	QString strObjName;
	int nSize  = 0;
	QChar ch = '\0';
	bool bOpen = false;
	nSize = strInput.size();

	for (int nIndex=0; nIndex<nSize; nIndex++)
	{
		ch = strInput.at(nIndex);
		if( !bOpen && (ch == '^') )
		{
			bOpen = true;
			strOutput.append(ch);
			strObjName.clear();
			continue;
		}
		else if(ch == '^' )
		{
			bOpen = false;
			strObjName = strObjName.trimmed();

			if (!strObjName.isEmpty() && !strObjName.contains(" ") && !m_pGlblObjMapPtr->key(strObjName).isEmpty())
				strOutput.append(m_pGlblObjMapPtr->key(strObjName));
			else
				strOutput.append(strObjName);

			strOutput.append(ch);
			strObjName.clear();
			continue;
		}

		if(bOpen)
		{
			strObjName.append(ch);
			continue;
		}
		strOutput.append(ch);
	}

	if (bOpen && !strObjName.isEmpty())
	{
		strOutput.append(strObjName);
	}
}

CPlainTextObj::CPlainTextObj()
:m_pPainter(NULL),
m_Color(QColor(1,1,1)),
m_nXPos(0),
m_nYPos(0),
m_nAngle(0),
m_strText(""),
m_dr11(0),
m_dr12(0),
m_dtx(0),
m_dr21(0),
m_dr22(0),
m_dty(0)
{
	m_Rect.setRect(0,0,1,1);
	m_TemplateRect.setRect(0,0,1,1);

}

CPlainTextObj::~CPlainTextObj()
{
}

void CPlainTextObj::setData(QString strFont, QString strText, int nAngle, QRect Rect, QRect TmpRect, int nFontStyle, int nFontSize, QColor Color)
{
	if(strFont.isEmpty())
		strFont = "Arial";
	m_Font.setFamily(strFont);
	if(nFontSize == 0)
		nFontSize = 8;
	m_Font.setPointSize(nFontSize);
	if(nFontStyle == 1)
		m_Font.setBold(true);
	else if(nFontStyle == 2)
		m_Font.setItalic(true);
	else if(nFontStyle == 3)
	{
		m_Font.setBold(true);
		m_Font.setItalic(true);
	}

	m_strText = strText;
	m_nAngle = nAngle;
	m_Rect = Rect;
	m_TemplateRect = TmpRect;
	m_Color = Color;
}

void CPlainTextObj::setData(QString strText, QFont Font,QColor Color, int nAngle, QRect Rect, QRect TmpRect)
{	
	m_strText = strText;
	m_Font = Font;
	m_Color = Color;
	m_nAngle = nAngle;
	m_Rect = Rect;
	m_TemplateRect = TmpRect;
}

void CPlainTextObj::setData(QString strText, int nAngle, QRect Rect, QRect TmpRect)
{	
	m_strText = strText;
	m_nAngle = nAngle;
	m_Rect = Rect;
	m_TemplateRect = TmpRect;
}

QRect CPlainTextObj::getBoundingRect()
{
	return m_Rect;
}

void CPlainTextObj::setBoundingRect(const QRect& rect)
{
	if ( m_TemplateRect.width() <= 0 ) 
	{
		m_TemplateRect = rect;
	}
	else if(rect != m_TemplateRect )
	{
		int nX = rect.x() - m_Rect.x();
		int nY = rect.y() - m_Rect.y();
		int nW = rect.width() - m_Rect.width();
		int nH = rect.height() - m_Rect.height();

		int nWidth	= m_TemplateRect.width();
		int nHeight = m_TemplateRect.height();

		if ( nW == 0 && nH == 0 )
		{
			nX = rect.x() - m_Rect.x();
			nY = rect.y() - m_Rect.y();

			m_TemplateRect.setX( m_TemplateRect.x() + nX );
			m_TemplateRect.setY( m_TemplateRect.y() + nY );
		}
		else
		{
			nW = 0;
			nH = 0;

			if ( nX == 0 && nY == 0 )
			{
				nW = rect.width() - m_Rect.width();
				nH = rect.height() - m_Rect.height();
			}
		}

		m_TemplateRect.setWidth( nWidth + nW );
		m_TemplateRect.setHeight( nHeight + nH );

		m_Rect = rect;		
		
	}
	setTextRotation(m_nAngle);

	m_nXPos = m_Rect.topLeft().x();
	m_nYPos = m_Rect.topLeft().y();
}

void CPlainTextObj::draw(QPainter *pPainter)
{
	pPainter->save();
	pPainter->setBackgroundMode(Qt::TransparentMode);
	pPainter->setPen(m_Color);
	pPainter->setBrush(Qt::NoBrush);
	QRect rect = m_TemplateRect;
	QPen pen;
	pen.setWidth(1);
	pen.setStyle(Qt::DotLine);
	pen.setColor(Qt::blue);
	QMatrix trans;
	trans.translate(rect.center().x(), rect.center().y());
	trans.rotate(m_nAngle);
	trans.translate(-rect.center().x(), -rect.center().y());
	pPainter->setMatrix(trans, true);

	pPainter->save();
	pPainter->setPen(pen);
	QRect borderrect(rect.left() - pen.width() * 0.5, rect.top() - pen.width() * 0.5, rect.width() + pen.width(), rect.height() + pen.width());
	pPainter->drawRect(borderrect);
	/*pPainter->fillRect(rect, Qt::NoBrush); */
	pPainter->restore();

	pPainter->setFont(QFont(m_Font));
	QString strText = m_strText;
	pPainter->drawText(rect, Qt::AlignLeft|Qt::TextWordWrap, strText);
	pPainter->restore();	
}

void CPlainTextObj::drawImg(QPainter *pPainter)
{
	pPainter->save();
	QPen pen(Qt::lightGray);
	QRect rect = m_TemplateRect;
	QMatrix trans;
	trans.translate(rect.center().x(), rect.center().y());
	trans.rotate(m_nAngle);
	trans.translate(-rect.center().x(), -rect.center().y());
	pPainter->setMatrix(trans, true);

	pPainter->save();
	pPainter->setPen(pen);
	QRect borderrect(rect.left() - pen.width() * 0.5, rect.top() - pen.width() * 0.5, rect.width() + pen.width(), rect.height() + pen.width());
	pPainter->drawRect(borderrect);
	pPainter->restore();

	QImage imgObj;
	imgObj.load(m_strText);
	pPainter->drawImage(m_Rect, imgObj, imgObj.rect());
	pPainter->restore();
}

bool CPlainTextObj::HitTest(const QPoint& hitPoint) const
{
	return m_Rect.contains(hitPoint);
}

void CPlainTextObj::setRotation(int &nAngle)
{
	m_nAngle = nAngle;
}

void CPlainTextObj::setColor(QColor &Color)
{
	m_Color = Color;
}

void CPlainTextObj::setFont(QFont &Font)
{
	m_Font = Font;
}

void CPlainTextObj::setValues(QString strText, QFont font, QColor color, int nXPos, int nYPos, int nWidth, int nHeight, int nAngle)
{
	m_strText = strText;
	m_Font = font;
	m_Color = color;
	m_nXPos = nXPos;
	m_nYPos = nYPos;
	m_nAngle = nAngle;

	if(nWidth <= 0)
		nWidth = 10;
	if(nHeight <= 0)
		nHeight = 10;

	QRect rect;
	rect.setRect(nXPos, nYPos, nWidth, nHeight);
	setBoundingRect(rect);
}

void CPlainTextObj::setValues(QString strText, int nXPos, int nYPos, int nWidth, int nHeight, int nAngle)
{
	m_strText = strText;
	m_nXPos = nXPos;
	m_nYPos = nYPos;
	m_nAngle = nAngle;

	if(nWidth <= 0)
		nWidth = 10;
	if(nHeight <= 0)
		nHeight = 10;

	QRect rect;
	rect.setRect(nXPos, nYPos, nWidth, nHeight);
	setBoundingRect(rect);
}

void CPlainTextObj::setTextRotation(int rotation)
{
	m_nAngle = rotation;
	QPolygon polygon;
	QRect oRect = m_TemplateRect;

	QPoint oPoint  = RotateArbitrary(oRect.center(), rotation);
	QPoint oPoint5 = ReCalculateArbitrary(oRect.center());

	QPoint oPoint1 = ReCalculateArbitrary(oRect.topLeft());
	QPoint oPoint3 = ReCalculateArbitrary(oRect.topRight());
	QPoint oPoint2 = ReCalculateArbitrary(oRect.bottomRight());
	QPoint oPoint4 = ReCalculateArbitrary(oRect.bottomLeft());
	QVector<QPoint> oVector;

	oVector.append(oPoint1);
	oVector.append(oPoint3);
	oVector.append(oPoint4);
	oVector.append(oPoint2);

	polygon.clear();
	polygon << oPoint1 << oPoint3 << oPoint2 << oPoint4;
	
	m_Rect = polygon.boundingRect();	
	
	m_nXPos = m_Rect.topLeft().x();
	m_nYPos = m_Rect.topLeft().y();
}

QPoint CPlainTextObj::RotateArbitrary(QPoint ptxy, double dAngle)
{
	QPoint ptResult;
	double dRadAngle = dAngle;

	DegreesToRadians(dAngle, dRadAngle);

	m_dr11 = (float)cos(dRadAngle);
	m_dr12 = (float)-sin(dRadAngle);
	m_dtx = (float)((double)ptxy.x() * ((double)1- cos(dRadAngle)) + ((double)ptxy.y() * sin(dRadAngle)));
	m_dr21 = (float)sin(dRadAngle);
	m_dr22 = (float)cos(dRadAngle);
	m_dty = (float)((double)ptxy.y() * ((double)1 - cos(dRadAngle)) - ((double)ptxy.x() * sin(dRadAngle)));

	float dX = 0;
	float dY = 0;

	dX = ptxy.x() * m_dr11 + ptxy.y() * m_dr12 + m_dtx;
	dY = ptxy.x() * m_dr21 + ptxy.y() * m_dr22 + m_dty;

	ptResult.setX((qreal)dX);
	ptResult.setY((qreal)dY);

	return ptResult;
}

QPoint CPlainTextObj::ReCalculateArbitrary(QPoint ptxy)
{
	QPoint ptResult;

	float dX = 0;
	float dY = 0;

	dX = ptxy.x() * m_dr11 + ptxy.y() * m_dr12 + m_dtx;
	dY = ptxy.x() * m_dr21 + ptxy.y() * m_dr22 + m_dty;

	ptResult.setX((qreal)dX);
	ptResult.setY((qreal)dY);

	return ptResult;
}

void CPlainTextObj::DegreesToRadians(double dDegAngle, double& dRadAngle)
{
	dRadAngle = (dDegAngle * 3.14) / 180.0f;
}

CDSWatermarkProp::CDSWatermarkProp()
:m_strFont(""),
m_strText(""),
m_nAngle(0),
m_nFontStyle(0),
m_nFontSize(0),
m_Color(1,1,1),
m_bImageWM(false)
{

}

CDSWatermarkProp::~CDSWatermarkProp()
{

}
void CDSWatermarkProp::setValues(QString strFont, QString strText, int nAngle, QRect Rect, QRect TmpRect, int nFontStyle, int nFontSize, QColor Color, bool bImageWM)
{
	QFont Font;
	m_strFont = strFont;
	m_nFontSize = nFontSize;
	m_nFontStyle = nFontStyle;

	if(strFont.isEmpty())
		strFont = "Arial";
	Font.setFamily(strFont);
	if(nFontSize == 0)
		nFontSize = 8;
	Font.setPointSize(nFontSize);
	if(nFontStyle == 1)
	{
		Font.setBold(true);
		Font.setItalic(false);
	}
	else if(nFontStyle == 2)
	{
		Font.setItalic(true);
		Font.setBold(false);
	}
	else if(nFontStyle == 3)
	{
		Font.setBold(true);
		Font.setItalic(true);
	}
	else
	{
		Font.setBold(false);
		Font.setItalic(false);
	}
	m_Font = Font;
	m_strText = strText;
	m_nAngle = nAngle;
	m_Rect = Rect;
	m_TmpRect = TmpRect;
	m_Color = Color;
	m_bImageWM = bImageWM;
}

void CDSWatermarkProp::setValues(QString strText, QRect Rect, QRect TmpRect, int nAngle, QFont Font, QColor Color, bool bImageWM)
{
	m_strText = strText;
	m_nAngle = nAngle;
	m_Rect = Rect;
	m_TmpRect = TmpRect;
	m_Font = Font;
	m_Color = Color;
	m_bImageWM = bImageWM;

	m_nFontStyle = 0;
	if(Font.weight() > 50 && Font.italic())
	{
		m_nFontStyle = 3;
	}
	else if(Font.weight() > 50 && !Font.italic())
	{
		m_nFontStyle = 1;
	}
	else if(Font.weight() <= 50 && Font.italic())
	{
		m_nFontStyle = 2;
	}

	m_strFont = Font.family();
	if(m_strFont.isEmpty())
		m_strFont= "Arial";
	m_nFontSize = Font.pointSize();
	if(m_nFontSize <= 0)
		m_nFontSize = 8;
}

CPreviewFrame::CPreviewFrame(CAdvSettDlgPDF *pAdvSettDlgPDF):QFrame(pAdvSettDlgPDF)
{
	m_bMousePressed = false;
	setAutoFillBackground(true);
	setFrameShape(QFrame::Box);
	setFrameShadow(QFrame::Plain);
	setLineWidth(1);
	QPalette palette;
	palette.setColor(QPalette::Window, Qt::white);
	setPalette(palette);
	
	setMouseTracking(true);
	setAcceptDrops(true);
	m_pAdvSettDlgPDF = pAdvSettDlgPDF;
}

CPreviewFrame::~CPreviewFrame()
{
	
}

void CPreviewFrame::mouseMoveEvent(QMouseEvent *pEvent)
{

	QFrame::mouseMoveEvent(pEvent);

	QRect frameRect;
	int nXPos, nYPos = 0;

	if(m_pAdvSettDlgPDF && m_pAdvSettDlgPDF->m_pPlainTextObj)
		frameRect = this->frameRect();
	else
		return;

	int nFrmeLW = this->lineWidth();
	int nFrmeW = frameRect.width() - 2*nFrmeLW;
	int nFrmeH = frameRect.height() - 2*nFrmeLW;

	nXPos = (pEvent->pos()).x() - nFrmeLW;
	nYPos = (pEvent->pos()).y() - nFrmeLW;

	nXPos = (nXPos*756/nFrmeW);
	nYPos = (nYPos*1070/nFrmeH);
	
	QPoint LocalPoint;
	LocalPoint.setX(nXPos);
	LocalPoint.setY(nYPos);

	if(m_pAdvSettDlgPDF && m_pAdvSettDlgPDF->m_pPlainTextObj)
	{
		if((m_pAdvSettDlgPDF->m_pPlainTextObj->HitTest(LocalPoint)) ||  m_bMousePressed)
			setCursor(Qt::SizeAllCursor);
		else
			setCursor(Qt::ArrowCursor);
	}
}

void CPreviewFrame::mousePressEvent(QMouseEvent *pEvent)
{
	if(pEvent->button() == Qt::RightButton)
		return;

	QFrame::mousePressEvent(pEvent);

	QLabel *child = static_cast<QLabel*>(childAt(pEvent->pos()));
	if(!child)
		return;

	m_StartPos = pEvent->pos();
	m_bMousePressed = true;
	QRect frameRect;
	int nXPos, nYPos = 0;

	if(m_pAdvSettDlgPDF && m_pAdvSettDlgPDF->m_pPlainTextObj)
	{
		this->setFrameRect(QRect(0,0,260,362));
		setFrameRect(QRect(0,0,0,0));
		frameRect = this->frameRect();
	}
	else
		return;

	int nFrmeLW = this->lineWidth();
	int nFrmeW = frameRect.width() - 2*nFrmeLW;
	int nFrmeH = frameRect.height() - 2*nFrmeLW;

	nXPos = (pEvent->pos()).x() - nFrmeLW;
	nYPos = (pEvent->pos()).y() - nFrmeLW;

	nXPos = (nXPos*756/nFrmeW);
	nYPos = (nYPos*1070/nFrmeH);
	
	QPoint LocalPoint;
	LocalPoint.setX(nXPos);
	LocalPoint.setY(nYPos);

	if(m_pAdvSettDlgPDF && m_pAdvSettDlgPDF->m_pPlainTextObj)
	{
		if(m_pAdvSettDlgPDF->m_pPlainTextObj->HitTest(LocalPoint))
			setCursor(Qt::SizeAllCursor);
	}

    QPixmap pixmap = *child->pixmap();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << pixmap << QPoint(pEvent->pos() - child->pos());

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", itemData);
        
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);

	int w = pixmap.width();
	int h = pixmap.height();

	pixmap = pixmap.scaled(w*nFrmeW/756, h*nFrmeH/1070, Qt::IgnoreAspectRatio);
	
	drag->setPixmap(pixmap);
    drag->setHotSpot(pEvent->pos() - child->pos());

    QPixmap tempPixmap = pixmap;
    QPainter painter;
    painter.begin(&tempPixmap);
	painter.fillRect(pixmap.rect(), QBrush(Qt::white));
    painter.end();

    child->setPixmap(tempPixmap);

    if(drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
        child->close();
    else
	{
        child->show();
        child->setPixmap(pixmap);
    }
	m_pAdvSettDlgPDF->displayPreView(); //for not to Display text in place of image 
}

void CPreviewFrame::dragEnterEvent(QDragEnterEvent *pEvent)
{
	QFrame::dragEnterEvent(pEvent);   
	if(pEvent->mimeData()->hasFormat("application/x-dnditemdata"))
	{
		if(pEvent->source() == this)
		{
			setCursor(Qt::SizeAllCursor);
			pEvent->setDropAction(Qt::MoveAction);
			pEvent->accept();
		}
		else
		{
			pEvent->acceptProposedAction();
		}
	}
	else
		pEvent->ignore();
}

void CPreviewFrame::dragMoveEvent(QDragMoveEvent *pEvent)
{
	QFrame::dragMoveEvent(pEvent);   
	if(pEvent->mimeData()->hasFormat("application/x-dnditemdata"))
	{
		setCursor(Qt::SizeAllCursor);
		if(pEvent->source() == this)
		{
			pEvent->setDropAction(Qt::MoveAction);
			pEvent->accept();
		}
		else
			pEvent->acceptProposedAction();
	}
	else 
		pEvent->ignore();
}

void CPreviewFrame::dropEvent(QDropEvent *pEvent)
{
	if (pEvent->mimeData()->hasFormat("application/x-dnditemdata"))
	{
		int ndist = (m_StartPos - pEvent->pos()).manhattanLength();
		int ndrag = QApplication::startDragDistance();
		bool bCanStartDrag = (m_StartPos - pEvent->pos()).manhattanLength() > QApplication::startDragDistance();
		m_bMousePressed = false;
		
		QRect objRect, frameRect, objLocalRect;
		float nXPos, nYPos = 0;
			
		if(m_pAdvSettDlgPDF && m_pAdvSettDlgPDF->m_pPlainTextObj)
		{
			objRect = m_pAdvSettDlgPDF->m_pPlainTextObj->getBoundingRect();
			frameRect = this->frameRect();
		}
		else
			return;

		nXPos = (pEvent->pos()).x();
		nYPos = (pEvent->pos()).y();
		
		int nFrmeLW = this->lineWidth();
		int nFrmeW = frameRect.width() - 2*nFrmeLW;
		int nFrmeH = frameRect.height() - 2*nFrmeLW;
		
		nXPos = nXPos*756/nFrmeW;
		nYPos = nYPos*1070/nFrmeH;

		if(nXPos <= 0)
			nXPos = 0;
		else if((nXPos+ objRect.width()) >= 756)
			nXPos = 756 - objRect.width();

		if(nYPos <= 0)
			nYPos = 0;
		else if((nYPos+objRect.height()) >= 1070)
			nYPos = 1070 - objRect.height();

		QLabel *pTextLabel = new QLabel(this);
		m_pAdvSettDlgPDF->m_pTextLabel = pTextLabel;
		pTextLabel->setScaledContents(true);
		
		objLocalRect = m_pAdvSettDlgPDF->m_pPlainTextObj->getBoundingRect();
		nXPos = objLocalRect.x();
		nYPos = objLocalRect.y();

		QPixmap pm = QPixmap(objLocalRect.size() + QSize(1, 1));
		QPainter *pPainter = new QPainter(&pm);
		pPainter->fillRect(pm.rect(), QBrush(Qt::white));
		pPainter->scale(1,1);
		pPainter->translate(-objLocalRect.x() + 1, -objLocalRect.y() + 1);
		m_pAdvSettDlgPDF->m_pPlainTextObj->draw(pPainter);
		if(pPainter)
		{
			delete pPainter;
			pPainter = NULL;
		}

		pTextLabel->setPixmap(pm);

		QSize size = pm.size();
		int w = size.width();
		int h = size.height();

		size.scale(w*nFrmeW/756, h*nFrmeH/1070, Qt::IgnoreAspectRatio);
		pTextLabel->resize(size);

		QPointF rectPoint = objLocalRect.topLeft();
		
		float nTmpX, nTmpY, nX, nY;

		nTmpX = rectPoint.x();
		nTmpY = rectPoint.y();
		nTmpX = nTmpX*nFrmeW/756;
		nTmpY = nTmpY*nFrmeH/1070;
		if(bCanStartDrag)
		{			
			rectPoint.setX(nTmpX);
			rectPoint.setY(nTmpY);

			QPointF distPoint = m_StartPos - pEvent->pos();
			QPointF movePoint = rectPoint - distPoint;

			nX = movePoint.x();
			nY = movePoint.y();
		}
		else
		{
			nX = nTmpX;
			nY = nTmpY;
		}

		pTextLabel->move(nX+nFrmeLW, nY+nFrmeLW);
		
		nX = nX*756/nFrmeW;
		nY = nY*1070/nFrmeH;
		
		QRect rect;
		rect.setRect(nX, nY, objLocalRect.width(), objLocalRect.height());
		m_pAdvSettDlgPDF->m_pPlainTextObj->setBoundingRect(rect);
			
		nX = m_pAdvSettDlgPDF->m_pPlainTextObj->m_nXPos;
		nY = m_pAdvSettDlgPDF->m_pPlainTextObj->m_nYPos;
		m_pAdvSettDlgPDF->setXPosValue(nX);
		m_pAdvSettDlgPDF->setYPosValue(nY);
		
		pTextLabel->show();

		if (pEvent->source() == this)
		{
			pEvent->setDropAction(Qt::MoveAction);
			pEvent->accept();
		}
		else
			pEvent->acceptProposedAction();
	}
	else
		pEvent->ignore();
}

//Bookmarks settings
//on right chicking an item. creating a context menu 
void CAdvSettDlgPDF::onBMItemRightClicked(QTreeWidgetItem* pItem, QPoint pos, const int &nCol)
{	 
	if(pItem)
	{
		QAction* pAction = NULL;

		QList<QAction*> menuList;
		menuList.clear();		

		m_pBMCurrentItem = pItem;
		m_pDocTreeWidget->HideWidgets();

		if(m_pDocTreeWidget->indexOfTopLevelItem(pItem) == 0 || m_pDocTreeWidget->indexOfTopLevelItem(pItem) == 1 || m_pDocTreeWidget->indexOfTopLevelItem(pItem) == 2) 
		{
			QAction* pAddAction(new QAction(QIcon(":/images/Add.png"), tr("&Add New"), this));
			menuList.insert(0, pAddAction);
			m_pDocTreeWidget->CreateSetRClkMenuData(menuList);
			pAction = m_pDocTreeWidget->ExecPopupMenu(pos);
			if(pAction!= NULL)
			{
				QString strActionText(pAction->text());
				if(strActionText.compare(menuList.value(0)->text()) == 0)//Add New
				{
					addBMNewAttribute();
				}
			}
		}
		else
		{
			QAction* pRenameAction(new QAction(QIcon(":/images/Rename.png"), tr("&Rename"), this));
			QAction* pDeleteAction(new QAction(QIcon(":/images/Delete.png"), tr("&Delete"), this));

			menuList.insert(0, pRenameAction);
			menuList.insert(1, pDeleteAction);

			m_pDocTreeWidget->CreateSetRClkMenuData(menuList);
			pAction = m_pDocTreeWidget->ExecPopupMenu(pos);
			if(pAction!= NULL)
			{
				QString strActionText(pAction->text());
				if(strActionText.compare(menuList.value(0)->text()) == 0)
				{
					onBMRename(pItem);
				}
				else
				{
					onBMDelete(pItem);
				}
			}
		}

		foreach(pAction, menuList)
		{
			if(pAction)
			{
				delete pAction;
				pAction = NULL;
			}			
		}

		menuList.clear();
	}
}

//Adding new attribute name
void CAdvSettDlgPDF::addBMNewAttribute()
{
	bool bOk = false;
	QString strName("");
	QString strLevelName(""); 
	int nPageCount = 0; 
	QTreeWidgetItem *pItem = NULL;
	
	strName = QInputDialog::getText(this, tr("Bookmark"), tr("Enter Bookmark Name"), QLineEdit::Normal, 0, &bOk);
	strLevelName = m_pBMCurrentItem->text(0);

	nPageCount = m_strPageList.size();

	if(strLevelName == tr("Page_Level"))
	{
		if(nPageCount && m_strPageList.contains(strName, Qt::CaseInsensitive))
		{
			QMessageBox::critical(this, tr("Bookmark"), tr("Name already exists."));
			return;
		}
	}

	if(bOk && !strName.isEmpty())
	{
		pItem = new QTreeWidgetItem(m_pBMCurrentItem);
		pItem->setText(0, strName);
		m_pBMCurrentItem->setExpanded(true);
		
		if(strLevelName == tr("Page_Level"))
			m_strPageList.append(strName);
	}
}

//Renaming the attribute name
void CAdvSettDlgPDF::onBMRename(QTreeWidgetItem *pItem)
{
	QString strOldName("");
	QString strNewName("");
	QString strLevelName(""); 

	bool bOk = false;
	strOldName = pItem->text(0);
	strNewName = QInputDialog::getText(this, tr("Bookmark"), tr("Enter Bookmark Name"), QLineEdit::Normal, strOldName, &bOk);
	strLevelName = m_pBMCurrentItem->parent()->text(0);
	
	if(bOk && !strNewName.isEmpty())
	{
		if(strLevelName == tr("Page_Level") && m_strPageList.contains(strNewName, Qt::CaseSensitive)) 
		{
			QMessageBox::information(0, tr("Bookmark"), tr("Name already exists"));
			//onRename(pItem);
			return;
		}
		else
		{
			pItem->setText(0, strNewName);
		}
	}
}

//On deleting the attribute name
void CAdvSettDlgPDF::onBMDelete(QTreeWidgetItem *pItem)
{
	QString strLevelName("");
	m_pBMCurrentItem = pItem;
	
	strLevelName = m_pBMCurrentItem->parent()->text(0);
	
	if(strLevelName == tr("Page_Level") && m_strPageList.contains(m_pBMCurrentItem->text(0), Qt::CaseSensitive)) 
	{
		int nRet = QMessageBox::question(this, tr("Bookmark"),tr("Are you sure you want to delete the bookmark from the document?"),QMessageBox::Ok | QMessageBox::Cancel); 
		if(nRet == QMessageBox::Ok)
		{
			int nPos = m_strPageList.indexOf(m_pBMCurrentItem->text(0));
			m_strPageList.removeAt(nPos);
			if(m_pBMCurrentItem)
			{
				delete m_pBMCurrentItem;
				m_pBMCurrentItem = NULL;
			}
		}
	}	
}

//On Tree item clicked in the Document Indexing tab
void CAdvSettDlgPDF::onBMItemClicked(QTreeWidgetItem* pItem, int nCol)
{
	m_pBMCurrentItem = pItem;
	QStringList strEntityList; 
	strEntityList = m_strPgLst;
	m_pDocTreeWidget->HideWidgets();
	if(m_pBMCurrentItem)
	{
		int nLevel = m_pDocTreeWidget->indexOfTopLevelItem(m_pBMCurrentItem);
		if(m_pDocTreeWidget->indexOfTopLevelItem(m_pBMCurrentItem) == 0 || m_pDocTreeWidget->indexOfTopLevelItem(m_pBMCurrentItem) == 1 || m_pDocTreeWidget->indexOfTopLevelItem(m_pBMCurrentItem) == 2) 
		{
			return;
		}
		else
		{
			if(nCol == 1)
				m_pDocTreeWidget->DisplayComboWidget(m_pBMCurrentItem, nCol, strEntityList); 
		}
	}
}

//On Attribute Value combo changes
void CAdvSettDlgPDF::onBMComboSelectionChange(const QString& strSelectedText)
{
	m_pDocTreeWidget->HideWidgets();

	if (m_pBMCurrentItem)
	{
		m_pBMCurrentItem->setText(1, strSelectedText);
	}
}
