#include "StdAfx.h"
#include "Mp4HeaderParser.h"


Mp4HeaderParser::Mp4HeaderParser(void):mAudioData(NULL)
{
}


Mp4HeaderParser::~Mp4HeaderParser(void)
{
	Sample_description * description = mMoovBox.trak[0]->mdia.minf.stbl.stsd.sample_description;
	if (description != NULL) 
		delete []description;

	mp4_sample_table *sampleTable = mMoovBox.trak[0]->mdia.minf.stbl.stts.time_to_sample_table;
	if (sampleTable != NULL)
		delete []sampleTable;

	uint32_t *stssTable = mMoovBox.trak[0]->mdia.minf.stbl.stss.sync_sample_table;
	if (stssTable != NULL)
		delete []stssTable;
	mp4_list_t *stscScmap = mMoovBox.trak[0]->mdia.minf.stbl.stsc.scmap;
	if (stscScmap != NULL)
		delete []stscScmap;

	uint32_t *stszTable = mMoovBox.trak[0]->mdia.minf.stbl.stsz.sample_size_table;
	if (stssTable != NULL)
		delete []stszTable;

	uint32_t * stcoTable = mMoovBox.trak[0]->mdia.minf.stbl.stco.chunk_offset_from_file_begin;
	if (stcoTable != NULL)
		delete []stcoTable;
}

void Mp4HeaderParser::init()
{
	std::string luo   = "panjinlian.mp4"; 
	FILE *fin       = fopen(luo.c_str(), "rb");

	if(fin == NULL){
		printf("failed to open\n");
		return ;
	}
	fseek(fin, 0, SEEK_END);
	int fileSize = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	mAudioData = new AudioHeaderData(fileSize);
	fread(mAudioData->getDataIndex(), 1, fileSize, fin);
	fseek(fin, 0, SEEK_SET);

}
bool Mp4HeaderParser::parser()
{
	int cur_pos = 0;
	while(!mAudioData->isEnd()){
		mAudioData->setCurrentIndex(cur_pos);
		printf("----------------------------------------level 0\n");
		cur_pos += mp4ReadRootBox();
	}

	// 测试
	double arrayIndex[] = { 0,  5.06, 1000*2600, 1000 * 4000, 1000* 5000, 1000*5331};
	for (int i = 0; i < 6; i++)
	{
		uint32_t destOffset = getOffsetByTime(arrayIndex[i]);
		printf("%d", destOffset);
	}
	return true;
}


/*final box
 */
int Mp4HeaderParser::mp4ReadMvhdBox(int size)  //level 3
{
    printf("\t+%s\n", "mvhd");
    VARIABLES;
    int level_4_box_size    = 0;

    mp4_mvhd_box mvhd;
    memset(&mvhd, 0, sizeof(mvhd));
    
    mvhd.size               = size;
    mvhd.type               = ('m' | 'v'<<8 | 'h'<<16 | 'd'<<24);
    mvhd.version            = readUint8();                 //1
	mAudioData->readData(mvhd.flags, sizeof(mvhd.flags));
    mvhd.creation_time      = readUint32Lit();            //4
    mvhd.modification_time  = readUint32Lit();            //4
    mvhd.timescale          = readUint32Lit();            //4
    mvhd.duration           = readUint32Lit();            //4
    mvhd.rate               = readUint32Lit();            //4
    mvhd.volume             = readUint16Big();            //2
	mAudioData->readData(&mvhd.reserved1, sizeof(mvhd.reserved1));
	mAudioData->readData(&mvhd.reserved2, sizeof(mvhd.reserved2));
	mAudioData->readData(mvhd.matrix, sizeof(mvhd.matrix));
	mAudioData->readData(mvhd.pre_defined, sizeof(mvhd.pre_defined));
    mvhd.next_track_id      = readUint32Lit();            //4
    mMoovBox.mvhd               = mvhd;

    
    printf("\t\ttimescale: %u\n", mMoovBox.mvhd.timescale);
    printf("\t\tduration: %u\n", mMoovBox.mvhd.duration);
    printf("\t\trate: %u\n", mMoovBox.mvhd.rate );
    printf("\t\tvolume: 0x%x\n", mMoovBox.mvhd.volume);
    
    printf("\t\tmatrix:\n");
    for(int i = 0; i < 3; ++i){
        printf("\t\t");
        for(int j = 0; j < 3; ++j){
            printf(" %8u ", mMoovBox.mvhd.matrix[i*3+j]);
        }
        printf("\n");
    }
    
    printf("\t\tnext track id: %u\n", mMoovBox.mvhd.next_track_id);
   
    printf("\n");

	return 0;
}


/*final box
 */
