/*=====================================================================
ģ���� ������������ģ��
�ļ��� ��OpenATCFlowProcManager.cpp
����ļ���OpenATCFlowProcManager.h
          OpenATCParameter.h
          OpenATCRunStatus.h
ʵ�ֹ��ܣ��������������ͳ��
���� ��������
��Ȩ ��<Copyright(c) 2019-2020 Suzhou Keda Technology Co.,Ltd. All right reserved.>
-----------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾     �޸���     �߶���     �޸ļ�¼
2019/9/14       V1.0     ������                ����ģ��
=====================================================================*/
#include "OpenATCFlowProcManager.h"
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <io.h> 
#else
#include <stdio.h> 
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdlib.h>
#endif

COpenATCFlowProcManager * COpenATCFlowProcManager::s_pData;
const int C_N_VEHINITPASSTIME = 3000;
COpenATCFlowProcManager::COpenATCFlowProcManager()
{
    m_nLastMin = 0;
	m_hThread = 0;
    m_dwThreadRet = 0;

    memset(&m_tStatisticFlowData,0,sizeof(m_tStatisticFlowData));
	memset(&m_tOldStatisticFlowData,0,sizeof(m_tOldStatisticFlowData));
}

COpenATCFlowProcManager::~COpenATCFlowProcManager()
{
}

