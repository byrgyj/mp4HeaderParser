#include "StdAfx.h"
#include "AudioHeaderData.h"


AudioHeaderData::AudioHeaderData(uint32_t size) : mData(NULL), mCurrentIndex(0), mDataLength(size)
{
	if (size > 0)
		mData = new uint8_t[size];
}


AudioHeaderData::~AudioHeaderData(void)
{
	if(mData != NULL)
		delete []mData;
}

uint32_t AudioHeaderData::readData(void *buf, uint32_t size)
{
	if (buf == NULL || size <= 0)
		return 0;

	size = (size > mDataLength - mCurrentIndex) ? mDataLength - mCurrentIndex : size;

	memcpy(buf, mData+mCurrentIndex, size);

	mCurrentIndex += size;
}
void AudioHeaderData::appendData(uint8_t *buf, uint32_t size)
{
	if (mData==NULL || size == 0 || buf == NULL)
		return;

	size = mDataLength > size ? size : mDataLength;
	memcpy(mData, buf, size);
	mDataLength += size;
}
uint32_t AudioHeaderData::getCurrentIndex()
{
	return mCurrentIndex;
}
void AudioHeaderData::setCurrentIndex(uint32_t index)
{
	mCurrentIndex = index;
}

bool AudioHeaderData::isEnd()
{
	 return mCurrentIndex == mDataLength; 
}