mp4_tkhd_box Mp4HeaderParser::mp4ReadTkhdBox(int size)  //level 4
{
    printf("\t\t+%s\n", "tkhd"); 
    
    VARIABLES;
    mp4_tkhd_box box;
        
    box.size              = size;
    box.type              = 0;
    box.version           = readUint8();
	mAudioData->readData(&box.flags, sizeof(box.flags));
    box.creation_time     = readUint32Lit();
    box.modification_time = readUint32Lit();
    box.track_ID          = readUint32Lit();
    box.reserved1         = readUint32Lit();
    box.duration          = readUint32Lit();
	mAudioData->readData(&box.reserved2, sizeof(box.reserved2));
    box.layer             = readUint16Big();
    box.alternate_group   = readUint16Big();
    box.volume            = readUint16Big();
    box.reserved3         = readUint16Big();
	mAudioData->readData(&box.matrix, sizeof(box.matrix));
    box.width             = readUint32Lit();
    box.height            = readUint32Lit();

    printf("\t\t\ttrack id: %u\n", box.track_ID);
    printf("\t\t\tduration: %d\n",box.duration);
    printf("\t\t\tlayer: %d\n",box.layer);
    printf("\t\t\talternate group: %d\n", box.alternate_group);
    printf("\t\t\tvolume: 0x%x\n", box.volume);
        
    printf("\t\t\tmatrix:\n");
    for(int i = 0; i < 3; ++i){
        printf("\t\t\t");
        for(int j = 0; j < 3; ++j){
            printf(" %8u ", box.matrix[i*3+j]);
        }
        printf("\n");
    }

    printf("\t\t\twidth: %u\n",box.width);
    printf("\t\t\theight: [%u].[%u]\n",
           box.height & 0xffff0000 >> 16,
           box.height & 0xffff);
  
    printf("\n");
    
    return box;
}

/*container box
  (elst)
 */
mp4_edts_box Mp4HeaderParser::mp4ReadEdtsBox(FILE *f, int size)
{
    printf("\t\t+%s\n", "edts");
    
    return  mp4_edts_box();
}

/*final box
 */
mp4_mdhd_box Mp4HeaderParser::mp4ReadMdhdBox(int size)
{
    printf("\t\t\t+mdhd\n");
    mp4_mdhd_box box;

    VARIABLES;
        
    box.size              = size;
    box.type              = 0;
    box.version           = readUint8();
	mAudioData->readData(&box.flags, sizeof(box.flags));
    box.creation_time     = readUint32Lit();
    box.modification_time = readUint32Lit();
    box.timescale         = readUint32Lit();
    box.duration          = readUint32Lit();
    box.language          = readUint16Big();
    box.pre_defined       = readUint16Big();

    printf("\t\t\t\ttimescale: %u\n", box.timescale);
    printf("\t\t\t\tduration: %u\n", box.duration);
    printf("\t\t\t\tlanguage: %u\n", box.language);

    return box;
}

/*filnal box
 */
mp4_hdlr_box Mp4HeaderParser::mp4ReadHdlrBox(int size)
{
    printf("\t\t\t%s\n", "+hdlr");
    struct mp4_hdlr_box box;
   
    VARIABLES;
    
    box.size            = size;
    box.type            = 'h'|'d'<<8|'l'<<16|'r'<<24;
    box.version         = readUint8();
	mAudioData->readData(&box.flags, sizeof(box.flags));
    box.pre_defined     = readUint32Lit();
	mAudioData->readData(&box.handler_type, sizeof(box.handler_type));
    box.handler_type[4] = 0;
	mAudioData->readData(&box.reserved, sizeof(box.reserved));
	mAudioData->readData(&inner_size, 1);
    box.name            = new char[inner_size + 1];
    for(int i = 0; i < inner_size; ++i)
		mAudioData->readData(&box.name[i], sizeof(unsigned char));

    printf("\t\t\t\thandler type: %s\n", box.handler_type);
    printf("\t\t\t\tname: %s\n", box.name);  //I have no idea what
                                             //is this using for
    return box;
}

/*final box
 */
// mp4_vmhd_box * Mp4HeaderParser::mp4ReadVmhdBox(int size)
// {
//     printf("\t\t\t\t+%s\n", "vmhd");
// 
//     VARIABLES;
//     mp4_vmhd_box *box  = new mp4_vmhd_box;
//     
//     box->size          = size;
//     box->type          = 0;
//     box->version       = readUint8();
// 	mAudioData->readData(&(box->flags), sizeof(box->flags));
//     box->graphics_mode = readUint32Lit();
//       mAudioData->readData(&(box->opcolor), sizeof(box->opcolor));  
// 
//     printf("\t\t\t\t\tgraphics mode: %u\n", box->graphics_mode);
//     printf("\t\t\t\t\topcolor: %u,%u,%u,%u\n",
//            box->opcolor[0],  box->opcolor[1],
//            box->opcolor[2],  box->opcolor[3]);
// 
// 	return box;
// }

/*final box
 */
