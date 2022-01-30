/************************************************************/
/*****   CAdvSettDlgPDF Class Handles all the Events 	*****/
/*****	    of the Advance Settings Dialog of PDF		*****/
/************************************************************/
/*****		  Written by								*****/
/*****      for ReportSuite6 on							*****/
/************************************************************/



#ifndef ADVSETTDLGPDF_H
#define ADVSETTDLGPDF_H123+456

#include <QtGui/QDialog>
#include "ui_advSettingsPDF.h"
#include "rsencrypt.h"
#include <QDomElement>
#include <QFrame>
#include "rsdbtreewidget.h"
class CGroupInfo;
class CDSData;
class CPlainTextObj;
class CDSWatermarkProp;
class CPreviewFrame;
class CAdvSettDlgPDF : public QDialog
{
    Q_OBJECT

public:
    CAdvSettDlgPDF( const  QMap<QString,QString>& pMap,QList<CGroupInfo*>& lstGrpInfo, QWidget *parent = 0, Qt::WFlags flags = 0);
    ~CAdvSettDlgPDF();
	inline QDomElement getAdvSettNode() { return m_oAdvSettNode; }
	void setAdvSettNode(QDomElement oAdvSettNode);
	inline void setTemplatePath(QString strTmpltName){m_strTemplatePath = strTmpltName;}
	inline void setOpenSSLPath(QString strOpenSSLPath){m_strOpenSSLPath = strOpenSSLPath;}
	QString getTemplatePath(){return m_strTemplatePath;}
	QList<CGroupInfo*> * m_pListGrpInfo;
public:
    Ui::AdvSettPDF ui;
	QRSDBTreeWidget* m_pTreeWidget;
	QTreeWidgetItem* m_pTreeWidgetItem;
	QTreeWidgetItem* m_pCurrentItem;
	QTreeWidgetItem* m_pRenameItem;
	QStringList m_strLstSignNames;
	QStringList m_strPgLst;
	const QMap<QString,QString> *m_pGlblObjMapPtr;
	QMap<QString, QString> m_PageInfoMap;
	CPlainTextObj *m_pPlainTextObj;
	CPreviewFrame *m_pFrame;
	QLabel *m_pTextLabel;

	QRSDBTreeWidget *m_pDocTreeWidget;
	QTreeWidgetItem *m_pPageTreeWidgetItem;
	QTreeWidgetItem *m_pBMCurrentItem;
	QStringList m_strPageList;

private:
	void initialize(const QMap<QString,QString>& Map, QList<CGroupInfo*>& lstGrpInfo);
	void calculatePermissions(int nIndex, unsigned char *chPerm);
	void convertCharToHex(const QString &strChar, QString &strHex);
	void convertHexToChar(const QString &strInput, QString &strDest);
	void convertCharToHex(char *strTmp, int nLen, QString &strHex);
	void setData(CDSData *pData);
	void addNewItem();
	void onRename(QTreeWidgetItem *pItem);
	void onEdit();
	void onDelete();
	bool saveToXml();
	void setValidation();
	void loadDigitalSignNode();
	bool GenerateCertificate(QString strpfxPath,QString strPassword,QString &strpemPath,QString &strcertPath);

	void displayDSProperties(int nIndex);
	void ResetDSProperties();
	bool ValidateInput();
	bool validateImgSett();

	void addBMNewAttribute();
	void onBMRename(QTreeWidgetItem *pItem);
	void onBMDelete(QTreeWidgetItem *pItem);

public:
	void displayPreView();
	void setXPosValue(int nVal);
	void setYPosValue(int nVal);
	void replaceKeysWithNames(const QString &strInput, QString &strOutput);
	void replaceNamesWithKeys(const QString& strInput,QString& strOutput);

private:
	QDomElement m_oAdvSettNode;
	QStringList m_strLstCmbBoxItems;
	CRSEncrypt *m_pEncrObj,*m_pEncrObj1;
	bool m_bLoadValues;
	bool m_bEditMargin;
	bool m_bAddNew;
	QString m_strTemplatePath;

	/* Document Properties */
	QString m_strTitle;
	QString m_strAuthor;
	QString m_strSubject;
	QString m_strKeywords;
	/*Watermark settings */
	QString m_strEditText; 
	QFont m_Font;
	QColor m_Color;
	int m_nXPos;
	int m_nYPos;
	int m_nWidth;
	int m_nHeight;
	int m_nAngle;
	QRect m_Rect;
	QRect m_TmpRect;
	bool m_bImageWM;

	QString m_strPgWMText; 
	QString m_strPgWMFont;
	int m_nPgWMFntStyle;   
	int m_nPgWMFntSize;
	QColor m_PgWMColor;
	int m_nPgWMAngle;
	QRect m_PgWMRect;
	QRect m_PgWMTmpRect;
	bool m_bPgImageWM;


	/*Resource Image settings */
	QString m_strImgLocPath;
	QString m_strImgUrlPath;
	
	int m_nIndex;
	QMap <int, CDSWatermarkProp> m_mapDSProp;
	bool m_bPgWatermark;

	QString m_strOpenSSLPath;

private slots:
	void on_chkBox_txtAccess_clicked();
	void on_chkBox_Copy_stateChanged(int);
	void on_chkBox_MasterPwd_stateChanged(int);
	void on_chkBox_UserPwd_stateChanged(int);
	void on_cmbBox_EncrLevel_currentIndexChanged(int);

