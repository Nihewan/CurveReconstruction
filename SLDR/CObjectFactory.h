#pragma once
#include <map>
#include <string>
using namespace std;

/* Object factory : 
	create object, and copy object by class name
*/

typedef CObject*(*ObjCreator)();
typedef CObject*(*ObjCopier)(const CObject *);
typedef map<string, ObjCreator> ObjectCreatorMap;
typedef map<string, ObjCopier> ObjectCopierMap;

class CObjectFactory
{
public:
	~CObjectFactory(void);

private:
	static ObjectCreatorMap s_objCreatorMap;
	static ObjectCopierMap s_objCopierMap;

public:
	static bool		RegisterCreator(string className, ObjCreator creator);
	static bool		RegisterCopier(string className, ObjCopier copier);
	static CObject* CreateObject(string className);
	static CObject* CopyObject(string className, CObject &pro);

private:
	CObjectFactory(void);
};

template <class T>
CObject* CreateT() {
	return new T;
}

template <class T>
CObject* CopyT(const CObject *src) {
	return new T(*((T *)src));
}


