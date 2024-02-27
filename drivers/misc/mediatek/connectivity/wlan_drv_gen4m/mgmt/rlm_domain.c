/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/rlm_domain.c#2
 */

/*! \file   "rlm_domain.c"
 *    \brief
 *
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "rlm_txpwr_init.h"
extern int g_productName;
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
/* Tx Power Control Labels */
char *g_ENUM_TX_POWER_CTRL_TYPE_LABEL[] = {
	"PWR_CTRL_TYPE_DOMAIN",
	"PWR_CTRL_TYPE_BANDEDGE_2G",
	"PWR_CTRL_TYPE_BANDEDGE_5G",
	"PWR_CTRL_TYPE_FCC_WIFION",
	"PWR_CTRL_TYPE_ENABLE_FCC_IOCTL",
	"PWR_CTRL_TYPE_DISABLE_FCC_IOCTL",
	"PWR_CTRL_TYPE_ENABLE_SAR_IOCTL",
	"PWR_CTRL_TYPE_DISABLE_SAR_IOCTL",
	"PWR_CTRL_TYPE_ENABLE_TXPWR_SCENARIO",
	"PWR_CTRL_TYPE_DISABLE_TXPWR_SCENARIO",
	"PWR_CTRL_TYPE_ENABLE_3STEPS_BACKOFF",
	"PWR_CTRL_TYPE_DISABLE_3STEPS_BACKOFF",
	"PWR_CTRL_TYPE_NUM"
};

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
/* Tx Power Control Setting for Bandedge */
uint8_t g_aucBandEdge5G[2] = {48, 138};

uint16_t g_pwrLimitSize = 0;

#if (CFG_SUPPORT_FCC_POWER_BACK_OFF || CFG_SUPPORT_FCC_DYNAMIC_TX_PWR_ADJUST)
/* Tx Power Control Setting for FCC */
struct FCC_TX_PWR_ADJUST g_rFccTxPwrAdjust = {
	1,  /* 1:enable; 0:disable */
	14, /* Offset_CCK: drop 7dB */
	16, /* Offset_HT20: drop 8dB */
	14, /* Offset_HT40: drop 7dB */
	{12, 13}, /* Channel_CCK[0]: start channel */
		  /* Channel_CCK[1]: end channel */
	{12, 13}, /* Channel_HT20[0]: start channel */
		  /* Channel_HT20[1]: end channel */
	{8, 9} /* Channel_HT40[0]: start channel,
		*     primiary channel 12, HT40, center channel (10) -2
		* Channel_HT40[1]: end channel, primiary channel 12,
		*     HT40,  center channel (11) -2
		*/
};
#endif /* (CFG_SUPPORT_FCC_POWER_BACK_OFF ||
	*  CFG_SUPPORT_FCC_DYNAMIC_TX_PWR_ADJUST)
	*/

#if CFG_SUPPORT_TX_POWER_BACK_OFF
/* Tx Power Control Setting for SAR */
uint8_t g_bTxPowerLimitEnable2G = 1;
uint8_t g_cTxBackOffMaxPower2G = 10;
uint8_t g_bTxPowerLimitEnable5G = 1;
uint8_t g_cTxBackOffMaxPower5G = 10;

/* TxPwrBackOffParam's 0th byte contains enable/disable TxPowerBackOff for 2G */
/* TxPwrBackOffParam's 1st byte contains default TxPowerBackOff value for 2G */
/* TxPwrBackOffParam's 2nd byte contains enable/disable TxPowerBackOff for 5G */
/* TxPwrBackOffParam's 3rd byte contains default TxPowerBackOff value for 5G */
uint32_t g_TxPwrBackOffParam;

/* set tx power scenario */
int32_t g_iTxPwrScenarioIdx = -1; /* -1:reset, >=0:scenario index */
bool g_bTxPwrScenarioEnable2G = TRUE;
uint8_t g_acTxPwrScenarioMaxPower2G[5] = { 10, 12, 14, 16, 18 };
bool g_bTxPwrScenarioEnable5G = TRUE;
uint8_t g_acTxPwrScenarioMaxPower5G[5] = { 20, 22, 24, 26, 28 };

/* 3 steps Wi-Fi power back-off */
int32_t g_i3StepsBackOffIdx = -1; /* 0:reset, 1:-2db, 2:-4db, 3:-6db */
int8_t g_ac3StepsPoewrOffset[] = { -2, -4, -6 };

#endif /* CFG_SUPPORT_TX_POWER_BACK_OFF */

/* The following country or domain shall be set from host driver.
 * And host driver should pass specified DOMAIN_INFO_ENTRY to MT6620 as
 * the channel list of being a STA to do scanning/searching AP or being an
 * AP to choose an adequate channel if auto-channel is set.
 */

/* Define mapping tables between country code and its channel set
 */
static const uint16_t g_u2CountryGroup0[] = { COUNTRY_CODE_JP };

static const uint16_t g_u2CountryGroup1[] = {
	COUNTRY_CODE_AS, COUNTRY_CODE_AI, COUNTRY_CODE_BM, COUNTRY_CODE_KY,
	COUNTRY_CODE_GU, COUNTRY_CODE_FM, COUNTRY_CODE_PR, COUNTRY_CODE_VI,
	COUNTRY_CODE_AZ, COUNTRY_CODE_BW, COUNTRY_CODE_KH, COUNTRY_CODE_CX,
	COUNTRY_CODE_CO, COUNTRY_CODE_CR, COUNTRY_CODE_GD, COUNTRY_CODE_GT,
	COUNTRY_CODE_KI, COUNTRY_CODE_LB, COUNTRY_CODE_LR, COUNTRY_CODE_MN,
	COUNTRY_CODE_AN, COUNTRY_CODE_NI, COUNTRY_CODE_PW, COUNTRY_CODE_WS,
	COUNTRY_CODE_LK, COUNTRY_CODE_TT, COUNTRY_CODE_MM
};

static const uint16_t g_u2CountryGroup2[] = {
	COUNTRY_CODE_AW, COUNTRY_CODE_LA, COUNTRY_CODE_AE, COUNTRY_CODE_UG
};

static const uint16_t g_u2CountryGroup3[] = {
	COUNTRY_CODE_AR, COUNTRY_CODE_BR, COUNTRY_CODE_HK, COUNTRY_CODE_OM,
	COUNTRY_CODE_PH, COUNTRY_CODE_SA, COUNTRY_CODE_SG, COUNTRY_CODE_ZA,
	COUNTRY_CODE_VN, COUNTRY_CODE_BD, COUNTRY_CODE_DO, COUNTRY_CODE_FK,
	COUNTRY_CODE_KZ, COUNTRY_CODE_MZ, COUNTRY_CODE_NA, COUNTRY_CODE_LC,
	COUNTRY_CODE_VC, COUNTRY_CODE_UA, COUNTRY_CODE_UZ, COUNTRY_CODE_ZW,
	COUNTRY_CODE_MP, COUNTRY_CODE_KR
};

static const uint16_t g_u2CountryGroup4[] = {
	COUNTRY_CODE_AT, COUNTRY_CODE_BE, COUNTRY_CODE_BG, COUNTRY_CODE_HR,
	COUNTRY_CODE_CZ, COUNTRY_CODE_DK, COUNTRY_CODE_FI, COUNTRY_CODE_FR,
	COUNTRY_CODE_GR, COUNTRY_CODE_HU, COUNTRY_CODE_IS, COUNTRY_CODE_IE,
	COUNTRY_CODE_IT, COUNTRY_CODE_LU, COUNTRY_CODE_NL, COUNTRY_CODE_NO,
	COUNTRY_CODE_PL, COUNTRY_CODE_PT, COUNTRY_CODE_RO, COUNTRY_CODE_SK,
	COUNTRY_CODE_SI, COUNTRY_CODE_ES, COUNTRY_CODE_SE, COUNTRY_CODE_CH,
	COUNTRY_CODE_GB, COUNTRY_CODE_AL, COUNTRY_CODE_AD, COUNTRY_CODE_BY,
	COUNTRY_CODE_BA, COUNTRY_CODE_VG, COUNTRY_CODE_CV, COUNTRY_CODE_CY,
	COUNTRY_CODE_EE, COUNTRY_CODE_ET, COUNTRY_CODE_GF, COUNTRY_CODE_PF,
	COUNTRY_CODE_TF, COUNTRY_CODE_GE, COUNTRY_CODE_DE, COUNTRY_CODE_GH,
	COUNTRY_CODE_GP, COUNTRY_CODE_IQ, COUNTRY_CODE_KE, COUNTRY_CODE_LV,
	COUNTRY_CODE_LS, COUNTRY_CODE_LI, COUNTRY_CODE_LT, COUNTRY_CODE_MK,
	COUNTRY_CODE_MT, COUNTRY_CODE_MQ, COUNTRY_CODE_MR, COUNTRY_CODE_MU,
	COUNTRY_CODE_YT, COUNTRY_CODE_MD, COUNTRY_CODE_MC, COUNTRY_CODE_ME,
	COUNTRY_CODE_MS, COUNTRY_CODE_RE, COUNTRY_CODE_MF, COUNTRY_CODE_SM,
	COUNTRY_CODE_SN, COUNTRY_CODE_RS, COUNTRY_CODE_TR, COUNTRY_CODE_TC,
	COUNTRY_CODE_VA, COUNTRY_CODE_EU, COUNTRY_CODE_DZ
};

static const uint16_t g_u2CountryGroup5[] = {
	COUNTRY_CODE_AU, COUNTRY_CODE_NZ, COUNTRY_CODE_EC, COUNTRY_CODE_PY,
	COUNTRY_CODE_PE, COUNTRY_CODE_TH, COUNTRY_CODE_UY
};

static const uint16_t g_u2CountryGroup6[] = { COUNTRY_CODE_RU };

static const uint16_t g_u2CountryGroup7[] = {
	COUNTRY_CODE_CL, COUNTRY_CODE_EG, COUNTRY_CODE_IN, COUNTRY_CODE_AG,
	COUNTRY_CODE_BS, COUNTRY_CODE_BH, COUNTRY_CODE_BB, COUNTRY_CODE_BN,
	COUNTRY_CODE_MV, COUNTRY_CODE_PA, COUNTRY_CODE_ZM, COUNTRY_CODE_CN
};

static const uint16_t g_u2CountryGroup8[] = { COUNTRY_CODE_MY };

static const uint16_t g_u2CountryGroup9[] = { COUNTRY_CODE_NP };

static const uint16_t g_u2CountryGroup10[] = {
	COUNTRY_CODE_IL, COUNTRY_CODE_AM, COUNTRY_CODE_KW, COUNTRY_CODE_MA,
	COUNTRY_CODE_NE, COUNTRY_CODE_TN
};

static const uint16_t g_u2CountryGroup11[] = {
	COUNTRY_CODE_JO, COUNTRY_CODE_PG
};

static const uint16_t g_u2CountryGroup12[] = { COUNTRY_CODE_AF };

static const uint16_t g_u2CountryGroup13[] = { COUNTRY_CODE_NG };

static const uint16_t g_u2CountryGroup14[] = {
	COUNTRY_CODE_PK, COUNTRY_CODE_QA, COUNTRY_CODE_BF, COUNTRY_CODE_GY,
	COUNTRY_CODE_HT, COUNTRY_CODE_JM, COUNTRY_CODE_MO, COUNTRY_CODE_MW,
	COUNTRY_CODE_RW, COUNTRY_CODE_KN, COUNTRY_CODE_TZ
};

static const uint16_t g_u2CountryGroup15[] = { COUNTRY_CODE_ID };

static const uint16_t g_u2CountryGroup16[] = {
	COUNTRY_CODE_AO, COUNTRY_CODE_BZ, COUNTRY_CODE_BJ, COUNTRY_CODE_BT,
	COUNTRY_CODE_BO, COUNTRY_CODE_BI, COUNTRY_CODE_CM, COUNTRY_CODE_CF,
	COUNTRY_CODE_TD, COUNTRY_CODE_KM, COUNTRY_CODE_CD, COUNTRY_CODE_CG,
	COUNTRY_CODE_CI, COUNTRY_CODE_DJ, COUNTRY_CODE_GQ, COUNTRY_CODE_ER,
	COUNTRY_CODE_FJ, COUNTRY_CODE_GA, COUNTRY_CODE_GM, COUNTRY_CODE_GN,
	COUNTRY_CODE_GW, COUNTRY_CODE_RKS, COUNTRY_CODE_KG, COUNTRY_CODE_LY,
	COUNTRY_CODE_MG, COUNTRY_CODE_ML, COUNTRY_CODE_NR, COUNTRY_CODE_NC,
	COUNTRY_CODE_ST, COUNTRY_CODE_SC, COUNTRY_CODE_SL, COUNTRY_CODE_SB,
	COUNTRY_CODE_SO, COUNTRY_CODE_SR, COUNTRY_CODE_SZ, COUNTRY_CODE_TJ,
	COUNTRY_CODE_TG, COUNTRY_CODE_TO, COUNTRY_CODE_TM, COUNTRY_CODE_TV,
	COUNTRY_CODE_VU, COUNTRY_CODE_YE
};

static const uint16_t g_u2CountryGroup17[] = {
	COUNTRY_CODE_US, COUNTRY_CODE_CA, COUNTRY_CODE_TW
};

static const uint16_t g_u2CountryGroup18[] = {
	COUNTRY_CODE_DM, COUNTRY_CODE_SV, COUNTRY_CODE_HN
};

static const uint16_t g_u2CountryGroup19[] = {
	COUNTRY_CODE_MX, COUNTRY_CODE_VE
};

static const uint16_t g_u2CountryGroup20[] = {
	COUNTRY_CODE_CK, COUNTRY_CODE_CU, COUNTRY_CODE_TL, COUNTRY_CODE_FO,
	COUNTRY_CODE_GI, COUNTRY_CODE_GG, COUNTRY_CODE_IR, COUNTRY_CODE_IM,
	COUNTRY_CODE_JE, COUNTRY_CODE_KP, COUNTRY_CODE_MH, COUNTRY_CODE_NU,
	COUNTRY_CODE_NF, COUNTRY_CODE_PS, COUNTRY_CODE_PN, COUNTRY_CODE_PM,
	COUNTRY_CODE_SS, COUNTRY_CODE_SD, COUNTRY_CODE_SY
};

#if (CFG_SUPPORT_SINGLE_SKU == 1)
struct mtk_regd_control g_mtk_regd_control = {
	.en = FALSE,
	.state = REGD_STATE_UNDEFINED
};

