/*=====================================================================
模块名 ：单向队列
文件名 ：OneWayQueue.h
实现功能：单向的队列,一个任务pop,一个任务push,队列满时无法继续push。
作者 ：刘黎明
版权 ：<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
修改记录：
日 期           版本     修改人     走读人     修改记录
2019/9/26       V1.0     刘黎明     刘黎明     创建模块
=====================================================================*/

#ifndef ONEWAYQUEUE_H
#define ONEWAYQUEUE_H

#include <iostream>

template <typename T>
class COneWayQueue
{
public:
	COneWayQueue();
	virtual ~COneWayQueue();

    //队列初始化
    void Init(int nQueueLen);

    //push一个对象
    bool Push(T & tObject);

    //pop一个对象
    bool Pop(T & tObject);

private:
    //释放资源
    void Release();    
    
    //对列长度
    int m_nQueueLen;

    //数据指针数组
    T * m_apQueue;

    //队列头位置
    int m_nHead;

    //队列尾位置
    int m_nTail;
};

template <typename T>
COneWayQueue<T>::COneWayQueue()
{
    m_nQueueLen = 0;
    m_apQueue = NULL;
}

template <typename T>
COneWayQueue<T>::~COneWayQueue()
{
    Release();
}

template <typename T>
void COneWayQueue<T>::Init(int nQueueLen)
{
    if (nQueueLen > 0)
    {
        m_apQueue = new T[nQueueLen];
        m_nQueueLen = nQueueLen;

        m_nHead = 0;
        m_nTail = 0;
    }
}

template <typename T>
bool COneWayQueue<T>::Push(T & tObject)
{
    if (m_nQueueLen == 0)
    {
        return false;
    }

    int nTmp = m_nHead;
    nTmp ++;
    if (nTmp == m_nQueueLen)
    {
        nTmp = 0;
    }
    //留一个空挡,防止写满
    if (nTmp == m_nTail)
    {
        return false;
    }

    memcpy(&m_apQueue[m_nHead],&tObject,sizeof(T));
    m_nHead++;
    if (m_nHead == m_nQueueLen)
    {
        m_nHead = 0;
    }
    
    return true;
}

template <typename T>
bool COneWayQueue<T>::Pop(T & tObject)
{
    //队列未初始化
    if (m_nQueueLen == 0)
    {
        return false;
    }

    //队列空
    if (m_nHead == m_nTail)
    {
        return false;
    }

    memcpy(&tObject,&m_apQueue[m_nTail],sizeof(T));
    m_nTail++;
    if (m_nTail == m_nQueueLen)
    {
        m_nTail = 0;
    }
    
    return true;
}

template <typename T>
void COneWayQueue<T>::Release()
{
    if (m_apQueue != NULL)
    {
        delete []m_apQueue;
        m_apQueue = NULL;
        m_nQueueLen = 0;
    }
}

#endif // !defined ONEWAYQUEUE_H