	void onItemRightClicked(QTreeWidgetItem*, QPoint, const int &);
	void onItemClicked(QTreeWidgetItem *, int);
	void onComboBoxItemChanged(int nIndex);
	void onPFXFilePath();
	void onPasswordEdit();
	void onLocationEdit();
	void onReasonEdit();
	void onTimeStampEdit();
	void onLeftEdit();
	void onTopEdit();
	void onWidthEdit();
	void onHeightEdit();
	void onTreeCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void onCmbBoxUserNameChanged(int);
	void onTimestampStateChanged(int);
		
	void onCancel();
	void onOk();
	void onBrowse();
	bool validateInput();
	void onCurrentTabChanged(int nIndex);

	void on_cmbBox_DS_currentIndexChanged(int nIndex);
	void onImgOptionSelected(bool bStatus);
	void onDSFontClicked();
	void onDSColorClicked();
	void onDataObjClicked(const QString& strDataObjName);
	void buttonInsertClicked();
	void buttonBrowseImgClicked();
	void setChanged();
	void onXValEntered(int);
	void onYValEntered(int);
	void onWidthValEntered(int);
	void onHeightValEntered(int);
	void onAngleValChanged(int nAngle);
	void onDSSaveClicked();
	void onDSRestBtnClicked();
	void onFileBrowseButtonClicked();
	void OnURLRdBtnClick();
	void OnLocalRdBtnClick();
	void OnSetUrlPath(const QString& strURLPath);
	void OnSetLocalPath(const QString& strLclPath);

	// Bookmarks sett
	void onBMItemRightClicked(QTreeWidgetItem*, QPoint, const int &);
	void onBMItemClicked(QTreeWidgetItem *, int);
	void onBMComboSelectionChange(const QString&);
};
class CDSData
{
public:
	CDSData();
	//CDSData(QString& strPfxPathName, QString& strPassword, QString& strLocation, QString& strReason, QString& strTimeStamp, int& nPageNo, int& nLeft, int& nTop, int& nWidth, int& nHeight);
	~CDSData();

public:
	QString m_strSignName;
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
};

Q_DECLARE_METATYPE(CDSData*)

class CDSWatermarkProp
{
public:
	CDSWatermarkProp();
	~CDSWatermarkProp();
	void setValues(QString strFont, QString strText, int nAngle, QRect Rect, QRect TmpRect, int nFontStyle, int nFontSize, QColor Color, bool ImgWM);
	void setValues(QString strText, QRect Rect, QRect TmpRect, int nAngle, QFont Font, QColor Color, bool ImgWM);
public:
	QString m_strFont;
	QString m_strText;
	int m_nAngle; 
	int m_nFontStyle;
	int m_nFontSize;
	QColor m_Color;
	QFont m_Font;
	QRect m_Rect;
	QRect m_TmpRect;
	bool m_bImageWM;
};

Q_DECLARE_METATYPE(CDSWatermarkProp*)

class CPlainTextObj : public QObject
{
public:
	QPainter *m_pPainter;
	QFont m_Font;
	QColor m_Color;
	int m_nXPos;
	int m_nYPos;
	int m_nAngle;
	QString m_strText;
	QRect m_Rect, m_TemplateRect;
	float m_dr11, m_dr12, m_dtx, m_dr21, m_dr22, m_dty;

public:
	CPlainTextObj();
	~CPlainTextObj();
	
	void setValues(QString strText, QFont font, QColor color, int nXPos, int nYPos, int nWidth, int nHeight, int nAngle);
	void setValues(QString strText, int nXPos, int nYPos, int nWidth, int nHeight, int nAngle);
	void setData(QString strFont, QString strText, int nAngle, QRect Rect, QRect TmpRect, int nFontStyle, int nFontSize, QColor Color);
	void setData(QString strText, QFont Font,QColor Color, int nAngle, QRect Rect, QRect TmpRect);
	void setData(QString strText, int nAngle, QRect Rect, QRect TmpRect);
	QRect getBoundingRect();
	void setBoundingRect(const QRect& rect);
	void draw(QPainter *pPainter);
	void drawImg(QPainter *pPainter);

	void setTextRotation(int rotation);
	QPoint RotateArbitrary(QPoint ptxy, double dAngle);
	QPoint ReCalculateArbitrary(QPoint ptxy);
	void DegreesToRadians(double dDegAngle, double& dRadAngle);
	bool HitTest(const QPoint& hitPoint) const;

	void setRotation(int &nAngle);
	void setColor(QColor &Color);
	void setFont(QFont &Font);
};

Q_DECLARE_METATYPE(CPlainTextObj*)

/* for watermark Preview */
class CPreviewFrame : public QFrame
{
	Q_OBJECT

public:
	CPreviewFrame(CAdvSettDlgPDF* pAdvSettDlgPDF);
	~CPreviewFrame();
private:
	CAdvSettDlgPDF *m_pAdvSettDlgPDF;
	QRect m_TemplateRect;
	QPoint m_StartPos;
	bool m_bMousePressed;
public:
    void mousePressEvent(QMouseEvent *pEvent);
    void dragEnterEvent(QDragEnterEvent *pEvent);
    void dragMoveEvent(QDragMoveEvent *pEvent);
    void dropEvent(QDropEvent *pEvent);
	void mouseMoveEvent(QMouseEvent *pEvent);
};
Q_DECLARE_METATYPE(CPreviewFrame*)

#endif // ADVSETTDLGPDF_H