#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
const struct ieee80211_regdomain default_regdom_ww = {
	.n_reg_rules = 4,
	.alpha2 = "99",
	.reg_rules = {
	/* channels 1..13 */
	REG_RULE_LIGHT(2412-10, 2472+10, 40, 0),
	/* channels 14 */
	REG_RULE_LIGHT(2484-10, 2484+10, 20, 0),
	/* channel 36..64 */
	REG_RULE_LIGHT(5150-10, 5350+10, 80, 0),
	/* channel 100..165 */
	REG_RULE_LIGHT(5470-10, 5850+10, 80, 0),
	}
};
#endif

#endif

struct DOMAIN_INFO_ENTRY arSupportedRegDomains[] = {
	{
		(uint16_t *) g_u2CountryGroup0, sizeof(g_u2CountryGroup0) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{82, BAND_2G4, CHNL_SPAN_5, 14, 1, FALSE}
			,			/* CH_SET_2G4_14_14 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_NULL, 0, 0, 0, FALSE}
				/* CH_SET_UNII_UPPER_NA */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup1, sizeof(g_u2CountryGroup1) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup2, sizeof(g_u2CountryGroup2) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup3, sizeof(g_u2CountryGroup3) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup4, sizeof(g_u2CountryGroup4) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup5, sizeof(g_u2CountryGroup5) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 5, TRUE}
			,			/* CH_SET_UNII_WW_100_116 */
			{121, BAND_5G, CHNL_SPAN_20, 132, 3, TRUE}
			,			/* CH_SET_UNII_WW_132_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
				/* CH_SET_UNII_UPPER_149_165 */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup6, sizeof(g_u2CountryGroup6) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 132, 3, TRUE}
			,			/* CH_SET_UNII_WW_132_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup7, sizeof(g_u2CountryGroup7) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup8, sizeof(g_u2CountryGroup8) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 8, TRUE}
			,			/* CH_SET_UNII_WW_100_128 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup9, sizeof(g_u2CountryGroup9) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup10, sizeof(g_u2CountryGroup10) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup11, sizeof(g_u2CountryGroup11) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup12, sizeof(g_u2CountryGroup12) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup13, sizeof(g_u2CountryGroup13) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup14, sizeof(g_u2CountryGroup14) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup15, sizeof(g_u2CountryGroup15) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup16, sizeof(g_u2CountryGroup16) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup17, sizeof(g_u2CountryGroup17) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
					/* CH_SET_UNII_UPPER_149_165 */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup18, sizeof(g_u2CountryGroup18) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 5, TRUE}
			,			/* CH_SET_UNII_WW_100_116 */
			{121, BAND_5G, CHNL_SPAN_20, 132, 3, TRUE}
			,			/* CH_SET_UNII_WW_132_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
						/* CH_SET_UNII_UPPER_149_165 */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup19, sizeof(g_u2CountryGroup19) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup20, sizeof(g_u2CountryGroup20) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		/* Note: Default group if no matched country code */
		NULL, 0,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
};

static const uint16_t g_u2CountryGroup0_Passive[] = {
	COUNTRY_CODE_TW
};

struct DOMAIN_INFO_ENTRY arSupportedRegDomains_Passive[] = {
	{
		(uint16_t *) g_u2CountryGroup0_Passive,
		sizeof(g_u2CountryGroup0_Passive) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 0, FALSE}
			,			/* CH_SET_2G4_1_14_NA */
			{82, BAND_2G4, CHNL_SPAN_5, 14, 0, FALSE}
			,

			{115, BAND_5G, CHNL_SPAN_20, 36, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
		}
	}
	,
	{
		/* Default passive scan channel table: ch52~64, ch100~144 */
		NULL,
		0,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 0, FALSE}
			,			/* CH_SET_2G4_1_14_NA */
			{82, BAND_2G4, CHNL_SPAN_5, 14, 0, FALSE}
			,

			{115, BAND_5G, CHNL_SPAN_20, 36, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
		}
	}
};

struct SUBBAND_CHANNEL g_rRlmSubBand[] = {

	{BAND_2G4_LOWER_BOUND, BAND_2G4_UPPER_BOUND, 1, 0}
	,			/* 2.4G */
	{UNII1_LOWER_BOUND, UNII1_UPPER_BOUND, 2, 0}
	,			/* ch36,38,40,..,48 */
	{UNII2A_LOWER_BOUND, UNII2A_UPPER_BOUND, 2, 0}
	,			/* ch52,54,56,..,64 */
	{UNII2C_LOWER_BOUND, UNII2C_UPPER_BOUND, 2, 0}
	,			/* ch100,102,104,...,144 */
	{UNII3_LOWER_BOUND, UNII3_UPPER_BOUND, 2, 0}
				/* ch149,151,153,....,165 */
};

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
struct DOMAIN_INFO_ENTRY *rlmDomainGetDomainInfo(struct ADAPTER *prAdapter)
{
#define REG_DOMAIN_GROUP_NUM  \
	(sizeof(arSupportedRegDomains) / sizeof(struct DOMAIN_INFO_ENTRY))
#define REG_DOMAIN_DEF_IDX	(REG_DOMAIN_GROUP_NUM - 1)

	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct REG_INFO *prRegInfo;
	uint16_t u2TargetCountryCode;
	uint16_t i, j;

	ASSERT(prAdapter);

	if (prAdapter->prDomainInfo)
		return prAdapter->prDomainInfo;

	prRegInfo = prAdapter->prGlueInfo->prRegInfo;

	DBGLOG(RLM, TRACE, "eRegChannelListMap=%d, u2CountryCode=0x%04x\n",
			   prRegInfo->eRegChannelListMap,
			   prAdapter->rWifiVar.rConnSettings.u2CountryCode);

	/*
	 * Domain info can be specified by given idx of arSupportedRegDomains
	 * table, customized, or searched by country code,
	 * only one is set among these three methods in NVRAM.
	 */
	if (prRegInfo->eRegChannelListMap == REG_CH_MAP_TBL_IDX &&
	    prRegInfo->ucRegChannelListIndex < REG_DOMAIN_GROUP_NUM) {
		/* by given table idx */
		DBGLOG(RLM, TRACE, "ucRegChannelListIndex=%d\n",
		       prRegInfo->ucRegChannelListIndex);
		prDomainInfo = &arSupportedRegDomains
					[prRegInfo->ucRegChannelListIndex];
	} else if (prRegInfo->eRegChannelListMap == REG_CH_MAP_CUSTOMIZED) {
		/* by customized */
		prDomainInfo = &prRegInfo->rDomainInfo;
	} else {
		/* by country code */
		u2TargetCountryCode =
				prAdapter->rWifiVar.rConnSettings.u2CountryCode;

		for (i = 0; i < REG_DOMAIN_GROUP_NUM; i++) {
			prDomainInfo = &arSupportedRegDomains[i];

			if ((prDomainInfo->u4CountryNum &&
			     prDomainInfo->pu2CountryGroup) ||
			    prDomainInfo->u4CountryNum == 0) {
				for (j = 0;
				     j < prDomainInfo->u4CountryNum;
				     j++) {
					if (prDomainInfo->pu2CountryGroup[j] ==
							u2TargetCountryCode)
						break;
				}
				if (j < prDomainInfo->u4CountryNum)
					break;	/* Found */
			}
		}

		/* If no matched country code,
		 * use the default regulatory domain
		 */
		if (i >= REG_DOMAIN_GROUP_NUM) {
			DBGLOG(RLM, INFO,
			       "No matched country code, use the default regulatory domain\n");
			prDomainInfo = &arSupportedRegDomains
							[REG_DOMAIN_DEF_IDX];
		}
	}

	prAdapter->prDomainInfo = prDomainInfo;
	return prDomainInfo;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve the supported channel list of specified band
 *
 * \param[in/out] eSpecificBand:   BAND_2G4, BAND_5G or BAND_NULL
 *                                 (both 2.4G and 5G)
 *                fgNoDfs:         whether to exculde DFS channels
 *                ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainGetChnlList_V2(struct ADAPTER *prAdapter,
			enum ENUM_BAND eSpecificBand, u_int8_t fgNoDfs,
			uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
			struct RF_CHANNEL_INFO *paucChannelList)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	enum ENUM_BAND band;
	uint8_t max_count, i, ucNum;
	struct channel *prCh;

