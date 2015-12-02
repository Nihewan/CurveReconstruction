#include "stdafx.h"
#include "CFaceCombination.h"
#include "faceident_utils.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include "CBoundBox2D.h"
#include "base_geometricoper.h"
#include "topo_geometricoper.h"
#include "CLog.h"

CObArray CFaceCombination::s_faceArr;

CFaceCombination::CFaceCombination(void)
{
}


CFaceCombination::~CFaceCombination(void)
{
}

// 将所有子形体中的面环合并到完整形体中
void CFaceCombination::CombineFaces(CP_AssembledBody *pAsmbBody)
{
	//处理子形体
	s_faceArr.RemoveAll();
	
	int bodySize = pAsmbBody->GetBodyNumber();
	for(int i = 1; i < bodySize; i++)
	{
		CP_Body * pBody = pAsmbBody->GetBody(i);
		int faceNum = pBody->GetFaceNumber();
		for(int j = 0; j < faceNum; j++)
		{
			CP_Face * pFace = pBody->GetFace(j);
			CP_LoopExt *pLoop = (CP_LoopExt *)pFace->GetLoop(0);
			CP_LoopExt *pLoopParent = (CP_LoopExt *)pLoop->GetParent();
			if(pLoopParent != pLoop) continue;
			int halfSize = pLoop->GetHalfNumber();

			CP_LoopExt * nLoop = new CP_LoopExt();
			for(int k = 0; k < halfSize; k++)
			{
				CP_HalfExt * half = (CP_HalfExt *)pLoop->GetHalf(k);
				CP_Half * nHalf = (CP_Half *)half->GetParent();
				if(nHalf->m_pLoop != NULL)
				{
					if (ConstructLoop(nHalf,half)) {
						delete nLoop;
						nLoop = NULL;
						break;
					}
				}
				nLoop->AddHalf(nHalf);
				nHalf->m_pLoop = nLoop;
			}
			if(nLoop == NULL) continue;
			pLoop->SetParent(nLoop);
			nLoop->m_cutFlag = -1;
			s_faceArr.Add(nLoop);
		}
	}

	//根据AILine合并内外面环
	faceident_utils::AddFace2FullBody(pAsmbBody, s_faceArr);
}

// 有重复边时构造合并面环
bool CFaceCombination::ConstructLoop(CP_Half * baseHalf, CP_Half * subHalf)
{
	CP_Half * baseHalfArr[2];
	baseHalfArr[0] = baseHalf;
	baseHalfArr[1] = baseHalf->m_pTwin;
	CP_Half * subHalfArr[2];
	subHalfArr[0] = subHalf;
	subHalfArr[1] = subHalf->m_pTwin;

	CP_Half * cutBaseHalf = NULL;
	CP_HalfExt * cutSubHalf = NULL;

	//寻找最平行的两个loop // 此处有Bug需要修改，不能只根据平行性寻找剪裁面
	double cosMax = -1.0;
	for(int i = 0; i < 2; i++)
	{
		CP_Half * half0 = baseHalfArr[i];
		for(int j = 0; j < 2; j++)
		{
			CP_HalfExt * half1 = (CP_HalfExt *)subHalfArr[j];
			double cosValue = GetCutCosValue(half0->m_pLoop, half1->m_pLoop, baseHalf, subHalf);
			CLog::log("Loop0:");
			topo_geometric::OutputLoop(half0->m_pLoop);
			CLog::log("Loop1:");
			topo_geometric::OutputLoop(half1->m_pLoop);
			if(cosValue > cosMax)
			{
				cutBaseHalf = half0;
				cutSubHalf = half1;
				cosMax = cosValue;
			}
		}
	}

	CBoundBox2D bBox0;
	bBox0.InitBoundBox(cutBaseHalf->m_pLoop);
	CBoundBox2D bBox1;
	bBox1.InitBoundBox(cutSubHalf->m_pLoop);
	if (!bBox0.Contain(bBox1) && !bBox1.Contain(bBox0))
		return false;

	//检查两个loop是否面环顺序一致
	if(cutBaseHalf != ((CP_Half *)cutSubHalf->GetParent())->m_pTwin)
	{
		CP_Loop * loop = cutBaseHalf->m_pLoop;
		faceident_utils::ReverseLoop(loop);
		cutBaseHalf = cutBaseHalf->m_pTwin;
	}

	//获得两个面环除去cutHalf之后的结果
	CP_Loop * baseLoop = cutBaseHalf->m_pLoop;
	CP_LoopExt * subLoop = (CP_LoopExt *)cutSubHalf->m_pLoop;
	CutHalfFromLoop(baseLoop,cutBaseHalf);
	CutHalfFromLoop(subLoop,cutSubHalf);

	//将子面环的边合并到父面环中
	int halfSize = subLoop->GetHalfNumber();
	for(int i = 0; i < halfSize; i++)
	{
		CP_HalfExt * half = (CP_HalfExt *)subLoop->GetHalf(i);
		CP_Half * pHalf = (CP_Half *)half->GetParent();
		baseLoop->AddHalf(pHalf);
		pHalf->m_pLoop = baseLoop;
	}
	subLoop->AddHalf(cutSubHalf);
	subLoop->SetParent(baseLoop);

	//处理cutHalf的另一邻接子面环
	CP_LoopExt * twinLoop = (CP_LoopExt *)cutSubHalf->m_pTwin->m_pLoop;
	CP_Loop * nLoop = new CP_LoopExt();
	halfSize = twinLoop->GetHalfNumber();
	for(int i = 0; i < halfSize; i++)
	{
		CP_HalfExt * half = (CP_HalfExt *)twinLoop->GetHalf(i);
		CP_Half * pHalf = (CP_Half *)half->GetParent();
		nLoop->AddHalf(pHalf);
		pHalf->m_pLoop = nLoop;
	}
	twinLoop->SetParent(nLoop);
	s_faceArr.Add(nLoop);
	return true;
}