mp4_smhd_box *Mp4HeaderParser::mp4ReadSmhdBox(int size)
{
    printf("\t\t\t\t+%s\n", "smhd");
    mp4_smhd_box box;
    
    box.size             = size;
    box.type             = 's'|'m'<<8|'h'<<16|'d'<<24;
    box.version          = readUint8();
	mAudioData->readData(&box.flags, sizeof(box.flags));
    box.balance          = readUint16Big();
    box.reserved         = readUint16Big();        

    printf("\t\t\t\t\tbalance: %d,%d\n",
           box.balance & 0xff00 >> 8, box.balance & 0xff);
    mp4_smhd_box *newbox = new mp4_smhd_box;
    *newbox              = box;
    return newbox;
}

struct mp4_stts_box Mp4HeaderParser::mp4ReadSttsBox(int size)  //level 8
{
    printf("\t\t\t\t\t+%s\n", "stts");
    mp4_stts_box box;

    box.size                 = size;
    box.version              = readUint8();
	mAudioData->readData(box.flags, sizeof(box.flags));
    box.number_of_entries    = readUint32Lit();

    printf("\t\t\t\t\t\tnumber of entries: %u\n",box.number_of_entries);
     
    printf("\t\t\t\t\t\tentries:\n\t\t\t\t\t\t");
	box.time_to_sample_table = new mp4_sample_table[box.number_of_entries];

	int cur_pos = mAudioData->getCurrentIndex();

    for(int i = 0; i < box.number_of_entries; ++i){
        box.time_to_sample_table[i].sample_count = readUint32Lit();
		box.time_to_sample_table[i].sample_duration = readUint32Lit();
    }

	cur_pos = mAudioData->getCurrentIndex();
	
	mAudioData->setCurrentIndex(cur_pos - sizeof(int) * box.number_of_entries);
    
    printf("\n");

    return box;
}

/*final box*/
mp4_avcC_box *Mp4HeaderParser::mp4ReadAvccBox()
{
    struct mp4_avcC_box *ptr       = new mp4_avcC_box;
    
    ptr->size                      = readUint32Lit();
    printf("avcC_size:%u\n",ptr->size);
    
    ptr->type                      = readUint32Lit();
    if( ptr->type                  ==  ('a'<<24|'v'<<16|'c'<<8|'C')){
        printf("type:avcC\n");
    }
    
    ptr->configurationVersion      = readUint8();
    printf("configurationVersion:%u\n", ptr->configurationVersion);
    ptr->AVCProfileIndication      = readUint8();
    printf("AVCProfileIndication: %x\n", ptr->AVCProfileIndication);
    ptr->profile_compatibility     = readUint8();
    ptr->AVCLevelIndication        = readUint8();
    ptr->lengthSizeMinusOne        = 0x3 & readUint8();  //2 bit
    printf("lengthSizeMinusOne:%u\n", ptr->lengthSizeMinusOne);
    ptr->numOfSequenceParameterSet = 0x1F & readUint8();  //5bit
    printf("numOfSequenceParameterSet:%x\n", ptr->numOfSequenceParameterSet);
    ptr->sps                       = new mp4_avcC_box::SPS[ptr->numOfSequenceParameterSet];
    
    for( int i = 0; i < ptr->numOfSequenceParameterSet; ++i ){
        ptr->sps[i].sequenceParameterSetLength = readUint16Lit();
        printf("sequenceParameterSetLength: %u\n", ptr->sps[i].sequenceParameterSetLength);
        
        ptr->sps[i].sequenceParameterSetNALUnit =
            new uint8_t[ptr->sps[i].sequenceParameterSetLength];
		mAudioData->readData((ptr->sps[i].sequenceParameterSetNALUnit), (ptr->sps[i].sequenceParameterSetLength));
        
        for(int j = 0; j < ptr->sps[i].sequenceParameterSetLength; ++j){
            printf("%x", ptr->sps[i].sequenceParameterSetNALUnit[j]);
        }
        printf("============\n");
    }

    ptr->numOfPictureParameterSets = readUint8();
    printf("numOfPictureParameterSets:%u\n", ptr->numOfPictureParameterSets);
    ptr->pps                       = new mp4_avcC_box::PPS[ptr->numOfPictureParameterSets];
    for( int i = 0; i < ptr->numOfPictureParameterSets; ++i){
        ptr->pps[i].pictureParameterSetLength = readUint16Lit();
        printf("%d\n", ptr->pps[i].pictureParameterSetLength);
        
        ptr->pps[i].pictureParameterSetNALUnit = new uint8_t[ptr->pps[i].pictureParameterSetLength];
        mAudioData->readData(ptr->pps[i].pictureParameterSetNALUnit, ptr->pps[i].pictureParameterSetLength);
        
        for(int j = 0; j < ptr->pps[i].pictureParameterSetLength; ++j){
            printf("%x", ptr->pps[i].pictureParameterSetNALUnit[j]);
        }
        printf("============\n");
    }
    
    return ptr;
}