	if (eSpecificBand == BAND_2G4) {
		i = 0;
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else if (eSpecificBand == BAND_5G) {
		i = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else {
		i = 0;
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	}

	ucNum = 0;
	for (; i < max_count; i++) {
		prCh = rlmDomainGetActiveChannels() + i;
		if (fgNoDfs && (prCh->flags & IEEE80211_CHAN_RADAR))
			continue; /*not match*/

		if (i < rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ))
			band = BAND_2G4;
		else
			band = BAND_5G;

		paucChannelList[ucNum].eBand = band;
		paucChannelList[ucNum].ucChannelNum = prCh->chNum;

		ucNum++;
		if (ucMaxChannelNum == ucNum)
			break;
	}

	*pucNumOfChannel = ucNum;
#else
	*pucNumOfChannel = 0;
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Check if Channel supported by HW
 *
 * \param[in/out] eBand:          BAND_2G4, BAND_5G or BAND_NULL
 *                                (both 2.4G and 5G)
 *                ucNumOfChannel: channel number
 *
 * \return TRUE/FALSE
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmIsValidChnl(struct ADAPTER *prAdapter, uint8_t ucNumOfChannel,
			enum ENUM_BAND eBand)
{
	struct ieee80211_supported_band *channelList;
	int i, chSize;
	struct GLUE_INFO *prGlueInfo;
	struct wiphy *prWiphy;

	prGlueInfo = prAdapter->prGlueInfo;
	prWiphy = priv_to_wiphy(prGlueInfo);

	if (eBand == BAND_5G) {
		channelList = prWiphy->bands[KAL_BAND_5GHZ];
		chSize = channelList->n_channels;
	} else if (eBand == BAND_2G4) {
		channelList = prWiphy->bands[KAL_BAND_2GHZ];
		chSize = channelList->n_channels;
	} else
		return FALSE;

	for (i = 0; i < chSize; i++) {
		if ((channelList->channels[i]).hw_value == ucNumOfChannel)
			return TRUE;
	}
	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve the supported channel list of specified band
 *
 * \param[in/out] eSpecificBand:   BAND_2G4, BAND_5G or BAND_NULL
 *                                 (both 2.4G and 5G)
 *                fgNoDfs:         whether to exculde DFS channels
 *                ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainGetChnlList(struct ADAPTER *prAdapter,
		     enum ENUM_BAND eSpecificBand, u_int8_t fgNoDfs,
		     uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
		     struct RF_CHANNEL_INFO *paucChannelList)
{
	uint8_t i, j, ucNum, ch;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	ASSERT(prAdapter);
	ASSERT(paucChannelList);
	ASSERT(pucNumOfChannel);

	if (regd_is_single_sku_en())
		return rlmDomainGetChnlList_V2(prAdapter, eSpecificBand,
					       fgNoDfs, ucMaxChannelNum,
					       pucNumOfChannel,
					       paucChannelList);

	/* If no matched country code, the final one will be used */
	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	ucNum = 0;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_NULL ||
		    prSubband->ucBand >= BAND_NUM ||
		    (prSubband->ucBand == BAND_5G &&
		     !prAdapter->fgEnable5GBand))
			continue;

		/* repoert to upper layer only non-DFS channel
		 * for ap mode usage
		 */
		if (fgNoDfs == TRUE && prSubband->fgDfs == TRUE)
			continue;

		if (eSpecificBand == BAND_NULL ||
		    prSubband->ucBand == eSpecificBand) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if (ucNum >= ucMaxChannelNum)
					break;

				ch = prSubband->ucFirstChannelNum +
				     j * prSubband->ucChannelSpan;
				if (!rlmIsValidChnl(prAdapter, ch,
						prSubband->ucBand)) {
					DBGLOG(RLM, INFO,
					       "Not support ch%d!\n", ch);
					continue;
				}
				paucChannelList[ucNum].eBand =
							prSubband->ucBand;
				paucChannelList[ucNum].ucChannelNum = ch;
				paucChannelList[ucNum].eDFS = prSubband->fgDfs;
				ucNum++;
			}
		}
	}

	*pucNumOfChannel = ucNum;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve DFS channels from 5G band
 *
 * \param[in/out] ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmDomainGetDfsChnls(struct ADAPTER *prAdapter,
			  uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
			  struct RF_CHANNEL_INFO *paucChannelList)
{
	uint8_t i, j, ucNum, ch;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	ASSERT(prAdapter);
	ASSERT(paucChannelList);
	ASSERT(pucNumOfChannel);

	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	ucNum = 0;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G) {
			if (!prAdapter->fgEnable5GBand)
				continue;

			if (prSubband->fgDfs == TRUE) {
				for (j = 0; j < prSubband->ucNumChannels; j++) {
					if (ucNum >= ucMaxChannelNum)
						break;

					ch = prSubband->ucFirstChannelNum +
					     j * prSubband->ucChannelSpan;
					if (!rlmIsValidChnl(prAdapter, ch,
							prSubband->ucBand)) {
						DBGLOG(RLM, INFO,
					       "Not support ch%d!\n", ch);
						continue;
					}

					paucChannelList[ucNum].eBand =
					    prSubband->ucBand;
					paucChannelList[ucNum].ucChannelNum =
					    ch;
					ucNum++;
				}
			}
		}
	}

	*pucNumOfChannel = ucNum;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendCmd(struct ADAPTER *prAdapter)
{
	if (!regd_is_single_sku_en())
		rlmDomainSendPassiveScanInfoCmd(prAdapter);
	rlmDomainSendDomainInfoCmd(prAdapter);
#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
	rlmDomainSendPwrLimitCmd(prAdapter, PWR_CTRL_TYPE_DOMAIN);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendDomainInfoCmd_V2(struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	u8 max_channel_count;
	u32 buff_max_size, buff_valid_size;
	struct CMD_SET_DOMAIN_INFO_V2 *prCmd;
	struct acctive_channel_list *prChs;
	struct wiphy *pWiphy;


	pWiphy = priv_to_wiphy(prAdapter->prGlueInfo);
	max_channel_count = pWiphy->bands[KAL_BAND_2GHZ]->n_channels
				+ pWiphy->bands[KAL_BAND_5GHZ]->n_channels;

	if (max_channel_count == 0) {
		DBGLOG(RLM, ERROR, "%s, invalid channel count.\n", __func__);
		ASSERT(0);
	}


	buff_max_size = sizeof(struct CMD_SET_DOMAIN_INFO_V2) +
			       max_channel_count * sizeof(struct channel);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, buff_max_size);
	prChs = &(prCmd->active_chs);



	/*
	 * Fill in the active channels
	 */
	rlmExtractChannelInfo(max_channel_count, prChs);

	prCmd->u4CountryCode = rlmDomainGetCountryCode();
	prCmd->uc2G4Bandwidth = prAdapter->rWifiVar.rConnSettings.
							uc2G4BandwidthMode;
	prCmd->uc5GBandwidth = prAdapter->rWifiVar.rConnSettings.
							uc5GBandwidthMode;
	prCmd->aucReserved[0] = 0;
	prCmd->aucReserved[1] = 0;

	buff_valid_size = sizeof(struct CMD_SET_DOMAIN_INFO_V2) +
				 (prChs->n_channels_2g + prChs->n_channels_5g) *
				 sizeof(struct channel);

	DBGLOG(RLM, INFO,
	       "rlmDomainSendDomainInfoCmd_V2(), buff_valid_size = 0x%x\n",
	       buff_valid_size);


	/* Set domain info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
			    CMD_ID_SET_DOMAIN_INFO, /* ucCID */
			    TRUE,  /* fgSetQuery */
			    FALSE, /* fgNeedResp */
			    FALSE, /* fgIsOid */
			    NULL,  /* pfCmdDoneHandler */
			    NULL,  /* pfCmdTimeoutHandler */
			    buff_valid_size,
			    (uint8_t *) prCmd, /* pucInfoBuffer */
			    NULL,  /* pvSetQueryBuffer */
			    0      /* u4SetQueryBufferLen */
	    );

	cnmMemFree(prAdapter, prCmd);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendDomainInfoCmd(struct ADAPTER *prAdapter)
{
	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct CMD_SET_DOMAIN_INFO *prCmd;
	struct DOMAIN_SUBBAND_INFO *prSubBand;
	uint8_t i;

	if (regd_is_single_sku_en())
		return rlmDomainSendDomainInfoCmd_V2(prAdapter);


	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			    sizeof(struct CMD_SET_DOMAIN_INFO));
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, sizeof(struct CMD_SET_DOMAIN_INFO));

	prCmd->u2CountryCode = prAdapter->rWifiVar.rConnSettings.u2CountryCode;
	prCmd->u2IsSetPassiveScan = 0;
	prCmd->uc2G4Bandwidth = prAdapter->rWifiVar.rConnSettings.
							uc2G4BandwidthMode;
	prCmd->uc5GBandwidth = prAdapter->rWifiVar.rConnSettings.
							uc5GBandwidthMode;
	prCmd->aucReserved[0] = 0;
	prCmd->aucReserved[1] = 0;

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubBand = &prDomainInfo->rSubBand[i];

		prCmd->rSubBand[i].ucRegClass = prSubBand->ucRegClass;
		prCmd->rSubBand[i].ucBand = prSubBand->ucBand;

		if (prSubBand->ucBand != BAND_NULL && prSubBand->ucBand
								< BAND_NUM) {
			prCmd->rSubBand[i].ucChannelSpan
						= prSubBand->ucChannelSpan;
			prCmd->rSubBand[i].ucFirstChannelNum
						= prSubBand->ucFirstChannelNum;
			prCmd->rSubBand[i].ucNumChannels
						= prSubBand->ucNumChannels;
		}
	}

	/* Set domain info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
		CMD_ID_SET_DOMAIN_INFO, /* ucCID */
		TRUE,  /* fgSetQuery */
		FALSE, /* fgNeedResp */
		FALSE, /* fgIsOid */
		NULL,  /* pfCmdDoneHandler */
		NULL,  /* pfCmdTimeoutHandler */
		sizeof(struct CMD_SET_DOMAIN_INFO), /* u4SetQueryInfoLen */
		(uint8_t *) prCmd, /* pucInfoBuffer */
		NULL,  /* pvSetQueryBuffer */
		0      /* u4SetQueryBufferLen */
	    );

	cnmMemFree(prAdapter, prCmd);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendPassiveScanInfoCmd(struct ADAPTER *prAdapter)
{
#define REG_DOMAIN_PASSIVE_DEF_IDX	1
#define REG_DOMAIN_PASSIVE_GROUP_NUM \
	(sizeof(arSupportedRegDomains_Passive)	\
	 / sizeof(struct DOMAIN_INFO_ENTRY))

	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct CMD_SET_DOMAIN_INFO *prCmd;
	struct DOMAIN_SUBBAND_INFO *prSubBand;
	uint16_t u2TargetCountryCode;
	uint8_t i, j;

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			    sizeof(struct CMD_SET_DOMAIN_INFO));
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, sizeof(struct CMD_SET_DOMAIN_INFO));

	prCmd->u2CountryCode = prAdapter->rWifiVar.rConnSettings.u2CountryCode;
	prCmd->u2IsSetPassiveScan = 1;
	prCmd->uc2G4Bandwidth = prAdapter->rWifiVar.rConnSettings.
							uc2G4BandwidthMode;
	prCmd->uc5GBandwidth = prAdapter->rWifiVar.rConnSettings.
							uc5GBandwidthMode;
	prCmd->aucReserved[0] = 0;
	prCmd->aucReserved[1] = 0;

	DBGLOG(RLM, TRACE, "u2CountryCode=0x%04x\n",
	       prAdapter->rWifiVar.rConnSettings.u2CountryCode);

	u2TargetCountryCode = prAdapter->rWifiVar.rConnSettings.u2CountryCode;

	for (i = 0; i < REG_DOMAIN_PASSIVE_GROUP_NUM; i++) {
		prDomainInfo = &arSupportedRegDomains_Passive[i];

		for (j = 0; j < prDomainInfo->u4CountryNum; j++) {
			if (prDomainInfo->pu2CountryGroup[j] ==
						u2TargetCountryCode)
				break;
		}
		if (j < prDomainInfo->u4CountryNum)
			break;	/* Found */
	}

	if (i >= REG_DOMAIN_PASSIVE_GROUP_NUM)
		prDomainInfo = &arSupportedRegDomains_Passive
					[REG_DOMAIN_PASSIVE_DEF_IDX];

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubBand = &prDomainInfo->rSubBand[i];

		prCmd->rSubBand[i].ucRegClass = prSubBand->ucRegClass;
		prCmd->rSubBand[i].ucBand = prSubBand->ucBand;

		if (prSubBand->ucBand != BAND_NULL && prSubBand->ucBand
		    < BAND_NUM) {
			prCmd->rSubBand[i].ucChannelSpan =
						prSubBand->ucChannelSpan;
			prCmd->rSubBand[i].ucFirstChannelNum =
						prSubBand->ucFirstChannelNum;
			prCmd->rSubBand[i].ucNumChannels =
						prSubBand->ucNumChannels;
		}
	}

	/* Set passive scan channel info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
		CMD_ID_SET_DOMAIN_INFO, /* ucCID */
		TRUE,  /* fgSetQuery */
		FALSE, /* fgNeedResp */
		FALSE, /* fgIsOid */
		NULL,  /* pfCmdDoneHandler */
		NULL,  /* pfCmdTimeoutHandler */
		sizeof(struct CMD_SET_DOMAIN_INFO), /* u4SetQueryInfoLen */
		(uint8_t *) prCmd, /* pucInfoBuffer */
		NULL,  /* pvSetQueryBuffer */
		0      /* u4SetQueryBufferLen */
	    );

	cnmMemFree(prAdapter, prCmd);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return TRUE  Legal channel
 *         FALSE Illegal channel for current regulatory domain
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainIsLegalChannel_V2(struct ADAPTER *prAdapter,
				    enum ENUM_BAND eBand,
				    uint8_t ucChannel)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	uint8_t idx, start_idx, end_idx;
	struct channel *prCh;

	if (eBand == BAND_2G4) {
		start_idx = 0;
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else {
		start_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
	}

	for (idx = start_idx; idx < end_idx; idx++) {
		prCh = rlmDomainGetActiveChannels() + idx;

		if (prCh->chNum == ucChannel)
			return TRUE;
	}

	return FALSE;
#else
	return FALSE;
#endif
}

u_int8_t rlmDomainIsLegalChannel(struct ADAPTER *prAdapter,
				 enum ENUM_BAND eBand, uint8_t ucChannel)
{
	uint8_t i, j;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	if (regd_is_single_sku_en())
		return rlmDomainIsLegalChannel_V2(prAdapter, eBand, ucChannel);


	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G && !prAdapter->fgEnable5GBand)
			continue;

		if (prSubband->ucBand == eBand) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if ((prSubband->ucFirstChannelNum + j *
				    prSubband->ucChannelSpan) == ucChannel) {
					if (!rlmIsValidChnl(prAdapter,
						    ucChannel,
						    prSubband->ucBand)) {
						DBGLOG(RLM, INFO,
						       "Not support ch%d!\n",
						       ucChannel);
						return FALSE;
					} else
						return TRUE;

				}
			}
		}
	}

	return FALSE;
}

u_int8_t rlmDomainIsLegalDfsChannel(struct ADAPTER *prAdapter,
		enum ENUM_BAND eBand, uint8_t ucChannel)
{
	uint8_t i, j;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G
			&& !prAdapter->fgEnable5GBand)
			continue;

		if (prSubband->ucBand == eBand
			&& prSubband->fgDfs == TRUE) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if ((prSubband->ucFirstChannelNum + j *
					prSubband->ucChannelSpan)
					== ucChannel) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/

uint32_t rlmDomainSupOperatingClassIeFill(uint8_t *pBuf)
{
	/*
	 *  The Country element should only be included for
	 *  Status Code 0 (Successful).
	 */
	uint32_t u4IeLen;
	uint8_t aucClass[12] = { 0x01, 0x02, 0x03, 0x05, 0x16, 0x17, 0x19, 0x1b,
		0x1c, 0x1e, 0x20, 0x21
	};

	/*
	 * The Supported Operating Classes element is used by a STA to
	 * advertise the operating classes that it is capable of operating
	 * with in this country.
	 * The Country element (see 8.4.2.10) allows a STA to configure its
	 * PHY and MAC for operation when the operating triplet of Operating
	 * Extension Identifier, Operating Class, and Coverage Class fields
	 * is present.
	 */
	SUP_OPERATING_CLASS_IE(pBuf)->ucId = ELEM_ID_SUP_OPERATING_CLASS;
	SUP_OPERATING_CLASS_IE(pBuf)->ucLength = 1 + sizeof(aucClass);
	SUP_OPERATING_CLASS_IE(pBuf)->ucCur = 0x0c;	/* 0x51 */
	kalMemCopy(SUP_OPERATING_CLASS_IE(pBuf)->ucSup, aucClass,
		   sizeof(aucClass));
	u4IeLen = (SUP_OPERATING_CLASS_IE(pBuf)->ucLength + 2);
	pBuf += u4IeLen;

	COUNTRY_IE(pBuf)->ucId = ELEM_ID_COUNTRY_INFO;
	COUNTRY_IE(pBuf)->ucLength = 6;
	COUNTRY_IE(pBuf)->aucCountryStr[0] = 0x55;
	COUNTRY_IE(pBuf)->aucCountryStr[1] = 0x53;
	COUNTRY_IE(pBuf)->aucCountryStr[2] = 0x20;
	COUNTRY_IE(pBuf)->arCountryStr[0].ucFirstChnlNum = 1;
	COUNTRY_IE(pBuf)->arCountryStr[0].ucNumOfChnl = 11;
	COUNTRY_IE(pBuf)->arCountryStr[0].cMaxTxPwrLv = 0x1e;
	u4IeLen += (COUNTRY_IE(pBuf)->ucLength + 2);

	return u4IeLen;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (fgValid) : 0 -> inValid, 1 -> Valid
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainCheckChannelEntryValid(struct ADAPTER *prAdapter,
					 uint8_t ucCentralCh)
{
	u_int8_t fgValid = FALSE;
	uint8_t ucTemp = 0xff;
	uint8_t i;
	/*Check Power limit table channel efficient or not */

	/* CH50 is not located in any FCC subbands
	 * but it's a valid central channel for 160C
	 */
	if (ucCentralCh == 50) {
		fgValid = TRUE;
		return fgValid;
	}

	for (i = POWER_LIMIT_2G4; i < POWER_LIMIT_SUBAND_NUM; i++) {
		if ((ucCentralCh >= g_rRlmSubBand[i].ucStartCh) &&
				    (ucCentralCh <= g_rRlmSubBand[i].ucEndCh))
			ucTemp = (ucCentralCh - g_rRlmSubBand[i].ucStartCh) %
				 g_rRlmSubBand[i].ucInterval;
	}

	if (ucTemp == 0)
		fgValid = TRUE;
	return fgValid;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return
 */
/*----------------------------------------------------------------------------*/
uint8_t rlmDomainGetCenterChannel(enum ENUM_BAND eBand, uint8_t ucPriChannel,
				  enum ENUM_CHNL_EXT eExtend)
{
	uint8_t ucCenterChannel;

	if (eExtend == CHNL_EXT_SCA)
		ucCenterChannel = ucPriChannel + 2;
	else if (eExtend == CHNL_EXT_SCB)
		ucCenterChannel = ucPriChannel - 2;
	else
		ucCenterChannel = ucPriChannel;

	return ucCenterChannel;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return
 */
/*----------------------------------------------------------------------------*/
u_int8_t
rlmDomainIsValidRfSetting(struct ADAPTER *prAdapter,
			  enum ENUM_BAND eBand,
			  uint8_t ucPriChannel,
			  enum ENUM_CHNL_EXT eExtend,
			  enum ENUM_CHANNEL_WIDTH eChannelWidth,
			  uint8_t ucChannelS1, uint8_t ucChannelS2)
{
	uint8_t ucCenterCh = 0;
	uint8_t  ucUpperChannel;
	uint8_t  ucLowerChannel;
	u_int8_t fgValidChannel = TRUE;
	u_int8_t fgUpperChannel = TRUE;
	u_int8_t fgLowerChannel = TRUE;
	u_int8_t fgValidBW = TRUE;
	u_int8_t fgValidRfSetting = TRUE;
	uint32_t u4PrimaryOffset;

	/*DBG msg for Channel InValid */
	if (eChannelWidth == CW_20_40MHZ) {
		ucCenterCh = rlmDomainGetCenterChannel(eBand, ucPriChannel,
						       eExtend);

		/* Check Central Channel Valid or Not */
		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);
		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: CentralCh=%d\n", ucCenterCh);

		/* Check Upper Channel and Lower Channel */
		switch (eExtend) {
		case CHNL_EXT_SCA:
			ucUpperChannel = ucPriChannel + 4;
			ucLowerChannel = ucPriChannel;
			break;
		case CHNL_EXT_SCB:
			ucUpperChannel = ucPriChannel;
			ucLowerChannel = ucPriChannel - 4;
			break;
		default:
			ucUpperChannel = ucPriChannel;
			ucLowerChannel = ucPriChannel;
			break;
		}

		fgUpperChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								ucUpperChannel);
		if (fgUpperChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: UpperCh=%d\n", ucUpperChannel);

		fgLowerChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								ucLowerChannel);
		if (fgLowerChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: LowerCh=%d\n", ucLowerChannel);

	} else if ((eChannelWidth == CW_80MHZ) ||
		   (eChannelWidth == CW_160MHZ)) {
		ucCenterCh = ucChannelS1;

		/* Check Central Channel Valid or Not */
		if (eChannelWidth != CW_160MHZ) {
			/* BW not check , ex: primary 36 and
			 * central channel 50 will fail the check
			 */
			fgValidChannel =
				rlmDomainCheckChannelEntryValid(prAdapter,
								ucCenterCh);
		}

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf80/160C: CentralCh=%d\n",
			       ucCenterCh);
	} else if (eChannelWidth == CW_80P80MHZ) {
		ucCenterCh = ucChannelS1;

		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf160NC: CentralCh1=%d\n",
			       ucCenterCh);

		ucCenterCh = ucChannelS2;

		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf160NC: CentralCh2=%d\n",
			       ucCenterCh);

		/* Check Central Channel Valid or Not */
	} else {
		DBGLOG(RLM, ERROR, "Wrong BW =%d\n", eChannelWidth);
		fgValidChannel = FALSE;
	}

	/* Check BW Setting Correct or Not */
	if (eBand == BAND_2G4) {
		if (eChannelWidth != CW_20_40MHZ) {
			fgValidBW = FALSE;
			DBGLOG(RLM, WARN, "Rf: B=%d, W=%d\n",
			       eBand, eChannelWidth);
		}
	} else {
		if ((eChannelWidth == CW_80MHZ) ||
				(eChannelWidth == CW_80P80MHZ)) {
			u4PrimaryOffset = CAL_CH_OFFSET_80M(ucPriChannel,
							    ucChannelS1);
			if (u4PrimaryOffset >= 4) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN, "Rf: PriOffSet=%d, W=%d\n",
				       u4PrimaryOffset, eChannelWidth);
			}
			if (ucPriChannel == 165) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN,
				       "Rf: PriOffSet=%d, W=%d C=%d\n",
				       u4PrimaryOffset, eChannelWidth,
				       ucPriChannel);
			}
		} else if (eChannelWidth == CW_160MHZ) {
			u4PrimaryOffset = CAL_CH_OFFSET_160M(ucPriChannel,
							     ucCenterCh);
			if (u4PrimaryOffset >= 8) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN,
				       "Rf: PriOffSet=%d, W=%d\n",
				       u4PrimaryOffset, eChannelWidth);
			}
		}
	}

	if ((fgValidBW == FALSE) || (fgValidChannel == FALSE) ||
	    (fgUpperChannel == FALSE) || (fgLowerChannel == FALSE))
		fgValidRfSetting = FALSE;

	return fgValidRfSetting;

}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
void rlmSetTxPwrLmtCmdValue(struct tx_pwr_element *pEle,
			    struct CMD_CHANNEL_POWER_LIMIT_V2 *pCmd)
{
	memcpy(pCmd, pEle, sizeof(struct CMD_CHANNEL_POWER_LIMIT_V2));
	pCmd->ucCentralCh = pEle->channel_num;
}

