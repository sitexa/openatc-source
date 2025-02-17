/*=====================================================================
ģ���� ��ͨ�ſ��ƹ���ģ��
�ļ��� ��OpenATCComCtlManager.h
����ļ���
ʵ�ֹ��ܣ�ͨ�ſ���ģ����������࣬���������ù��߼�ƽ̨��Ϣ������
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/26       V1.0     ������     �� ��      ����ģ��
=====================================================================*/

#ifndef OPENATCCOMCTLMANAGER_H
#define OPENATCCOMCTLMANAGER_H

#include "../Include/OpenATCParameter.h"
#include "../Include/OpenATCRunStatus.h"
#include "../Include/OpenATCRunStatusDefine.h"
#include "../Include/OpenATCLog.h"

#include "comctl/OpenATCCommWithCenterThread.h"
#include "comctl/OpenATCCommWithCfgSWThread.h"
#include "comctl/OpenATCCommWithITS300Thread.h"
#include "comctl/OpenATCCommWithCameraListenThread.h"
#include "comctl/OpenATCCommWithGB20999Thread.h"

/*=====================================================================
���� ��COpenATCComCtlManager
���� ��ͨ�ſ���ģ����������࣬���������ù��߼�ƽ̨��Ϣ������
��Ҫ�ӿڣ�
��ע ��
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��          �汾     �޸���     �߶���       �޸ļ�¼
2019/09/14     V1.0     ������     ����         ������
=====================================================================*/
class COpenATCComCtlManager  
{
public:
    //�ඨ��Ϊ����
    static COpenATCComCtlManager * getInstance();

    //��ĳ�ʼ������
    void Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog, const char * pOpenATCVersion);

    //���ֹͣ���ͷ�
    void Stop();

private:
	COpenATCComCtlManager();
	~COpenATCComCtlManager();

private:
    static COpenATCComCtlManager * s_pData;				//������ָ��

    COpenATCParameter * m_pLogicCtlParam;               //����������ָ��

    COpenATCRunStatus * m_pLogicCtlStatus;              //����״̬��ָ��

	COpenATCLog       * m_pOpenATCLog;                  //��־��ָ��

public:
	COpenATCCommWithCfgSWThread   * m_openATCCommWithCfgSWThread;
	COpenATCCommWithCenterThread  * m_openATCCommWithCenterThread;
	COpenATCCommWithITS300Thread  * m_openATCCommWithITS300Thread;
	COpenATCCommWithCameraListenThread  * m_openATCCommWithCameraListenThread;
	COpenATCCommWithCameraListenThread  * m_openATCCommWithCfgListenThread;
	COpenATCCommWithCenterThread  * m_openATCCommWithSimulateThread;
    COpenATCCommWithITS300Thread  * m_openATCCommWithDetectorThread;
    COpenATCCommWithGB20999Thread* m_openATCCommWithGB20999Thread;
};

#endif // !ifndef OPENATCCOMCTLMANAGER_H


/*=====================================================================

�ó�����һ��C++ͷ�ļ���������һ��ͨ�ſ��ƹ���ģ����� `COpenATCComCtlManager`�������ǶԸó���ķ�����

### 1. ģ�����
- **ģ����**��ͨ�ſ��ƹ���ģ��
- **����**����ģ����Ҫ���������ù��߼�ƽ̨��Ϣ���н���������������ȡ�

### 2. ��Ҫ����
- **������ͷ�ļ�**�����������һЩ������ͷ�ļ�����Щͷ�ļ������˲���������״̬����־����ع��ܡ�
- **�ඨ��**��`COpenATCComCtlManager` ���Ǹ�ģ��ĺ��ģ����õ���ģʽ��Singleton Pattern����ȷ��������������ֻ��һ��ʵ����

### 3. ��ĳ�Ա
- **���г�Ա����**��
  - `static COpenATCComCtlManager * getInstance();`����ȡ����ʵ����
  - `void Init(...)`����ʼ����Ĳ��������ܶ������������״̬����־��
  - `void Stop();`��ֹͣ���ͷ������Դ��

- **˽�г�Ա**��
  - ���캯��������������ȷ���ⲿ�޷�ֱ�Ӵ������������ʵ����
  - һЩָ���Ա����������ָ����������������״̬����־���ʵ����

- **���г�Ա����**������߳����ָ�룬�����벻ͬ��ϵͳ�������ͨ�š�

### 4. ���ģʽ
- **����ģʽ**��ͨ�� `getInstance()` ����ʵ�֣�ȷ�����Ψһ�ԡ�

### 5. ������
- ����ṹ������ע����ϸ��������⡣
- ʹ����������ı����ͺ������ƣ����ϴ���ɶ���ԭ��

### 6. ���ܵĸĽ�
- **������**���ڳ�ʼ����ֹͣ�����У�������Ҫ��Ӵ�������ƣ���ȷ������Ľ�׳�ԡ�
- **�ĵ�ע��**�����Կ����ں���ʵ������Ӹ��������ע�ͣ��Ա��ں���ά����

������˵���ó���ṹ����������ȷ���ʺ�����ͨ�ſ��ƹ���ĳ�����
=====================================================================*/