/*==================================================================== 
������ ��getInstance 
���� �����ص�����COpenATCFlowProcManager��ʵ��ָ�� 
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵����������COpenATCFlowProcManager��ʵ��ָ��
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
COpenATCFlowProcManager * COpenATCFlowProcManager::getInstance()
{
    if (s_pData == NULL)
    {
        s_pData = new COpenATCFlowProcManager();
    }

    return s_pData;
}

/*==================================================================== 
������ ��Init 
���� �����ڶ���COpenATCFlowProcManager�ĳ�ʼ������ 
�㷨ʵ�� �� 
����˵�� �� pParameter������������ָ��
            pRunStatus������״̬��ָ��
			pOpenATCLog����־��ָ��
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCFlowProcManager::Init(COpenATCParameter * pParameter,COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog)
{
    if (pParameter != NULL)
    {
        m_pLogicCtlParam = pParameter;
    }

    if (pRunStatus != NULL)
    {
        m_pLogicCtlStatus = pRunStatus;
    }

	if (pOpenATCLog != NULL)
    {
        m_pOpenATCLog = pOpenATCLog;
    }

    memset(&m_tStatisticFlowData,0,sizeof(m_tStatisticFlowData));

    m_tStatisticFlowData.m_nCounter = m_pLogicCtlStatus->GetGlobalCounter();

    TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
    memset(atVehDetector, 0, sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
    m_pLogicCtlParam->GetVehicleDetectorTable(atVehDetector);

    int i = 0;
    for (i = 0;i < MAX_VEHICLEDETECTOR_COUNT;i++)
    {
        if (atVehDetector[i].m_byVehicleDetectorNumber != 0)
        {
            m_tStatisticFlowData.m_nDetNum += 1;
        }

		m_bGreenStartFlag[i] = false;
		m_nGreenStartCounter[i] = 0;
    }

	m_FlowDataQueue.Init(C_N_MAX_FAULTQUEUE_SIZE);

	//�����������ݴ����߳�
	bool bOK = true;
#ifdef _WIN32
	m_hThread = CreateThread(NULL, 0, (unsigned long(FLOWPROCMANAGER_CALLBACK *)(void *))&FlowDataThread, this, 0, NULL);
    if (NULL == m_hThread)
    {
        bOK = false;
    }
#else
    if (0 != pthread_create(&m_hThread, NULL, FlowDataThread, this))
    {
        bOK = false;
    }
#endif
}

/*==================================================================== 
������ ��Stop 
���� �����ڶ���COpenATCFlowProcManager���˳��ͷŲ��� 
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCFlowProcManager::Stop()
{
#ifdef _WIN32
    while (1)
    {
        if (0 == GetExitCodeThread(m_hThread, &m_dwThreadRet))
        {
            break;
        }
        else
        {
            if (m_dwThreadRet == STILL_ACTIVE)
            {
                OpenATCSleep(100);
                //continue;
            }
            CloseHandle(m_hThread);
            break;
        }
    }
#else
	pthread_detach(m_hThread);
	pthread_join(m_hThread, NULL);
#endif
}

/*==================================================================== 
������ ��Work
���� ����COpenATCFlowProcManager�������̺��� 
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCFlowProcManager::Work()
{
    if (m_pLogicCtlParam == NULL ||
        m_pLogicCtlStatus == NULL)
    {
        return;
    }

    ProcRTVehDetData();

    ProcSTVehDetData();    
}

/*==================================================================== 
������ ��ProcRTVehDetData
���� ��������巢�͵�ʵʱ�������ݣ����ɸ�Ӧ���� 
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCFlowProcManager::ProcRTVehDetData()
{
    TVehDetBoardData tVehDetData;
    m_pLogicCtlStatus->GetVehDetBoardData(tVehDetData);
    TRealTimeVehDetData tRTVehDetData;
    m_pLogicCtlStatus->GetRTVehDetData(tRTVehDetData);

	bool bGreenFlag = false;
	TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
	memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
	m_pLogicCtlParam->GetVehicleDetectorTable(atVehDetector);

	TChannel atChannelInfo[MAX_CHANNEL_COUNT];
	memset(atChannelInfo, 0, sizeof(atChannelInfo));
	m_pLogicCtlParam->GetChannelTable(atChannelInfo);

	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
	m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);
    
    bool bVehChg = false;
    for (int i = 0;i < C_N_MAXDETBOARD_NUM;i ++)
    {
        if (tVehDetData.m_atVehDetData[i].m_nVehDetBoardID == 0)
        {
            continue;
        }

        for (int j = 0;j < C_N_MAXDETINPUT_NUM;j ++)
        {
            int nIndex = i * C_N_MAXDETINPUT_NUM + j;

            if (tVehDetData.m_atVehDetData[i].m_bDetFaultStatus[j] != tRTVehDetData.m_bDetFaultStatus[nIndex])
            {
                tRTVehDetData.m_bDetFaultStatus[nIndex] = tVehDetData.m_atVehDetData[i].m_bDetFaultStatus[j];
                bVehChg = true;
            }

            //���¼����������Ϣ
            if (tRTVehDetData.m_bVehDetExist[nIndex] != tVehDetData.m_atVehDetData[i].m_bVehDetExist[j])
            {
                tRTVehDetData.m_bVehDetExist[nIndex] = tVehDetData.m_atVehDetData[i].m_bVehDetExist[j];
                bVehChg = true;
            }

			bGreenFlag = false;
			if (atVehDetector[nIndex].m_byVehicleDetectorNumber != 0)
			{
				for (int k = 0;k < MAX_CHANNEL_COUNT;k++)
				{
					if (atVehDetector[nIndex].m_byVehicleDetectorCallPhase == atChannelInfo[k].m_byChannelControlSource)
					{
						if (tLampClrStatus.m_achLampClr[k * C_N_CHANNEL_OUTPUTNUM + 2] == LAMP_CLR_ON ||
							tLampClrStatus.m_achLampClr[k * C_N_CHANNEL_OUTPUTNUM + 2] == LAMP_CLR_FLASH)//�̵ƻ�����
						{
							bGreenFlag = true;
						}
						break;
					}
				}
			}

            //�г�
            if (tVehDetData.m_atVehDetData[i].m_achVehChgVal[j] == 1)
            {
                if (tRTVehDetData.m_chDetStatus[nIndex] == 0)
                {
                    tRTVehDetData.m_bIsNewVehCome[nIndex] = true;
                    tRTVehDetData.m_chDetStatus[nIndex] = 1;
                    tRTVehDetData.m_anDetStatusCounter[nIndex] = tVehDetData.m_atVehDetData[i].m_anVehChgValCounter[j];
                    bVehChg = true;

                    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcRTVehDetData VehDetBoardIndex:%d DetectorIndex:%d Vehicle Comein", i, nIndex); 

					if (bGreenFlag)//�������̵�ʱ������Ȧ����ʼ����
					{
						if (tRTVehDetData.m_chDetStatusInGreen[nIndex] == 0)
						{
							int nVehicleIntervalCounterDiff = 0;
							if (tRTVehDetData.m_anDetStatusCounterInGreen[nIndex] > 0)
							{
								nVehicleIntervalCounterDiff = CalcCounter(tRTVehDetData.m_anDetStatusCounterInGreen[nIndex],
																		  tVehDetData.m_atVehDetData[i].m_anVehChgValCounter[j],
															              C_N_MAXGLOBALCOUNTER);
								//�����(ǰһ����β������һ����ͷ�ı�׼����)���ڱ�׼����࣬���ñ�׼����࣬�������ʵ�ʳ����
								if ((nVehicleIntervalCounterDiff * C_N_TIMER_MILLSECOND) > C_N_VEHINITPASSTIME)
								{
									nVehicleIntervalCounterDiff = C_N_VEHINITPASSTIME / C_N_TIMER_MILLSECOND;
								}
							}
							m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounterInGreen += nVehicleIntervalCounterDiff;//���ϳ����

							tRTVehDetData.m_chDetStatusInGreen[nIndex] = 1;
							tRTVehDetData.m_anDetStatusCounterInGreen[nIndex] = tVehDetData.m_atVehDetData[i].m_anVehChgValCounter[j];//����ʱ�����

							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcRTVehDetData VehDetBoardIndex:%d DetectorIndex:%d VehicleIntervalCounterDiff:%d Vehicle Comein In Green", i, nIndex, nVehicleIntervalCounterDiff); 
						}
					}
                }
				else
				{
					if (bGreenFlag)//���ڷ���ʱ������Ȧ���ڱ���ʱ�ٿ�ʼ����
					{
						if (tRTVehDetData.m_chDetStatusInGreen[nIndex] == 0)
						{
							tRTVehDetData.m_chDetStatusInGreen[nIndex] = 1;
							tRTVehDetData.m_anDetStatusCounterInGreen[nIndex] = m_pLogicCtlStatus->GetGlobalCounter();//�õ�ǰ��ʱ����Ϊ����ʱ�����

							bVehChg = true;

							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcRTVehDetData VehDetBoardIndex:%d DetectorIndex:%d Vehicle ReComein In Green", i, nIndex); 
						}
					}
					else//�������̵�ʱû���뿪��Ȧ������״̬ʱ����������ռ��ʱ�䣬����Ϊ�뿪�̵�״̬�����¸������ڱ���ʱ�ٿ�ʼ����
					{
						if (tRTVehDetData.m_chDetStatusInGreen[nIndex] == 1)
						{
							int nOldCounterDiff = CalcCounter(tRTVehDetData.m_anDetStatusCounterInGreen[nIndex],
															  m_pLogicCtlStatus->GetGlobalCounter(),
															  C_N_MAXGLOBALCOUNTER);
							m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounterInGreen += nOldCounterDiff;//���ϼ����ռ��ʱ��

							tRTVehDetData.m_chDetStatusInGreen[nIndex] = 0;
							tRTVehDetData.m_anDetStatusCounterInGreen[nIndex] = m_pLogicCtlStatus->GetGlobalCounter();//�õ�ǰ��ʱ����Ϊ�뿪ʱ�����

							bVehChg = true;

							m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Set VehDetBoardIndex:%d DetectorIndex:%d CounterDiff:%d TotalVehCounter:%d Non Green Vehicle Comeout", i, nIndex, nOldCounterDiff, m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounterInGreen); 
						}
					}
				}
            }
            else
            {
                if (tRTVehDetData.m_chDetStatus[nIndex] == 1)
                {
                    tRTVehDetData.m_chDetStatus[nIndex] = 0;
                    bVehChg = true;

                    int nOldCounterDiff = CalcCounter(tRTVehDetData.m_anDetStatusCounter[nIndex],
                                                                          tVehDetData.m_atVehDetData[i].m_anVehChgValCounter[j],
                                                                          C_N_MAXGLOBALCOUNTER);
                    tRTVehDetData.m_anDetStatusCounter[nIndex] = tVehDetData.m_atVehDetData[i].m_anVehChgValCounter[j];

                    //��Ҫ����������ռ�����ۼ�
                    int nType = GetVehType(nOldCounterDiff);

                    switch (nType)
                    {
                        case VEH_TYPE_SMALL:
                            m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nSmallVehNum += 1;
                            break;
                        case VEH_TYPE_MIDDLE:
                            m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nMiddleVehNum += 1;
                            break;
                        case VEH_TYPE_LARGE:
                            m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nLargeVehNum += 1;
                            break;
                    }

                    m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounter += nOldCounterDiff;

                    m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcRTVehDetData VehDetBoardIndex:%d DetectorIndex:%d CounterDiff:%d TotalVehCounter:%d Vehicle Comeout SmallVehNum:%d ", i, nIndex, nOldCounterDiff, m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounter, m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nSmallVehNum); 

					if (tRTVehDetData.m_chDetStatusInGreen[nIndex] == 1)//�����뿪��Ȧ�������̵Ƽ����ռ��ʱ��
					{
						nOldCounterDiff = CalcCounter(tRTVehDetData.m_anDetStatusCounterInGreen[nIndex],
													  tVehDetData.m_atVehDetData[i].m_anVehChgValCounter[j],
													  C_N_MAXGLOBALCOUNTER);
						m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounterInGreen += nOldCounterDiff;//���ϼ����ռ��ʱ��

						tRTVehDetData.m_chDetStatusInGreen[nIndex] = 0;
						tRTVehDetData.m_anDetStatusCounterInGreen[nIndex] = tVehDetData.m_atVehDetData[i].m_anVehChgValCounter[j];//�뿪ʱ�����

						m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "ProcRTVehDetData VehDetBoardIndex:%d DetectorIndex:%d CounterDiff:%d TotalVehCounter:%d Vehicle Comeout In Green", i, nIndex, nOldCounterDiff, m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounterInGreen); 
					}
                }
            }
        }    
    }

    if (bVehChg)
    {
        m_pLogicCtlStatus->SetRTVehDetData(tRTVehDetData);
    }

}

/*==================================================================== 
������ ��ProcSTVehDetData
���� ��������巢�͵�ʵʱ�������ݣ�����ͳ������ 
�㷨ʵ�� �� 
����˵�� �� 
����ֵ˵������
----------------------------------------------------------------------
�޸ļ�¼ �� 
�� ��          �汾 �޸���  �߶���  �޸ļ�¼ 
2019/09/14     V1.0 ������          �������� 
====================================================================*/ 
void COpenATCFlowProcManager::ProcSTVehDetData()
{
    char szInfo[256] = {0};
    int  i = 0, j = 0;
    int  nSmallVehNum = 0;
    int  nMiddleVehNum = 0;
    int  nLargeVehNum = 0;
    int  nOccupyCounter = 0; 

	bool bFlag = false;
	BYTE byChannelIndex = 0;
	TVehicleDetector atVehDetector[MAX_VEHICLEDETECTOR_COUNT];
	memset(atVehDetector,0,sizeof(TVehicleDetector) * MAX_VEHICLEDETECTOR_COUNT);
	m_pLogicCtlParam->GetVehicleDetectorTable(atVehDetector);

	TChannel atChannelInfo[MAX_CHANNEL_COUNT];
	memset(atChannelInfo, 0, sizeof(atChannelInfo));
	m_pLogicCtlParam->GetChannelTable(atChannelInfo);

	TLampClrStatus tLampClrStatus;
	memset(&tLampClrStatus,0,sizeof(TLampClrStatus)); 
	m_pLogicCtlStatus->GetLampClrStatus(tLampClrStatus);

    time_t nowTime;
	nowTime = time(NULL);
	struct tm *pLocalTime;
	pLocalTime = localtime(&nowTime);
    if (pLocalTime->tm_min % 5 == 0 && pLocalTime->tm_sec == 0 && m_nLastMin != pLocalTime->tm_min)
    {
        m_nLastMin = pLocalTime->tm_min;

        unsigned long nCurCounter = m_pLogicCtlStatus->GetGlobalCounter();
        unsigned long nCounterDiff = CalcCounter(m_tStatisticFlowData.m_nCounter,nCurCounter,C_N_MAXGLOBALCOUNTER);

		SetDetectorStatusCounter();

        for (i = 0;i < C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM;i++)
        {
            m_tStatisticFlowData.m_atDetFlowInfo[i].m_chOccupyRate = (unsigned char)(100 * m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalVehCounter / nCounterDiff);
			if (m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalVehCounter > 0)
			{
				//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "1111111111111  DetectorIndex:%d %d %d %d",  i, m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalVehCounter, nCounterDiff, m_tStatisticFlowData.m_atDetFlowInfo[i].m_chOccupyRate);
			}
			if (m_bGreenStartFlag[i])
			{
				m_bGreenStartFlag[i] = false;
				m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter += CalcCounter(m_nGreenStartCounter[i],m_pLogicCtlStatus->GetGlobalCounter(),C_N_MAXGLOBALCOUNTER);
				//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "4444444444444  DetectorIndex:%d  Total:%d", i, m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter);
			}
			if (m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter > 0)
			{
				m_tStatisticFlowData.m_atDetFlowInfo[i].m_chGreenUsage = (unsigned char)(100 * m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalVehCounterInGreen / m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter);
				//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "2222222222222 In Green DetectorIndex:%d %d  %d  %d", i, m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalVehCounterInGreen, m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter, m_tStatisticFlowData.m_atDetFlowInfo[i].m_chGreenUsage);
			}
        } 

		GetFlowDataQueue().Push(m_tStatisticFlowData);
		memcpy(&m_tOldStatisticFlowData, &m_tStatisticFlowData, sizeof(m_tStatisticFlowData));

        for (i = 0;i < C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM;i++)
        {
            m_tStatisticFlowData.m_atDetFlowInfo[i].m_nSmallVehNum = 0;
            m_tStatisticFlowData.m_atDetFlowInfo[i].m_nMiddleVehNum = 0;
            m_tStatisticFlowData.m_atDetFlowInfo[i].m_nLargeVehNum = 0;
            m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalVehCounter = 0;
			m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalVehCounterInGreen = 0;
			m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter = 0;

			m_bGreenStartFlag[i] = false;
			m_nGreenStartCounter[i] = 0;
        }   

        m_tStatisticFlowData.m_bSaveFlag = true;
        m_tStatisticFlowData.m_nCounter = nCurCounter;//m_pLogicCtlStatus->GetGlobalCounter();
    }
	else
	{
		for (i = 0;i < C_N_MAXDETBOARD_NUM * C_N_MAXDETINPUT_NUM;i++)
		{
			if (atVehDetector[i].m_byVehicleDetectorNumber == 0)
			{
				continue;
			}

			bFlag = false;
			for (j = 0;j < MAX_CHANNEL_COUNT;j++)
			{
				if (atVehDetector[i].m_byVehicleDetectorCallPhase == atChannelInfo[j].m_byChannelControlSource)
				{
					bFlag = true;
					byChannelIndex = j;
					break;
				}
			}

			if (bFlag)
			{
				if (tLampClrStatus.m_achLampClr[byChannelIndex * C_N_CHANNEL_OUTPUTNUM + 2] == LAMP_CLR_ON ||
					tLampClrStatus.m_achLampClr[byChannelIndex * C_N_CHANNEL_OUTPUTNUM + 2] == LAMP_CLR_FLASH)//�̵ƻ�����
				{
					if (!m_bGreenStartFlag[i])
					{
						m_bGreenStartFlag[i] = true;
						m_nGreenStartCounter[i] = m_pLogicCtlStatus->GetGlobalCounter();
						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "3333333333333  DetectorIndex:%d  Start:%d", i, m_nGreenStartCounter[i]);
					}
				}
				else
				{
					if (m_bGreenStartFlag[i])
					{
						m_bGreenStartFlag[i] = false;
						m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter += CalcCounter(m_nGreenStartCounter[i],m_pLogicCtlStatus->GetGlobalCounter(),C_N_MAXGLOBALCOUNTER);
						//m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "4444444444444  DetectorIndex:%d  Total:%d", i, m_tStatisticFlowData.m_atDetFlowInfo[i].m_nTotalGreenCounter);
					}


				}
			}
		}
	}
}