/*final box
  @a special contain a avcC box
 */
mp4_avc1_box* Mp4HeaderParser::mp4ReadAvc1Box(int size)  //level 9
{
    mp4_avc1_box *ptr = new mp4_avc1_box;
    
    ptr->size                   = size;
    ptr->type                   = (('a'<<24)|('v'<<16)|('c'<<8)|('1'));
    
	mAudioData->readData(ptr->reserved, sizeof(ptr->reserved));
    ptr->data_reference_index   = readUint16Lit();   //2
    ptr->pre_defined            = readUint16Big();   //2
    ptr->reserved1              = readUint16Big();   //2
	mAudioData->readData(ptr->pre_defined1, sizeof(ptr->pre_defined1));
	
    ptr->width                  = readUint16Lit();   //2
    ptr->height                 = readUint16Lit();   //2
    ptr->horiz_res              = readUint32Lit();   //4
    ptr->vert_res               = readUint32Lit();   //4
    ptr->reserved2              = readUint32Lit();   //4
    ptr->frames_count           = readUint16Lit();   //2
	mAudioData->readData(ptr->compressor_name, sizeof(ptr->compressor_name));
    ptr->bit_depth              = readUint8();        //1
    ptr->pre_defined2           = readUint16Big();   //2

    printf("-------------------------------------\n");
    printf("data_reference_index:%x\n", ptr->data_reference_index);
    printf("width:%u\n", ptr->width);
    printf("height:%u\n", ptr->height);
    printf("frames_count:%x\n", ptr->frames_count);
    printf("bit_depth:%u\n", ptr->bit_depth);
    printf("pre_defined2: %x\n", ptr->pre_defined2);
    
    ptr->avcC = mp4ReadAvccBox();
    
    printf("-------------------------------------\n");
    return ptr;
}

/*final box
 */
mp4_mp4a_box * Mp4HeaderParser::mp4ReadMp4aBox(int size)  //level 9
{
    printf("\t\t\t\t\t\t+mp4a\n");
    return 0;
}

/*container box
  (mp4a,avc1)
 */
mp4_stsd_box Mp4HeaderParser::mp4ReadStsdBox(int size)  //level 8
{
    printf("\t\t\t\t\t+%s\n", "stsd");
    mp4_stsd_box box;

    box.size               = size;
    box.version            = readUint8();
	mAudioData->readData(box.flags, sizeof(box.flags));
    box.number_of_entries  = readUint32Lit();

    printf("\t\t\t\t\t\tnumber of entries: %u\n", box.number_of_entries);
    
    box.sample_description = new Sample_description[box.number_of_entries];
    for(int i =0; i < box.number_of_entries; ++i){
        box.sample_description[i].size = readUint32Lit();
        box.sample_description[i].type = readUint32Lit();
        
        if( box.sample_description[i].type == (('m'<<24)|('p'<<16)|('4'<<8)|('a'))){
            box.sample_description[i].mp4a
                = mp4ReadMp4aBox(box.sample_description[i].size);
            box.sample_description[i].avc1 = 0;
            
        }else if(box.sample_description[i].type
                 == (('a'<<24)|('v'<<16)|('c'<<8)|('1'))){
            box.sample_description[i].avc1
                = mp4ReadAvc1Box(box.sample_description[i].size);
            box.sample_description[i].mp4a = 0;
        } 
    }

    return box;
}

/*final box
 */
mp4_stss_box Mp4HeaderParser::mp4ReadStssBox(int size)  //level 8
{
    printf("\t\t\t\t\t+%s\n", "stss");
    mp4_stss_box box;
    box.size              = size;
    box.version           = readUint8();
	mAudioData->readData(box.flags, sizeof(box.flags));
    box.number_of_entries = readUint32Lit();
    printf("\t\t\t\t\t\tnumber of entries: %u\n",box.number_of_entries);

    printf("\t\t\t\t\t\tentries:\n");
    box.sync_sample_table = new uint32_t[box.number_of_entries];
    for(int i =0; i < box.number_of_entries; ++i){
        box.sync_sample_table[i] = readUint32Lit();
        printf("%6u ", box.sync_sample_table[i]);
        if( (i+1)%12 == 0) printf("\n");
    }

    printf("\n");

    return box;
}

/*final box
 */
