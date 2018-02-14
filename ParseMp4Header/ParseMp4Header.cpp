// ParseMp4Header.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Mp4HeaderParser.h"

int _tmain(int argc, _TCHAR* argv[]){
	Mp4HeaderParser mp;
	mp.init();
	mp.parser();
	
    return 0;
}





