#include "stdafx.h"
#include "MontageDlg.h"

#define monXScale 300
#define monYScale 300
#define POINT_RAD 5

CMontageDlg::CMontageDlg() : CDialogEx(CMontageDlg::IDD)
{
}
//CMontageDlg::~CMontageDlg()
//{
//	if (crtMon)
//	{
//		delete crtMon;
//		crtMon = NULL;
//	}
//}
void CMontageDlg::DrawMontage(CDC* dc, Amontage mMon)
{
	CDC memDC;
	CBitmap bmp;
	memDC.CreateCompatibleDC(dc);
	bmp.CreateCompatibleBitmap(dc, monXScale + 20, monYScale);
	memDC.SelectObject(bmp);
	Graphics graphics(memDC.m_hDC);
	graphics.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);

	//Draw montage area
	SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
	Pen blackPen(Gdiplus::Color::Black);
	graphics.FillRectangle(&whiteBrush, 0, 0, monXScale + 20 - 1, monYScale - 1);
	
	graphics.DrawEllipse(&blackPen, 10, 0, monXScale - 1, monYScale - 1);

	//draw elec point
	for (int i = 0; i < theApp.elecNum; i++)
	{
		SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
		graphics.FillEllipse(&blackBrush, theApp.mElec[i].ePos.x - POINT_RAD, theApp.mElec[i].ePos.y - POINT_RAD, 2 * POINT_RAD, 2 * POINT_RAD);

		// Initialize arguments.
		Gdiplus::Font myFont(L"Arial", 8);
		PointF origin(theApp.mElec[i].ePos.x + 2, theApp.mElec[i].ePos.y - 2);
		graphics.DrawString(theApp.mElec[i].eName, theApp.mElec[i].eName.GetLength(),
			&myFont,
			origin,
			&blackBrush);
	}

	//POSITION pos =  crtMon.mList.GetHeadPosition();
	for (int i = 0; i < crtMon.mList.GetSize(); i++)
	{
		Alead curr = crtMon.mList.GetAt(i);
		CPoint* p1 = getElecPoint(curr.lFirstID);
		CPoint* p2 = getElecPoint(curr.lSecondID);
		if (p1 != NULL && p2 != NULL)
		{			
			AdjustableArrowCap arrowCap(7.0, 4.0, TRUE);
			blackPen.SetCustomEndCap(&arrowCap);

			graphics.DrawLine(&blackPen, p1->x, p1->y, p2->x, p2->y);
		}
	}
	dc->BitBlt(220, 35, monXScale + 20, monYScale, &memDC, 0, 0, SRCCOPY);
	return;
}

void CMontageDlg::OnPaint()
{
	CPaintDC dc(this);

		// TODO: Add your message handler code here
		// Do not call CDialogEx::OnPaint() for painting messages

	DrawMontage(&dc, crtMon);
}


int CMontageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	BOOL flag = FALSE;

	mon_l1.SetCurSel(0);
	mon_l2.SetCurSel(0);
	mon_lName.SetCurSel(0);
	CAmekaDoc* doc = CAmekaDoc::GetDoc();
	mon_list.ResetContent();
	mon_lName.ResetContent();
	int count = 0;
	//POSITION pos =  theApp.monList.GetHeadPosition();
	for (int i = 0; i < theApp.monList.GetSize(); i++)
	{
		Amontage mon =  theApp.monList.GetAt( i );
		
		//POSITION pos1 =  mon.mList.GetHeadPosition();
		mon_lName.AddString(mon.mName);
		if (flag != TRUE)
			crtMon = mon;
		if (doc != NULL && mon.mName == doc->mMon.mName)
			flag = TRUE;
	}
	mon_lName.SetWindowTextW(crtMon.mName);
	//pos =  crtMon.mList.GetHeadPosition();
	for (int i = 0; i < crtMon.mList.GetSize(); i++)
	{
		Alead lead = crtMon.mList.GetAt( i );
		CString tmp;
		tmp = getElecName(lead.lFirstID) + "   ->   " + getElecName(lead.lSecondID);
		mon_list.AddString(tmp);
	}

	for(int i = 0; i < theApp.elecNum; i++)
	{
		mon_l1.AddString(theApp.mElec[i].eName);
		mon_l2.AddString(theApp.mElec[i].eName);
	}
	mon_l1.SetCurSel(0);
	mon_l2.SetCurSel(0);
	return 0;
}

void CMontageDlg::OnMonListSelChange()
{
	// TODO: Add your control notification handler code here
	mon_list.ResetContent();
	CString tmp;
	mon_lName.GetLBText(mon_lName.GetCurSel(), tmp);
	//POSITION pos =  theApp.monList.GetHeadPosition();
	for (int i = 0; i < theApp.monList.GetSize(); i++)
	{
		Amontage mon =  theApp.monList.GetAt( i );
		if (tmp == mon.mName)
		{
			for (int j = 0; j < mon.mList.GetSize(); j++)
			{
				Alead lead = mon.mList.GetAt( j );
				CString tmp;
				tmp = getElecName(lead.lFirstID) + "   ->   " + getElecName(lead.lSecondID);
				mon_list.AddString(tmp);
			}
			crtMon = mon;
			this->OnPaint();
			this->Invalidate();
			this->UpdateWindow();
			return;
		}
	}
}