u_int8_t rlmDomainGetTxPwrLimit(u32 country_code,
		struct GLUE_INFO *prGlueInfo,
		struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *pSetCmd_2g,
		struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *pSetCmd_5g)
{
	int ret;
	u32 start_offset, ch_idx;
	const struct firmware *file;
	struct tx_pwr_element *pEle;
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *pSetCmd;
	struct CMD_CHANNEL_POWER_LIMIT_V2 *pCmd;
	u_int8_t error = FALSE;

	#define IS_5GHZ_CH(n) ((n) > 14)
	#define TX_PWR_LIMIT_FILE "MT_TxPwrLimit.dat"


	/*open file*/

	/*
	 * Driver support request_firmware() to get files
	 * Android path: "/etc/firmware", "/vendor/firmware", "/firmware/image"
	 * Linux path: "/lib/firmware", "/lib/firmware/update"
	 */
	ret = REQUEST_FIRMWARE(&file, TX_PWR_LIMIT_FILE, prGlueInfo->prDev);

	if (ret) {
		DBGLOG(RLM, WARN,
		       "\n===WARNING===\n%s(); Open file [%s] failed.\n",
		       __func__, TX_PWR_LIMIT_FILE);
		DBGLOG(RLM, WARN,
		       "MaxTxPowerLimit is disable.\n===WARNING===\n");

		/*error*/
		return TRUE;
	}

	DBGLOG(RLM, INFO, "%s(); country_code = 0x%x\n",
		   __func__, country_code);

	/*search country code*/
	start_offset = rlmDomainSearchCountrySection(country_code, file);
	if (!start_offset) {
		DBGLOG(RLM, WARN,
		       "\n===WARNING===\n%s(): Cannot find match country code: 0x%x\n",
		       __func__, country_code);
		DBGLOG(RLM, WARN,
		       "MaxTxPowerLimit is disable.\n===WARNING===\n");

		error = TRUE;
		goto END;
	}

	while (!rlmDomainIsTheEndOfCountrySection(start_offset, file)) {

		/*getting and assign tx power*/
		/*pointer to data base*/
		pEle = (struct tx_pwr_element *)(file->data + start_offset);

		if (pEle->prefix == ELEMENT_PREFIX) {
			/* search the home of this channel and
			 * update the tx pwr
			 */
			if (IS_5GHZ_CH(pEle->channel_num))
				pSetCmd = pSetCmd_5g;
			else
				pSetCmd = pSetCmd_2g;

			if (!pSetCmd)
				continue;


			for (ch_idx = 0; ch_idx < pSetCmd->ucNum; ch_idx++) {
				pCmd = &(pSetCmd->rChannelPowerLimit[ch_idx]);

				if (pCmd->ucCentralCh == pEle->channel_num) {
					rlmSetTxPwrLmtCmdValue(pEle, pCmd);

					break;
				}
			}

			if (ch_idx == pSetCmd->ucNum) {
				DBGLOG(RLM, WARN,
				       "%s(); The channel 0x%x is not active.\n",
				       __func__, pEle->channel_num);
			}
		}
		start_offset += sizeof(struct tx_pwr_element);
	}

END:
	/*close file*/
	release_firmware(file);

	return error;
}
#endif

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY

/*----------------------------------------------------------------------------*/
/*!
 * @brief Check if power limit setting is in the range [MIN_TX_POWER,
 *        MAX_TX_POWER]
 *
 * @param[in]
 *
 * @return (fgValid) : 0 -> inValid, 1 -> Valid
 */
/*----------------------------------------------------------------------------*/
u_int8_t
rlmDomainCheckPowerLimitValid(struct ADAPTER *prAdapter,
			      struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION
						rPowerLimitTableConfiguration,
			      uint8_t ucPwrLimitNum)
{
	uint8_t i;
	u_int8_t fgValid = TRUE;
	int8_t *prPwrLimit;

	prPwrLimit = &rPowerLimitTableConfiguration.aucPwrLimit[0];

	for (i = 0; i < ucPwrLimitNum; i++, prPwrLimit++) {
		if (*prPwrLimit > MAX_TX_POWER || *prPwrLimit < MIN_TX_POWER) {
			fgValid = FALSE;
			break;	/*Find out Wrong Power limit */
		}
	}
	return fgValid;

}

void rlmDomainPowerLimitInit(void)
{
	if (g_productName == 1){
		g_pwrLimitSize = sizeof(g_rRlmPowerLimitConfigurationAK)/sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION);
		g_rRlmPowerLimitConfiguration = g_rRlmPowerLimitConfigurationAK;
	} else {
		g_pwrLimitSize = sizeof(g_rRlmPowerLimitConfigurationMJ)/sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION);
		g_rRlmPowerLimitConfiguration = g_rRlmPowerLimitConfigurationMJ;
	}
}
/*----------------------------------------------------------------------------*/
/*!
 * @brief 1.Check if power limit configuration table valid(channel intervel)
 *	2.Check if power limit configuration/default table entry repeat
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainCheckCountryPowerLimitTable(struct ADAPTER *prAdapter)
{
#define PwrLmtConf g_rRlmPowerLimitConfiguration
	uint16_t i, j;
	uint16_t u2CountryCodeTable, u2CountryCodeCheck;
	u_int8_t fgChannelValid = FALSE;
	u_int8_t fgPowerLimitValid = FALSE;
	u_int8_t fgEntryRepetetion = FALSE;
	u_int8_t fgTableValid = TRUE;
	rlmDomainPowerLimitInit();

	/*1.Configuration Table Check */
	for (i = 0; i < g_pwrLimitSize; i++) {
		/*Table Country Code */
		WLAN_GET_FIELD_BE16(&PwrLmtConf[i].aucCountryCode[0],
				    &u2CountryCodeTable);

		/*<1>Repetition Entry Check */
		for (j = i + 1; j < g_pwrLimitSize; j++) {

			WLAN_GET_FIELD_BE16(&PwrLmtConf[j].aucCountryCode[0],
					    &u2CountryCodeCheck);
			if (((PwrLmtConf[i].ucCentralCh) ==
			     PwrLmtConf[j].ucCentralCh)
			    && (u2CountryCodeTable == u2CountryCodeCheck)) {
				fgEntryRepetetion = TRUE;
				DBGLOG(RLM, LOUD,
				       "Domain: Configuration Repetition CC=%c%c, Ch=%d\n",
				       PwrLmtConf[i].aucCountryCode[0],
				       PwrLmtConf[i].aucCountryCode[1],
				       PwrLmtConf[i].ucCentralCh);
			}
		}

		/*<2>Channel Number Interval Check */
		fgChannelValid =
		    rlmDomainCheckChannelEntryValid(prAdapter,
						    PwrLmtConf[i].ucCentralCh);

		/*<3>Power Limit Range Check */
		fgPowerLimitValid =
		    rlmDomainCheckPowerLimitValid(prAdapter,
						  PwrLmtConf[i],
						  PWR_LIMIT_NUM);

		if (fgChannelValid == FALSE || fgPowerLimitValid == FALSE) {
			fgTableValid = FALSE;
			DBGLOG(RLM, LOUD,
				"Domain: CC=%c%c, Ch=%d, Limit: %d,%d,%d,%d,%d,%d,%d,%d,%d, Valid:%d,%d\n",
				PwrLmtConf[i].aucCountryCode[0],
				PwrLmtConf[i].aucCountryCode[1],
				PwrLmtConf[i].ucCentralCh,
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_CCK],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_20M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_20M_H],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_40M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_40M_H],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_80M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_80M_H],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_160M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_160M_H],
				fgChannelValid,
				fgPowerLimitValid);
		}

		if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			DBGLOG(RLM, LOUD, "Domain: Full search down\n");
			break;	/*End of country table entry */
		}

	}

	if (fgEntryRepetetion == FALSE)
		DBGLOG(RLM, TRACE,
		       "Domain: Configuration Table no Repetiton.\n");

	/*Configuration Table no error */
	if (fgTableValid == TRUE)
		prAdapter->fgIsPowerLimitTableValid = TRUE;
	else
		prAdapter->fgIsPowerLimitTableValid = FALSE;

	/*2.Default Table Repetition Entry Check */
	fgEntryRepetetion = FALSE;
	for (i = 0; i < sizeof(g_rRlmPowerLimitDefault) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); i++) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[i].
							aucCountryCode[0],
				    &u2CountryCodeTable);

		for (j = i + 1; j < sizeof(g_rRlmPowerLimitDefault) /
		     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); j++) {
			WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[j].
							aucCountryCode[0],
					    &u2CountryCodeCheck);
			if (u2CountryCodeTable == u2CountryCodeCheck) {
				fgEntryRepetetion = TRUE;
				DBGLOG(RLM, LOUD,
				       "Domain: Default Repetition CC=%c%c\n",
				       g_rRlmPowerLimitDefault[j].
							aucCountryCode[0],
				       g_rRlmPowerLimitDefault[j].
							aucCountryCode[1]);
			}
		}
	}
	if (fgEntryRepetetion == FALSE)
		DBGLOG(RLM, TRACE, "Domain: Default Table no Repetiton.\n");
#undef PwrLmtConf
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (u2TableIndex) : if  0xFFFF -> No Table Match
 */