mp4_stsc_box Mp4HeaderParser::mp4ReadStscBox(int size)  //level 8
{
    printf("\t\t\t\t\t+%s\n", "stsc");

    mp4_stsc_box box;
    
    box.version    = readUint8();
	mAudioData->readData(&box.flags, sizeof(box.flags));
    box.map_amount = readUint32Lit();
   
    printf("\t\t\t\t\t\tmap-amount: %u\n", box.map_amount);
    
    box.scmap      = new mp4_list_t[box.map_amount];
    printf("\t\t\t\t\t\tfirst trunk:\tsamples-per-thunk:\tsample-description-ID\n");
    for(int i = 0; i < box.map_amount; ++i){
        box.scmap[i].first_chunk_num = readUint32Lit();
        box.scmap[i].sample_amount_in_cur_table = readUint32Lit();
        box.scmap[i].sample_description_id = readUint32Lit();
        
        printf("\t\t\t\t\t\t%13d", box.scmap[i].first_chunk_num);
        printf("\t%13d", box.scmap[i].sample_amount_in_cur_table);
        printf("\t%13d\n", box.scmap[i].sample_description_id);
    }
    return box;
}

/*final box
 */
mp4_stsz_box Mp4HeaderParser::mp4ReadStszBox(int size)  //level 8
{
    printf("\t\t\t\t\t+%s\n", "stsz");
    mp4_stsz_box box;

    box.size                 = size;
    box.version              = readUint8();
	mAudioData->readData(box.flags, sizeof(box.flags));
    box.samples_size_intotal = readUint32Lit();
    
    if(box.samples_size_intotal == 0){
        box.table_size = readUint32Lit();
        box.sample_size_table = new uint32_t[box.table_size];
        printf("\t\t\t\t\t\tall samples amount: %u\n", box.samples_size_intotal);
        printf("\t\t\t\t\t\tsample table size: %u\n", box.table_size);
        
        printf("\t\t\t\t\t\tsmple_size_table:\n");
        for(int i = 0; i < box.table_size; ++i){
            box.sample_size_table[i] = readUint32Lit();
            
            printf("%8u ", box.sample_size_table[i]);
            if((i+1) % 10 == 0) printf("\n");
        }
        printf("\n");
    }
    
    return box;
}

/*final box
 */
mp4_stco_box Mp4HeaderParser::mp4ReadStcoBox(int size)
{
    printf("\t\t\t\t\t+%s\n", "stco");
    mp4_stco_box box;

    box.version                      = readUint8();
	mAudioData->readData(box.flags, sizeof(box.flags));
    box.chunk_offset_amount          = readUint32Lit();

    printf("\t\t\t\t\t\tflags: 0x %x\n",
           box.flags[0]|box.flags[1]|box.flags[2]);
    printf("\t\t\t\t\t\tchunk offest amount: %u\n",
           box.chunk_offset_amount);

    printf("\t\t\t\t\t\tchunk offset:\n");
    box.chunk_offset_from_file_begin = new uint32_t[box.chunk_offset_amount];
    for(int i = 0 ; i < box.chunk_offset_amount; ++i){
        box.chunk_offset_from_file_begin[i] = readUint32Lit();
        printf("%d ", box.chunk_offset_from_file_begin[i]);
        if((i+1) % 10 == 0) printf("\n");
    }

    return box;
}


/*container box
  (stsd, stts, stsz|stz2, stsc, stco|co64, ctts, stss)
*/
mp4_stbl_box Mp4HeaderParser::mp4ReadStblBox(int size)
{
    printf("\t\t\t\t+stbl\n");
    
    mp4_stbl_box box;
    
    VARIABLES;
    int box_size          = 0;
    
	int cur_pos = mAudioData->getCurrentIndex();
	if (stblNeedToOffset(cur_pos)){
		cur_pos++;
	}

    do{
		mAudioData->setCurrentIndex(cur_pos);
        
        box_size          = readUint32Lit();
		mAudioData->readData(p, 4);
        p[4]              = 0;
        
        std::string name  = (char*)p;
        if(name == "stsd") {
            box.stsd      = mp4ReadStsdBox(box_size);
            //根据不同的编码方案和存储数据的文件数目，
            //每个media可以有一个到多个sample description。
            //sample-to-chunk atom通过这个索引表，
            //找到合适medai中每个sample的description。
            
        } else if(name == "stts"){
            box.stts      = mp4ReadSttsBox(box_size);
            //Time-to-sample atoms存储了media sample的duration 信息，
            //提供了时间对具体data sample的映射方法，通过这个atom，
            //你可以找到任何时间的sample，类型是'stts'。
            
        } else if(name == "stss"){
            box.stss      = mp4ReadStssBox(box_size);
            //sync sample atom确定media中的关键帧。
        } else if(name == "stsc"){
            box.stsc      = mp4ReadStscBox(box_size);       
        } else if(name == "stsz" || name == "stz2"){
            box.stsz      = mp4ReadStszBox(box_size);
        } else if(name == "stco" || name == "c064"){
            box.stco      = mp4ReadStcoBox(box_size);
            
        } else if(name == "ctts") {
        } else {    
            printf("\t\t\t\t\t+%s===============mark undifined\n", p);
        }
        
        cur_pos    += box_size;
        inner_size += box_size;
    } while(inner_size+8 != size);

    return box;
}

