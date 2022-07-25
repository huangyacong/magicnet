#include "SeSort.h"
#include "SeTime.h"

SeSort::SeSort()
{
	m_bChange = false;
}

SeSort::~SeSort()
{

}

void SeSort::ModifySortItem(unsigned long long ullID, long long llScore)
{
	int iScoreNo = 0;

	std::map<unsigned long long, long long>::iterator itr = m_kRItem.find(ullID);
	if (itr != m_kRItem.end())
	{
		iScoreNo = GetScoreNoByID(ullID);

		RItem kRItem;
		kRItem.ullID = itr->first;
		kRItem.llScore = itr->second;
		m_kSortRItem.erase(kRItem);
		m_kRItem.erase(itr);
	}
	else
	{
		if (!m_vecSortRItem.empty())
		{
			iScoreNo = (int)m_kRItem.size() + 1;
			m_vecSortRItem.push_back(ullID);
		}
	}

	m_bChange = true;

	RItem kRItem;
	kRItem.ullID = ullID;
	kRItem.llScore = llScore;

	m_kRItem[ullID] = llScore;
	m_kSortRItem[kRItem] = iScoreNo;
}

int SeSort::GetScoreNoByID(unsigned long long ullID)
{
	std::map<unsigned long long, long long>::iterator itr = m_kRItem.find(ullID);
	if (itr == m_kRItem.end())
		return 0;
	RItem kRItem;
	kRItem.ullID = itr->first;
	kRItem.llScore = itr->second;
	if (m_kSortRItem.find(kRItem) == m_kSortRItem.end())
		return 0;
	return m_kSortRItem[kRItem];
}

unsigned long long SeSort::GetRank(int iScoreNo)
{
	iScoreNo--;
	if (iScoreNo < 0 || iScoreNo >= (int)m_vecSortRItem.size())
		return 0;
	return m_vecSortRItem[iScoreNo];
}

const std::vector<unsigned long long>& SeSort::GetAllRank()
{
	return m_vecSortRItem;
}

void SeSort::Sort()
{
	if (!m_bChange)
		return;

	m_bChange = false;
	m_vecSortRItem.clear();

	int iScoreNo = 0;
	for (std::map<RItem, int>::iterator itr = m_kSortRItem.begin(); itr != m_kSortRItem.end(); itr++)
	{
		iScoreNo++;
		itr->second = iScoreNo;
		m_vecSortRItem.push_back(itr->first.ullID);
	}
}

void SeSort::Clear()
{
	m_kSortRItem.clear();
	m_vecSortRItem.clear();
	m_kRItem.clear();
}