/*----------------------------------------------------------------------------*/
uint16_t rlmDomainPwrLimitDefaultTableDecision(struct ADAPTER *prAdapter,
					       uint16_t u2CountryCode)
{

	uint16_t i;
	uint16_t u2CountryCodeTable = COUNTRY_CODE_NULL;
	uint16_t u2TableIndex = POWER_LIMIT_TABLE_NULL;	/* No Table Match */

	/*Default Table Index */
	for (i = 0; i < sizeof(g_rRlmPowerLimitDefault) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); i++) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[i].
						aucCountryCode[0],
				    &u2CountryCodeTable);

		if (u2CountryCodeTable == u2CountryCode) {
			u2TableIndex = i;
			break;	/*match country code */
		} else if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			u2TableIndex = i;
			break;	/*find last one country- Default */
		}
	}

	return u2TableIndex;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Fill power limit CMD by Power Limit Default Table(regulation)
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainBuildCmdByDefaultTable(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT
				*prCmd,
				uint16_t u2DefaultTableIndex)
{
	uint8_t i, k;
	struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT *prPwrLimitSubBand;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	prPwrLimitSubBand = &g_rRlmPowerLimitDefault[u2DefaultTableIndex];

	/*Build power limit cmd by default table information */

	for (i = POWER_LIMIT_2G4; i < POWER_LIMIT_SUBAND_NUM; i++) {
		if (prPwrLimitSubBand->aucPwrLimitSubBand[i] <= MAX_TX_POWER) {
			for (k = g_rRlmSubBand[i].ucStartCh;
			     k <= g_rRlmSubBand[i].ucEndCh;
			     k += g_rRlmSubBand[i].ucInterval) {
				if ((prPwrLimitSubBand->ucPwrUnit
							& BIT(i)) == 0) {
					prCmdPwrLimit->ucCentralCh = k;
					kalMemSet(&prCmdPwrLimit->cPwrLimitCCK,
						  prPwrLimitSubBand->
							aucPwrLimitSubBand[i],
						  PWR_LIMIT_NUM);
				} else {
					/* ex: 40MHz power limit(mW\MHz)
					 *	= 20MHz power limit(mW\MHz) * 2
					 * ---> 40MHz power limit(dBm)
					 *	= 20MHz power limit(dBm) + 6;
					 */
					prCmdPwrLimit->ucCentralCh = k;
					/* BW20 */
					prCmdPwrLimit->cPwrLimitCCK =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];
					prCmdPwrLimit->cPwrLimit20L =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];
					prCmdPwrLimit->cPwrLimit20H =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];

					/* BW40 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 6 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit40L =
								   MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit40H =
								   MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit40L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 6;
						prCmdPwrLimit->cPwrLimit40H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 6;
					}

					/* BW80 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 12 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit80L =
								MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit80H =
								MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit80L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 12;
						prCmdPwrLimit->cPwrLimit80H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 12;
					}

					/* BW160 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 18 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit160L =
								MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit160H =
								MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit160L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 18;
						prCmdPwrLimit->cPwrLimit160H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 18;
					}

				}
				/* save to power limit array per
				 * subband channel
				 */
				prCmdPwrLimit++;
				prCmd->ucNum++;
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Fill power limit CMD by Power Limit Configurartion Table
 * (Bandedge and Customization)
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainBuildCmdByConfigTable(struct ADAPTER *prAdapter,
			struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd)
{
#define PwrLmtConf g_rRlmPowerLimitConfiguration
	uint16_t i, k;
	uint16_t u2CountryCodeTable = COUNTRY_CODE_NULL;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;
	u_int8_t fgChannelValid;

	/*Build power limit cmd by configuration table information */

	for (i = 0; i < g_pwrLimitSize; i++) {

		WLAN_GET_FIELD_BE16(&PwrLmtConf[i].aucCountryCode[0],
				    &u2CountryCodeTable);

		fgChannelValid =
		    rlmDomainCheckChannelEntryValid(prAdapter,
						    PwrLmtConf[i].ucCentralCh);

		if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			break;	/*end of configuration table */
		} else if ((u2CountryCodeTable == prCmd->u2CountryCode)
				&& (fgChannelValid == TRUE)) {

			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];

			if (prCmd->ucNum != 0) {
				for (k = 0; k < prCmd->ucNum; k++) {
					if (prCmdPwrLimit->ucCentralCh ==
						PwrLmtConf[i].ucCentralCh) {

						/* Cmd setting (Default table
						 * information) and Conf table
						 * has repetition channel entry,
						 * ex : Default table (ex: 2.4G,
						 *      limit = 20dBm) -->
						 *      ch1~14 limit =20dBm,
						 * Conf table (ex: ch1, limit =
						 *      22dBm) --> ch 1 = 22 dBm
						 * Cmd final setting --> ch1 =
						 *      22dBm, ch2~14 = 20dBm
						 */
						kalMemCopy(&prCmdPwrLimit->
								cPwrLimitCCK,
							   &PwrLmtConf[i].
								aucPwrLimit,
							   PWR_LIMIT_NUM);

						DBGLOG(RLM, LOUD,
						       "Domain: CC=%c%c,ReplaceCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
						       ((prCmd->u2CountryCode &
								0xff00) >> 8),
						       (prCmd->u2CountryCode &
									0x00ff),
						       prCmdPwrLimit->
								ucCentralCh,
						       prCmdPwrLimit->
								cPwrLimitCCK,
						       prCmdPwrLimit->
								cPwrLimit20L,
						       prCmdPwrLimit->
								cPwrLimit20H,
						       prCmdPwrLimit->
								cPwrLimit40L,
						       prCmdPwrLimit->
								cPwrLimit40H,
						       prCmdPwrLimit->
								cPwrLimit80L,
						       prCmdPwrLimit->
								cPwrLimit80H,
						       prCmdPwrLimit->
								cPwrLimit160L,
						       prCmdPwrLimit->
								cPwrLimit160H,
						       prCmdPwrLimit->ucFlag);

						break;
					}
					/* To search next entry in
					 * rChannelPowerLimit[k]
					 */
					prCmdPwrLimit++;
				}
				if (k == prCmd->ucNum) {

					/* Full search cmd(Default table
					 * setting) no match channel,
					 *  ex : Default table (ex: 2.4G, limit
					 *       =20dBm) -->ch1~14 limit =20dBm,
					 *  Configuration table(ex: ch36, limit
					 *       =22dBm) -->ch 36 = 22 dBm
					 *  Cmd final setting -->
					 *       ch1~14 = 20dBm, ch36 = 22dBm
					 */
					prCmdPwrLimit->ucCentralCh =
						PwrLmtConf[i].ucCentralCh;
					kalMemCopy(&prCmdPwrLimit->cPwrLimitCCK,
					      &PwrLmtConf[i].aucPwrLimit,
					      PWR_LIMIT_NUM);
					/* Add this channel setting in
					 * rChannelPowerLimit[k]
					 */
					prCmd->ucNum++;

					DBGLOG(RLM, LOUD,
					       "Domain: CC=%c%c,AddCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
					       ((prCmd->u2CountryCode & 0xff00)
									>> 8),
					       (prCmd->u2CountryCode & 0x00ff),
					       prCmdPwrLimit->ucCentralCh,
					       prCmdPwrLimit->cPwrLimitCCK,
					       prCmdPwrLimit->cPwrLimit20L,
					       prCmdPwrLimit->cPwrLimit20H,
					       prCmdPwrLimit->cPwrLimit40L,
					       prCmdPwrLimit->cPwrLimit40H,
					       prCmdPwrLimit->cPwrLimit80L,
					       prCmdPwrLimit->cPwrLimit80H,
					       prCmdPwrLimit->cPwrLimit160L,
					       prCmdPwrLimit->cPwrLimit160H,
					       prCmdPwrLimit->ucFlag);

				}
			} else {

				/* Default table power limit value are max on
				 * all subbands --> cmd table no channel entry
				 *  ex : Default table (ex: 2.4G, limit = 63dBm)
				 *  --> no channel entry in cmd,
				 *  Configuration table(ex: ch36, limit = 22dBm)
				 *  --> ch 36 = 22 dBm
				 *  Cmd final setting -->  ch36 = 22dBm
				 */
				prCmdPwrLimit->ucCentralCh =
						PwrLmtConf[i].ucCentralCh;
				kalMemCopy(&prCmdPwrLimit->cPwrLimitCCK,
					   &PwrLmtConf[i].aucPwrLimit,
					   PWR_LIMIT_NUM);
				/* Add this channel setting in
				 * rChannelPowerLimit[k]
				 */
				prCmd->ucNum++;

				DBGLOG(RLM, LOUD,
				       "Domain: Default table power limit value are max on all subbands.\n");
				DBGLOG(RLM, LOUD,
				       "Domain: CC=%c%c,AddCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
				       ((prCmd->u2CountryCode & 0xff00) >> 8),
				       (prCmd->u2CountryCode & 0x00ff),
				       prCmdPwrLimit->ucCentralCh,
				       prCmdPwrLimit->cPwrLimitCCK,
				       prCmdPwrLimit->cPwrLimit20L,
				       prCmdPwrLimit->cPwrLimit20H,
				       prCmdPwrLimit->cPwrLimit40L,
				       prCmdPwrLimit->cPwrLimit40H,
				       prCmdPwrLimit->cPwrLimit80L,
				       prCmdPwrLimit->cPwrLimit80H,
				       prCmdPwrLimit->cPwrLimit160L,
				       prCmdPwrLimit->cPwrLimit160H,
				       prCmdPwrLimit->ucFlag);
			}
		}
	}
#undef PwrLmtConf
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmDomainSendPwrLimitCmd_V2(struct ADAPTER *prAdapter)
{
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

#if (CFG_SUPPORT_SINGLE_SKU == 1)
	uint8_t i;
	uint32_t u4SetQueryInfoLen;
	uint32_t ch_cnt;
	struct wiphy *wiphy;
	u8 band_idx, ch_idx;
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *prCmd[KAL_NUM_BANDS]
								= {NULL};
	uint32_t u4SetCmdTableMaxSize[KAL_NUM_BANDS] = {0};

	DBGLOG(RLM, INFO, "rlmDomainSendPwrLimitCmd_V2()\n");

	wiphy = priv_to_wiphy(prAdapter->prGlueInfo);
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		struct ieee80211_supported_band *sband;
		struct ieee80211_channel *chan;

		sband = wiphy->bands[band_idx];
		if (!sband)
			continue;

		ch_cnt = rlmDomainGetActiveChannelCount(band_idx);
		if (!ch_cnt)
			continue;

		u4SetCmdTableMaxSize[band_idx] =
			sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2) +
			ch_cnt * sizeof(struct CMD_CHANNEL_POWER_LIMIT_V2);

		prCmd[band_idx] = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
					      u4SetCmdTableMaxSize[band_idx]);

		if (!prCmd[band_idx]) {
			DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
			return WLAN_STATUS_RESOURCES;
		}

		/*initialize tw pwr table*/
		kalMemSet(prCmd[band_idx], MAX_TX_POWER,
			  u4SetCmdTableMaxSize[band_idx]);

		prCmd[band_idx]->ucNum = ch_cnt;
		prCmd[band_idx]->eband = (band_idx == KAL_BAND_2GHZ) ?
					 BAND_2G4 : BAND_5G;
		prCmd[band_idx]->countryCode = rlmDomainGetCountryCode();

		DBGLOG(RLM, INFO, "%s, active n_channels=%d, band=%d\n",
		       __func__, ch_cnt, prCmd[band_idx]->eband);

		i = 0;
		for (ch_idx = 0; ch_idx < sband->n_channels; ch_idx++) {
			chan = &sband->channels[ch_idx];
			if (chan->flags & IEEE80211_CHAN_DISABLED)
				continue;

			prCmd[band_idx]->rChannelPowerLimit[i].ucCentralCh =
								chan->hw_value;

			i++; /*point to the next entry*/
			if (i == ch_cnt)
				break;
		}
	}


	/*
	 * Get Max Tx Power from MT_TxPwrLimit.dat
	 */
	rlmDomainGetTxPwrLimit(rlmDomainGetCountryCode(),
							prAdapter->prGlueInfo,
							prCmd[KAL_BAND_2GHZ],
							prCmd[KAL_BAND_5GHZ]);


	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (!prCmd[band_idx])
			continue;

		u4SetQueryInfoLen = u4SetCmdTableMaxSize[band_idx];

		/* Update tx max. power info to chip */
		rStatus = wlanSendSetQueryCmd(prAdapter, /* prAdapter */
				CMD_ID_SET_COUNTRY_POWER_LIMIT, /* ucCID */
				TRUE, /* fgSetQuery */
				FALSE, /* fgNeedResp */
				FALSE, /* fgIsOid */
				NULL, /* pfCmdDoneHandler */
				NULL, /* pfCmdTimeoutHandler */
				u4SetQueryInfoLen, /* u4SetQueryInfoLen */
				(uint8_t *) prCmd[band_idx], /* pucInfoBuffer */
				NULL, /* pvSetQueryBuffer */
				0 /* u4SetQueryBufferLen */
				);

		cnmMemFree(prAdapter, prCmd[band_idx]);
	}
#endif

	return rStatus;
}

uint32_t rlmDomainSendPwrLimitCmd(struct ADAPTER *prAdapter,
				  enum ENUM_TX_POWER_CTRL_TYPE eCtrlType)
{
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd = NULL;
	uint32_t fgMask;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint8_t i;
	uint16_t u2DefaultTableIndex;
	uint32_t u4SetCmdTableMaxSize;
	uint32_t u4SetQueryInfoLen;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;	/* for print usage */
	struct REG_INFO *prRegInfo;

	if (!prAdapter) {
		DBGLOG(RLM, ERROR, "prAdapter is NULL\n");
		return WLAN_STATUS_ADAPTER_NOT_READY;
	}

	fgMask = prAdapter->fgTxPwrLimitMask;

	DBGLOG(RLM, INFO, "tx power control: eCtrlType(%d,%d)=[%s], mask=%u\n",
		eCtrlType, PWR_CTRL_TYPE_NUM,
		g_ENUM_TX_POWER_CTRL_TYPE_LABEL[eCtrlType], fgMask);

	if (regd_is_single_sku_en())
		return rlmDomainSendPwrLimitCmd_V2(prAdapter);

	if (eCtrlType == PWR_CTRL_TYPE_DISABLE_FCC_IOCTL) {
		if (fgMask & PWR_CRTL_MASK_FCC_IOCTL)
			fgMask -= PWR_CRTL_MASK_FCC_IOCTL;
		eCtrlType = PWR_CTRL_TYPE_DOMAIN;
	} else if (eCtrlType == PWR_CTRL_TYPE_DISABLE_SAR_IOCTL) {
		if (fgMask & PWR_CRTL_MASK_SAR_IOCTL)
			fgMask -= PWR_CRTL_MASK_SAR_IOCTL;
		eCtrlType = PWR_CTRL_TYPE_DOMAIN;
	} else if (eCtrlType == PWR_CTRL_TYPE_DISABLE_TXPWR_SCENARIO) {
		if (fgMask & PWR_CRTL_MASK_TXPWR_SCENARIO)
			fgMask -= PWR_CRTL_MASK_TXPWR_SCENARIO;
		eCtrlType = PWR_CTRL_TYPE_DOMAIN;
	} else if (eCtrlType == PWR_CTRL_TYPE_DISABLE_3STEPS_BACKOFF) {
		if (fgMask & PWR_CRTL_MASK_3STEPS_BACKOFF)
			fgMask -= PWR_CRTL_MASK_3STEPS_BACKOFF;
		eCtrlType = PWR_CTRL_TYPE_DOMAIN;
	}

