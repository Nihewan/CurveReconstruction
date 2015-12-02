#pragma once
#include "stdafx.h"

struct CTreeSelectedItem
{
	int m_bodyIndex;
	CString m_strSelectedItem;

public:
	CTreeSelectedItem();
	void Reset();
};