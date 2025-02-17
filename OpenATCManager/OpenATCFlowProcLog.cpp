/*====================================================================
ģ���� ����ͨ����־ģ��
�ļ��� ��OpenATCFaultLog.cpp
����ļ���OpenATCFaultLog.h
ʵ�ֹ��ܣ�ʵ�ֽ�ͨ����־��صĹ���
���� ������
��Ȩ ��<Copyright(C) 2019-2020 Suzhou Keda Technology Co., Ltd. All rights reserved.>
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/09/06      V1.0    ����                ����             ����ģ��
====================================================================*/
#include "OpenATCFlowProcLog.h"
#include "OpenATCLog.h"
#include <iostream>
#include <string>
#include "string.h"

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#endif

COpenATCFlowProcLog::COpenATCFlowProcLog()
{
	m_pFaultBuffer = new char[C_N_MAXFLOWTBUFFER_SIZE];
	m_nAscFlowFileSize = C_N_MAX_FLOW_FILE_SIZE;   //���25M
	FILE *pf = fopen(FLOW_FILE_LOCAL_PATH, "rb");
	if (pf)
	{
		memset(m_pFaultBuffer, 0x00, strlen((char*)m_pFaultBuffer));
		int nMyFaultBufferSize = fread(m_pFaultBuffer, 1, C_N_MAXFLOWTBUFFER_SIZE, pf);
		fclose(pf);
	}
}

COpenATCFlowProcLog::~COpenATCFlowProcLog()
{
	delete[] m_pFaultBuffer;
}


/*====================================================================
������ ��CreateFlowFile
����   ���ڱ��ش�����ͨ��json�ļ�
�㷨ʵ�֣�pdir �ļ���Ŀ¼
����˵����
����ֵ˵�� ����
------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/10/9       V1.0    ����                ����                 ��
====================================================================*/
void COpenATCFlowProcLog::CreateFlowFile()
{
	if (access(FLOW_FILE_LOCAL_PATH, 0) == 0)
	{
		time_t nowTime;
		nowTime = time(NULL);
		struct tm *pLocalTime;
		pLocalTime = localtime(&nowTime);
		int nHour = pLocalTime->tm_hour;
		int nMin = pLocalTime->tm_min;
		int nSec = pLocalTime->tm_sec;
		FILE *pf = fopen(FLOW_FILE_LOCAL_PATH, "rb");
		if (pf)
		{
			fseek(pf, 0, SEEK_END);   //���ļ�ָ���ƶ��ļ���β
			unsigned long size = ftell(pf);   //�����ǰ�ļ�ָ������ļ���ʼ���ֽ���
			fclose(pf);
			if (size > m_nAscFlowFileSize || size == 0)
			{
				cJSON* flowBody;
				flowBody = cJSON_CreateObject();
				if (flowBody)
				{
					cJSON* flowArrayInfo;
					flowArrayInfo = cJSON_CreateArray();
					if (flowArrayInfo)
					{
						cJSON_AddItemToObject(flowBody, "flowInfo", flowArrayInfo);
					}
					pf = fopen(FLOW_FILE_LOCAL_PATH, "wb");
					if (pf)
					{
						char* pFlowStr = cJSON_Print(flowBody);
						if (pFlowStr)
						{
                            memcpy(m_pFaultBuffer, pFlowStr, strlen((char*)pFlowStr));

							fwrite(pFlowStr, 1, strlen(pFlowStr), pf);//��ʼ���ļ�
							cJSON_Delete(flowBody);
							free(pFlowStr);
						}
						fclose(pf);
					}
				}
			}
		}
	}
	else if (access(FLOW_FILE_LOCAL_PATH, 0) != 0)
	{
		cJSON* flowBody = cJSON_CreateObject();
		if (flowBody)
		{
			cJSON* flowArrayInfo;
			flowArrayInfo = cJSON_CreateArray();
			if (flowArrayInfo)
			{
				cJSON_AddItemToObject(flowBody, "flowInfo", flowArrayInfo);
			}
			FILE *pfile = fopen(FLOW_FILE_LOCAL_PATH, "wb");
			if (pfile)
			{
				char* pFlowStr = cJSON_Print(flowBody);
				if (pFlowStr)
				{
                    memcpy(m_pFaultBuffer, pFlowStr, strlen((char*)pFlowStr));

					fwrite(pFlowStr, 1, strlen(pFlowStr), pfile);//��ʼ���ļ�
					cJSON_Delete(flowBody);
					free(pFlowStr);
				}
				fclose(pfile);
			}
		}
	}
}

