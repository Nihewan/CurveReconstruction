#include "stdafx.h"
#include "CP_TopologyExt.h"
#include <typeinfo>
#include "CLog.h"
#include <string>
#include "gl/GL.h"

/************************************************************************/
/* CP_IBaseExt                                                          */
/************************************************************************/
CP_AbstractBaseExt::CP_AbstractBaseExt(): m_pParent(NULL) {
}

CP_AbstractBaseExt::~CP_AbstractBaseExt(){}

void CP_AbstractBaseExt::SetProperties(string propertyName, CObject *pValue)  {
	m_extCObjPtProperties[propertyName] = pValue;
}

void CP_AbstractBaseExt::SetProperties(string propertyName, int value)  {
	m_extIntProperties[propertyName] = value;
}

void CP_AbstractBaseExt::SetProperties(string propertyName, CP_BaseGeometricType *value) {
	m_extBaseGeoProperties[propertyName] = value;
}

int CP_AbstractBaseExt::GetIntegralProperties(string propertyName) const {
	ASSERT(HasProperty(propertyName));
	return m_extIntProperties.at(propertyName);
}

CObject* CP_AbstractBaseExt::GetCObjPtProperties(string propertyName) const{
	ASSERT(HasProperty(propertyName));
	return m_extCObjPtProperties.at(propertyName);
}

CP_BaseGeometricType* CP_AbstractBaseExt::GetBaseGeoProperties(string propertyName) const{
	ASSERT(HasProperty(propertyName));
	return m_extBaseGeoProperties.at(propertyName);
}


bool CP_AbstractBaseExt::HasProperty(string propertyName) const{
	map<string, int>::const_iterator iter0 = m_extIntProperties.find(propertyName);
	if (iter0 != m_extIntProperties.end())
		return true;

	map<string, CObject *>::const_iterator iter1 = m_extCObjPtProperties.find(propertyName);
	if (iter1 != m_extCObjPtProperties.end())
		return true;

	map<string, CP_BaseGeometricType*>::const_iterator iter2 = m_extBaseGeoProperties.find(propertyName);
	if (iter2 != m_extBaseGeoProperties.end())
		return true;
	return false;
}

bool CP_AbstractBaseExt::RemoveProperty(string propertyName) {
	map<string, int>::iterator iter0 = m_extIntProperties.find(propertyName);
	if (iter0 != m_extIntProperties.end()) {
		m_extIntProperties.erase(iter0);
		return true;
	}

	map<string, CObject *>::iterator iter1 = m_extCObjPtProperties.find(propertyName);
	if (iter1 != m_extCObjPtProperties.end()) {
		m_extCObjPtProperties.erase(iter1);
		return true;
	}

	map<string, CP_BaseGeometricType*>::iterator iter2 = m_extBaseGeoProperties.find(propertyName);
	if (iter2 != m_extBaseGeoProperties.end()) {
		m_extBaseGeoProperties.erase(iter2);
		return true;
	}
	return false;
} 

CObject *CP_AbstractBaseExt::GetParent() {
	if(m_pParent == NULL) return dynamic_cast<CObject *>(this);
	return m_pParent;
}

void CP_AbstractBaseExt::SetParent(CObject *pObj) {
	CP_AbstractBaseExt *pBEObj = dynamic_cast<CP_AbstractBaseExt *>(pObj);
	if (pBEObj == NULL)
		m_pParent = pObj;
	else 
		m_pParent = pBEObj->GetParent();
}

/************************************************************************/
/* CP_AssembledBody                                                     */
/************************************************************************/

CP_AssembledBody::~CP_AssembledBody() {
	int num = m_bodyArr.GetSize();
	for (int i = 0; i < num; ++i) {
		delete m_bodyArr[i];
	}
	m_bodyArr.RemoveAll();
}

CP_Body* CP_AssembledBody::GetBody(int index) {
	if (index < 0 || index >= m_bodyArr.GetSize())
		return NULL;
	return (CP_Body *)m_bodyArr[index];
}

void CP_AssembledBody::AddBody(CP_Body *pBody) {
	m_bodyArr.Add(pBody);
}

int CP_AssembledBody::GetBodyNumber() {
	return m_bodyArr.GetSize();
}

