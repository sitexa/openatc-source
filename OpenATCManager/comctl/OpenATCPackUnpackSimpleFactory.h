/*=====================================================================
模块名 ：数据解析、打包方式创建类工厂
文件名 ：OpenATCPackUnpackSimpleFactory.h
相关文件：
实现功能：实现具体数据解析、打包方式创建接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/25       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLMODESIMPLEFACTORY_H
#define LOGICCTLMODESIMPLEFACTORY_H

class COpenATCPackUnpackBase;

/*=====================================================================
类名 ：COpenATCPackUnpackSimpleFactory
功能 ：数据解析、打包类工厂。根据数据解析、打包来创建具体的数据解析、打包实现子类
主要接口：Create
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     刘黎明       创建类
=====================================================================*/
class COpenATCPackUnpackSimpleFactory  
{
public:
	COpenATCPackUnpackSimpleFactory();
	virtual ~COpenATCPackUnpackSimpleFactory();

    static COpenATCPackUnpackBase * Create(int nCtlMode);

};

#endif // ifndef LOGICCTLMODESIMPLEFACTORY_H