/*====================================================================
������ ��SaveTrafficFlowInfo
����   ���洢������Ϣ������
�㷨ʵ�֣�pdir �ļ���Ŀ¼
����˵����
����ֵ˵�� ����
------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/10/9       V1.0    ����                ����                 ��
====================================================================*/
void COpenATCFlowProcLog::SaveTrafficFlowInfo(TStatisticVehDetData *pTStatisticVehDetData, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog)
{
	cJSON* detectorInfoArray[C_N_MAX_VEHICLEDETECTOR_COUNT] = { NULL };
	CreateFlowFile();
	time_t nowTime;
	nowTime = time(NULL);
	struct tm *pLocalTime;
	pLocalTime = localtime(&nowTime);

	if (pLocalTime->tm_hour == 0 && pLocalTime->tm_min == 0)
	{
		BackUpLogFile(FLOW_FILE_DISK_PATH, pRunStatus, pOpenATCLog);
	}

	cJSON *flowInfoBody = NULL;
	if (strlen(m_pFaultBuffer) > 0)
	{
		flowInfoBody = cJSON_Parse((char*)m_pFaultBuffer);
		if (flowInfoBody)
		{
			cJSON* flowArray = NULL;
			flowArray = cJSON_GetObjectItem(flowInfoBody, "flowInfo");
			if (flowArray)
			{
				cJSON* flowArrayInfo = cJSON_CreateObject();
				cJSON_AddItemToObject(flowArray, "", flowArrayInfo);
				//��ȡ��ǰ�洢ʱ��
				char bufTime[C_N_MAX_TIME_BUFF_SIZE];
				memset(bufTime, 0x00, sizeof(unsigned char) * C_N_MAX_TIME_BUFF_SIZE);
				strftime(bufTime, C_N_MAX_TIME_BUFF_SIZE, "%Y-%m-%d %H:%M:%S", pLocalTime);

				cJSON_AddStringToObject(flowArrayInfo, "time", bufTime);
				cJSON* flowArrayListInfo = cJSON_CreateArray();
				cJSON_AddItemToObject(flowArrayInfo, "detector", flowArrayListInfo);
				for (int i = 0; i < pTStatisticVehDetData->m_nDetNum; i++)
				{
					detectorInfoArray[i] = cJSON_CreateObject();
					cJSON_AddItemToArray(flowArrayListInfo, detectorInfoArray[i]);
					cJSON_AddNumberToObject(detectorInfoArray[i], "id", i + 1);
					cJSON_AddNumberToObject(detectorInfoArray[i], "largevehnum", pTStatisticVehDetData->m_atDetFlowInfo[i].m_nLargeVehNum);
					cJSON_AddNumberToObject(detectorInfoArray[i], "middleVehnum", pTStatisticVehDetData->m_atDetFlowInfo[i].m_nMiddleVehNum);
					cJSON_AddNumberToObject(detectorInfoArray[i], "smallvehnum", pTStatisticVehDetData->m_atDetFlowInfo[i].m_nSmallVehNum);
					cJSON_AddNumberToObject(detectorInfoArray[i], "totalVehtime", pTStatisticVehDetData->m_atDetFlowInfo[i].m_nTotalVehCounter * C_N_TIMER_MILLSECOND);
					cJSON_AddNumberToObject(detectorInfoArray[i], "occupyrate", pTStatisticVehDetData->m_atDetFlowInfo[i].m_chOccupyRate);
					cJSON_AddNumberToObject(detectorInfoArray[i], "greenusage", pTStatisticVehDetData->m_atDetFlowInfo[i].m_chGreenUsage);
				}
				FILE *pf = fopen(FLOW_FILE_LOCAL_PATH, "wb");
				if (pf)
				{
					m_pFaultBuffer = cJSON_Print(flowInfoBody);
					if (m_pFaultBuffer)
					{
						fwrite(m_pFaultBuffer, 1, strlen(m_pFaultBuffer), pf);//�����ļ�
					}
					fclose(pf);
				}
			}
			cJSON_Delete(flowInfoBody);
		}
	}
}

