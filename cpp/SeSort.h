#ifndef __SE_SORT_H__
#define __SE_SORT_H__

#include <map>
#include <vector>

struct RItem
{
	long long llScore;
	unsigned long long ullID;

	bool operator < (const RItem& s) const
	{
		if (llScore == s.llScore)
			return ullID < s.ullID;
		return llScore > s.llScore;
	}
};

class SeSort
{
public:
	SeSort();
	~SeSort();

public:

	void ModifySortItem(unsigned long long ullID, long long llScore);

	int GetScoreNoByID(unsigned long long ullID);

	unsigned long long GetRank(int iScoreNo);

	const std::vector<unsigned long long>& GetAllRank();

	void Sort();

	void Clear();

private:

	bool m_bChange;
	std::map<RItem, int> m_kSortRItem;
	std::vector<unsigned long long> m_vecSortRItem;
	std::map<unsigned long long, long long> m_kRItem;
};

#endif


