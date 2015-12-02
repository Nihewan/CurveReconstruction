#include "stdafx.h"
#include "CTreeSelectedItem.h"

CTreeSelectedItem::CTreeSelectedItem(): m_bodyIndex(-1), m_strSelectedItem("")
{
}

void CTreeSelectedItem::Reset() {
	m_strSelectedItem = "";
	m_bodyIndex = -1;
}