/*container box
  ()
 */
struct mp4_dinf_box Mp4HeaderParser::mp4ReadDinfBox(int size)
{
    printf("\t\t\t\t+dinf\n");
    struct mp4_dinf_box box;

    return box;
}

/*container box
  (vmhd, smhd, hmhd, nmhd)
*/
mp4_minf_box Mp4HeaderParser::mp4ReadMinfBox(int size)
{
    struct mp4_minf_box box;
    
    printf("\t\t\t+%s\n", "minf");

    VARIABLES;
    int level_5_box_size = 0;
	int cur_pos = mAudioData->getCurrentIndex();

    if (minfNeedToOffset(cur_pos)){
		cur_pos += 1;
	}


    do{
		mAudioData->setCurrentIndex(cur_pos);
        
        level_5_box_size = readUint32Lit();
		mAudioData->readData(p, 4);
        p[4] = 0;
        
        std::string name = (char*)p;
        if(name == "vmhd") {
            //box.vmhd = mp4ReadVmhdBox(level_5_box_size);        
        } else if(name == "dinf") {
            //box.dinf = mp4ReadDinfBox(level_5_box_size);
        } else if(name == "stbl") {
            box.stbl = mp4ReadStblBox(level_5_box_size);
        }else if(name == "smhd"){
            box.smhd = mp4ReadSmhdBox(level_5_box_size);  
        } else {
            printf("\t\t====size: %u\n", level_5_box_size);
            printf("\t\t====type: %s\n", p);
        }
        
        cur_pos    += level_5_box_size;
        inner_size += level_5_box_size;
    } while(inner_size+8 != size);
    
    printf("\n");
    return box;
}

/*container box
  (mdhd, hdlr, minf)
*/
mp4_mdia_box Mp4HeaderParser::mp4ReadMdiaBox(int size)
{
    printf("\t\t+%s\n", "mdia");
    
    mp4_mdia_box box;
    box.size              = size;
    box.type              = 'm' | 'd'<<8 | 'i'<<16 | 'a'<<24;
    VARIABLES;
    int box_size          = 0;
	int cur_pos = mAudioData->getCurrentIndex();
    do{
        mAudioData->setCurrentIndex(cur_pos);
        box_size          = readUint32Lit();
		mAudioData->readData(p, 4);
        p[4]              = 0;
        
        std::string name  = (char*)p;
        if(name == "mdhd") {
            box.mdhd      = mp4ReadMdhdBox(box_size);
        } else if(name == "hdlr") {
            box.hdlr      = mp4ReadHdlrBox(box_size);
        }else if(name == "minf"){
            box.minf      = mp4ReadMinfBox(box_size);            
        } else {
            printf("\t\t====type: %s\n", p);
        }
        
        cur_pos    += box_size;
        inner_size += box_size;
    } while(inner_size+8 != size);
    printf("\n");
    
    return box;
}

/*container box
 */
// struct  mp4_udta_box Mp4HeaderParser::mp4ReadUdtaBox(int size)
// {
//     printf("\t\t+%s\n", "udta");
// 
//     return mp4_udta_box();
// }


void Mp4HeaderParser::mp4ReadTrakBox(int size)
{
    printf("\t+%s\n", "trak");
    
    mp4_trak_box *trak  = new  mp4_trak_box;
    VARIABLES;
    int box_size               = 0;
	int cur_pos = mAudioData->getCurrentIndex();
    do{
		mAudioData->setCurrentIndex(cur_pos);
        box_size               = readUint32Lit();
		mAudioData->readData(p, 4);
        p[4]                   = 0;
        std::string name       = (char*)p;
        if(name == "tkhd") {
            trak->tkhd         = mp4ReadTkhdBox(box_size);
        }else if(name == "mdia"){
            trak->mdia         = mp4ReadMdiaBox(box_size);
        } else if(name == "udta"){
            //trak->udta         = mp4ReadUdtaBox(box_size);
        } else {
            printf("\t\t+%s===========\n", p);
        }
        
        cur_pos    += box_size;
        inner_size += box_size;
    } while(inner_size+8 != size);

    mMoovBox.trak.push_back(trak);
}

/*final box
 */
// void Mp4HeaderParser::mp4ReadIodsBox(int size)  //level 3
// {
//     printf("\t+iods-------------------------undefined\n\n");
// }

