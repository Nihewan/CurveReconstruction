#pragma once
#include <vector>

template<typename T>
struct DataWithVLabel
{
	T *m_data;					// Memory managed by DataWithVLabel
	int m_blockIndex;
	int m_dataIndexInBlock;
	int m_label;
	
	DataWithVLabel(T *d, int l){
		m_data = d;
		m_label = l;
	}
	DataWithVLabel(T *d) {
		m_data = d;
		m_label = 0;
	}
	~DataWithVLabel() {
		delete m_data;
		m_data = NULL;
	}
};

template<typename T>
class AbstractDataBlock
{

protected:
	std::vector<std::vector<DataWithVLabel<T>*>> m_blocks;		// Memory managed by AbstractDataBlock
	double m_radius;

public:
	AbstractDataBlock(double radius):m_radius(radius){};
	virtual ~AbstractDataBlock(){
		for (int i = 0; i < GetBlockSize(); ++i) {
			for (int j = 0; j < GetDataSizeOfBlock(i); ++j) {
				DataWithVLabel<T> *p = GetMember(i, j);
				delete p;
				p = NULL;
			}
			GetBlock(i).clear();
		}
		m_blocks.clear();
	};

public:
	virtual void			AddMember2Block(int blockIndex, DataWithVLabel<T>* data) {m_blocks.at(blockIndex).push_back(data);};
	virtual DataWithVLabel<T>* GetMember(int blockIndex, int memIndex) {return m_blocks.at(blockIndex).at(memIndex);};
	virtual void			AddBlock() {m_blocks.push_back(std::vector<DataWithVLabel<T>*>());};
	virtual std::vector<DataWithVLabel<T>*> GetBlock(int blockIndex) {return m_blocks.at(blockIndex);};
	virtual void			SetRadius(double radius){m_radius = radius;};
	virtual double			GetRadius() {return m_radius;};
	virtual int				GetDataSize() {
		int count = 0;
		for (int i = 0; i < GetBlockSize(); ++i)
			count += GetDataSizeOfBlock(i);
		return count;
	}
	virtual int				GetBlockSize() {return m_blocks.size();};
	virtual int				GetDataSizeOfBlock(int blockIndex) {return m_blocks.at(blockIndex).size();};
	
	virtual void GetNeighborData(DataWithVLabel<T>* data, std::vector<DataWithVLabel<T>*> *vResult) = 0;	// Get neighbor data whose distance to data is less than radius
	virtual int GetBlockIndex(DataWithVLabel<T>* data) = 0;
};
