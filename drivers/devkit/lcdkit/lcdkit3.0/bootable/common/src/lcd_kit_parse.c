#include "lcd_kit_common.h"

#ifdef MTK_LCD_KIT_SUPPORT
#include <libfdt.h>
#define mem_alloc malloc
#else
#define mem_alloc alloc
#endif

int conver2le(u32 *src, u32 *desc, u32 len)
{
	int i = 0;
	u32 temp = 0;

	if ((src == NULL) || (desc == NULL)) {
		LCD_KIT_ERR("src or desc is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < len; i++) {
		temp = ((src[i] & 0xff) << 24);
		temp |= (((src[i] >> 8) & 0xff) << 16);
		temp |= (((src[i] >> 16) & 0xff) << 8);
		temp |= ((src[i] >> 24) & 0xff);
		desc[i] = temp;
		temp = 0;
	}
	return LCD_KIT_OK;
}

int lcd_kit_parse_dcs_cmds(const char* compatible, const char* propertyname, char* link_key,
								struct lcd_kit_dsi_panel_cmds* pcmds)
{
	#define LINK_STATE_LEN 20
	int i = 0, j = 0, len = 0, cnt = 0;
	u32 blen = 0;
	char* buf = NULL, *bp = NULL;
	struct lcd_kit_dsi_cmd_desc_header* dchdr = NULL;
	int* data = NULL;
	struct lcd_kit_adapt_ops* adpat_ops = NULL;
	char link_key_str[LINK_STATE_LEN + 1] = {0};

	adpat_ops = lcd_kit_get_adapt_ops();
	if (!adpat_ops) {
		LCD_KIT_ERR("get adpat_ops error\n");
		return LCD_KIT_FAIL;
	}
	if (adpat_ops->get_data_by_property) {
		adpat_ops->get_data_by_property(compatible, propertyname, &data, &blen);
	}
	/* scan dcs commands */
	if (adpat_ops->buf_trans) {
		adpat_ops->buf_trans((char*)data, blen, &buf, &len);
	}
	bp = buf;
	while (len >= (int)sizeof(struct lcd_kit_dsi_cmd_desc_header)) {
		dchdr = (struct lcd_kit_dsi_cmd_desc_header*)bp;
		//dchdr->dlen = ntohs(dchdr->dlen);
		bp += sizeof(struct lcd_kit_dsi_cmd_desc_header);
		len -= (int)sizeof(struct lcd_kit_dsi_cmd_desc_header);

		if (dchdr->dlen > len) {
			LCD_KIT_ERR("cmd = 0x%x parse error, len = %d\n", dchdr->dtype, dchdr->dlen);
			goto exit_free;
		}

		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}
	if (len != 0) {
		LCD_KIT_ERR("dcs_cmd parse error! cmd len = %d\n", len);
		goto exit_free;
	}

	pcmds->cmds = mem_alloc(cnt * sizeof(struct lcd_kit_dsi_cmd_desc));
	if (!pcmds->cmds) {
		goto exit_free;
	}

	pcmds->cmd_cnt = cnt;
	bp = buf;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct lcd_kit_dsi_cmd_desc_header*)bp;
		len -= (int)sizeof(struct lcd_kit_dsi_cmd_desc_header);
		bp += sizeof(struct lcd_kit_dsi_cmd_desc_header);
		pcmds->cmds[i].dtype    = dchdr->dtype;
		pcmds->cmds[i].last		= dchdr->last;
		pcmds->cmds[i].vc       = dchdr->vc;
		pcmds->cmds[i].ack      = dchdr->ack;
		pcmds->cmds[i].wait     = dchdr->wait;
		pcmds->cmds[i].waittype = dchdr->waittype;
		pcmds->cmds[i].dlen     = dchdr->dlen;
		pcmds->cmds[i].payload  = mem_alloc(dchdr->dlen * sizeof(char));
		for (j = 0; j < dchdr->dlen; j++) {
			pcmds->cmds[i].payload[j] = bp[j];
		}
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}
	/*Set default link state to HS Mode*/
	pcmds->link_state = LCD_KIT_DSI_HS_MODE;
	if (link_key) {
		if (adpat_ops->get_string_by_property) {
			adpat_ops->get_string_by_property(compatible, link_key, link_key_str, sizeof(link_key_str));
		}
		if (!strncmp(link_key_str, "dsi_lp_mode", strlen("dsi_lp_mode"))) {
			pcmds->link_state = LCD_KIT_DSI_LP_MODE;
		} else {
			pcmds->link_state = LCD_KIT_DSI_HS_MODE;
		}
	}
	/*free buf*/
	free(buf);
	return LCD_KIT_OK;
exit_free:
	free(buf);
	return LCD_KIT_FAIL;

}

