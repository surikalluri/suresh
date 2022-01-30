// rsdbinfodlg.cpp : implementation file
/////////////////////////////////////////////////////////

#include "rsdbinfodlg.h"
#include "rsdbadvancedlg.h"
#include <QMessageBox>
#include <QtSql>

///////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////

CRSDBInfoDlg::CRSDBInfoDlg(const QString strIcon, QWidget* parent, Qt::WFlags flags) : QDialog(parent, flags )
{
	CleanMembers();

	m_Wnd.setupUi(this);
	
	QRegExp regExpName("[_a-zA-Z][_a-zA-Z0-9]*");
	QRegExpValidator *pNameValidator = new QRegExpValidator(regExpName, m_Wnd.m_editServerName);
	m_Wnd.m_editServerName->setValidator(pNameValidator);

	setPalette(QPalette(QColor(211, 229, 250)));

	m_strExtConnList.clear();

	connect(m_Wnd.m_editConnName,	SIGNAL(textChanged(const QString &)), this, SLOT(OnChangeEditConnName(const QString &)));
	connect(m_Wnd.m_comboDatabases,	SIGNAL(activated(const QString&)),	this, SLOT(OnDBTypeChange(const QString&)));
	connect(m_Wnd.m_editServerName,	SIGNAL(textChanged(const QString &)), this, SLOT(OnChangeEditServer(const QString &)));
	connect(m_Wnd.m_editDBUserName,	SIGNAL(textChanged(const QString &)), this, SLOT(OnChangeEditUserName(const QString &)));
	connect(m_Wnd.m_editPassword,	SIGNAL(textChanged(const QString &)), this, SLOT(OnChangeEditPwd(const QString &)));
	
	connect(m_Wnd.m_btnTestConnection,	SIGNAL(clicked()), this, SLOT(OnTestConnection()));
	connect(m_Wnd.m_btnAdvance,			SIGNAL(clicked()), this, SLOT(OnClickBtnAdvance()));

	connect(m_Wnd.m_btnOk,		SIGNAL(clicked()), this, SLOT(OnOK()));
	connect(m_Wnd.m_btnCancel,	SIGNAL(clicked()), this, SLOT(OnCancel()));

	LoadDatabaseTypes();

	m_Wnd.m_btnAdvance->setEnabled(false);
	
	setWindowIcon(QIcon(strIcon));
	setWindowTitle(tr("Database Information"));
	//setFixedSize(size());
}