	/* construct tx power table by domain */
	u4SetCmdTableMaxSize =
			sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT) +
			MAX_CMD_SUPPORT_CHANNEL_NUM *
			sizeof(struct CMD_CHANNEL_POWER_LIMIT);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, u4SetCmdTableMaxSize);
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Domain: Alloc cmd buffer failed\n");
		return WLAN_STATUS_RESOURCES;
	}
	kalMemZero(prCmd, u4SetCmdTableMaxSize);

	u2DefaultTableIndex =
			rlmDomainPwrLimitDefaultTableDecision(prAdapter,
			prAdapter->rWifiVar.rConnSettings.u2CountryCode
			);
	if (u2DefaultTableIndex != POWER_LIMIT_TABLE_NULL) {
		WLAN_GET_FIELD_BE16(
			&g_rRlmPowerLimitDefault[u2DefaultTableIndex].
				aucCountryCode[0],
				&prCmd->u2CountryCode);

		/* Initialize channel number */
		prCmd->ucNum = 0;

		/* <1> default table information, fill all subband */
		rlmDomainBuildCmdByDefaultTable(prCmd, u2DefaultTableIndex);

		/* <2> configuration table information,
		 *     replace specified channel
		 */
		rlmDomainBuildCmdByConfigTable(prAdapter, prCmd);
	}

	DBGLOG(RLM, INFO,
	       "Domain: PwrLimitChNum=%d, ValidCC=%c%c, PwrLimitCC=%c%c\n",
	       prCmd->ucNum,
	       (prAdapter->rWifiVar.rConnSettings.u2CountryCode & 0xff00) >> 8,
	       (prAdapter->rWifiVar.rConnSettings.u2CountryCode & 0x00ff),
	       ((prCmd->u2CountryCode & 0xff00) >> 8),
	       (prCmd->u2CountryCode & 0x00ff));

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	for (i = 0; i < prCmd->ucNum; i++) {
		DBGLOG(RLM, TRACE,
		       "Original Domain: Ch=%d, Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d, Fg=%d\n",
		       prCmdPwrLimit->ucCentralCh,
		       prCmdPwrLimit->cPwrLimitCCK,
		       prCmdPwrLimit->cPwrLimit20L,
		       prCmdPwrLimit->cPwrLimit20H,
		       prCmdPwrLimit->cPwrLimit40L,
		       prCmdPwrLimit->cPwrLimit40H,
		       prCmdPwrLimit->cPwrLimit80L,
		       prCmdPwrLimit->cPwrLimit80H,
		       prCmdPwrLimit->cPwrLimit160L,
		       prCmdPwrLimit->cPwrLimit160H,
		       prCmdPwrLimit->ucFlag);
		prCmdPwrLimit++;
	}

	if (!(prAdapter->prGlueInfo) || !(prAdapter->prGlueInfo->prRegInfo)) {
		DBGLOG(RLM, ERROR, "prGlueInfo/prRegInfo is NULL\n");
		rStatus = WLAN_STATUS_ADAPTER_NOT_READY;
		goto freeMemLabel;
	}
	prRegInfo = prAdapter->prGlueInfo->prRegInfo;

	/* case 1: set tx power for domain */
	if (eCtrlType == PWR_CTRL_TYPE_DOMAIN)
		fgMask |= PWR_CRTL_MASK_DOMAIN;

	/* case 2: set band edge tx power for 2.4G */
	if (prRegInfo->fg2G4BandEdgePwrUsed &&
	    ((eCtrlType == PWR_CTRL_TYPE_BANDEDGE_2G) ||
	     (fgMask & PWR_CRTL_MASK_BANDEDGE_2G4))) {
		uint8_t ucStartChl = 0;
		uint8_t ucStopChl = ucStartChl;
		uint8_t ucStartChlIdx = 0;
		uint8_t ucStopChlIdx = ucStartChlIdx;
		uint8_t ucCCK = prRegInfo->cBandEdgeMaxPwrCCK;
		uint8_t ucOFDM20 = prRegInfo->cBandEdgeMaxPwrOFDM20;
		uint8_t ucOFDM40 = prRegInfo->cBandEdgeMaxPwrOFDM40;

		prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
		for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
			uint8_t curChl = prCmdPwrLimit->ucCentralCh;

			if (curChl > 14)
				continue;

			if ((ucStartChl == 0) || (curChl < ucStartChl)) {
				ucStartChl = curChl;
				ucStartChlIdx = i;
			}

			if ((ucStopChl == 0) || (curChl > ucStopChl)) {
				ucStopChl = curChl;
				ucStopChlIdx = i;
			}
		}

		DBGLOG(RLM, INFO,
		       "bandedge 2.4G: start chl=%u,%u stop chl=%u,%u ucCCK=%u, ucOFDM20=%u, ucOFDM40=%u\n",
		       ucStartChl, ucStartChlIdx, ucStopChl, ucStopChlIdx,
		       ucCCK, ucOFDM20, ucOFDM40);

		if ((ucStartChl != 0) && (ucStopChl != 0)) {
			for (i = 0; i < 2; i++) {
				uint8_t idx = (i == 0) ?
						ucStartChlIdx : ucStopChlIdx;
				prCmdPwrLimit = &prCmd->rChannelPowerLimit[idx];
				if (ucCCK < prCmdPwrLimit->cPwrLimitCCK)
					prCmdPwrLimit->cPwrLimitCCK = ucCCK;
				if (ucOFDM20 < prCmdPwrLimit->cPwrLimit20L)
					prCmdPwrLimit->cPwrLimit20L = ucOFDM20;
				if (ucOFDM20 < prCmdPwrLimit->cPwrLimit20H)
					prCmdPwrLimit->cPwrLimit20H = ucOFDM20;
				if (ucOFDM40 < prCmdPwrLimit->cPwrLimit40L)
					prCmdPwrLimit->cPwrLimit40L = ucOFDM40;
				if (ucOFDM40 < prCmdPwrLimit->cPwrLimit40H)
					prCmdPwrLimit->cPwrLimit40H = ucOFDM40;
			}
		}
		fgMask |= PWR_CRTL_MASK_BANDEDGE_2G4;
	}

#if CFG_SUPPORT_NVRAM_5G
	/* case 3: set band edge tx power for 5G */
	if (prRegInfo->ucEnable5GBand) {
		struct BANDEDGE_5G *pr5GBandEdge =
				&prRegInfo->prOldEfuseMapping->r5GBandEdgePwr;

		if (pr5GBandEdge->uc5GBandEdgePwrUsed &&
		    ((eCtrlType == PWR_CTRL_TYPE_BANDEDGE_5G) ||
		     (fgMask & PWR_CRTL_MASK_BANDEDGE_5G))) {
			struct BANDEDGE_5G *pr5GBandEdge =
				&prAdapter->prGlueInfo->prRegInfo->
					prOldEfuseMapping->r5GBandEdgePwr;
			uint8_t ucOFDM20 =
				pr5GBandEdge->c5GBandEdgeMaxPwrOFDM20;
			uint8_t ucOFDM40 =
				pr5GBandEdge->c5GBandEdgeMaxPwrOFDM40;
			uint8_t ucOFDM80 =
				pr5GBandEdge->c5GBandEdgeMaxPwrOFDM80;

			DBGLOG(RLM, INFO,
			       "bandedge 5G: start chl=%u stop chl=%u ucOFDM20=%u, ucOFDM40=%u, ucOFDM80=%u\n",
			       g_aucBandEdge5G[0], g_aucBandEdge5G[1],
			       ucOFDM20, ucOFDM40, ucOFDM80);

			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
			for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
				uint8_t curChl = prCmdPwrLimit->ucCentralCh;

				if ((curChl > 14) &&
				    ((curChl <= g_aucBandEdge5G[0]) ||
				     (curChl >= g_aucBandEdge5G[1]))) {
					prCmdPwrLimit->cPwrLimitCCK = 0;
					if (ucOFDM20 <
					    prCmdPwrLimit->cPwrLimit20L)
						prCmdPwrLimit->cPwrLimit20L =
								ucOFDM20;
					if (ucOFDM20 <
					    prCmdPwrLimit->cPwrLimit20H)
						prCmdPwrLimit->cPwrLimit20H =
								ucOFDM20;
					if (ucOFDM40 <
					    prCmdPwrLimit->cPwrLimit40L)
						prCmdPwrLimit->cPwrLimit40L =
								ucOFDM40;
					if (ucOFDM40 <
					    prCmdPwrLimit->cPwrLimit40H)
						prCmdPwrLimit->cPwrLimit40H =
								ucOFDM40;
					if (ucOFDM80 <
					    prCmdPwrLimit->cPwrLimit80L)
						prCmdPwrLimit->cPwrLimit80L =
								ucOFDM80;
					if (ucOFDM80 <
					    prCmdPwrLimit->cPwrLimit80H)
						prCmdPwrLimit->cPwrLimit80H =
								ucOFDM80;
				}
			}
			fgMask |= PWR_CRTL_MASK_BANDEDGE_5G;
		}
	}
#endif /* #if CFG_SUPPORT_NVRAM_5G */

#if CFG_SUPPORT_FCC_DYNAMIC_TX_PWR_ADJUST
	/* case 4: set tx power for FCC at wifi on */
	if (g_rFccTxPwrAdjust.fgFccTxPwrAdjust &&
	    ((eCtrlType == PWR_CTRL_TYPE_FCC_WIFION) ||
	     (fgMask & PWR_CRTL_MASK_FCC_WIFION))) {
		struct FCC_TX_PWR_ADJUST *prFccTxPwrAdjust =
			(struct FCC_TX_PWR_ADJUST *)&g_rFccTxPwrAdjust;
		uint8_t uOffsetCCK = prFccTxPwrAdjust->uOffsetCCK;
		uint8_t uOffsetHT20 = prFccTxPwrAdjust->uOffsetHT20;
		uint8_t uOffsetHT40 = prFccTxPwrAdjust->uOffsetHT40;
		uint8_t uStartCCKChl =
				(uint8_t)prFccTxPwrAdjust->aucChannelCCK[0];
		uint8_t uStopCCKChl =
				(uint8_t)prFccTxPwrAdjust->aucChannelCCK[1];
		uint8_t uStartHT20Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT20[0];
		uint8_t uStopHT20Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT20[1];
		uint8_t uStartHT40Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT40[0];
		uint8_t uStopHT40Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT40[1];

		DBGLOG(RLM, INFO,
		       "FCC wifion: uOffsetCCK=%u(%u,%u) uOffsetHT20=%u(%u,%u), uOffsetHT40=%u(%u,%u)\n",
		       uOffsetCCK, uStartCCKChl, uStopCCKChl,
		       uOffsetHT20, uStartHT20Chl, uStopHT20Chl,
		       uOffsetHT40, uStartHT40Chl, uStopHT40Chl);

		prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
		for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
			uint8_t curChl = prCmdPwrLimit->ucCentralCh;

			if ((curChl >= uStartCCKChl) &&
			    (curChl <= uStopCCKChl)) {
				if (prCmdPwrLimit->cPwrLimitCCK > uOffsetCCK)
					prCmdPwrLimit->cPwrLimitCCK -=
								uOffsetCCK;
				else
					prCmdPwrLimit->cPwrLimitCCK = 0;
			}
			if ((curChl >= uStartHT20Chl) &&
			    (curChl <= uStopHT20Chl)) {
				if (prCmdPwrLimit->cPwrLimit20L > uOffsetHT20)
					prCmdPwrLimit->cPwrLimit20L -=
								uOffsetHT20;
				else
					prCmdPwrLimit->cPwrLimit20L = 0;
				if (prCmdPwrLimit->cPwrLimit20H > uOffsetHT20)
					prCmdPwrLimit->cPwrLimit20H -=
								uOffsetHT20;
				else
					prCmdPwrLimit->cPwrLimit20H = 0;
			}
			if ((curChl >= uStartHT40Chl) &&
			    (curChl <= uStopHT40Chl)) {
				if (prCmdPwrLimit->cPwrLimit40L > uOffsetHT40)
					prCmdPwrLimit->cPwrLimit40L -=
								uOffsetHT40;
				else
					prCmdPwrLimit->cPwrLimit40L -= 0;
				if (prCmdPwrLimit->cPwrLimit40H > uOffsetHT40)
					prCmdPwrLimit->cPwrLimit40H -=
								uOffsetHT40;
				else
					prCmdPwrLimit->cPwrLimit40H = 0;
			}
		}
		fgMask |= PWR_CRTL_MASK_FCC_WIFION;
	}
#endif /* CFG_SUPPORT_FCC_DYNAMIC_TX_PWR_ADJUST */

#if CFG_SUPPORT_FCC_POWER_BACK_OFF
	/* case 5: set tx power for FCC from ioctl */
	if (g_rFccTxPwrAdjust.fgFccTxPwrAdjust &&
	    ((eCtrlType == PWR_CTRL_TYPE_ENABLE_FCC_IOCTL) ||
	     (fgMask & PWR_CRTL_MASK_FCC_IOCTL))) {
		struct FCC_TX_PWR_ADJUST *prFccTxPwrAdjust =
			(struct FCC_TX_PWR_ADJUST *)&g_rFccTxPwrAdjust;
		uint8_t uOffsetCCK = prFccTxPwrAdjust->uOffsetCCK;
		uint8_t uOffsetHT20 = prFccTxPwrAdjust->uOffsetHT20;
		uint8_t uOffsetHT40 = prFccTxPwrAdjust->uOffsetHT40;
		uint8_t uStartCCKChl =
				(uint8_t)prFccTxPwrAdjust->aucChannelCCK[0];
		uint8_t uStopCCKChl =
				(uint8_t)prFccTxPwrAdjust->aucChannelCCK[1];
		uint8_t uStartHT20Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT20[0];
		uint8_t uStopHT20Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT20[1];
		uint8_t uStartHT40Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT40[0];
		uint8_t uStopHT40Chl =
				(uint8_t)prFccTxPwrAdjust->aucChannelHT40[1];

		DBGLOG(RLM, INFO,
		       "FCC ioctl: uOffsetCCK=%u(%u,%u) uOffsetHT20=%u(%u,%u), uOffsetHT40=%u(%u,%u)\n",
		       uOffsetCCK, uStartCCKChl, uStopCCKChl,
		       uOffsetHT20, uStartHT20Chl, uStopHT20Chl,
		       uOffsetHT40, uStartHT40Chl, uStopHT40Chl);

		prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
		for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
			uint8_t curChl = prCmdPwrLimit->ucCentralCh;

			if ((curChl >= uStartCCKChl) &&
			    (curChl <= uStopCCKChl)) {
				if (prCmdPwrLimit->cPwrLimitCCK > uOffsetCCK)
					prCmdPwrLimit->cPwrLimitCCK -=
								uOffsetCCK;
				else
					prCmdPwrLimit->cPwrLimitCCK = 0;
			}
			if ((curChl >= uStartHT20Chl) &&
			    (curChl <= uStopHT20Chl)) {
				if (prCmdPwrLimit->cPwrLimit20L > uOffsetHT20)
					prCmdPwrLimit->cPwrLimit20L -=
								uOffsetHT20;
				else
					prCmdPwrLimit->cPwrLimit20L = 0;
				if (prCmdPwrLimit->cPwrLimit20H > uOffsetHT20)
					prCmdPwrLimit->cPwrLimit20H -=
								uOffsetHT20;
				else
					prCmdPwrLimit->cPwrLimit20H = 0;
			}
			if ((curChl >= uStartHT40Chl) &&
			    (curChl <= uStopHT40Chl)) {
				if (prCmdPwrLimit->cPwrLimit40L > uOffsetHT40)
					prCmdPwrLimit->cPwrLimit40L -=
								uOffsetHT40;
				else
					prCmdPwrLimit->cPwrLimit40L = 0;
				if (prCmdPwrLimit->cPwrLimit40H > uOffsetHT40)
					prCmdPwrLimit->cPwrLimit40H -=
								uOffsetHT40;
				else
					prCmdPwrLimit->cPwrLimit40H = 0;
			}
		}
		fgMask |= PWR_CRTL_MASK_FCC_IOCTL;
	}