int CP_AssembledBody::FindBodyIndex(CP_Body *pBody) {
	for (int i = 0; i < GetBodyNumber(); ++i) {
		if (pBody == GetBody(i))
			return i;
	}
	return -1;
}

void CP_AssembledBody::RemoveBody(int index) {
	CP_Body *pBody = GetBody(index);
	if (pBody != NULL) {
		delete pBody;
		pBody = NULL;
		m_bodyArr.RemoveAt(index);
	}
}

void CP_AssembledBody::RemoveBody(CP_Body *pBody) {
	for (int i = 0; i < GetBodyNumber(); ++i) {
		if (pBody == GetBody(i)) {
			RemoveBody(i);
			return;
		}
	}
}



/************************************************************************/
/* CP_LoopExt                                                           */
/************************************************************************/

CP_LoopExt::CP_LoopExt():CP_Loop(), m_adjFaceNum(0), m_cutFlag(-1) {
}

CP_LoopExt::~CP_LoopExt() {

}

CP_LoopExt::CP_LoopExt(CP_Face *face):CP_Loop(face), m_adjFaceNum(0), m_cutFlag(-1) {
}

bool CP_LoopExt::IsLoopClosed() {
	return GetFirstHalf()->m_pStartVt == GetLastHalf()->m_pEndVt;
}

// 合并之后重新计算loop的邻接优先级
void CP_LoopExt::CalcLoopAdjFaceNum()
{
	CObArray tmpAdjLoopArr;

	int halfSize = GetHalfNumber();
	for(int i = 0; i < halfSize; i++)
	{
		CP_LoopExt * adjLoop = (CP_LoopExt *)GetHalf(i)->m_pTwin->m_pLoop;
		if(adjLoop->m_adjFaceNum != -1) continue;
		int j;
		for (j = 0; j < tmpAdjLoopArr.GetSize(); ++j) {
			if (tmpAdjLoopArr[j] == adjLoop)
				break;
		}
		if (j == tmpAdjLoopArr.GetSize()) tmpAdjLoopArr.Add(adjLoop);
	}
	m_adjFaceNum = tmpAdjLoopArr.GetSize();
	tmpAdjLoopArr.RemoveAll();
}

/************************************************************************/
/* CP_ParallelLoopsGroup                                                */
/************************************************************************/

CP_ParallelLoopsGroup::CP_ParallelLoopsGroup(CP_Loop * loop0, CP_Loop * loop1, double cos)
{
	loopExtd = loop0;
	loopNext = loop1;
	loopRest = NULL;
	cosValue = cos;
}

CP_ParallelLoopsGroup::CP_ParallelLoopsGroup(CP_Loop * loop0, CP_Loop * loop1, CP_Loop * loop2, double cos)
{
	loopExtd = loop0;
	loopNext = loop1;
	loopRest = loop2;
	cosValue = cos;
}

CP_ParallelLoopsGroup::~CP_ParallelLoopsGroup()
{
	loopExtd = NULL;
	loopNext = NULL;
	loopRest = NULL;
}

void DeleteParaLoopsGroupsArr(CP_ParaLoopsGroupsArr *& arr)
{
	if(arr->empty()) {
		delete arr;
		arr = NULL;
		return;
	}

	int size = arr->size();
	for(int i = 0; i < size; i++) {
		CP_ParallelLoopsGroup * plg = arr->at(i);
		delete plg;
	}
	arr->clear();
	delete arr;
	arr = NULL;
}

/************************************************************************/
/* CP_EdgeExt                                                           */
/************************************************************************/

CP_EdgeExt::CP_EdgeExt(CP_Curve3D* curve, CP_Vertex* start, CP_Vertex* end)
{
	m_pCurve3D = curve;
	m_pStartVt = start;
	m_pEndVt = end;
	m_pHalfs[0] = new CP_HalfExt(m_pStartVt, m_pEndVt,this);
	m_pStartVt->AddOutHalf(m_pHalfs[0]);
	m_pHalfs[1] = new CP_HalfExt(m_pEndVt, m_pStartVt,this);
	m_pEndVt->AddOutHalf(m_pHalfs[1]);
	m_pHalfs[0]->m_pTwin = m_pHalfs[1];
	m_pHalfs[1]->m_pTwin = m_pHalfs[0];
}