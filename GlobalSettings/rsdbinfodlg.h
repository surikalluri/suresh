// rsdbinfodlg.h: interface for the CRSDBInfoDlg class.
//////////////////////////////////////////////////////////////////////

#ifndef RSDBINFODLG_H
#define RSDBINFODLG_H

#include <QSqlDatabase>

#include "ui_dbinfodlg.h"
#include "ui_executequery.h"

using namespace Ui;

class CRSDBInfoDlg : public QDialog
{
    Q_OBJECT

public:
	CRSDBInfoDlg(const QString strIcon, QWidget* parent, Qt::WFlags flags = 0);
	~CRSDBInfoDlg();

public:
	Ui::DBInfoDlg m_Wnd;
	
public slots:
	void OnOK();
	void OnCancel();

	void OnChangeEditConnName(const QString &);
	void OnDBTypeChange(const QString& strSelText);
	void OnChangeEditServer(const QString & strText);
	void OnChangeEditUserName(const QString & strText);
	void OnChangeEditPwd(const QString & strText);

	void OnTestConnection();
	void OnClickBtnAdvance();
	inline const QString& GetDBHost() {return m_strHost;}
	inline const QString& GetDBPort() {return m_strPort;}

public:
	void InitDialog(const QString& strConnName, const QString& strDBType, const QString& strDatabase, const QString& strUserName, const QString& strUserPwd, const QString& strTableSpace, const QString& strTableSchema,const QString&strHost,const QString&strPort);
	QString GetServiceName() {return m_strConnName;}

private:
	void InitDialog();
	void CleanMembers();

	bool TestConnection(QString& strError);
	void CreateConnString();

	void LoadDatabaseTypes();
	bool IsInputValid(QString& strError);

	bool GetTableSpaces(QStringList& strListTableSpaces);
	bool GetSchemas(QStringList& strListSchemas);

public:
	QString m_strConnName;
	QString m_strDBType;
	QString m_strDBTypeStr;
	QString m_strDatabase;
	QString m_strUserName;
	QString m_strUserPwd;
	QString m_strTableSpace;
	QString m_strTableSchema;

	QString m_strConnString;
	QStringList m_strExtConnList;

	QString m_strPort;
	QString m_strHost;

private:
	QStringList m_strListDBs;
	QString m_strServiceName;
};


class CExecuteQuery: public QDialog
{
	Q_OBJECT

public:
	CExecuteQuery(QWidget* parent, Qt::WFlags flags = 0);
	~CExecuteQuery();

public:
	Ui::ExecuteQuery m_EQWnd;

	bool InitExecQueryDlg(const QString& strDBType, const QString& strDatabase, const QString& strUserName, const QString& strUserPwd, const QString& strTableSpace, const QString& strTableSchema, QString& strError, const QString&strHost, const QString&strPort );
	bool ExecuteQuery(QSqlDatabase oDatabase, const QString& strQuery, QStringList &strLst, QStringList &strLstResults, QString& strError);

public:
	QString m_strDBType;
	QString m_strDBTypeStr;
	QString m_strDatabase;
	QString m_strUserName;
	QString m_strUserPwd;
	QString m_strTableSpace;
	QString m_strTableSchema;
	QString m_strPort;
	QString m_strHost;

public slots:
	void OnExecute();
	void OnOK();
};
#endif	//RSDBINFODLG_H