/*container box
  (mvhd, trak)
*/
void Mp4HeaderParser::mp4ReadMoovBox(int size)   //level 2
{
    printf("+%s\n",  "mMoovBox");  
    mMoovBox.size             = size;
    mMoovBox.type             = 'm'|'o'<<8|'o'<<16|'v'<<24;
    VARIABLES;
    int level_2_box_size  = 0;
    
	int cur_pos = mAudioData->getCurrentIndex();
    do{
		mAudioData->setCurrentIndex(cur_pos);
        
        level_2_box_size  = readUint32Lit();
       
		mAudioData->readData(&k, sizeof(k));
        indian_a.size     = k;
        memcpy(p, indian_a.byte, sizeof(indian_a.byte));
        p[4]              = 0;
        
        std::string name  = (char*)p;
        if(name == "mvhd"){
            mp4ReadMvhdBox(level_2_box_size);
        } else if(name == "trak") {
            mp4ReadTrakBox(level_2_box_size);
        } else if(name == "iods"){
            //mp4ReadIodsBox(level_2_box_size);    
        } else if(name == "udta"){
            //mp4ReadUdtaBox(level_2_box_size);
        } else {
            printf("====%s\n\n", p);
        }
        
        cur_pos    += level_2_box_size;
        inner_size += level_2_box_size;
    }while( inner_size+8 != size);
}

/*final box
 */
void Mp4HeaderParser::mp4ReadFtypBox(int size)  //level 2
{
    printf("+ftyp\n");
    
    size -= 8;
    char *ch = new char[size+1];
    for(int i = 0; i < size; ++i) {
		mAudioData->readData(&ch[i], sizeof(char));
    }
    ch[size] = 0;
    
    printf("\tftyp: %s\n",  ch);
    delete [] ch;
}

/*@a special final box
 */
void Mp4HeaderParser::mp4ReadMdatBox(int size)
{
    printf("\t+%s\n", "mdat");
    printf("\t\tthis is the real media data\n");
}

/*container box
  (ftyp, free, mdat, moov)
*/
int Mp4HeaderParser::mp4ReadRootBox() //level 1
{
    int k                 = 0;
    unsigned char p[5];
    
    int level_1_box_size  = readUint32Lit();
    
    //fread(&k, sizeof(k), 1, f);  //read byte
	mAudioData->readData(&k, sizeof(k));
    indian_a.size         = k;
    memcpy(p, indian_a.byte, sizeof(indian_a.byte));
    p[4]                  = 0;
    
    std::string name      = (char*)p;
    if(name == "moov"){
        mp4ReadMoovBox(level_1_box_size);
        //@a http://www.52rd.com/Blog/Detail_RD.Blog_wqyuwss_7900.html
        
    }else if(name == "ftyp"){
        mp4ReadFtypBox(level_1_box_size);
        
    } else if(level_1_box_size == 0){  //till the end of file
        return 1;
        
    }  else if(name == "mdat"){
        mp4ReadMdatBox(level_1_box_size);

    } else if(name == "free"){
        printf("+free\n");
        
    } else {
        //printf("==%u\n", level_1_box_size);
        printf("%s==mark undifined\n", p);
    }
    
    return level_1_box_size;
}


void Mp4HeaderParser::print_trak(const struct mp4_trak_box * trak)
{
    printf("trak\n");

    
    printf("\t+%s\n", "tkhd");
    printf("\t\ttrack id: %u\n", trak->tkhd.track_ID);
    printf("\t\tduration: %d\n", trak->tkhd.duration);
    printf("\t\t\tlayer: %d\n",trak->tkhd.layer);
    printf("\t\t\talternate group: %d\n", trak->tkhd.alternate_group);
    printf("\t\t\tvolume: 0x%x\n", trak->tkhd.volume);
    printf("\t\t\tmatrix:\n");
    for(int i = 0; i < 3; ++i){
        printf("\t\t\t");
        for(int j = 0; j < 3; ++j){
            printf(" %8u ", trak->tkhd.matrix[i*3+j]);
        }
        printf("\n");
    }
    printf("\t\t\twidth: %u\n",trak->tkhd.width);
    printf("\t\t\theight: [%u].[%u]\n",
           trak->tkhd.height & 0xffff0000 >> 16,
           trak->tkhd.height & 0xffff);


    
    printf("\t+%s\n", "mdia");
    
    printf("\t\t\t\t\t\t+stsd\n");
    printf("\t\t\t\t\t\t\tnumber of entries: %u\n",
           trak->mdia.minf.stbl.stsd.number_of_entries);
    

    printf("\t\t\t\t\t\t+stsc:sample to chunk table\n");
    printf("\t\t\t\t\t\tfirst trunk:\tsamples-per-thunk:\tsample-description-ID\n");
    for(int i = 0; i < trak->mdia.minf.stbl.stsc.map_amount; ++i){
        printf("\t\t\t\t\t\t%13d", trak->mdia.minf.stbl.stsc.scmap[i].first_chunk_num);
        printf("\t%13d", trak->mdia.minf.stbl.stsc.scmap[i].sample_amount_in_cur_table);
        printf("\t%13d\n", trak->mdia.minf.stbl.stsc.scmap[i].sample_description_id);
    }   printf("\n");

    printf("\t\t\t\t\t\t+%s\n", "stsz");
    printf("\t\t\t\t\t\t\tsmple_size_table:\n\t\t\t\t\t\t");
    for(int i = 0; i < trak->mdia.minf.stbl.stsz.table_size; ++i){
        printf("%8u ", trak->mdia.minf.stbl.stsz.sample_size_table[i]);
    }   printf("\n");

    printf("\t\t\t\t\t\t+stco\n");
    printf("\t\t\t\t\t\tchunk offset:\n\t\t\t\t\t");
    for(int i = 0 ; i < trak->mdia.minf.stbl.stco.chunk_offset_amount; ++i){
        printf("%8u ",trak->mdia.minf.stbl.stco.chunk_offset_from_file_begin[i]);
    }
}

