#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include "AudioHeaderData.h"

#ifndef RELEASE
#define RELEASE 1
#endif

#ifndef VARIABLES
#define VARIABLES                               \
	int k = 0;                                  \
	unsigned char p[5];                         \
	int inner_size = 0;
#endif

#ifndef MP4_BASE_BOX
#define MP4_BASE_BOX                            \
	uint32_t type;                              \
	uint32_t  size;
#endif

#ifndef MP4_FULL_BOX
#define MP4_FULL_BOX                            \
	MP4_BASE_BOX;                               \
	uint8_t version;                            \
	uint8_t flags[3]
#endif

#ifndef MP4_SAMPLE_ENTRY_FIELDS
#define MP4_SAMPLE_ENTRY_FIELDS                    \
	MP4_BASE_BOX;                                \
	char reserved[6];                            \
	uint16_t data_reference_index;
#endif

#ifndef VISUAL_SAMPLE_ENTRY_FIELDS
#define VISUAL_SAMPLE_ENTRY_FIELDS                \
	MP4_SAMPLE_ENTRY_FIELDS;                    \
	uint16_t      pre_defined;                  \
	uint16_t      reserved1;                    \
	uint32_t      pre_defined1[3];              \
	uint16_t      width;                        \
	uint16_t      height;                       \
	uint32_t      horiz_res;                    \
	uint32_t      vert_res;                     \
	uint32_t      reserved2;                    \
	/**/uint16_t  frames_count;                 \
	/*以下是AVCDecoderConfigurationRecord*/     \
	/**/char      compressor_name[33];          \
	/**/uint16_t  bit_depth;                    \
	/**/int16_t   pre_defined2
#endif

struct mp4_box{
	MP4_BASE_BOX;
};

struct mp4_avcC_box{
	uint32_t      size;
	uint32_t      type;
	uint8_t       configurationVersion;      //=1
	uint8_t       AVCProfileIndication;
	uint8_t       profile_compatibility;
	uint8_t       AVCLevelIndication;

	uint8_t       lengthSizeMinusOne;        // & 0x3,  ==2 bit
	uint8_t       numOfSequenceParameterSet; // & 0x1F  ==5bit
	struct SPS{
		uint16_t  sequenceParameterSetLength;
		uint8_t   *sequenceParameterSetNALUnit;
	}             *sps;

	uint8_t       numOfPictureParameterSets;
	struct PPS{
		uint16_t  pictureParameterSetLength;
		uint8_t   *pictureParameterSetNALUnit;
	}             *pps; 
};


struct mp4_avc1_box {
	VISUAL_SAMPLE_ENTRY_FIELDS;
	struct mp4_avcC_box *avcC;
	struct mp4_btrt_box *btrt;
	/*ext descriptors */
	struct mp4_m4ds_box *m4ds;
};

struct mp4_tkhd_box {
	MP4_FULL_BOX;
	uint32_t creation_time;
	uint32_t modification_time;
	uint32_t track_ID;
	uint32_t reserved1;
	uint32_t duration;

	uint32_t reserved2[2];
	uint16_t layer;
	uint16_t alternate_group;

	uint16_t volume;
	uint16_t reserved3;
	uint32_t matrix[9];
	uint32_t width;
	uint32_t height;
};

struct editList{
	uint32_t trak_duration;
	uint32_t duration;
	uint32_t rate;
};

struct mp4_elst_box{
	MP4_FULL_BOX;
	uint32_t amount;
	editList *edit_list;
};

struct mp4_mdhd_box {
	MP4_FULL_BOX;
	uint32_t creation_time;
	uint32_t modification_time;
	uint32_t timescale;
	uint32_t duration;
	uint16_t language;
	uint16_t pre_defined;
};

struct mp4_vmhd_box{
	MP4_FULL_BOX;
	uint32_t graphics_mode;
	uint8_t opcolor[4];  //==============take care
};