CRSDBInfoDlg::~CRSDBInfoDlg()
{
	m_strListDBs.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////
void CRSDBInfoDlg::InitDialog(const QString& strConnName, const QString& strDBType, const QString& strDatabase, const QString& strUserName, const QString& strUserPwd, const QString& strTableSpace, const QString& strTableSchema,const QString &strHost,const QString &strPort)
{
	m_strConnName = strConnName;
	m_strDBType		= strDBType;
	m_strDatabase	= strDatabase;
	m_strUserName	= strUserName;
	m_strUserPwd	= strUserPwd;
	m_strTableSpace = strTableSpace;
	m_strTableSchema= strTableSchema;
	m_strPort = strPort;
	m_strHost = strHost;

	m_strDBType		= strDBType;
	QString strTempType;

	if(strDBType == "QOCI")
	strTempType = QString("ORACLE ( 9i, 10g and Above )");

   if(strDBType == "QDB2")
	strTempType = QString("IBM DB2 ( 7.1 and Above)");
	
   if(strDBType == "QMYSQL")
	strTempType = QString("MYSQL ( 4 and Above )");
	
   if(strDBType == "QMSSQL")
	strTempType = QString("Microsoft SQL Server (MSSQL)");
	
   if(strDBType == "QODBC")
	strTempType = QString("Open Database Connectivity (ODBC)");

   	int nIndex = m_Wnd.m_comboDatabases->findText(strTempType, Qt::MatchContains);
	
	if (nIndex == -1)
		nIndex = 0;

	if (nIndex >= 0)
		m_Wnd.m_comboDatabases->setCurrentIndex(nIndex);

	m_Wnd.m_editConnName->setDisabled(true);
	m_Wnd.m_editConnName->setText(strConnName);	
	m_Wnd.m_editServerName->setText(strDatabase);
	m_Wnd.m_editDBUserName->setText(strUserName);	
	m_Wnd.m_editPassword->setText(strUserPwd);

	OnDBTypeChange(m_strListDBs.value(nIndex));
}

//To load the type of databases supported
void CRSDBInfoDlg::LoadDatabaseTypes()
{
	QStringList strListItems = QSqlDatabase::drivers();  //loads the available drivers

    // Remove Compatible Names.. These are for older versions of QT
    strListItems.removeAll("QMYSQL3");
    strListItems.removeAll("QOCI8");
    strListItems.removeAll("QODBC3");
    strListItems.removeAll("QPSQL7");
    strListItems.removeAll("QTDS7");
		
   QString strDrivers = QString("~%1~").arg(strListItems.join("~")); strListItems.clear(); 
	
   if(strDrivers.contains("~QOCI~"))
	strListItems.append(tr("ORACLE ( 9i, 10g and Above )"));

   if(strDrivers.contains("~QDB2~"))
	strListItems.append(tr("IBM DB2 ( 7.1 and Above)"));
	
   if(strDrivers.contains("~QMYSQL~"))
	strListItems.append(tr("MYSQL ( 4 and Above )"));
	
   if(strDrivers.contains("~QMSSQL~"))
	strListItems.append(tr("Microsoft SQL Server (MSSQL)"));
	
   if(strDrivers.contains("~QODBC~"))
	strListItems.append("Open Database Connectivity (ODBC)");

    m_strListDBs = strListItems;
	
	m_Wnd.m_comboDatabases->clear();
	m_Wnd.m_comboDatabases->addItems(m_strListDBs);
	m_Wnd.m_comboDatabases->setCurrentIndex(0);	
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CRSDBInfoDlg::CleanMembers()
{
	m_strListDBs.clear();

	m_strDBType.clear();
	m_strDBTypeStr.clear();
	m_strDatabase.clear();
	m_strUserName.clear();
	m_strConnName.clear();
	m_strUserPwd.clear();
	m_strTableSpace.clear();
	m_strTableSchema.clear();

	m_strConnString.clear();	

	m_strPort.clear();
	m_strHost.clear();
}

bool CRSDBInfoDlg::IsInputValid(QString& strError)
{
	bool bIsValid = false;

	if (m_strConnName.isEmpty())
	{
		strError = tr("Connection name not defined");
	}
	else if (m_strDBTypeStr.isEmpty())
	{
		strError = tr("Database not defined");
	}
	else if (m_strDatabase.isEmpty())
	{
		strError = tr("Database Name not specified");
	}
	else if (m_strUserName.isEmpty())
	{
		strError = tr("User not defined");
	}
	else if (m_strUserPwd.isEmpty())
	{
		strError = tr("Password not defined");
	}
	else
	{
		bIsValid = true;
	}

	return bIsValid;
}

bool CRSDBInfoDlg::TestConnection(QString& strError)
{
	bool bSuccess = false;

	CreateConnString();

	if (IsInputValid(strError))
	{
		QSqlDatabase oDatabase = QSqlDatabase::addDatabase(m_strDBType, m_strConnName);
		oDatabase.setHostName(m_strHost);
		oDatabase.setDatabaseName(m_strDatabase);
		oDatabase.setUserName(m_strUserName);
		oDatabase.setPassword(m_strUserPwd);
		oDatabase.setPort(m_strPort.toInt());

		if(m_strDBType == "QOCI")
			m_strDBType = "Oracle";
		if(m_strDBType=="QMYSQL")
			m_strDBType = "MySql";
		if(m_strDBType == "QDB2")
			m_strDBType = "DB2";
		if(m_strDBType=="QMSSQL")
			m_strDBType = "MSSql";
		if(m_strDBType=="QODBC")
			m_strDBType = "ODBC";
		
		bSuccess = oDatabase.open();

		if (bSuccess)
		{
			//Connection succeded			
		}
		else {strError = oDatabase.lastError().text();}
	}

	return bSuccess;
}

void CRSDBInfoDlg::CreateConnString()
{
	QString strTemp = ("DRIVER=%1;HOST=%2;DATABASE=%3;USER=%4;PWD=%5;Port=%6;");
	m_strDBTypeStr = m_Wnd.m_comboDatabases->currentText();

	if (m_strDBTypeStr.contains("DB2"))
	{
		m_strDBType = "QDB2";
		m_strConnString = strTemp.arg("DB2").arg(("")).arg(m_strDatabase).arg(m_strUserName).arg(m_strUserPwd).arg("");
	}
	else if (m_strDBTypeStr.contains("ORACLE"))
	{
		m_strDBType = "QOCI";
		m_strConnString = strTemp.arg("ORACLE").arg(("")).arg(m_strDatabase).arg(m_strUserName).arg(m_strUserPwd).arg("");
	}
	/*else if (m_strDBTypeStr.contains("Access"))
	{
		m_strDBType = "DB_ACCESS";
		m_strConnString = strTemp.arg("MSACCESS").arg("").arg(m_strDatabase).arg(m_strUserName).arg(m_strUserPwd);
	}
	else if (m_strDBTypeStr.contains("Excel"))
	{
		m_strDBType = "DB_EXCEL";
		m_strConnString = strTemp.arg("MSEXCEL").arg("").arg(m_strDatabase).arg(m_strUserName).arg(m_strUserPwd);
	}*/
	else if (m_strDBTypeStr.contains("MYSQL"))
	{
		m_strDBType = "QMYSQL";
		m_strConnString = strTemp.arg("MYSQL").arg((m_strHost)).arg(m_strDatabase).arg(m_strUserName).arg(m_strUserPwd).arg(m_strPort);
	}
	else if (m_strDBTypeStr.contains("MSSQL"))
	{
		m_strDBType = "QMSSQL";
		m_strConnString = strTemp.arg("MSSQL").arg(("")).arg(m_strDatabase).arg(m_strUserName).arg(m_strUserPwd);
	}
	else if (m_strDBTypeStr.contains("ODBC"))
	{
		m_strDBType = "QODBC";
		m_strConnString = strTemp.arg("ODBC").arg(("")).arg(m_strDatabase).arg(m_strUserName).arg(m_strUserPwd);
	}

	strTemp.clear();
}

void CRSDBInfoDlg::OnClickBtnAdvance()
{
	bool bValid = false;

	QString strError("");
	QStringList strListTableSpaces, strListSchemas;
	//if (TestConnection(strError))
	{
		if (m_strDBType == "QDB2" || m_strDBType == "QMSSQL" || m_strDBType == "QMYSQL")
		{
		//	QStringList strListTableSpaces, strListSchemas;

			if (GetTableSpaces(strListTableSpaces) && GetSchemas(strListSchemas))
			{
				CRSDBAdvanceDlg dlg(this);
				dlg.InitDialog(strListTableSpaces, strListSchemas, m_strTableSpace, m_strTableSchema);

				if (m_Wnd.m_editDBUserName->isEnabled() == false)
				{
					dlg.m_Wnd.m_comboSchema->setDisabled(true);
				}

				    dlg.m_Wnd.m_lineEditHost->setText(m_strHost);
					dlg.m_Wnd.m_lineEditPort->setText(m_strPort);
				if (dlg.exec() == QDialog::Accepted)
				{
					m_strTableSpace = dlg.m_strTableSpace;
					m_strTableSchema = dlg.m_strTableSchema;
					m_strPort = dlg.m_strPort;
					m_strHost = dlg.m_strHost;
				}

				bValid = true;
			}
			else
			{
				//strError = >GetDBError();
			}
		}
		else {return;}
	}
	
	if (!bValid)
	{
		QMessageBox::critical(this, tr("Database Login"), strError);
	}
}

bool CRSDBInfoDlg::GetTableSpaces(QStringList& strListTableSpaces)
{
	//TODO::
	return true;
}
	
bool CRSDBInfoDlg::GetSchemas(QStringList& strListSchemas)
{
	//TODO::
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CRSDBInfoDlg::OnDBTypeChange(const QString& text)
{
	m_strDBTypeStr = text;

	if (text.contains("ORACLE"))
		m_Wnd.m_btnAdvance->setEnabled(false);
	else
		m_Wnd.m_btnAdvance->setEnabled(true);	

	CreateConnString();

	//if (TestConnection(strError))
	{
		if (m_strDBType == "QDB2" || m_strDBType == "QMSSQL" || m_strDBType == "QMYSQL")
		{
			m_Wnd.m_btnAdvance->setEnabled(true);
		}
	}

	/*m_Wnd.m_btnAdvance->setEnabled(text.contains("DB2"));
	m_Wnd.m_btnAdvance->setEnabled(text.contains("MSSQL"));
	m_Wnd.m_btnAdvance->setEnabled(text.contains("MYSQL"));*/
}

void CRSDBInfoDlg::OnChangeEditServer(const QString & text)
{
	m_strDatabase = text;
}

void CRSDBInfoDlg::OnChangeEditUserName(const QString &text)
{
	m_strUserName = text;
}

void CRSDBInfoDlg::OnChangeEditPwd(const QString & text)
{
	m_strUserPwd = text;
}

void CRSDBInfoDlg::OnChangeEditConnName(const QString &text)
{
	m_strConnName = text;
}

void CRSDBInfoDlg::OnTestConnection()
{
	QString strError("");

	if (TestConnection(strError))
	{
		QMessageBox::information(this, tr("Database Login"), tr("Test Connection Succeeded."));
	}
	else
	{
		QMessageBox::critical(this, tr("Database Login"), strError);
		strError.clear();
	}
}
void CRSDBInfoDlg::OnOK()
{
	QString strError("");

	if(!IsInputValid(strError))
	{
		QMessageBox::critical(this, tr("Database Login"), strError);
		return;
	}

	if(m_strExtConnList.isEmpty() || !m_strExtConnList.contains(m_strConnName, Qt::CaseInsensitive))
	{
		accept();	
	}
	else
	{
		QMessageBox::critical(this, tr("Database Login"), tr("Connection with specified name already exists. Please specify another name."));
		m_Wnd.m_editConnName->setFocus();
	}	
}

void CRSDBInfoDlg::OnCancel()
{
	reject();
}


CExecuteQuery::CExecuteQuery(QWidget* parent, Qt::WFlags flags)
:QDialog(parent)
{
	m_EQWnd.setupUi(this);

	setPalette(QPalette(QColor(211, 229, 250)));

	connect(m_EQWnd.m_btnOk,		SIGNAL(clicked()), this, SLOT(OnOK()));
	connect(m_EQWnd.m_pushBtnExecute,	SIGNAL(clicked()), this, SLOT(OnExecute()));	
}
	
CExecuteQuery::~CExecuteQuery()
{}

bool CExecuteQuery::InitExecQueryDlg(const QString& strDBType, const QString& strDatabase, const QString& strUserName, const QString& strUserPwd, const QString& strTableSpace, const QString& strTableSchema, QString& strError,const QString&strHost,const QString&strPort)
{
	//before that need to cal test connection 
	//QString strError;

	m_strDBType		= strDBType;
	m_strDatabase	= strDatabase;
	m_strUserName	= strUserName;
	m_strUserPwd	= strUserPwd;
	m_strTableSpace = strTableSpace;
	m_strTableSchema= strTableSchema;
	m_strPort = strPort;
	m_strHost = strHost;

	m_EQWnd.m_DBlabel->setText(m_strDatabase.toUpper());

	bool bSuccess = false;

	//if(TestConnection(strError))
	{
		//m_EQWnd.exec();
		QSqlDatabase oDatabase = QSqlDatabase::addDatabase(m_strDBType);
		oDatabase.setHostName(m_strHost);
		oDatabase.setDatabaseName(m_strDatabase);
		oDatabase.setUserName(m_strUserName);
		oDatabase.setPassword(m_strUserPwd);
		oDatabase.setPort(m_strPort.toInt());

		bSuccess = oDatabase.open();

		if (bSuccess)
		{
			//Connection succeded			
		}
		else {strError = oDatabase.lastError().text();}
	}
	/*else
	{
		QMessageBox::critical(this, tr("Database Connection Failure"), strError);
	}*/

	return bSuccess;
}

void  CExecuteQuery::OnExecute()
{
	m_EQWnd.m_pResultsTree->clear();

	if(m_EQWnd.plainTextEdit->toPlainText().isEmpty())
	{
		QMessageBox::critical(this, tr("Execute Query"), tr("Please enter a query to execute."));
		return;
	}

	QSqlDatabase oDatabase = QSqlDatabase::addDatabase(m_strDBType);
	oDatabase.setHostName(m_strHost);
	oDatabase.setDatabaseName(m_strDatabase);
	oDatabase.setUserName(m_strUserName);
	oDatabase.setPassword(m_strUserPwd);
	oDatabase.setPort(m_strPort.toInt());

	QString strError;

	if(oDatabase.open())
	{
		QStringList strLstResults;
		QStringList strLstHeaders;		

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		if(ExecuteQuery(oDatabase, m_EQWnd.plainTextEdit->toPlainText(), strLstHeaders, strLstResults, strError))
		{
			if (strLstHeaders.count() > 0)
			{
				m_EQWnd.m_pResultsTree->setColumnCount(strLstHeaders.count());

				for (int nIndex = 0; nIndex < strLstHeaders.count(); nIndex++)
				{
					m_EQWnd.m_pResultsTree->headerItem()->setText(nIndex, strLstHeaders.at(nIndex));  // Filling column headers
				}
			}
			if(strLstResults.count() > 0)
			{
				QTreeWidgetItem *pItem = NULL;
				QString strTemp("");
				QStringList strTempList;

				int nLoop = 0;

				for (int nIndex = 0; nIndex < strLstResults.count(); nIndex++)
				{
					if(nIndex == 50)
						break;

					strTemp.append(strLstResults.value(nIndex));

					strTempList = strTemp.split("~^~^~", QString::KeepEmptyParts, Qt::CaseInsensitive);
					nLoop = strTempList.size();

					pItem = new QTreeWidgetItem(m_EQWnd.m_pResultsTree);

					for ( int nIndex2 = 0; nIndex2 < nLoop; nIndex2++)
						pItem->setText(nIndex2, strTempList.value(nIndex2));  // Filling Resultset

					strTemp.clear();
					pItem = NULL;
				}
				m_EQWnd.m_labelStatus->setText(tr("Query Executed successfully."));
			}
			else
				m_EQWnd.m_labelStatus->setText(tr("Query Executed successfully - No Records found."));

			m_EQWnd.m_pResultsTree->setColumnWidth(0, 120);
		}
		else
		{
			m_EQWnd.m_labelStatus->setText(tr("Query Execution Failed -%1").arg(strError));
			m_EQWnd.m_labelStatus->setToolTip(tr("Query Execution Failed -%1").arg(strError));
		}
		QApplication::restoreOverrideCursor();
	}
	else
	{
		strError = oDatabase.lastError().text();

		m_EQWnd.m_labelStatus->setText(tr("Connection Error -%1").arg(strError));
		m_EQWnd.m_labelStatus->setToolTip(tr("Connection Error -%1").arg(strError));
	}
	//m_EQWnd.m_labelStatus->setText("");
}

bool CExecuteQuery::ExecuteQuery(QSqlDatabase db, const QString& strQuery, QStringList &strlstHeaders, QStringList &strlstResults, QString& strError)
{
	bool bSuccess(false);

	QSqlQuery Recordset(db);

	Recordset.setForwardOnly(true);

	bSuccess = Recordset.exec(strQuery);

	if(!bSuccess)
	{
		//Check for connection availability, if not there open again
		if (!db.isOpen())
		{
			db.open();

			Recordset.clear();

			Recordset.setForwardOnly(true);

			bSuccess = Recordset.exec(strQuery);
		}

		if (!bSuccess)
			strError = Recordset.lastError().text();
	}

    QString strDriver = db.driverName();

	if(bSuccess)
	{
		if(strQuery.startsWith("SELECT ", Qt::CaseInsensitive) == true)
		{
			if(Recordset.isActive() == true)
			{
				if(Recordset.next() == true)
				{
					int nFieldSubType =-1;

					QString strRecordData = "";
					QString strSeparator("~^~^~");
					QString strName("");
					//QString strDriver("");

					int nFieldScale = 0;
					int type = 0;

					bool bFillHeaders(true);

					QVariant varVal;

					while(Recordset.isValid())
					{
						for(int nIndex = 0;nIndex < Recordset.record().count();nIndex++)
						{
							if(bFillHeaders)
							   strlstHeaders.append(Recordset.record().field(nIndex).name());

							nFieldScale = Recordset.record().field(nIndex).precision();

							switch (Recordset.record().field(nIndex).type())
							{
							case QVariant::Bool:
								type = 5;
								break;
							case QVariant::Int:
							case QVariant::UInt:
							case QVariant::LongLong:;
							case QVariant::ULongLong:
								type = 1;
								{
									if(nFieldScale > 0)
										type = 2;
								}
								break;
							case QVariant::Double:
								type = 2;
								break;
							case QVariant::String:
								type = 4;
								break;
							case QVariant::Date:
								type = 3;
								break;
							case QVariant::Time:
								type = 7;
								break;
							case QVariant::DateTime:
								type = 8;
								break;
							case QVariant::ByteArray:
								type = 6;
								break;
							default:
								type = 0;
							}

							nFieldSubType = Recordset.record().field(nIndex).typeID();

							if(Recordset.record().field(nIndex).type() == QVariant::ByteArray && (!(strDriver == "QODBC") && (nFieldSubType ==1 || nFieldSubType ==12 ||nFieldSubType == -2)))
							{
								strRecordData.append("(BINARY)");
							}
							else if((strDriver == "QOCI")  && (type == 6) )
							{
								strRecordData.append("(BINARY)");
							}
							else
							{
								varVal = Recordset.value(nIndex);

								if(Recordset.lastError().type() != QSqlError::NoError)
								{
									strError = Recordset.lastError().text();
									strRecordData.clear();
									bSuccess = false;
									break;
								}

								if((strDriver == "QOCI"  && type == 5)|| varVal.type() == QVariant::Bool)
								{
									varVal.convert(QVariant::UInt);
								}
								strRecordData.append(varVal.toString());

								varVal.clear();
							}
							if((nIndex + 1) < Recordset.record().count())
							{
								strRecordData.append(strSeparator);
							}
							bSuccess = true;
						}
						if(bSuccess == false) { break; }
                        
						bFillHeaders = false;
							  
						strlstResults.append(strRecordData);

						strRecordData.clear();

						Recordset.next();
					}
				}
				else
				{
					strError = "Recordset is empty";
				}
			}
		}
	}
	else
	{
		if (strDriver == "QDB2")
		{
			strError = Recordset.lastError().text();

			if (strError.contains("SQL0100W") && strError.contains("SQLSTATE=02000"))
			{
				bSuccess = true;
				strError.clear();
			}
		}
	}
	return bSuccess;	
}

void CExecuteQuery::OnOK()
{
	accept();
}
