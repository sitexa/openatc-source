/*=====================================================================
模块名 ：数据解析、打包方式创建类工厂
文件名 ：OpenATCPackUnpackSimpleFactory.cpp
相关文件：OpenATCPackUnpackSimpleFactory.h
          OpenATCPackUnpackBase.h
实现功能：根据控制方式创建具体数据解析、打包方式实现对象
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/14       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#include "OpenATCPackUnpackSimpleFactory.h"
#include "OpenATCPackUnpackBase.h"
#include "OpenATCDataPackUnpack.h"
#include "OpenATCITS300DataPackUnpack.h"
#include "OpenATCCameraDataPackUnpack.h"
#include "OpenATCGB20999DataPackUnpack.h"
#include <stdio.h>

COpenATCPackUnpackSimpleFactory::COpenATCPackUnpackSimpleFactory()
{

}

COpenATCPackUnpackSimpleFactory::~COpenATCPackUnpackSimpleFactory()
{

}

/*==================================================================== 
函数名 ：Create 
功能 ：返回具体控制方式对象指针
算法实现 ： 
参数说明 ：nCtlMode，控制方式 
返回值说明：nCtlMode表示的控制方式实现对象的指针
----------------------------------------------------------------------
修改记录 ： 
日 期          版本 修改人  走读人  修改记录 
2019/09/14     V1.0 刘黎明          创建函数 
====================================================================*/ 
COpenATCPackUnpackBase * COpenATCPackUnpackSimpleFactory::Create(int nCtlMode)
{
    COpenATCPackUnpackBase * pBase = NULL;
    if (nCtlMode == PACK_UNPACK_MODE_ITS300) 
    {
        pBase = new COpenATCITS300DataPackUnpack();
    }
	else if (nCtlMode == PACK_UNPACK_MODE_CAMERA) 
    {
        pBase = new COpenATCCameraDataPackUnpack();
    }
    else if (nCtlMode == PACK_UNPACK_MODE_GB20999)
    {
        pBase = new COpenATCGB20999DataPackUnpack();
    }
    else
    {
        pBase = new COpenATCDataPackUnpack();
    }
    
    return pBase; 
}
