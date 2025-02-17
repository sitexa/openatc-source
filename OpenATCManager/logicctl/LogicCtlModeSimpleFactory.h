/*=====================================================================
模块名 ：控制方式创建工厂
文件名 ：LogicCtlModeSimpleFactory.h
相关文件：
实现功能：实现具体控制模式创建接口
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/25       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef LOGICCTLMODESIMPLEFACTORY_H
#define LOGICCTLMODESIMPLEFACTORY_H

class CLogicCtlMode;

/*=====================================================================
类名 ：CLogicCtlModeSimpleFactory
功能 ：控制方式类工厂。根据控制方式来创建具体的控制方式实现子类
主要接口：Create
备注 ：
-----------------------------------------------------------------------
修改记录：
日 期          版本     修改人     走读人       修改记录
2019/09/14     V1.0     刘黎明     刘黎明       创建类
=====================================================================*/
class CLogicCtlModeSimpleFactory  
{
public:
	CLogicCtlModeSimpleFactory();
	virtual ~CLogicCtlModeSimpleFactory();

    static CLogicCtlMode * Create(int nCtlMode);

};

#endif // ifndef LOGICCTLMODESIMPLEFACTORY_H