int lcd_kit_parse_get_u32_default(const char* compatible, const char* propertyname, u32* value, u32 def_val)
{
	struct lcd_kit_adapt_ops* adpat_ops = NULL;
	int* data = NULL;
	u32 blen = 0;
	int retTmp;

	adpat_ops = lcd_kit_get_adapt_ops();
	if (!adpat_ops) {
		LCD_KIT_ERR("get adpat_ops error\n");
		return LCD_KIT_FAIL;
	}
	if (adpat_ops->get_data_by_property) {
		adpat_ops->get_data_by_property(compatible, propertyname, &data, &blen);
	}
	if (!data) {
		LCD_KIT_INFO("data is null\n");
		return LCD_KIT_FAIL;
	}
	retTmp = *data;
	retTmp = (int)fdt32_to_cpu((uint32_t)retTmp);
	*value = retTmp;
	return LCD_KIT_OK;

}

int lcd_kit_parse_get_u8_default(const char *compatible,
	const char *propertyname, u8 *value, u32 def_val)
{
	struct lcd_kit_adapt_ops *adpat_ops = NULL;
	int *data = NULL;
	u32 blen = 0;
	int retTmp;

	adpat_ops = lcd_kit_get_adapt_ops();
	if (!adpat_ops) {
		LCD_KIT_ERR("get adpat_ops error\n");
		return LCD_KIT_FAIL;
	}
	if (adpat_ops->get_data_by_property) {
		adpat_ops->get_data_by_property(compatible, propertyname,
			&data, &blen);
	}
	if (!data) {
		LCD_KIT_ERR("data is null\n");
		return LCD_KIT_FAIL;
	}
	retTmp = *data;
	retTmp = (int)fdt32_to_cpu((uint32_t)retTmp);
	*value = retTmp;
	return LCD_KIT_OK;
}

int lcd_kit_parse_array(const char* compatible, const char* propertyname,
								struct lcd_kit_array_data* out)
{
	struct lcd_kit_adapt_ops* adpat_ops = NULL;
	u32 blen = 0;
	int* data = NULL;
	u32* buf = NULL;
	int ret = 0;

	adpat_ops = lcd_kit_get_adapt_ops();
	if (!adpat_ops) {
		LCD_KIT_ERR("get adpat_ops error\n");
		return LCD_KIT_FAIL;
	}
	if (adpat_ops->get_data_by_property) {
		ret = adpat_ops->get_data_by_property(compatible, propertyname, &data, &blen);
    	if (ret < 0) {
    		return LCD_KIT_FAIL;
    	}
	}

	blen = blen/4;
	buf = (u32*)mem_alloc(blen * sizeof(u32));
	if (!buf) {
		LCD_KIT_ERR("alloc buf fail\n");
		return LCD_KIT_FAIL;
	}
	memcpy((void *)buf, (const void *)data, blen * sizeof(u32));
	conver2le(buf, buf, blen);
	out->buf = buf;
	out->cnt = blen;
	return LCD_KIT_OK;

}

int lcd_kit_parse_arrays(const char* compatible, const char* propertyname,
								struct lcd_kit_arrays_data* out, u32 num)
{
	struct lcd_kit_adapt_ops* adpat_ops = NULL;
	int i = 0, cnt = 0;
	u32 blen, length;
	int* data = NULL, *bp = NULL;
	int* buf = NULL;

	adpat_ops = lcd_kit_get_adapt_ops();
	if (!adpat_ops) {
		LCD_KIT_ERR("get adpat_ops error\n");
		return LCD_KIT_FAIL;
	}
	if (adpat_ops->get_data_by_property) {
		adpat_ops->get_data_by_property(compatible, propertyname, &data, &blen);
	}
	blen = blen/4;
	length = blen;
	buf = (u32*)mem_alloc(blen * sizeof(u32));
	if (!buf) {
		LCD_KIT_ERR("alloc buf fail\n");
		return LCD_KIT_FAIL;
	}
	memcpy((void *)buf, (const void *)data, blen * sizeof(u32));
	conver2le((unsigned int *)buf, (unsigned int *)buf, blen);
	while (length >= num) {
		if (num > length) {
			LCD_KIT_ERR("data length = %x error\n", length);
			return LCD_KIT_FAIL;
		}
		length -= num;
		cnt++;
	}
	if (length != 0) {
		LCD_KIT_ERR("dts data parse error! data len = %d\n", length);
		return LCD_KIT_FAIL;
	}
	out->cnt = cnt;
	out->arry_data = (struct lcd_kit_array_data *)mem_alloc(cnt * sizeof(struct lcd_kit_array_data));
	if (!out->arry_data) {
		LCD_KIT_ERR("out->arry_data is null\n");
		return LCD_KIT_FAIL;
	}
	bp = buf;
	for (i = 0; i < cnt; i++) {
		out->arry_data[i].buf = bp;
		out->arry_data[i].cnt = num;
		bp += num;
	}
	return LCD_KIT_OK;
}
