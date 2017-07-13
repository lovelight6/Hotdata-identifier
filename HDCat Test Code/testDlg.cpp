// testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "test.h"
#include "testDlg.h"

#define _WIN32_WINNT 0x0400
#include <tchar.h>
#include <wincrypt.h>
#include <string>
#include <fstream>
#include <sstream> 
#include<utility>
#include<list>
#include<set>
#include<map>
#include<vector>
#pragma warning(disable:4786)
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define ONEDAY 864000000000
#define ONEHOUR 36000000000
CString RackName[3]={"rack1","rack2","rack3"};
_int64 startTimestamp=128166372003061629;
_int64 MatchTime;//Number of hits
_int64 TransferTime;//Number of data transfers
_int64 TotalTimeN[8];//Total number of times
_int64 MatchTimeN[8];//Number of hits
_int64 TransferTimeN[8];//Number of data transfers
_int64 TotalTimeNN[8][24];//Total number of times
_int64 MatchTimeNN[8][24];//Number of hits
_int64 TransferTimeNN[8][24];//Number of data transfers


//Multi-hash algorithm data
#define HASHLENGTH 5//Hash bucket capacity:36^5-1=1679615
set<_int64> hashHotList;	//List of hot nodes
map<CString,int> hashCounter;//D-bit counter
_int64 hashListLength;//List length
int hashHotListMatchCount;//Used to reduce counter
HCRYPTPROV hProv;//hash provsdf

//Two level LRU algorithm data
int TLLListLength[3]={88000,12000,40000};//In fact, three tests are using this length.
set<_int64> tllHotList;	//List of hot nodes
set<_int64> tllCandidateList;//List of alternate nodes
_int64 tllListLength;//List length

//My HDCat algorithm data
struct newListNode//Used for hotList and candidateList
{
	_int64 DataID;
	int counter;
	bool recency;
};
vector<newListNode> newHotList;			//Hot data queue
vector<newListNode> newCandidateList;		//Candidate Hot Data Queue
_int64 newListLength;//List length
int newHotListMatchCount;//Used to reduce counter
int newCandidateListMatchCount;//Used to reduce counter
#define HD_HOTTHRESHOLD 5				//HotThreshold
#define HD_PASS_PROBABILITY_FULL 0.5	//When the candidatelist is full, the probability of new data passing
#define HD_PASS_PROBABILITY_NOT_FULL 0.9	//When the candidatelist is not full, the probability of new data passing

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDlg dialog

CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestDlg)
	DDX_Control(pDX, IDC_EDIT2, m_ctrFinish);
	DDX_Control(pDX, IDC_EDIT1, m_ctrResult);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestDlg, CDialog)
	//{{AFX_MSG_MAP(CTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTONLOADDATA, OnButtonloaddata)
	ON_BN_CLICKED(IDC_BUTTONLOADDATA2, OnButtonloaddata2)
	ON_BN_CLICKED(IDC_BUTTONLOADDATA3, OnButtonloaddata3)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
	ON_BN_CLICKED(IDC_BUTTON9, OnButton9)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER_NEW_MESSAGE,AddResultInfo)
	ON_MESSAGE(WM_USER_NEW_MESSAGE2,AddResultInfo2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDlg message handlers

BOOL CTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
void sendResult(CString text)
{
	AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE, 0,
       (LPARAM) (LPCTSTR) text);
}
bool LoadDataToDataBase(CString name,CString table)
{
	CString temp,sql;
	long i;
	long Interval=10000;
	
	MYSQL mysql; //Database connection handle.
	mysql_init (&mysql);
	SYSTEMTIME sys; 
	GetLocalTime( &sys ); 

	if(!mysql_real_connect(&mysql,"localhost",DBACCOUNT,DBPASSWORD,DBNAME,3306,NULL,0))  
	{
		sendResult("Database connection failed\r\n"); 
		return false;
	}
	else
		sendResult("The database connection is successful\r\n"); 

	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--Start reading%s data into the database\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds,name);
	sendResult(temp);
	temp.Format("D:\\TestData\\%s.csv",name);
	ifstream in(temp);
	string s;
	i=0;

	sql="insert into "+table+"(Timestamp,Hostname,DiskNumber,Type,Offset,Size,ResponseTime) values";
	while(getline(in,s))
	{
		i++;
		temp.Format("%s",s.c_str());
		temp.Replace(",","\',\'");
		temp="(\'"+temp+"\'),";
		sql+=temp;
		if(i%Interval==0)
		{
			sql=sql.Left(sql.GetLength()-1);
			sql+=";";
			if(mysql_real_query(&mysql,(char*)(LPCTSTR)sql,(UINT)sql.GetLength())!=0)
			{ 
				sendResult("Add failure\r\n"); 
				return 0;
			}
			sql="insert into "+table+
				"(Timestamp,Hostname,DiskNumber,Type,Offset,Size,ResponseTime) values";
			temp.Format("%lu",i);
			AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
				(LPARAM) (LPCTSTR) temp);

		}
		
	}
	if((i)%Interval!=0)
	{
		sql=sql.Left(sql.GetLength()-1);
		sql+=";";
		if(mysql_real_query(&mysql,(char*)(LPCTSTR)sql,(UINT)sql.GetLength())!=0)
		{ 
			sendResult("Add failure\r\n"); 
			return 0;
		}
		temp.Format("%lu",i);
		AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
			(LPARAM) (LPCTSTR)temp);
	}
	GetLocalTime( &sys ); 
	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--%s data has all been imported into the database£¬A total of %lu\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds,name,i);
	sendResult(temp);
	mysql_close(&mysql);
	in.close();
	return true;
}
UINT ThreadFuncLoadData1(LPVOID pParam)//Used to read data to the database thread
{
	LoadDataToDataBase("hm_0","rack1");
	LoadDataToDataBase("hm_1","rack1");
	return 0;
}
UINT ThreadFuncLoadData2(LPVOID pParam)//Used to read data to the database thread
{

	LoadDataToDataBase("wdev_0","rack2");
	LoadDataToDataBase("wdev_1","rack2");
	LoadDataToDataBase("wdev_2","rack2");
	LoadDataToDataBase("wdev_3","rack2");

	return 0;
}
UINT ThreadFuncLoadData3(LPVOID pParam)//Used to read data to the database thread
{
	LoadDataToDataBase("rsrch_0","rack3");
	LoadDataToDataBase("rsrch_1","rack3");
	LoadDataToDataBase("rsrch_2","rack3");

	return 0;
}
void CTestDlg::OnButtonloaddata() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncLoadData1,NULL,THREAD_PRIORITY_NORMAL);	
}


LRESULT CTestDlg::AddResultInfo(WPARAM wParam, LPARAM lParam)
{
	m_strResultInfo=(LPCTSTR)lParam+m_strResultInfo;
	m_ctrResult.SetWindowText(m_strResultInfo);
	return 0;
}
LRESULT CTestDlg::AddResultInfo2(WPARAM wParam, LPARAM lParam)
{
	CString temp;
	temp=(LPCTSTR)lParam;

	m_ctrFinish.SetWindowText(temp);
	return 0;
}

void CTestDlg::OnButtonloaddata2() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncLoadData2,NULL,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButtonloaddata3() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncLoadData3,NULL,THREAD_PRIORITY_NORMAL);
}
//Return 0 for no hit, return 1 to hit CandidateList, 2 for HotList
int tllMatch(_int64 id,_int64 timestamp)
{
	int day=(timestamp-startTimestamp)/ONEDAY+1;//day
	int hour=((timestamp-startTimestamp)%ONEDAY)/ONEHOUR+1;//hour
	set<_int64>::iterator index;
	TotalTimeN[day-1]++;
	TotalTimeNN[day-1][hour-1]++;
	if(tllHotList.find(id)!=tllHotList.end())//hot list hit
	{
		if(day>=2)//Calculated from the next day
		{
			MatchTime++;
			MatchTimeN[day-1]++;
			MatchTimeNN[day-1][hour-1]++;
		}
			
		return 2;
	}
	if((index=tllCandidateList.find(id))!=tllCandidateList.end())//candidate hit
	{ 
		tllHotList.insert(id);
		if(tllHotList.size()>tllListLength)//Hot list is full of migration to the candidate list
		{
			tllCandidateList.insert(*(tllHotList.begin()));
			tllHotList.erase(tllHotList.begin());
			if(tllCandidateList.size()>tllListLength)//Candidate list is full, expelled one.
			{
				tllCandidateList.erase(tllCandidateList.begin());	
			}
			if(day>=2)//Calculated from the next day
			{
				TransferTime++;
				TransferTimeN[day-1]++;
				TransferTimeNN[day-1][hour-1]++;
			}
					
		}
		if(day>=2)//Calculated from the next day
		{
			TransferTime++;
			TransferTimeN[day-1]++;
			TransferTimeNN[day-1][hour-1]++;

		}
			
		return 1;
	}

	//One did not hit
	tllCandidateList.insert(id);
	return 0;
}
void tllInitData()
{
	int i,j;
	tllHotList.clear();
	tllCandidateList.clear();
	MatchTime=0;//Number of hits
	TransferTime=0;//Number of data transfers
	for(i=0;i<8;i++)
	{
		MatchTimeN[i]=0;//Number of hits
		TransferTimeN[i]=0;//Number of data transfers
		TotalTimeN[i]=0;
	}
	for(i=0;i<8;i++)
	{
		for(j=0;j<24;j++)
		{
			MatchTimeNN[i][j]=0;//Number of hits
			TransferTimeNN[i][j]=0;//Number of data transfers
			TotalTimeNN[i][j]=0;
		}
	}
}
	