/*====================================================================
������ ��NewDirCreate
����   ���������ϴ����ļ���
�㷨ʵ�֣�pdir �ļ���Ŀ¼
����˵����
����ֵ˵�� ����
------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/10/9       V1.0    ����                ����                 ��
====================================================================*/
void COpenATCFlowProcLog::NewDirCreate(const char* pdir,int& nResult,COpenATCLog * pOpenATCLog)
{
#ifdef _WIN32
	int m = 0, n;
	std::string str1, str2;
	str1 = pdir;
	str2 = str1.substr(0, 2);   
	str1 = str1.substr(3, str1.size()); //  '//'��һ���ַ���ת���ַ�
	while (m >= 0)
	{
		m = str1.find('\\');    
		str2 += '\\' + str1.substr(0, m);
		n = access(str2.c_str(), 0); 
		if (n == -1)
		{
			//����Ŀ¼
		    nResult = _mkdir(str2.c_str());         
		}
		else if (n == 0)
		{
			nResult = 0;
		}
		str1 = str1.substr(m + 1, str1.size());
	}
#else
	 if (access(FLOW_FILE_DISK_PATH, R_OK) == -1)
	 {
		 char cmd[C_N_MAX_FILEPATH_SIZE] = {0};
		 
		 memset(cmd, 0, sizeof(cmd));
		 sprintf(cmd, "mkdir %s", FLOW_FILE_DISK_PATH);
		 pid_t status;    
		 status = system(cmd);
		 if (-1 != status && WIFEXITED(status) && 0 == WEXITSTATUS(status))
		 {
			 nResult = 0;
		 }
		 else
		 {
			 pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCFlowProcLog NewDirCreate create TrafficFlowLog folder failed!");
			 nResult = -1;
		 }
	 }
#endif
}

/*====================================================================
������ ��CreateLogFile
����   ����U�����ļ����а�����д����������
�㷨ʵ�֣�pdir �ļ���Ŀ¼
����˵����
����ֵ˵�� ����
------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/10/9       V1.0    ����                ����                 ��
====================================================================*/
void COpenATCFlowProcLog::BackUpLogFile(const char* pdir, COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog)
{		
	if (OPENATC_RTN_FAILED == MountUSBDevice(pRunStatus, pOpenATCLog))
	{
		pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "COpenATCFlowProcLog mount usb device failed!");
		return;
	}
	
	// ����Ƿ��г���ά��ʱ�޵������ļ�����ɾ����ʱ���ļ�
	FlowDataFilesMaintenance(pOpenATCLog);

	int nResult = 0;
	NewDirCreate(pdir, nResult, pOpenATCLog);
	if (nResult == 0)
	{
		time_t nowTime;
		nowTime = time(NULL) - 86400;
		struct tm *pLocalTime;
		pLocalTime = localtime(&nowTime);
		char bufTime[C_N_MAX_TIME_BUFF_SIZE];
		memset(bufTime, 0x00, C_N_MAX_TIME_BUFF_SIZE);
		char filePath[C_N_MAX_FILEPATH_SIZE] = { 0 };
		memset(filePath, 0x00, C_N_MAX_FILEPATH_SIZE);
		strftime(bufTime, C_N_MAX_TIME_BUFF_SIZE, "%Y-%m-%d %H-%M", pLocalTime);
#if _WIN32
		sprintf(filePath, "%s\\%s.json", pdir, bufTime);
#else
		sprintf(filePath, "%s/%s.json", pdir, bufTime);
#endif
		if (access(FLOW_FILE_LOCAL_PATH, 0) == 0)
		{
			// ��ȡ��ǰĿ¼�´洢����������
			FILE *pf = fopen(FLOW_FILE_LOCAL_PATH, "rb");
			if (pf)
			{
				memset(m_pFaultBuffer, 0x00, strlen(m_pFaultBuffer));
				int nMyflowInfoSize = fread(m_pFaultBuffer, 1, C_N_MAXFLOWTBUFFER_SIZE, pf);

				// ��յ�ǰĿ¼�´洢����������
				fclose(pf);

				// ����ǰĿ¼�´洢���������ݱ��ݵ�U����
				FILE *pfile = fopen(filePath, "wb");
				if (pfile)
				{
					fwrite(m_pFaultBuffer, 1, strlen(m_pFaultBuffer), pfile);
					fclose(pfile);

					pf = fopen(FLOW_FILE_LOCAL_PATH, "w");
					if (pf)
					{						
						fclose(pf);
						CreateFlowFile();
					}
				}
			}
		}
	}

	UnmountUSBDevice(pRunStatus, pOpenATCLog);
}