int COpenATCFlowProcManager::GetVehType(long nExistTimeMs)
{
    
    return VEH_TYPE_SMALL;
}

/****************************************************
��������GetStatisticVehDetData
���ܣ���ȡ��ǰ������ͳ������
�㷨ʵ��:
����˵�� ����
����ֵ˵������ǰ������ͳ������
--------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����Ƽ             ����Ƽ             ����
====================================================================*/
TStatisticVehDetData  &  COpenATCFlowProcManager::GetCurrentStatisticVehDetData()
{
    return m_tOldStatisticFlowData;
}


void* COpenATCFlowProcManager::FlowDataThread(void *pParam)
{
    COpenATCFlowProcManager *pThis = (COpenATCFlowProcManager *)pParam;
	
    int nRet = pThis->Run();
    return (void *)nRet;
}

int COpenATCFlowProcManager::Run()
{
#ifndef _WIN32
	prctl(15,"FlowProcManager",0,0,0);
#endif
	TStatisticVehDetData tFlowData;
	
	while (true)
    { 
		memset(&tFlowData, 0, sizeof(TStatisticVehDetData));
		GetFlowDataQueue().Pop(tFlowData);
		if (tFlowData.m_nDetNum > 0)
		{
			m_OpenATCFlowProcLog.SaveTrafficFlowInfo(&tFlowData, m_pLogicCtlStatus, m_pOpenATCLog);
		}

        OpenATCSleep(1000);
    }

	m_pOpenATCLog->LogOneMessage(LEVEL_INFO, "COpenATCFlowProcManager flow data manager thread exit!");

    return OPENATC_RTN_OK;
}