#endif /* CFG_SUPPORT_FCC_POWER_BACK_OFF */

#if CFG_SUPPORT_TX_POWER_BACK_OFF
	/* case 6: set tx power for SAR at ioctl */
	if ((eCtrlType == PWR_CTRL_TYPE_ENABLE_SAR_IOCTL) ||
	    (fgMask & PWR_CRTL_MASK_SAR_IOCTL)) {
		uint8_t aucParam[4];

		kalMemCopy(&aucParam, &g_TxPwrBackOffParam, 4);

		DBGLOG(RLM, INFO,
		       "SAR ioctl: 2.4G(enable:%u, pwr=%u), 5G(enable:%u, pwr=%u)\n",
		       aucParam[0], aucParam[1], aucParam[2], aucParam[3]);

		if (aucParam[0] != 0) {
			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
			for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
				if (prCmdPwrLimit->ucCentralCh > 14)
					continue;
				if (prCmdPwrLimit->cPwrLimitCCK > aucParam[1])
					prCmdPwrLimit->cPwrLimitCCK =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit20L > aucParam[1])
					prCmdPwrLimit->cPwrLimit20L =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit20H > aucParam[1])
					prCmdPwrLimit->cPwrLimit20H =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit40L > aucParam[1])
					prCmdPwrLimit->cPwrLimit40L =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit40H > aucParam[1])
					prCmdPwrLimit->cPwrLimit40H =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit80L > aucParam[1])
					prCmdPwrLimit->cPwrLimit80L =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit80H > aucParam[1])
					prCmdPwrLimit->cPwrLimit80H =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit160L > aucParam[1])
					prCmdPwrLimit->cPwrLimit160L =
								aucParam[1];
				if (prCmdPwrLimit->cPwrLimit160H > aucParam[1])
					prCmdPwrLimit->cPwrLimit160H =
								aucParam[1];
			}
		}

		if (aucParam[2] != 0) {
			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
			for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
				if (prCmdPwrLimit->ucCentralCh <= 14)
					continue;
				if (prCmdPwrLimit->cPwrLimitCCK > aucParam[3])
					prCmdPwrLimit->cPwrLimitCCK =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit20L > aucParam[3])
					prCmdPwrLimit->cPwrLimit20L =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit20H > aucParam[3])
					prCmdPwrLimit->cPwrLimit20H =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit40L > aucParam[3])
					prCmdPwrLimit->cPwrLimit40L =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit40H > aucParam[3])
					prCmdPwrLimit->cPwrLimit40H =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit80L > aucParam[3])
					prCmdPwrLimit->cPwrLimit80L =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit80H > aucParam[3])
					prCmdPwrLimit->cPwrLimit80H =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit160L > aucParam[3])
					prCmdPwrLimit->cPwrLimit160L =
								aucParam[3];
				if (prCmdPwrLimit->cPwrLimit160H > aucParam[3])
					prCmdPwrLimit->cPwrLimit160H =
								aucParam[3];
			}
		}
		fgMask |= PWR_CRTL_MASK_SAR_IOCTL;
	}

	/* case 7: set tx power scenario */
	if ((eCtrlType == PWR_CTRL_TYPE_ENABLE_TXPWR_SCENARIO) ||
	    (fgMask & PWR_CRTL_MASK_TXPWR_SCENARIO)) {
		uint8_t u2G4Power, u5GPower;

		if ((g_iTxPwrScenarioIdx >= 0) && (g_iTxPwrScenarioIdx < 5))
			DBGLOG(RLM, INFO,
			       "set tx power scenario=%d, 2.4G(enable:%u, pwr=%u), 5G(enable:%u, pwr=%u)\n",
			       g_iTxPwrScenarioIdx,
			       g_bTxPwrScenarioEnable2G,
			       g_acTxPwrScenarioMaxPower2G[g_iTxPwrScenarioIdx],
			       g_bTxPwrScenarioEnable5G,
			       g_acTxPwrScenarioMaxPower5G[g_iTxPwrScenarioIdx]
			      );
		else {
			rStatus = WLAN_STATUS_INVALID_DATA;
			goto freeMemLabel;
		}

		if (g_bTxPwrScenarioEnable2G) {
			u2G4Power = g_acTxPwrScenarioMaxPower2G[
							g_iTxPwrScenarioIdx];
			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
			for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
				if (prCmdPwrLimit->ucCentralCh > 14)
					continue;
				if (prCmdPwrLimit->cPwrLimitCCK > u2G4Power)
					prCmdPwrLimit->cPwrLimitCCK = u2G4Power;
				if (prCmdPwrLimit->cPwrLimit20L > u2G4Power)
					prCmdPwrLimit->cPwrLimit20L = u2G4Power;
				if (prCmdPwrLimit->cPwrLimit20H > u2G4Power)
					prCmdPwrLimit->cPwrLimit20H = u2G4Power;
				if (prCmdPwrLimit->cPwrLimit40L > u2G4Power)
					prCmdPwrLimit->cPwrLimit40L = u2G4Power;
				if (prCmdPwrLimit->cPwrLimit40H > u2G4Power)
					prCmdPwrLimit->cPwrLimit40H = u2G4Power;
				if (prCmdPwrLimit->cPwrLimit80L > u2G4Power)
					prCmdPwrLimit->cPwrLimit80L = u2G4Power;
				if (prCmdPwrLimit->cPwrLimit80H > u2G4Power)
					prCmdPwrLimit->cPwrLimit80H = u2G4Power;
				if (prCmdPwrLimit->cPwrLimit160L > u2G4Power)
					prCmdPwrLimit->cPwrLimit160L =
								u2G4Power;
				if (prCmdPwrLimit->cPwrLimit160H > u2G4Power)
					prCmdPwrLimit->cPwrLimit160H =
								u2G4Power;
			}
		}

		if (g_bTxPwrScenarioEnable5G) {
			u5GPower = g_acTxPwrScenarioMaxPower2G[
							g_iTxPwrScenarioIdx];
			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
			for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
				if (prCmdPwrLimit->ucCentralCh <= 14)
					continue;
				if (prCmdPwrLimit->cPwrLimitCCK > u5GPower)
					prCmdPwrLimit->cPwrLimitCCK = u5GPower;
				if (prCmdPwrLimit->cPwrLimit20L > u5GPower)
					prCmdPwrLimit->cPwrLimit20L = u5GPower;
				if (prCmdPwrLimit->cPwrLimit20H > u5GPower)
					prCmdPwrLimit->cPwrLimit20H = u5GPower;
				if (prCmdPwrLimit->cPwrLimit40L > u5GPower)
					prCmdPwrLimit->cPwrLimit40L = u5GPower;
				if (prCmdPwrLimit->cPwrLimit40H > u5GPower)
					prCmdPwrLimit->cPwrLimit40H = u5GPower;
				if (prCmdPwrLimit->cPwrLimit80L > u5GPower)
					prCmdPwrLimit->cPwrLimit80L = u5GPower;
				if (prCmdPwrLimit->cPwrLimit80H > u5GPower)
					prCmdPwrLimit->cPwrLimit80H = u5GPower;
				if (prCmdPwrLimit->cPwrLimit160L > u5GPower)
					prCmdPwrLimit->cPwrLimit160L =
								u5GPower;
				if (prCmdPwrLimit->cPwrLimit160H > u5GPower)
					prCmdPwrLimit->cPwrLimit160H =
								u5GPower;
			}
		}
		fgMask |= PWR_CRTL_MASK_TXPWR_SCENARIO;
	}

	/* case 8: 3-steps power back off */
	if ((eCtrlType == PWR_CTRL_TYPE_ENABLE_3STEPS_BACKOFF) ||
	    (fgMask & PWR_CRTL_MASK_3STEPS_BACKOFF)) {
		int8_t iPwrOffset;

		if ((g_i3StepsBackOffIdx >= 1) && (g_i3StepsBackOffIdx <= 3)) {
			iPwrOffset = g_ac3StepsPoewrOffset[
					g_i3StepsBackOffIdx - 1];
			DBGLOG(RLM, INFO,
			       "set 3-steps power back off, idx=%d, offset=%d\n",
			       g_i3StepsBackOffIdx, iPwrOffset);
		} else {
			rStatus = WLAN_STATUS_INVALID_DATA;
			goto freeMemLabel;
		}

		prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
		for (i = 0; i < prCmd->ucNum; i++, prCmdPwrLimit++) {
			if ((prCmdPwrLimit->cPwrLimitCCK + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimitCCK += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit20L + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit20L += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit20H + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit20H += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit40L + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit40L += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit40H + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit40H += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit80L + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit80L += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit80H + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit80H += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit160L + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit160L += iPwrOffset;
			if ((prCmdPwrLimit->cPwrLimit160H + iPwrOffset) > 0)
				prCmdPwrLimit->cPwrLimit160H += iPwrOffset;
		}

		fgMask |= PWR_CRTL_MASK_3STEPS_BACKOFF;
	}
#endif /* CFG_SUPPORT_TX_POWER_BACK_OFF */

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	for (i = 0; i < prCmd->ucNum; i++) {
		DBGLOG(RLM, TRACE,
			"Modified Domain: Ch=%d, Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d, Fg=%d\n",
			prCmdPwrLimit->ucCentralCh,
			prCmdPwrLimit->cPwrLimitCCK,
			prCmdPwrLimit->cPwrLimit20L,
			prCmdPwrLimit->cPwrLimit20H,
			prCmdPwrLimit->cPwrLimit40L,
			prCmdPwrLimit->cPwrLimit40H,
			prCmdPwrLimit->cPwrLimit80L,
			prCmdPwrLimit->cPwrLimit80H,
			prCmdPwrLimit->cPwrLimit160L,
			prCmdPwrLimit->cPwrLimit160H,
			prCmdPwrLimit->ucFlag);
		prCmdPwrLimit++;
	}

	u4SetQueryInfoLen =
		(sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT) +
		(prCmd->ucNum) * sizeof(struct CMD_CHANNEL_POWER_LIMIT));

	/* Update domain info to chip */
	if (prCmd->ucNum <= MAX_CMD_SUPPORT_CHANNEL_NUM) {
		DBGLOG(RLM, INFO, "send CMD_ID_SET_COUNTRY_POWER_LIMIT\n");
		wlanSendSetQueryCmd(prAdapter, /* prAdapter */
			      CMD_ID_SET_COUNTRY_POWER_LIMIT, /* ucCID */
			      TRUE, /* fgSetQuery */
			      FALSE, /* fgNeedResp */
			      FALSE, /* fgIsOid */
			      NULL, /* pfCmdDoneHandler */
			      NULL, /* pfCmdTimeoutHandler */
			      u4SetQueryInfoLen, /* u4SetQueryInfoLen */
			      (uint8_t *) prCmd, /* pucInfoBuffer */
			      NULL, /* pvSetQueryBuffer */
			      0 /* u4SetQueryBufferLen */
		    );
	} else {
		DBGLOG(RLM, ERROR, "Domain: illegal power limit table\n");
		rStatus = WLAN_STATUS_FAILURE;
	}

	prAdapter->fgTxPwrLimitMask = fgMask;

	DBGLOG(RLM, INFO, "rStatus=%u, prCmd->ucNum=%u, fgTxPwrLimitMask=%u\n",
	       rStatus, prCmd->ucNum, prAdapter->fgTxPwrLimitMask);

freeMemLabel:
	if (prCmd != NULL)
		cnmMemFree(prAdapter, prCmd);

	return rStatus;
}
#endif /* CFG_SUPPORT_PWR_LIMIT_COUNTRY */

u_int8_t regd_is_single_sku_en(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.en;
#else
	return FALSE;
#endif
}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
u_int8_t rlmDomainIsCtrlStateEqualTo(enum regd_state state)
{
	return (g_mtk_regd_control.state == state) ? TRUE : FALSE;
}

enum regd_state rlmDomainGetCtrlState(void)
{
	return g_mtk_regd_control.state;
}


void rlmDomainResetActiveChannel(void)
{
	g_mtk_regd_control.n_channel_active_2g = 0;
	g_mtk_regd_control.n_channel_active_5g = 0;
}

void rlmDomainAddActiveChannel(u8 band)

{
	if (band == KAL_BAND_2GHZ)
		g_mtk_regd_control.n_channel_active_2g += 1;
	else if (band == KAL_BAND_5GHZ)
		g_mtk_regd_control.n_channel_active_5g += 1;
}

u8 rlmDomainGetActiveChannelCount(u8 band)
{
	if (band == KAL_BAND_2GHZ)
		return g_mtk_regd_control.n_channel_active_2g;
	else if (band == KAL_BAND_5GHZ)
		return g_mtk_regd_control.n_channel_active_5g;
	else
		return 0;
}

struct channel *rlmDomainGetActiveChannels(void)
{
	return g_mtk_regd_control.channels;
}

void rlmDomainSetDefaultCountryCode(void)
{
	g_mtk_regd_control.alpha2 = COUNTRY_CODE_WW;
}