/*
uint32_t Mp4HeaderParser::getSampleSize(const struct mp4_stsz_box &box,
                         const uint32_t chunk_index)  //[0, end)
{
    for(int i = 0; i < box.table_size; ++i) {
        if(chunk_index == i)
            return box.sample_size_table[i];
    }
}*/

uint32_t Mp4HeaderParser::getOffsetByTime(double seconds)
{
	mp4_trak_box *trakBox = mMoovBox.trak[0];
	if (trakBox == NULL)
		return -1;

	uint32_t timeScale = trakBox->mdia.mdhd.timescale;
	uint32_t innerTime = timeScale * seconds;
	uint32_t sampleDuration = 0;
	int destChunkIndex = 0;
	int sampleIndexInChunk = 0;
	int preSampleCount = 0;
	bool findPostion = false;

	// 每个sample的大小
	uint32_t sampleSize = trakBox->mdia.minf.stbl.stsz.samples_size_intotal;
	mp4_stts_box stts = trakBox->mdia.minf.stbl.stts;
	if (stts.number_of_entries > 0)
		sampleDuration = stts.time_to_sample_table[0].sample_duration;

	// sample对应的索引号从0开始的
	int sampleIndex = innerTime / sampleDuration;
	mp4_stsc_box stsc = trakBox->mdia.minf.stbl.stsc;

	for (int i = 0; i < stsc.map_amount; i++){
		int first = stsc.scmap[i].first_chunk_num;
		int samplePerChunk = stsc.scmap[i].sample_amount_in_cur_table;
		destChunkIndex =  (sampleIndex - preSampleCount) / samplePerChunk + (first - 1);
		sampleIndexInChunk = sampleIndex % samplePerChunk;

		if (i + 1 <= stsc.map_amount){
			if (destChunkIndex < stsc.scmap[i+1].first_chunk_num){
				findPostion = true;
				break;// find;
			}else{
				preSampleCount += (stsc.scmap[i+1].first_chunk_num -1) * samplePerChunk;
				continue;
			}
		} else {
			// 最后一个Chunk分组
		}
	}

	//UNILOGD("current time:%d, time scale:%d, sample size:%d, sample duration:%d", seconds, timeScale, sampleSize, sampleDuration);
	mp4_stco_box stco = trakBox->mdia.minf.stbl.stco;
	if (findPostion && destChunkIndex < stco.chunk_offset_amount)
		return stco.chunk_offset_from_file_begin[destChunkIndex] + sampleIndexInChunk * sampleSize;
	else
		return -1;
}

unsigned Mp4HeaderParser::readUint16Big()
{
	unsigned short k = 0;
	mAudioData->readData(&k, sizeof(k));
	return k;
};

uint16_t Mp4HeaderParser::readUint16Lit()
{
	uint16_t k;
	mAudioData->readData(&k, sizeof(k));
	return ((k&0xff00) >> 8)|((k&0xff) << 8);
}

unsigned char Mp4HeaderParser::readUint8()
{
	unsigned char x;
	mAudioData->readData(&x, sizeof(x));
	return x;
}

int Mp4HeaderParser::readUint32Lit(){
	int k = 0;
	mAudioData->readData(&k, sizeof(k));
	indian_a.size = k;
	for(int i = 0, j = 3; i < 4; i++, j--)
		indian_b.byte[i] = indian_a.byte[j];

	return indian_b.size;
}

bool Mp4HeaderParser::minfNeedToOffset(int curPos){
	mAudioData->setCurrentIndex(curPos + 4);
	char szHeader[5] = { 0 };
	mAudioData->readData(szHeader, 4);
	szHeader[4]= 0;
	std::string name = (char*)szHeader;

	return name != "smhd";
}

bool Mp4HeaderParser::stblNeedToOffset(int curPos){
	mAudioData->setCurrentIndex(curPos + 4);
	char szHeader[5] = { 0 };
	mAudioData->readData(szHeader, 4);
	szHeader[4]= 0;
	std::string name = (char*)szHeader;

	return name != "stsd";
}