/*====================================================================
������ ��FlowDataFilesMaintenance
����   ��U���ϵ������ļ�ά��
�㷨ʵ�֣�
����˵����
����ֵ˵�� ����
------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2020/09/17      V1.0    �º���                                ��
====================================================================*/
void COpenATCFlowProcLog::FlowDataFilesMaintenance(COpenATCLog * pOpenATCLog)
{
	char cPath[256]		= {0};
	char cFilePath[256] = {0};
	int  nCurMonth		= 0;
	int  nCurYear		= 0;
	int  nCurDay		= 0;
	int  nFileMonth		= 0;
	int  nFileYear		= 0;
	int  nFileDay		= 0;

#ifdef _WIN32
	intptr_t handle;
	_finddata_t findData;

	// ����Ŀ¼�еĵ�һ���ļ�
	sprintf(cPath, "%s//*", FLOW_FILE_DISK_PATH);
	handle = _findfirst(cPath, &findData);    
	if (handle == -1)
	{
		pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "Failed to find first file!\n");
		return;
	}

	SYSTEMTIME		stLocal;
	GetLocalTime(&stLocal);
	nCurYear	= stLocal.wYear;
	nCurMonth	= stLocal.wMonth;
	nCurDay		= stLocal.wDay;

	if (nCurMonth > FLOW_DATA_FILE_RETENTION_TIME)
	{
		nCurMonth = nCurMonth - FLOW_DATA_FILE_RETENTION_TIME;
	}
	else
	{
		nCurYear = nCurYear - 1;
		nCurMonth = nCurMonth + 12 - FLOW_DATA_FILE_RETENTION_TIME;
	}
	do
	{
		if(strcmp(findData.name,".") != 0  &&  strcmp(findData.name,"..") != 0)  
		{
			nFileYear  = (findData.name[0]-'0')*1000 + (findData.name[1]-'0')*100 + (findData.name[2]-'0')*10 + (findData.name[3]-'0');
			nFileMonth = (findData.name[5]-'0')*10 + (findData.name[6]-'0');
			nFileDay   = (findData.name[8]-'0')*10 + (findData.name[9]-'0');
			if ((nFileYear < nCurYear)
				|| (nFileYear == nCurYear && nFileMonth < nCurMonth)
				|| (nFileYear == nCurYear && nFileMonth == nCurMonth && nFileDay < nCurDay))
			{
				// ɾ�������������ļ�
				sprintf(cFilePath, "%s//%s", FLOW_FILE_DISK_PATH, findData.name);
				pOpenATCLog->LogOneMessage(LEVEL_INFO, "%s, %d, cFilePath=%s,nFileYear=%d.nFileMonth=%d.nFileDay = %d.\n", findData.name, findData.size, cFilePath, nFileYear, nFileMonth, nFileDay);
				remove(cFilePath);
			}
		}
	} while (_findnext(handle, &findData) == 0);    // ����Ŀ¼�е���һ���ļ�

	_findclose(handle);    // �ر��������
#else
	long dwCurTime = time(NULL);
	struct tm tLocalTime = {0};
	localtime_r(&dwCurTime, &tLocalTime);
	nCurYear	= 1900 + tLocalTime.tm_year;
	nCurMonth	= 1 + tLocalTime.tm_mon;
	nCurDay		= tLocalTime.tm_mday;

	if (nCurMonth > FLOW_DATA_FILE_RETENTION_TIME)
	{
		nCurMonth = nCurMonth - FLOW_DATA_FILE_RETENTION_TIME;
	}
	else
	{
		nCurYear = nCurYear - 1;
		nCurMonth = nCurMonth + 12 - FLOW_DATA_FILE_RETENTION_TIME;
	}

	DIR * dirp = NULL; 
	struct dirent * pDirent= NULL;

	dirp = opendir(FLOW_FILE_DISK_PATH);
	if (NULL == dirp)
	{
		pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "opendir %s failed!",FLOW_FILE_DISK_PATH);
		return;
	}

	while (NULL != (pDirent = readdir(dirp)))
	{
		if(pDirent->d_type == DT_REG)
		{
			nFileYear  = (pDirent->d_name[0]-'0')*1000 + (pDirent->d_name[1]-'0')*100 + (pDirent->d_name[2]-'0')*10 + (pDirent->d_name[3]-'0');
			nFileMonth = (pDirent->d_name[5]-'0')*10 + (pDirent->d_name[6]-'0');
			nFileDay   = (pDirent->d_name[8]-'0')*10 + (pDirent->d_name[9]-'0');
			if ((nFileYear < nCurYear) 	|| (nFileYear == nCurYear && nFileMonth < nCurMonth)
				|| (nFileYear == nCurYear && nFileMonth == nCurMonth && nFileDay < nCurDay))
			{
				// ɾ�������������ļ�
				sprintf(cFilePath, "%s/%s", FLOW_FILE_DISK_PATH, pDirent->d_name);
				pOpenATCLog->LogOneMessage(LEVEL_INFO, "%s, %d, cFilePath=%s,nFileYear=%d.nFileMonth=%d.nFileDay = %d.\n", pDirent->d_name, pDirent->d_reclen, cFilePath, nFileYear, nFileMonth, nFileDay);
				remove(cFilePath);
			}
		}
	}

	closedir(dirp);
#endif
}

/*====================================================================
������ ��MountUSBDevice
����   ������U��
�㷨ʵ�֣�
����˵����
����ֵ˵�� ����
------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/10/9       V1.0    ����                ����                 ��
====================================================================*/
int COpenATCFlowProcLog::MountUSBDevice(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog)
{
#ifndef _WIN32
	if (pRunStatus->GetUSBMountFlag())
	{
		return OPENATC_RTN_OK;
	}

	FILE *pf = fopen("/dev/sda1", "rb");//δ����U��
	if (pf == NULL)
	{
		//chFailedReason = USB_NOT_FIND;
		pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "USB not find.");
		return OPENATC_RTN_FAILED;
	}
	fclose(pf);

	char cmd[128] = {0};

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "mount /dev/sda1 /mnt");
	pid_t status;    
	status = system(cmd);
	if (-1 != status && WIFEXITED(status) && 0 == WEXITSTATUS(status))
	{
		pRunStatus->SetUSBMountFlag(true);
	}
	else
	{
		pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "USB mount failed.");
		return OPENATC_RTN_FAILED;
	}
#endif
	return OPENATC_RTN_OK;
}

/*====================================================================
������ ��UnmountUSBDevice
����   ��ж��U��
�㷨ʵ�֣�
����˵����
����ֵ˵�� ����
------------------------------------------------------------------------------------------------------------------
�޸ļ�¼��
�� ��           �汾    �޸���             �߶���             �޸ļ�¼
2019/10/9       V1.0    ����                ����                 ��
====================================================================*/
int COpenATCFlowProcLog::UnmountUSBDevice(COpenATCRunStatus * pRunStatus, COpenATCLog * pOpenATCLog)
{
#ifndef _WIN32
	if (false == pRunStatus->GetUSBMountFlag())
	{
		return OPENATC_RTN_OK;
	}

	char cmd[128] = {0};
	
	memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "umount /dev/sda1");
    pid_t status;    
   	status = system(cmd);
    if (-1 != status && WIFEXITED(status) && 0 == WEXITSTATUS(status))
	{
		pRunStatus->SetUSBMountFlag(false);
	}
	else
    {
		pOpenATCLog->LogOneMessage(LEVEL_CRITICAL, "USB unmount failed.");
        return OPENATC_RTN_FAILED;
    }
#endif
	return OPENATC_RTN_OK;
}

