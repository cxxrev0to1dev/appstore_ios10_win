#include "win_itunes/provide_payment_address.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "win_itunes/strings.h"
//by:panming

using namespace std;

namespace win_itunes{
	static std::vector<_CHINA_ADDRESS> m_vecAddress;

	GenerateChinaAddress::GenerateChinaAddress(const std::wstring& address_file) :address_file_(address_file)
	{
		m_nStateIndex = 0;
		m_sPostCode = "200000";
	}


	GenerateChinaAddress::~GenerateChinaAddress(void)
	{
	}

	bool GenerateChinaAddress::ReadAddressFile()
	{
		bool bRet = false;
		std::wstring sAddressFile = address_file_;

		if (m_vecAddress.size() <= 0)
		{
			// 读取文件
			char szLineText[100] = { NULL };
			vector<string> vecLine;

			std::ifstream fin(sAddressFile.c_str(), std::ios::in);

			if (fin.good())
			{
				while (!fin.eof())
				{
					std::string sLineText;

					if (std::getline(fin, sLineText))
					{
						vecLine = Strings::SplitArray(sLineText.c_str(), " ");
						if (vecLine.size() >= 5)
						{
							_CHINA_ADDRESS objAddress;
							objAddress.sState = vecLine[0];
							objAddress.sCity = vecLine[1];
							objAddress.sStreet = vecLine[4];
							objAddress.sFullAddress = sLineText;

							m_vecAddress.push_back(objAddress);

							bRet = true;
						}
					}
				}

				fin.clear();
				fin.close();
			}
		}
		else
		{
			bRet = true;
		}

		return bRet;
	}

	bool GenerateChinaAddress::GenerateAddress()
	{
		bool bRet = false;

		char* szState[] = {
			"上海", "云南", "内蒙古", "北京", "吉林", "四川", "天津", "宁夏", "安徽", "山东", "山西", "广东", "广西", "新疆", "江苏", "江西", "河北",
			"河南", "浙江", "海南", "湖北", "湖南", "甘肃", "福建", "西藏", "贵州", "辽宁", "重庆", "陕西", "青海", "黑龙江"
		};

		// 邮编
		char* arrPostCode[] = {
			"200000", "650000", "010000", "100000", "130000", "610000", "300000", "750000", "230000", "250000", "030000", "510000", "530000", "830000", "210000", "330000", "050000",
			"450000", "310000", "570100", "430000", "410000", "730000", "350000", "850000", "550000", "110000", "400000", "710000", "810000", "150000"
		};

		int nStateIndex = 0; //rand() % (sizeof(szState)/sizeof(char*));

		if (ReadAddressFile())
		{
			// 随机选一个地址
			srand((unsigned int)time(nullptr));
			int nRand = rand() % m_vecAddress.size();

			_CHINA_ADDRESS objAddress = m_vecAddress[nRand];
			m_sCity = objAddress.sCity;
			m_sStreet1 = objAddress.sStreet;

			for (int i = 0; i < (sizeof(szState) / sizeof(char*)); i++)
			{
				// 查找省下标
				if (objAddress.sFullAddress.find(szState[i]) != string::npos)
				{
					m_nStateIndex = i;
					m_sPostCode = arrPostCode[i];

					break;
				}
			}

			bRet = true;
		}

		return bRet;
	}
}