void tllTest(int RackNum)//two level lru
{
	CString temp,sql,table=RackName[RackNum-1];
	_int64 i;
	tllListLength=TLLListLength[RackNum-1];//The length of the two queues
	int day,hour;

	MYSQL mysql; //Database connection handle
	mysql_init (&mysql);
	SYSTEMTIME sys; 
	tllInitData();

	if(!mysql_real_connect(&mysql,"localhost",DBACCOUNT,DBPASSWORD,DBNAME,3306,NULL,0))  
	{
		sendResult("Database connection failed\r\n"); 
		return ;
	}
	else
		sendResult("The database connection is successful\r\n"); 
	GetLocalTime( &sys ); 
	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--TLL:Start testing %s data\r\n Database processing...\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds,table);
	sendResult(temp);
	sql.Format("select Timestamp,DiskNumber,Offset,Size from %s order by Timestamp ",table);
	if(mysql_real_query(&mysql,(char*)(LPCTSTR)sql,(UINT)sql.GetLength())!=0)
	{ 
		AfxMessageBox("The table in the database is in error"); 
		return ;
	}
	sendResult("The database connection is successful\r\n"); 
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(!(result=mysql_use_result(&mysql)))
	{ 
		AfxMessageBox("Failed to read dataset"); 
		return ;
	}

	//Start testing the data
	i=0;
	while(row=mysql_fetch_row(result))
	{
		_int64 ts=_atoi64(row[0]),os=_atoi64(row[2]),s=_atoi64(row[3]);		
		int dn=atoi(row[1]);
		_int64 id=os+1000000000000*(_int64)dn;
		int result=tllMatch(id,ts);

		i++;
		if(i%10000==0)
		{
			temp.Format("%I64d",i);
			AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
				(LPARAM) (LPCTSTR)temp);
		}
	}
	temp.Format("%I64d",i);
	AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
		(LPARAM) (LPCTSTR)temp);
	mysql_free_result(result);
	mysql_close(&mysql);//
	GetLocalTime( &sys ); 
	//The test is finished and ready to display the results.
	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--TLL:Finished test\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	sendResult(temp);
	temp.Format("Total number of hits:%I64d£¬Number of migrations:%I64d\r\n",MatchTime,TransferTime);
	sendResult(temp);
	
	for(day=0;day<7;day++)
	{
		CString temp2="";
		if(day==0)
			continue;//The first day is not counted
		temp.Format("Hit the number of times on the %dnd day:%I64d(%%%.2f),Number of migrations:%I64d\r\n",
			day,MatchTimeN[day],(double)MatchTimeN[day]/TotalTimeN[day]*100,TransferTimeN[day]);
		sendResult(temp);
		temp="Where the number of hits per hour:";
		temp2+=temp;
		for(hour=0;hour<24;hour++)
		{
			temp.Format("%I64d(%%%.2f)\t",MatchTimeNN[day][hour],
				(double)MatchTimeNN[day][hour]/TotalTimeNN[day][hour]*100);
			temp2+=temp;
		}
		temp2+="\r\n";
		temp="Where the number of times per hour is migrated:";
		temp2+=temp;
		for(hour=0;hour<24;hour++)
		{
			temp.Format("%I64d\t",TransferTimeNN[day][hour]);
			temp2+=temp;
		}
		temp2+="\r\n";
		sendResult(temp2);
	}
	//Finally clear the memory
	tllHotList.clear();
	tllCandidateList.clear();

}
void newInitData()
{
	int i,j;
	newHotList.clear();			//Hot data queue
	newCandidateList.clear();		//Candidate Hot Data Queue
	MatchTime=0;//Number of hits
	TransferTime=0;//Number of data transfers
	newHotListMatchCount=0;//Used to reduce counter
	newCandidateListMatchCount=0;//Used to reduce counter
	for(i=0;i<8;i++)
	{
		MatchTimeN[i]=0;//Number of hits
		TransferTimeN[i]=0;//Number of data transfers
		TotalTimeN[i]=0;
	}
	for(i=0;i<8;i++)
	{
		for(j=0;j<24;j++)
		{
			MatchTimeNN[i][j]=0;//Number of hits
			TransferTimeNN[i][j]=0;//Number of data transfers
			TotalTimeNN[i][j]=0;
		}
	}
}
int newDemoteHotList()//Choose one of the hot list to the candidate list
{
	vector<newListNode>::iterator index,minIndex;
	int minCounter;
	for(index=newHotList.begin();index!=newHotList.end();index++)//First find recency bit is false
	{
		if(!index->recency)
			break;
	}
	if(index!=newHotList.end())
	{
		minCounter=index->counter;
		
		for(minIndex=index;index!=newHotList.end();index++)
		{
			if(index->recency && index->counter<minCounter)
			{
				minIndex=index;
				minCounter=index->counter;
			}	
		}
		
	}
	else//All recency bits are true
	{
		index=newHotList.begin();
		minCounter=index->counter;
		for(minIndex=index;index!=newHotList.end();index++)
		{
			if(index->counter<minCounter)
			{
				minIndex=index;
				minCounter=index->counter;
			}	
		}
	}
	minIndex->counter=HD_HOTTHRESHOLD/2;
	newCandidateList.push_back(*minIndex);
	newHotList.erase(minIndex);
	return 1;
}
int newDemoteCandidateList()//Remove one from Candidate list
{
	vector<newListNode>::iterator index,minIndex;
	int minCounter;
	for(index=newCandidateList.begin();index!=newCandidateList.end();index++)//First find recency bit is false
	{
		if(!index->recency)
			break;
	}
	if(index!=newCandidateList.end())
	{
		minCounter=index->counter;
		
		for(minIndex=index;index!=newCandidateList.end();index++)
		{
			if(index->recency && index->counter<minCounter)
			{
				minIndex=index;
				minCounter=index->counter;
			}	
		}
		
	}
	else//All recency bits are true
	{
		index=newCandidateList.begin();
		minCounter=index->counter;
		for(minIndex=index;index!=newCandidateList.end();index++)
		{
			if(index->counter<minCounter)
			{
				minIndex=index;
				minCounter=index->counter;
			}	
		}
	}
	newCandidateList.erase(minIndex);
	return 1;
}
int newInsertCandidateList(newListNode newCandidateNode)
{
	float probability;
	probability=newCandidateList.size()<newListLength?HD_PASS_PROBABILITY_NOT_FULL:HD_PASS_PROBABILITY_FULL;
	srand(time(NULL)); 
	if(probability<((float)(rand()%100 + 100 )/100.0f-1.0f))//Data is not filtered
	{

		return 0;//Did not pass the screening
	}
	else//By screening
	{
		if(newCandidateList.size()+1>newListLength)
		{
			newDemoteCandidateList();
		}
		for(vector<newListNode>::iterator index=newCandidateList.begin();index!=newCandidateList.end();index++)
		{//Reset the recount bit of other nodes
			index->recency=false;
		}
		newCandidateList.push_back(newCandidateNode);
		return 1;
	}	

	return -1;
}
int newMatch(_int64 id,_int64 timestamp)
{
	int day=(timestamp-startTimestamp)/ONEDAY+1;//Day
	int hour=((timestamp-startTimestamp)%ONEDAY)/ONEHOUR+1;//Hour
	vector<newListNode>::iterator index;
	TotalTimeN[day-1]++;
	TotalTimeNN[day-1][hour-1]++;
	//This place can be replaced with the find function
	for(index=newHotList.begin();index!=newHotList.end();index++)//Detect hot list
	{
		if(index->DataID==id)//hot list£¬hit
		{
			index->counter++;
			index->recency=true;
			newHotListMatchCount++;
			if(newHotListMatchCount>newHotList.size()+1)//Reduce counter
			{
				for(vector<newListNode>::iterator index2=newHotList.begin();index2!=newHotList.end();index2++)
				{
					index2->counter--;
				}
				newHotListMatchCount=0;
			}

			if(day>=2)//Calculated from the next day
			{
				MatchTime++;
				MatchTimeN[day-1]++;
				MatchTimeNN[day-1][hour-1]++;
			}
			
			return 2;
		}
	}

	//Detection of candidate list
	for(index=newCandidateList.begin();index!=newCandidateList.end();index++)
	{
		if(index->DataID==id)//candidate list hit
		{
			if(index->recency)
				index->counter++;
			index->counter++;
			index->recency=true;
			newCandidateListMatchCount++;
			if(newCandidateListMatchCount>newListLength)//Reduce counter
			{
				for(vector<newListNode>::iterator index2=newCandidateList.begin();index2!=newCandidateList.end();index2++)
				{
					index2->counter--;
				}
				newCandidateListMatchCount=0;
			}
			if(index->counter>=HD_HOTTHRESHOLD)//Promoted to hot list
			{
				if(day>=2)//Calculated from the next day
				{
					TransferTime++;
					TransferTimeN[day-1]++;
					TransferTimeNN[day-1][hour-1]++;
				}
				if((newHotList.size()+1)>newListLength)//Hot list is full of migration to the candidate list
				{	
					newDemoteHotList();//Choose one of the hot list to the candidate list
					if(day>=2)//Calculated from the next day
					{
						TransferTime++;
						TransferTimeN[day-1]++;
						TransferTimeNN[day-1][hour-1]++;
					}
				}
				newHotList.push_back(*index);
				newCandidateList.erase(index);
				for(vector<newListNode>::iterator index2=newHotList.begin();index2!=newHotList.end();index2++)
				{//Reset the hot list the recency bit of the other node
					if(index2->DataID!=id)
					{
						index2->recency=false;
					}
				}
			}//Promoted to hot list
	
			return 1;
		}//Candidate list
	}//Detection of candidate list

	//One did not hit
	newListNode newCandidateNode;
	newCandidateNode.recency=true;
	newCandidateNode.counter=1;
	newCandidateNode.DataID=id;
	newInsertCandidateList(newCandidateNode);
	
	return 0;
}
void newTest(int RackNum)//My own algorithm
{
	CString temp,sql,table=RackName[RackNum-1];
	_int64 i;
	newListLength=TLLListLength[RackNum-1];//The length of the two queues is the same as the TLL
	int day,hour;

	MYSQL mysql; //Database connection handle
	mysql_init (&mysql);
	SYSTEMTIME sys; 
	newInitData();

	if(!mysql_real_connect(&mysql,"localhost",DBACCOUNT,DBPASSWORD,DBNAME,3306,NULL,0))  
	{
		sendResult("Database connection failed\r\n"); 
		return ;
	}
	else
		sendResult("The database connection is successful\r\n"); 
	GetLocalTime( &sys ); 
	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--NEW:Start testing %s data\r\nDatabase processing...\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds,table);
	sendResult(temp);
	sql.Format("select Timestamp,DiskNumber,Offset,Size from %s order by Timestamp ",table);
	if(mysql_real_query(&mysql,(char*)(LPCTSTR)sql,(UINT)sql.GetLength())!=0)
	{ 
		AfxMessageBox("The table in the database is in error"); 
		return ;
	}
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(!(result=mysql_use_result(&mysql)))
	{ 
		AfxMessageBox("Failed to read dataset"); 
		return ;
	}

	//Start testing the data
	i=0;
	while(row=mysql_fetch_row(result))
	{
		_int64 ts=_atoi64(row[0]),os=_atoi64(row[2]),s=_atoi64(row[3]);		
		int dn=atoi(row[1]);
		_int64 id=os+1000000000000*(_int64)dn;
		int result=newMatch(id,ts);

		i++;
		if(i%1000==0)
		{
			temp.Format("%I64d",i);
			AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
				(LPARAM) (LPCTSTR)temp);
		}
	}
	temp.Format("%I64d",i);
	AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
		(LPARAM) (LPCTSTR)temp);
	mysql_free_result(result);
	mysql_close(&mysql);//
	GetLocalTime( &sys ); 
	//The test is finished and ready to display the results
	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--NEW:Finished test\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	sendResult(temp);
	temp.Format("Total number of hits:%I64d£¬Number of migrations:%I64d\r\n",MatchTime,TransferTime);
	sendResult(temp);
	
	for(day=0;day<7;day++)
	{
		CString temp2="";
		if(day==0)
			continue;//The first day is not counted
		temp.Format("Hit the number of times on the %dnd day:%I64d(%%%.2f),Number of migrations:%I64d\r\n",
			day,MatchTimeN[day],(double)MatchTimeN[day]/TotalTimeN[day]*100,TransferTimeN[day]);
		sendResult(temp);
		temp="Where the number of hits per hour:";
		temp2+=temp;
		for(hour=0;hour<24;hour++)
		{
			temp.Format("%I64d(%%%.2f)\t",MatchTimeNN[day][hour],
				(double)MatchTimeNN[day][hour]/TotalTimeNN[day][hour]*100);
			temp2+=temp;
		}
		temp2+="\r\n";
		temp="Where the number of times per hour is migrated:";
		temp2+=temp;
		for(hour=0;hour<24;hour++)
		{
			temp.Format("%I64d\t",TransferTimeNN[day][hour]);
			temp2+=temp;
		}
		temp2+="\r\n";
		sendResult(temp2);
	}
	newHotList.clear();			//Hot data queue
	newCandidateList.clear();		//Candidate Hot Data Queue
}
void hashInitData()
{
	int i,j;
	hashHotList.clear();
	hashCounter.clear();
	MatchTime=0;//Number of hits
	TransferTime=0;//Number of data transfers
	hashHotListMatchCount=0;
	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);

	for(i=0;i<8;i++)
	{
		MatchTimeN[i]=0;//Number of hits
		TransferTimeN[i]=0;//Number of data transfers
		TotalTimeN[i]=0;
	}
	for(i=0;i<8;i++)
	{
		for(j=0;j<24;j++)
		{
			MatchTimeNN[i][j]=0;//Number of hits
			TransferTimeNN[i][j]=0;//Number of data transfers
			TotalTimeNN[i][j]=0;
		}
	}
}
//The function used to get the hash, this is the intermediate function
DWORD GetHash(BYTE *pbData, DWORD dwDataLen, ALG_ID algId, LPTSTR pszHash)
{
  DWORD dwReturn = 0;


 
  HCRYPTHASH hHash;
  //Alg Id:CALG_MD5,CALG_SHA
  if(!CryptCreateHash(hProv, algId, 0, 0, &hHash))
  {
    dwReturn = GetLastError();
    CryptReleaseContext(hProv, 0);
    return dwReturn;
  }
 
  if(!CryptHashData(hHash, pbData, dwDataLen, 0))
  {
    dwReturn = GetLastError();
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return dwReturn;
  }
 
  DWORD dwSize;
  DWORD dwLen = sizeof(dwSize);
  CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)(&dwSize), &dwLen, 0);
 
  BYTE* pHash = new BYTE[dwSize];
  dwLen = dwSize;
  CryptGetHashParam(hHash, HP_HASHVAL, pHash, &dwLen, 0);
 
  lstrcpy(pszHash, _T(""));
  TCHAR szTemp[3];
  for (DWORD i = 0; i < dwLen; ++i)
  {
    //wsprintf(szTemp, _T("%X%X"), pHash[i] >> 4, pHash[i] & 0xf);
    wsprintf(szTemp, "%02X", pHash[i]);
    lstrcat(pszHash, szTemp);
  }
  delete [] pHash;
 
  CryptDestroyHash(hHash);
  
  return dwReturn;

}
//This is used to return to the hash we deserve, only to return to the first HASHLENGTH
CString hashHash1(_int64 id)
{
	CString temp;
	TCHAR szHash[100] = {0};
	GetHash((BYTE*)&id, sizeof(id), CALG_MD5,szHash);
	temp.Format("%s",szHash);
	temp=temp.Left(HASHLENGTH);
	return temp;
}
//This is used to return to the hash we deserve, only to return to the first HASHLENGTH
CString hashHash2(_int64 id)
{
	CString temp;
	TCHAR szHash[100] = {0};
	GetHash((BYTE*)&id, sizeof(id), CALG_MD2,szHash);
	temp.Format("%s",szHash);
	temp=temp.Left(HASHLENGTH);
	return temp;
}
//This is used to return to the hash we deserve, only to return to the first HASHLENGTH
CString hashHash3(_int64 id)
{
	CString temp;
	TCHAR szHash[100] = {0};
	GetHash((BYTE*)&id, sizeof(id), CALG_SHA,szHash);
	temp.Format("%s",szHash);
	temp=temp.Left(HASHLENGTH);
	return temp;
}
//This is used to return to the hash we deserve, only to return to the first HASHLENGTH
CString hashHash4(_int64 id)
{
	CString temp;
	TCHAR szHash[100] = {0};
 
	GetHash((BYTE*)&id, sizeof(id), CALG_SHA,szHash);
	GetHash((BYTE*)szHash, strlen(szHash), CALG_MD5,szHash);
	temp.Format("%s",szHash);
	temp=temp.Left(HASHLENGTH);
	return temp;
}
//Do not reduce the counter, return 1 on behalf of the election from the hotlist, 0 that did not change
int hashReduction(_int64 id)
{


	CString hash1,hash2,hash3,hash4;//Get the hash value
	hash1=hashHash1(id);
	hash2=hashHash2(id);
	hash3=hashHash3(id);
	hash4=hashHash4(id);

	if(hashCounter[hash1]<=HD_HOTTHRESHOLD)
		return 1;
	if(hashCounter[hash2]<=HD_HOTTHRESHOLD)
		return 1;
	if(hashCounter[hash3]<=HD_HOTTHRESHOLD)
		return 1;
	if(hashCounter[hash4]<=HD_HOTTHRESHOLD)
		return 1;
	return 0;
}
//Increase counter, return 1 to enter the hotlist, 0 that did not change
int hashAddition (_int64 id)
{
	int flag=1;
	map<CString,int>::iterator map1,map2,map3,map4;
	CString hash1,hash2,hash3,hash4;//Get the hash value
	hash1=hashHash1(id);
	hash2=hashHash2(id);
	hash3=hashHash3(id);
	hash4=hashHash4(id);


	if((++hashCounter[hash1])<=HD_HOTTHRESHOLD)
		flag=0;
	if((++hashCounter[hash2])<=HD_HOTTHRESHOLD)
		flag=0;
	if((++hashCounter[hash3])<=HD_HOTTHRESHOLD)
		flag=0;
	if((++hashCounter[hash4])<=HD_HOTTHRESHOLD)
		flag=0;
	if(hashCounter[hash1]>15)//Max 15
		hashCounter[hash1]=15;
	if(hashCounter[hash2]>15)
		hashCounter[hash2]=15;
	if(hashCounter[hash3]>15)
		hashCounter[hash3]=15;
	if(hashCounter[hash4]>15)
		hashCounter[hash4]=15;
	return flag;
}
//Return 0 for no hit, return 1 to hit CandidateList, 2 for HotList
int hashMatch(_int64 id,_int64 timestamp)
{
	int day=(timestamp-startTimestamp)/ONEDAY+1;//Day
	int hour=((timestamp-startTimestamp)%ONEDAY)/ONEHOUR+1;//Hour
	set<_int64>::iterator index;
	TotalTimeN[day-1]++;
	TotalTimeNN[day-1][hour-1]++;
	CString hash1,hash2,hash3,hash4;//Get the hash value
	hash1=hashHash1(id);
	hash2=hashHash2(id);
	hash3=hashHash3(id);
	hash4=hashHash4(id);
	


	if(hashHotList.find(id)!=hashHotList.end())//hot list£¬hit
	{
		hashCounter[hash1]++;
		hashCounter[hash2]++;
		hashCounter[hash3]++;
		hashCounter[hash4]++;
		if(day>=2)//Calculated from the next day
		{
			MatchTime++;
			MatchTimeN[day-1]++;
			MatchTimeNN[day-1][hour-1]++;
		}

		hashHotListMatchCount++;
		

		if(hashHotListMatchCount>hashHotList.size()*2)
		{
			for(map<CString,int>::iterator index2=hashCounter.begin();index2!=hashCounter.end();)//All reduce counter
			{
				if(--index2->second<=0)
				{
					hashCounter.erase(index2++);
				}
				else
				{
					index2++;
				}			
			}
			for(index=hashHotList.begin();index!=hashHotList.end();)//Check if there is no hot list
			{
				if(hashReduction(*index))//Filtered by hot list and turned into cold data
				{
					hashHotList.erase(index++);
					if(day>=2)//Calculated from the next day
					{
						TransferTime++;
						TransferTimeN[day-1]++;
						TransferTimeNN[day-1][hour-1]++;
					}
				}
				else
				{
					index++;
				}
			}
			hashHotListMatchCount=0;
		}

		return 2;
	}
	else
	{
		if(hashAddition (id))//Joined the hot list
		{
			if(day>=2)//´ÓThe next day began to calculate
			{
				TransferTime++;
				TransferTimeN[day-1]++;
				TransferTimeNN[day-1][hour-1]++;
			}
			hashHotList.insert(id);
		}
	}

	//One did not hit
	return 0;
}
void hashTest(int RackNum)//Multi-hash algorithm
{
	CString temp,sql,table=RackName[RackNum-1];
	_int64 i;
	hashListLength=TLLListLength[RackNum-1];//The same as the TLL
	int day,hour;

	MYSQL mysql; //Database connection handle
	mysql_init (&mysql);
	SYSTEMTIME sys; 
	hashInitData();///////////////////////////

	if(!mysql_real_connect(&mysql,"localhost",DBACCOUNT,DBPASSWORD,DBNAME,3306,NULL,0))  
	{
		sendResult("Database connection failed\r\n"); 
		return ;
	}
	else
		sendResult("The database connection is successful\r\n"); 
	GetLocalTime( &sys ); 
	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--HASH:Start testing %s data\r\n Database processing...\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds,table);
	sendResult(temp);
	sql.Format("select Timestamp,DiskNumber,Offset,Size from %s order by Timestamp ",table);
	if(mysql_real_query(&mysql,(char*)(LPCTSTR)sql,(UINT)sql.GetLength())!=0)
	{ 
		AfxMessageBox("The table in the database is in error"); 
		return ;
	}
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(!(result=mysql_use_result(&mysql)))
	{ 
		AfxMessageBox("Failed to read dataset"); 
		return ;
	}

	//Start testing the data
	i=0;
	while(row=mysql_fetch_row(result))
	{
		_int64 ts=_atoi64(row[0]),os=_atoi64(row[2]),s=_atoi64(row[3]);		
		int dn=atoi(row[1]);
		_int64 id=os+1000000000000*(_int64)dn;
		int result=hashMatch(id,ts);////////////////////////////////////////////////////////

		i++;
		if(i%1000==0)
		{
			temp.Format("%I64d",i);
			AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
				(LPARAM) (LPCTSTR)temp);
		}

	}
	temp.Format("%I64d",i);
	AfxGetMainWnd ()->SendMessage (WM_USER_NEW_MESSAGE2, 0,
		(LPARAM) (LPCTSTR)temp);
	mysql_free_result(result);
	mysql_close(&mysql);//
	GetLocalTime( &sys ); 
	//The test is finished and ready to display the results
	temp.Format("%4d/%02d/%02d %02d:%02d:%02d.%03d--HASH:Finished test\r\n",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	sendResult(temp);
	temp.Format("Total number of hits:%I64d£¬Number of migrations:%I64d\r\n",MatchTime,TransferTime);
	sendResult(temp);
	
	for(day=0;day<7;day++)
	{
		CString temp2="";
		if(day==0)
			continue;//The first day is not counted
		temp.Format("Hit the number of times on the %dnd day:%I64d(%%%.2f),Number of migrations:%I64d\r\n",
			day,MatchTimeN[day],(double)MatchTimeN[day]/TotalTimeN[day]*100,TransferTimeN[day]);
		sendResult(temp);
		temp="Where the number of hits per hour:";
		temp2+=temp;
		for(hour=0;hour<24;hour++)
		{
			temp.Format("%I64d(%%%.2f)\t",MatchTimeNN[day][hour],
				(double)MatchTimeNN[day][hour]/TotalTimeNN[day][hour]*100);
			temp2+=temp;
		}
		temp2+="\r\n";
		temp="Where the number of times per hour is migrated:";
		temp2+=temp;
		for(hour=0;hour<24;hour++)
		{
			temp.Format("%I64d\t",TransferTimeNN[day][hour]);
			temp2+=temp;
		}
		temp2+="\r\n";
		sendResult(temp2);
	}
	hashHotList.clear();
	hashCounter.clear();
	CryptReleaseContext(hProv, 0);

}
UINT ThreadFuncTest(LPVOID pParam)//Used to read data to the database thread
{
	CString temp;
	int type;
	type=(int)pParam;
	switch(type/10)
	{
		case 1://two level lru
			tllTest(type%10);
			break;
		case 2:
			newTest(type%10);
			break;
		case 3:
			hashTest(type%10);
			break;
	}
	return 0;
}
void CTestDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)11,THREAD_PRIORITY_NORMAL);	
}

void CTestDlg::OnButton2() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)12,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButton3() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)13,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButton4() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)21,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButton5() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)22,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButton6() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)23,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButton7() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)31,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButton8() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)32,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnButton9() 
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(ThreadFuncTest,(LPVOID)33,THREAD_PRIORITY_NORMAL);
}

void CTestDlg::OnOK() 
{
	// TODO: Add extra validation here
	int i;
	i=5648384;
	CString temp;
	temp.Format("%d",i);
	AfxMessageBox(temp);
	CDialog::OnOK();
}
