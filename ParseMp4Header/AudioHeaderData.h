#pragma once
#include <stdint.h>
class AudioHeaderData
{
public:
	AudioHeaderData(uint32_t size = 944);
	~AudioHeaderData();

	uint32_t  readData(void *buf, uint32_t size);
	void appendData(uint8_t *buf, uint32_t size); 
	uint32_t getCurrentIndex();
	void setCurrentIndex(uint32_t index);
	uint8_t *getDataIndex() { return mData; };
	bool isEnd();
private:
	uint8_t *mData;
	uint32_t mDataLength;
	uint32_t mCurrentIndex;
};