struct mp4_dref_box{
	MP4_FULL_BOX;
	uint32_t entry_count;
	//there are many urls
	//======================================
};

struct mp4_list_t{
	uint32_t first_chunk_num;
	uint32_t sample_amount_in_cur_table;
	uint32_t sample_description_id;
};

struct mp4_stsc_box{  //Sample-to-Chunk Atoms
	mp4_stsc_box() : scmap(NULL) {}
	MP4_FULL_BOX;
	uint32_t map_amount;
	mp4_list_t *scmap; //sample-to-trunk表的结构
};

struct mp4_stsz_box{  //sample size
	mp4_stsz_box(): sample_size_table(NULL) {}
	MP4_FULL_BOX;
	uint32_t samples_size_intotal;
	uint32_t table_size;  // 全部sample的数目。如果所有的sample有相同的长度，这个字段就是这个值。
	// 否则，这个字段的值就是0。那些长度存在sample size表中
	uint32_t *sample_size_table;  //多个sample的大小相加就是整个电影的长度
};

struct mp4_stco_box{  //Chunk Offset
	mp4_stco_box() : chunk_offset_from_file_begin(NULL) {}
	MP4_FULL_BOX;
	uint32_t chunk_offset_amount;
	uint32_t *chunk_offset_from_file_begin;
};

struct mp4_stss_box{  //Sync Sample 
	mp4_stss_box(): sync_sample_table(NULL) {}
	MP4_FULL_BOX;
	uint32_t number_of_entries;
	uint32_t *sync_sample_table;
};

struct mp4_sample_table{
	uint32_t sample_count;
	uint32_t sample_duration;
};
struct mp4_stts_box{  //Time-to-Sample
	mp4_stts_box(): time_to_sample_table(NULL) {}
	MP4_FULL_BOX;
	uint32_t number_of_entries;
	//uint32_t *time_to_sample_table;
	mp4_sample_table *time_to_sample_table; // GYJ
};

#ifndef MP4_SAMPLE_ENTRY_FIELDS
#define MP4_SAMPLE_ENTRY_FIELDS                    \
	MP4_BASE_BOX;                                \
	char     reserved[6];                       \
	uint16_t data_reference_index;
#endif

struct mp4_mp4a_box{
};

struct Sample_description{
	uint32_t            size;
	uint32_t            type;
	struct mp4_avc1_box *avc1;
	struct mp4_mp4a_box *mp4a;
	//利用这个索引可以检索与当前sample description关联的数据。
	//数据引用存储在data reference atoms。
};

struct mp4_stsd_box{
	mp4_stsd_box() : sample_description(NULL) {}
	MP4_FULL_BOX;                           //12 bytes
	uint32_t            number_of_entries;  //4 bytes
	Sample_description *sample_description;
};

struct mp4_smhd_box{  //sound media header box
	MP4_FULL_BOX;
	uint16_t            balance;
	uint16_t            reserved;
};

struct mp4_hdlr_box {
	MP4_FULL_BOX;
	uint32_t       pre_defined;
	uint8_t        handler_type[5];
	uint32_t       reserved[3];
	char           *name;  //end with '\0'
};

/* 14496-12 8.2, media data container */
struct mp4_mdat_box {
	MP4_BASE_BOX;
	uint64_t data_size;
	char     *data;
};

struct mp4_mvhd_box{
	MP4_FULL_BOX;                //12
	uint32_t creation_time;      //4
	uint32_t modification_time;  //4
	uint32_t timescale;          //4
	uint32_t duration;           //4
	uint32_t rate;               //4
	uint16_t volume;             //2
	uint16_t reserved1;          //2
	uint32_t reserved2[2];       //8
	uint32_t matrix[9];          //36
	uint32_t pre_defined[6];     //24
	uint32_t next_track_id;      //4
};

struct mp4_iods_box{
};

struct mp4_udta_box{
};

struct mp4_dinf_box{
};

