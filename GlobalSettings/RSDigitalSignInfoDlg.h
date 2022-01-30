#ifndef RSDIGIITALSIGNINFODLG_H
#define RSDIGIITALSIGNINFODLG_H



#include "ui_DigitalSignatureSettings.h"
//#include "ui_executequery.h"

using namespace Ui;

class CRSDigitalSignatureInfoDlg : public QDialog
{
    Q_OBJECT

public:
	CRSDigitalSignatureInfoDlg(const QString strIcon, QWidget* parent, Qt::WFlags flags = 0);
	~CRSDigitalSignatureInfoDlg();

public:
	Ui::DigitalSignatureSettings m_oDlg;
public:
InitDialog(QString& m_strLocation, QString& m_strReason, QString& m_strTimeStamp, QString& m_strPageNo, QString& m_strLeft,QString& m_strTop ,QString& m_strWidth , QString& m_strHeight, QString& m_strPageId,
				bool& m_bTimestamp,bool& m_bSignImages,bool& m_b2048Bit );
		
	
public slots:
    void onDSSignNameEdit();
	void onPFXFilePath();
	void onPasswordEdit();
	void onLocationEdit();
	void onReasonEdit();
	void onTimeStampEdit();
	void onLeftEdit();
	void onTopEdit();
	void onWidthEdit();
	void onHeightEdit();
	bool validateInput();
	void onCmbBoxUserNameChanged(int);
	void onTimestampStateChanged(int);
		
	void onCancel();
	void onOk();
	void onBrowse();


public:
    QString m_strDSSignName;
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

private:

	
};


#endif	//RSDIGIITALSIGNINFODLG_H