void COpenATCFlowProcManager::OpenATCSleep(long nMsec)
{
#ifdef _WIN32
    Sleep(nMsec);
#else
    usleep(nMsec*1000);
#endif
}

unsigned long COpenATCFlowProcManager::CalcCounter(unsigned long nStart,unsigned long nEnd,unsigned long nMax)
{
    if (nEnd >= nStart)
    {
        return (nEnd - nStart);
    }
    else
    {
        return (nEnd + nMax - nStart);
    }
}

void COpenATCFlowProcManager::SetDetectorStatusCounter()
{
	TVehDetBoardData tVehDetData;
    m_pLogicCtlStatus->GetVehDetBoardData(tVehDetData);
    TRealTimeVehDetData tRTVehDetData;
    m_pLogicCtlStatus->GetRTVehDetData(tRTVehDetData);

	unsigned long nCurCounter = m_pLogicCtlStatus->GetGlobalCounter();

	int  i = 0, j = 0, nIndex = 0, nOldCounterDiff = 0;
    for (i = 0;i < C_N_MAXDETBOARD_NUM;i ++)
    {
        if (tVehDetData.m_atVehDetData[i].m_nVehDetBoardID == 0)
        {
            continue;
        }

        for (j = 0;j < C_N_MAXDETINPUT_NUM;j ++)
        {
            nIndex = i * C_N_MAXDETINPUT_NUM + j;

			if (tRTVehDetData.m_chDetStatus[nIndex] == 1)//��Խ����ͳ�����ڣ���ͳ��ʱ��������ռ��ʱ��
			{
				nOldCounterDiff = CalcCounter(tRTVehDetData.m_anDetStatusCounter[nIndex],
											  nCurCounter,
											  C_N_MAXGLOBALCOUNTER);
				m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounter += nOldCounterDiff;//���ϼ����ռ��ʱ��

				tRTVehDetData.m_anDetStatusCounter[nIndex] = nCurCounter;//�õ�ǰ��ʱ����Ϊ����ʱ�����

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetDetectorStatusCounter VehDetBoardIndex:%d DetectorIndex:%d CounterDiff:%d TotalVehCounter:%d Vehicle Comeout", i, nIndex, nOldCounterDiff, m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounter); 
			}

			if (tRTVehDetData.m_chDetStatusInGreen[nIndex] == 1)//�̵ƿ�Խ����ͳ�����ڣ���ͳ��ʱ��������ռ��ʱ��
			{
				nOldCounterDiff = CalcCounter(tRTVehDetData.m_anDetStatusCounterInGreen[nIndex],
											  nCurCounter,
											  C_N_MAXGLOBALCOUNTER);
				m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounterInGreen += nOldCounterDiff;//���ϼ����ռ��ʱ��

				tRTVehDetData.m_anDetStatusCounterInGreen[nIndex] = nCurCounter;//�õ�ǰ��ʱ����Ϊ����ʱ�����

				m_pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "SetDetectorStatusCounter VehDetBoardIndex:%d DetectorIndex:%d CounterDiff:%d TotalVehCounterInGreen:%d Vehicle Comeout", i, nIndex, nOldCounterDiff, m_tStatisticFlowData.m_atDetFlowInfo[nIndex].m_nTotalVehCounterInGreen); 
			}
        }
	}

	m_pLogicCtlStatus->SetRTVehDetData(tRTVehDetData);
}