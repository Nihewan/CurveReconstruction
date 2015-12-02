#include "stdafx.h"
#include "CObjectFactory.h"


CObjectFactory::CObjectFactory(void)
{
}


CObjectFactory::~CObjectFactory(void)
{
}

ObjectCopierMap CObjectFactory::s_objCopierMap;
ObjectCreatorMap CObjectFactory::s_objCreatorMap;

bool CObjectFactory::RegisterCreator(string className, ObjCreator creator) {
	return s_objCreatorMap.insert(pair<string, ObjCreator>(className, creator)).second != 0;
}

bool CObjectFactory::RegisterCopier(string className, ObjCopier copier) {
	return s_objCopierMap.insert(pair<string, ObjCopier>(className, copier)).second != 0;
}

CObject* CObjectFactory::CreateObject(string className) {
	ObjectCreatorMap::iterator iter =  s_objCreatorMap.find(className);
	if (iter != s_objCreatorMap.end()) {
		return (iter->second)();
	}
	return NULL;
}

CObject* CObjectFactory::CopyObject(string className, CObject &pro) {
	ObjectCopierMap::iterator iter = s_objCopierMap.find(className);
	if (iter != s_objCopierMap.end()) {
		return (iter->second)(&pro);
	}
	return NULL;
}