// 计算除了cutHalf之外其他边的平行性质
double CFaceCombination::GetCutCosValue(CP_Loop * loop0, CP_Loop * loop1, CP_Half * baseHalf, CP_Half * subHalf)
{
	int size0 = loop0->GetHalfNumber();
	int size1 = loop1->GetHalfNumber();
	if(size0 == 1 && size1 == 1) return 1.0;

	std::vector<double> cosArr;
	for (int i = 0; i < size0; i++)
	{
		CP_Half *half0 = loop0->GetHalf(i);
		if(half0 == baseHalf) continue;
		if(half0 == baseHalf->m_pTwin) continue;
		if(topo_geometric::GetCosValue2D(half0->m_pEdge, baseHalf->m_pEdge) > COS_MAX) continue;
		for (int j = 0; j < size1; j++)
		{
			CP_Half *half1 = loop1->GetHalf(j);
			if(half1 == subHalf) continue;
			if(half1 == subHalf->m_pTwin) continue;
			if(topo_geometric::GetCosValue2D(half1->m_pEdge, subHalf->m_pEdge) > COS_MAX) continue;
			double cosvalue = topo_geometric::GetCosValue2D(half0->m_pEdge, half1->m_pEdge);
			cosArr.push_back(cosvalue);
		}
	}
	std::sort(cosArr.begin(),cosArr.end());

	int size = cosArr.size()/2;
	for(std::vector<double>::iterator iter = cosArr.begin();iter != cosArr.end()-size;)
	{
		iter = cosArr.erase(iter);
	}
	size = cosArr.size();
	double sum = std::accumulate(cosArr.begin(),cosArr.end(),0.0);
	double retvalue = sum/size;

	return retvalue;
}

// 得到cutHalf的下一条half为起始的loop
void CFaceCombination::CutHalfFromLoop(CP_Loop * loop, CP_Half * cutHalf)
{
	std::vector<CP_Half *>  * halfArr = new std::vector<CP_Half *> ();
	CP_Half * half = loop->GetFirstHalf();
	while(half != cutHalf)
	{
		halfArr->push_back(half);
		loop->RemoveHalf(0);
		half = loop->GetFirstHalf();
	}
	loop->RemoveHalf(0);

	int halfSize = halfArr->size();
	for(int i = 0; i < halfSize; i++)
	{
		CP_Half * half = halfArr->at(i);
		loop->AddHalf(half);
	}
	delete halfArr;
}

