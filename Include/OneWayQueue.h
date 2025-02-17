/*=====================================================================
ģ���� ���������
�ļ��� ��OneWayQueue.h
ʵ�ֹ��ܣ�����Ķ���,һ������pop,һ������push,������ʱ�޷�����push��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     ������     ����ģ��
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

    //���г�ʼ��
    void Init(int nQueueLen);

    //pushһ������
    bool Push(T & tObject);

    //popһ������
    bool Pop(T & tObject);

private:
    //�ͷ���Դ
    void Release();    
    
    //���г���
    int m_nQueueLen;

    //����ָ������
    T * m_apQueue;

    //����ͷλ��
    int m_nHead;

    //����βλ��
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
    //��һ���յ�,��ֹд��
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
    //����δ��ʼ��
    if (m_nQueueLen == 0)
    {
        return false;
    }

    //���п�
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