void rlmDomainResetCtrlInfo(u_int8_t force)
{
	if ((g_mtk_regd_control.state == REGD_STATE_UNDEFINED) ||
	    (force == TRUE)) {
		memset(&g_mtk_regd_control, 0, sizeof(struct mtk_regd_control));

		g_mtk_regd_control.state = REGD_STATE_INIT;

		rlmDomainSetDefaultCountryCode();

#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
		g_mtk_regd_control.flag |= REGD_CTRL_FLAG_SUPPORT_LOCAL_REGD_DB;
#endif
	}
}

u_int8_t rlmDomainIsUsingLocalRegDomainDataBase(void)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	return (g_mtk_regd_control.flag & REGD_CTRL_FLAG_SUPPORT_LOCAL_REGD_DB)
		? TRUE : FALSE;
#else
	return FALSE;
#endif
}

bool rlmDomainIsSameCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 idx;
	u32 alpha2_hex = 0;

	for (idx = 0; idx < size_of_alpha2; idx++)
		alpha2_hex |= (alpha2[idx] << (idx * 8));

	return (rlmDomainGetCountryCode() == alpha2_hex) ? TRUE : FALSE;
}

void rlmDomainSetCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 idx, max;
	u8 buf_size;

	buf_size = sizeof(g_mtk_regd_control.alpha2);
	max = (buf_size < size_of_alpha2) ? buf_size : size_of_alpha2;

	g_mtk_regd_control.alpha2 = 0;

	for (idx = 0; idx < max; idx++)
		g_mtk_regd_control.alpha2 |= (alpha2[idx] << (idx * 8));

}
void rlmDomainSetDfsRegion(enum nl80211_dfs_regions dfs_region)
{
	g_mtk_regd_control.dfs_region = dfs_region;
}

enum nl80211_dfs_regions rlmDomainGetDfsRegion(void)
{
	return g_mtk_regd_control.dfs_region;
}

void rlmDomainSetTempCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 idx, max;
	u8 buf_size;

	buf_size = sizeof(g_mtk_regd_control.tmp_alpha2);
	max = (buf_size < size_of_alpha2) ? buf_size : size_of_alpha2;

	g_mtk_regd_control.tmp_alpha2 = 0;

	for (idx = 0; idx < max; idx++)
		g_mtk_regd_control.tmp_alpha2 |= (alpha2[idx] << (idx * 8));

}

enum regd_state rlmDomainStateTransition(enum regd_state request_state,
					 struct regulatory_request *pRequest)
{
	enum regd_state next_state, old_state;
	bool the_same = 0;

	old_state = g_mtk_regd_control.state;
	next_state = REGD_STATE_INVALID;

	if (old_state == REGD_STATE_INVALID)
		DBGLOG(RLM, ERROR,
		       "%s(): invalid state. trasntion is not allowed.\n",
		       __func__);

	switch (request_state) {
	case REGD_STATE_SET_WW_CORE:
		if ((old_state == REGD_STATE_SET_WW_CORE) ||
		    (old_state == REGD_STATE_INIT) ||
		    (old_state == REGD_STATE_SET_COUNTRY_USER) ||
		    (old_state == REGD_STATE_SET_COUNTRY_IE))
			next_state = request_state;
		break;
	case REGD_STATE_SET_COUNTRY_USER:
		/* Allow user to set multiple times */
		if ((old_state == REGD_STATE_SET_WW_CORE) ||
		    (old_state == REGD_STATE_INIT) ||
		    (old_state == REGD_STATE_SET_COUNTRY_USER) ||
		    (old_state == REGD_STATE_SET_COUNTRY_IE))
			next_state = request_state;
		else
			DBGLOG(RLM, ERROR, "Invalid old state = %d\n",
			       old_state);
		break;
	case REGD_STATE_SET_COUNTRY_DRIVER:
		if (old_state == REGD_STATE_SET_COUNTRY_USER) {
			/*
			 * Error.
			 * Mixing using set_country_by_user and
			 * set_country_by_driver is not allowed.
			 */
			break;
		}

		next_state = request_state;
		break;
	case REGD_STATE_SET_COUNTRY_IE:
		next_state = request_state;
		break;
	default:
		break;
	}

	if (next_state == REGD_STATE_INVALID) {
		DBGLOG(RLM, ERROR,
		       "%s():  ERROR. trasntion to invalid state. o=%x, r=%x, s=%x\n",
		       __func__, old_state, request_state, the_same);
	} else
		DBGLOG(RLM, INFO, "%s():  trasntion to state = %x (old = %x)\n",
		__func__, next_state, g_mtk_regd_control.state);

	g_mtk_regd_control.state = next_state;

	return g_mtk_regd_control.state;
}

u32 rlmDomainSearchCountrySection(u32 country_code, const struct firmware *file)
{
	u32 count;
	struct tx_pwr_section *pSection;

	if (!file) {
		DBGLOG(RLM, ERROR, "%s(): ERROR. file = null.\n", __func__);
		return 0;
	}

	count = SIZE_OF_VERSION;

	while (count < file->size) {
		pSection = (struct tx_pwr_section *)(file->data + count);
		/*prepare for the next search*/
		count += sizeof(struct tx_pwr_section);

		if ((pSection->prefix == SECTION_PREFIX) &&
			(pSection->country_code == country_code))
			return count;
	}

	return 0;
}

u_int8_t rlmDomainIsTheEndOfCountrySection(u32 start_offset,
					   const struct firmware *file)
{
	struct tx_pwr_section *pSection;

	if (start_offset >= file->size)
		return TRUE;

	pSection = (struct tx_pwr_section *)(file->data + start_offset);

	if (pSection->prefix == SECTION_PREFIX)
		return TRUE;
	else
		return FALSE;
}

/**
 * rlmDomainChannelFlagString - Transform channel flags to readable string
 *
 * @ flags: the ieee80211_channel->flags for a channel
 * @ buf: string buffer to put the transformed string
 * @ buf_size: size of the buf
 **/
void rlmDomainChannelFlagString(u32 flags, char *buf, size_t buf_size)
{
	int32_t buf_written = 0;

	if (!flags || !buf || !buf_size)
		return;

	if (flags & IEEE80211_CHAN_DISABLED) {
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "DISABLED ");
		/* If DISABLED, don't need to check other flags */
		return;
	}
	if (flags & IEEE80211_CHAN_PASSIVE_FLAG)
		LOGBUF(buf, ((int32_t)buf_size), buf_written,
		       IEEE80211_CHAN_PASSIVE_STR " ");
	if (flags & IEEE80211_CHAN_RADAR)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "RADAR ");
	if (flags & IEEE80211_CHAN_NO_HT40PLUS)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_HT40PLUS ");
	if (flags & IEEE80211_CHAN_NO_HT40MINUS)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_HT40MINUS ");
	if (flags & IEEE80211_CHAN_NO_80MHZ)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_80MHZ ");
	if (flags & IEEE80211_CHAN_NO_160MHZ)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_160MHZ ");
}

void rlmDomainParsingChannel(IN struct wiphy *pWiphy)
{
	u32 band_idx, ch_idx;
	u32 ch_count;
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *chan;
	struct channel *pCh;
	char chan_flag_string[64] = {0};


	if (!pWiphy) {
		DBGLOG(RLM, ERROR, "%s():  ERROR. pWiphy = NULL.\n", __func__);
		ASSERT(0);
		return;
	}


	/*
	 * Ready to parse the channel for bands
	 */

	rlmDomainResetActiveChannel();

	ch_count = 0;
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		sband = pWiphy->bands[band_idx];
		if (!sband)
			continue;

		for (ch_idx = 0; ch_idx < sband->n_channels; ch_idx++) {
			chan = &sband->channels[ch_idx];
			pCh = (rlmDomainGetActiveChannels() + ch_count);
			/* Parse flags and get readable string */
			rlmDomainChannelFlagString(chan->flags,
						   chan_flag_string,
						   sizeof(chan_flag_string));

			if (chan->flags & IEEE80211_CHAN_DISABLED) {
				DBGLOG(RLM, INFO,
				       "channels[%d][%d]: ch%d (freq = %d) flags=0x%x [ %s]\n",
				    band_idx, ch_idx, chan->hw_value,
				    chan->center_freq, chan->flags,
				    chan_flag_string);
				continue;
			}

			/* Allowable channel */
			if (ch_count == MAX_SUPPORTED_CH_COUNT) {
				DBGLOG(RLM, ERROR,
				       "%s(): no buffer to store channel information.\n",
				       __func__);
				break;
			}

			rlmDomainAddActiveChannel(band_idx);

			DBGLOG(RLM, INFO,
			       "channels[%d][%d]: ch%d (freq = %d) flgs=0x%x [%s]\n",
				band_idx, ch_idx, chan->hw_value,
				chan->center_freq, chan->flags,
				chan_flag_string);

			pCh->chNum = chan->hw_value;
			pCh->flags = chan->flags;

			ch_count += 1;
		}

	}
}
void rlmExtractChannelInfo(u32 max_ch_count,
			   struct acctive_channel_list *prBuff)
{
	u32 ch_count, idx;
	struct channel *pCh;

	prBuff->n_channels_2g = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	prBuff->n_channels_5g = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
	ch_count = prBuff->n_channels_2g + prBuff->n_channels_5g;

	if (ch_count > max_ch_count) {
		ch_count = max_ch_count;
		DBGLOG(RLM, WARN,
		       "%s(); active channel list is not a complete one.\n",
		       __func__);
	}

	for (idx = 0; idx < ch_count; idx++) {
		pCh = &(prBuff->channels[idx]);

		pCh->chNum = (rlmDomainGetActiveChannels() + idx)->chNum;
		pCh->flags = (rlmDomainGetActiveChannels() + idx)->flags;
	}

}

const struct ieee80211_regdomain
*rlmDomainSearchRegdomainFromLocalDataBase(char *alpha2)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	u8 idx;
	const struct mtk_regdomain *prRegd;

	idx = 0;
	while (g_prRegRuleTable[idx]) {
		prRegd = g_prRegRuleTable[idx];

		if ((prRegd->country_code[0] == alpha2[0]) &&
			(prRegd->country_code[1] == alpha2[1]) &&
			(prRegd->country_code[2] == alpha2[2]) &&
			(prRegd->country_code[3] == alpha2[3]))
			return prRegd->prRegdRules;

		idx++;
	}

	DBGLOG(RLM, ERROR,
	       "%s(): Error, Cannot find the correct RegDomain. country = %s\n",
	       __func__, alpha2);
	DBGLOG(RLM, INFO, "    Set as default WW.\n");

	return &default_regdom_ww; /*default world wide*/
#else
	return NULL;
#endif
}


const struct ieee80211_regdomain *rlmDomainGetLocalDefaultRegd(void)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	return &default_regdom_ww;
#else
	return NULL;
#endif
}
struct GLUE_INFO *rlmDomainGetGlueInfo(void)
{
	return g_mtk_regd_control.pGlueInfo;
}

bool rlmDomainIsEfuseUsed(void)
{
	return g_mtk_regd_control.isEfuseCountryCodeUsed;
}
#endif

uint32_t rlmDomainExtractSingleSkuInfoFromFirmware(IN struct ADAPTER *prAdapter,
						   IN uint8_t *pucEventBuf)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	struct SINGLE_SKU_INFO *prSkuInfo =
			(struct SINGLE_SKU_INFO *) pucEventBuf;

	g_mtk_regd_control.en = TRUE;

	if (prSkuInfo->isEfuseValid) {
		if (!rlmDomainIsUsingLocalRegDomainDataBase()) {

			DBGLOG(RLM, ERROR,
			       "%s(): Error. In efuse mode, must use local data base.\n",
			       __func__);

			ASSERT(0);
			/* force using local db if getting
			 * country code from efuse
			 */
			return WLAN_STATUS_NOT_SUPPORTED;
		}

		rlmDomainSetCountryCode((char *) &prSkuInfo->u4EfuseCountryCode,
					sizeof(prSkuInfo->u4EfuseCountryCode));
		g_mtk_regd_control.isEfuseCountryCodeUsed = TRUE;

	}
#endif

	return WLAN_STATUS_SUCCESS;
}

void rlmDomainSendInfoToFirmware(IN struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	struct regulatory_request request;
	struct regulatory_request *prReq = NULL;

	if (!regd_is_single_sku_en())
		return; /*not support single sku*/

	if (g_mtk_regd_control.isEfuseCountryCodeUsed) {
		request.initiator = NL80211_REGDOM_SET_BY_DRIVER;
		prReq = &request;
	}

	g_mtk_regd_control.pGlueInfo = prAdapter->prGlueInfo;
	mtk_reg_notify(priv_to_wiphy(prAdapter->prGlueInfo), prReq);
#endif
}

enum ENUM_CHNL_EXT rlmSelectSecondaryChannelType(struct ADAPTER *prAdapter,
						 enum ENUM_BAND band,
						 u8 primary_ch)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	u8 below_ch, above_ch;

	below_ch = primary_ch - CHNL_SPAN_20;
	above_ch = primary_ch + CHNL_SPAN_20;

	if (rlmDomainIsLegalChannel(prAdapter, band, above_ch))
		return CHNL_EXT_SCA;

	if (rlmDomainIsLegalChannel(prAdapter, band, below_ch))
		return CHNL_EXT_SCB;

#endif

	return CHNL_EXT_SCN;
}

void rlmDomainOidSetCountry(IN struct ADAPTER *prAdapter, char *country,
			    u8 size_of_country)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	struct regulatory_request request;

	if (rlmDomainIsUsingLocalRegDomainDataBase()) {
		rlmDomainSetTempCountryCode(country, size_of_country);
		request.initiator = NL80211_REGDOM_SET_BY_DRIVER;
		mtk_reg_notify(priv_to_wiphy(prAdapter->prGlueInfo), &request);
	} else {
		DBGLOG(RLM, INFO,
		       "%s(): Using driver hint to query CRDA getting regd.\n",
		       __func__);
		regulatory_hint(priv_to_wiphy(prAdapter->prGlueInfo), country);
	}
#endif
}

u32 rlmDomainGetCountryCode(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.alpha2;
#else
	return 0;
#endif
}

u32 rlmDomainGetTempCountryCode(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.tmp_alpha2;
#else
	return 0;
#endif
}

void rlmDomainAssert(u_int8_t cond)
{
	/* bypass this check because single sku is not enable */
	if (!regd_is_single_sku_en())
		return;

	if (!cond) {
		WARN_ON(1);
		DBGLOG(RLM, ERROR, "[WARNING!!] RLM unexpected case.\n");
	}

}