struct mp4_stbl_box{
	mp4_stsd_box stsd;
	mp4_stts_box stts;
	mp4_stss_box stss;
	mp4_stsc_box stsc;
	mp4_stsz_box stsz;
	mp4_stco_box stco;
};

struct mp4_minf_box{
	MP4_BASE_BOX;
	mp4_vmhd_box *vmhd;
	mp4_smhd_box *smhd;
	mp4_hdlr_box hdlr;
	mp4_dinf_box dinf;
	mp4_stbl_box stbl;
};

struct mp4_mdia_box{
	MP4_BASE_BOX;
	mp4_mdhd_box mdhd;
	mp4_hdlr_box hdlr;
	mp4_minf_box minf;
};

struct mp4_edts_box{
};

struct mp4_trak_box{
	mp4_tkhd_box tkhd;
	mp4_mdia_box mdia;
	mp4_edts_box edts;
	mp4_udta_box udta;
};



struct mp4_moov_box{
	MP4_BASE_BOX;
	mp4_mvhd_box mvhd;
	mp4_iods_box iods;
	std::vector<mp4_trak_box *>
		trak;
	mp4_udta_box udta;
};

static union ____indian_swap{
	unsigned char byte[4];
	unsigned int size;
}indian_a , indian_b;

class Mp4HeaderParser
{
public:
	Mp4HeaderParser(void);
	~Mp4HeaderParser(void);

	void init();
	bool parser();

	int mp4ReadMvhdBox(int size);  //level 3
	mp4_tkhd_box mp4ReadTkhdBox(int size);  //level 4
	mp4_edts_box mp4ReadEdtsBox(FILE *f, int size);
	mp4_mdhd_box mp4ReadMdhdBox(int size);
	mp4_hdlr_box mp4ReadHdlrBox(int size);
	//mp4_vmhd_box * mp4ReadVmhdBox(int size);
	mp4_smhd_box *mp4ReadSmhdBox(int size);
	struct mp4_stts_box mp4ReadSttsBox(int size);  //level 8
	mp4_avcC_box *mp4ReadAvccBox();
	mp4_avc1_box* mp4ReadAvc1Box(int size);  //level 9
	mp4_mp4a_box * mp4ReadMp4aBox(int size);  //level 9
	mp4_stsd_box mp4ReadStsdBox(int size);  //level 8
	mp4_stss_box mp4ReadStssBox(int size);  //level 8
	mp4_stsc_box mp4ReadStscBox(int size);  //level 8
	mp4_stsz_box mp4ReadStszBox(int size);  //level 8
	mp4_stco_box mp4ReadStcoBox(int size);
	bool stblNeedToOffset(int curPos);

/*container box
  (stsd, stts, stsz|stz2, stsc, stco|co64, ctts, stss)
*/
	mp4_stbl_box mp4ReadStblBox(int size);
	struct mp4_dinf_box mp4ReadDinfBox(int size);
	bool minfNeedToOffset(int curPos);
	mp4_minf_box mp4ReadMinfBox(int size);


/*container box
  (mdhd, hdlr, minf)
*/
	mp4_mdia_box mp4ReadMdiaBox(int size);
	//mp4_udta_box mp4ReadUdtaBox(int size);
	void mp4ReadTrakBox( int size);
	//void mp4ReadIodsBox(int size);  //level 3
	void mp4ReadMoovBox(int size);   //level 2
	void mp4ReadFtypBox(int size);  //level 2
	void mp4ReadMdatBox(int size);
	int mp4ReadRootBox(); //level 1
	void print_trak(const struct mp4_trak_box * trak);
	uint32_t getOffsetByTime(uint32_t seconds);

private:
	unsigned readUint16Big();
	uint16_t readUint16Lit();
	unsigned char readUint8();
	int readUint32Lit();

private:
	AudioHeaderData *mAudioData;
	mp4_moov_box mMoovBox;
};