void CMontageDlg::OnBnClickedadd()
{
	// TODO: Add your control notification handler code here
	int pos1 = mon_l1.GetCurSel();
	int pos2 = mon_l2.GetCurSel();
	mon_list.AddString(getElecName(pos1+1) + "   ->   " + getElecName(pos2+1));
	Alead node;
	node.lFirstID = pos1 + 1;
	node.lSecondID = pos2 + 1;

	//crtMon.mName = tmp;
	crtMon.mList.Add(node);
	//theApp.monList.Add(crtMon);
	//mon_lName.AddString(tmp);
	this->OnPaint();
	this->Invalidate();
	this->UpdateWindow();
}

void CMontageDlg::OnBnClickeddel()
{
	// TODO: Add your control notification handler code here
	int crtPos = mon_list.GetCurSel();
	CString data;
	mon_list.GetText(crtPos, data);
	vector<string> vecStr = Tokenize(data,"   ->   ");
	vector<string>::iterator it = vecStr.begin();
	CString cs1((*it).c_str());
	it++;
	CString cs2((*it).c_str());

	//POSITION pos =  crtMon.mList.GetHeadPosition();
	//POSITION savePos;
	for (int i = 0; i < crtMon.mList.GetSize(); i++)
	{ 
		//savePos = pos; 
		Alead curr = crtMon.mList.GetAt(i); 
		if (curr.lFirstID == getElecID(cs1) && curr.lSecondID == getElecID(cs2))
		{
			crtMon.mList.RemoveAt(i);
			mon_list.DeleteString(crtPos);
			this->OnPaint();
			this->Invalidate();
			this->UpdateWindow();
			return;
		}
	}
}

void CMontageDlg::OnBnClickedMonsave()
{
	// TODO: Add your control notification handler code here
	CString tmp;
	mon_lName.GetWindowText(tmp);

	crtMon.mName = tmp;

	//POSITION pos =  theApp.monList.GetHeadPosition();
	//POSITION savePos;
	for (int i = 0; i < theApp.monList.GetSize(); i++)
	{ 
		//savePos = pos; 
		Amontage curr = theApp.monList.GetAt(i); 
		if (curr.mName == tmp)
		{
			theApp.monList.SetAt(i, crtMon); 
			return;
		}
	}
	theApp.monList.Add(crtMon);
	mon_lName.AddString(crtMon.mName);
}

void CMontageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, mon_1, mon_l1);
	DDX_Control(pDX, mon_2, mon_l2);
	DDX_Control(pDX, IDC_LIST3, mon_list);
	DDX_Control(pDX, mon_name, mon_lName);
}

void CMontageDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CAmekaDoc* crtdoc = CAmekaDoc::GetDoc();
	crtdoc->mMon = crtMon;
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonComboBox* pMon = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_MonList));
	if (pMon != NULL)
		pMon->SetEditText(crtdoc->mMon.mName);
		
	TiXmlDocument doc;
	TiXmlElement* root = new TiXmlElement("root");
	doc.LinkEndChild(root);

	//POSITION pos =  theApp.monList.GetHeadPosition();
	for (int i = 0; i < theApp.monList.GetSize(); i++)
	{
		Amontage mon =  theApp.monList.GetAt( i );
		TiXmlElement* element = new TiXmlElement((LPCSTR)(CStringA)mon.mName);
		root->LinkEndChild(element);
		//POSITION pos1 =  mon.mList.GetHeadPosition();
		for (int j = 0; j < mon.mList.GetSize(); j++)
		{
			Alead lead = mon.mList.GetAt( j );
			TiXmlElement* element1 = new TiXmlElement((LPCSTR)(CStringA)mon.mName);
			element->LinkEndChild(element1);
			element1->SetAttribute("channel1", lead.lFirstID);
			element1->SetAttribute("channel2", lead.lSecondID);
		}
	}

	bool success = doc.SaveFile(xmlName);
	doc.Clear();

	CAmekaView* pView = CAmekaView::GetView();
	if (pView)
	{
		pView->OnDraw(pView->GetDC());
	}
	CDialogEx::OnOK();
}


void CMontageDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}



BEGIN_MESSAGE_MAP(CMontageDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMontageDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMontageDlg::OnBnClickedCancel)
	ON_BN_CLICKED(mon_add, &CMontageDlg::OnBnClickedadd)
	ON_BN_CLICKED(mon_save, &CMontageDlg::OnBnClickedMonsave)
	ON_CBN_SELCHANGE(mon_name, &CMontageDlg::OnMonListSelChange)
	ON_WM_PAINT()
	ON_BN_CLICKED(mon_del, &CMontageDlg::OnBnClickeddel)
END_MESSAGE_MAP()