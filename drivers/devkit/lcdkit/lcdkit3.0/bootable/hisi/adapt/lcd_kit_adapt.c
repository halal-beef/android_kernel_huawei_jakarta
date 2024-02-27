#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"
#include "lcd_kit_common.h"
#include "lcd_kit_adapt.h"

static void lcd_kit_dump_cmd(struct dsi_cmd_desc* cmd)
{
	int i = 0;

	LCD_KIT_DEBUG("cmd->dtype = 0x%x\n", cmd->dtype);
	LCD_KIT_DEBUG("cmd->vc = 0x%x\n", cmd->vc);
	LCD_KIT_DEBUG("cmd->wait = 0x%x\n", cmd->wait);
	LCD_KIT_DEBUG("cmd->waittype = 0x%x\n", cmd->waittype);
	LCD_KIT_DEBUG("cmd->dlen = 0x%x\n", cmd->dlen);
	LCD_KIT_DEBUG("cmd->payload:\n");
	if (g_lcd_kit_msg_level >= MSG_LEVEL_DEBUG) {
		for (i = 0; i < cmd->dlen; i++) {
			LCD_KIT_DEBUG("0x%x\n", cmd->payload[i]);
		}
	}
}

static int lcd_cmds_to_dsi_cmds_ex(struct lcd_kit_dsi_cmd_desc *lcd_kit_cmds,
	struct dsi_cmd_desc *cmd, int link_state)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null point!\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->dtype;
	if (link_state == LCD_KIT_DSI_LP_MODE)
		cmd->dtype  |= GEN_VID_LP_CMD;
	cmd->vc =  lcd_kit_cmds->vc;
	cmd->wait =  lcd_kit_cmds->wait;
	cmd->waittype =  lcd_kit_cmds->waittype;
	cmd->dlen =  lcd_kit_cmds->dlen;
	cmd->payload = lcd_kit_cmds->payload;
	lcd_kit_dump_cmd(cmd);
	return LCD_KIT_OK;
}

static int lcd_kit_cmds_to_dsi_cmds(struct lcd_kit_dsi_cmd_desc* lcd_kit_cmds, struct dsi_cmd_desc* cmd, int link_state)
{
	if (lcd_kit_cmds == NULL) {
		LCD_KIT_ERR("lcd_kit_cmds is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (cmd == NULL) {
		LCD_KIT_ERR("cmd is null point!\n");
		return LCD_KIT_FAIL;
	}
	cmd->dtype = lcd_kit_cmds->dtype;
	if(link_state == LCD_KIT_DSI_LP_MODE){
		cmd->dtype  |= GEN_VID_LP_CMD;
	}
	cmd->vc =  lcd_kit_cmds->vc;
	cmd->waittype =  lcd_kit_cmds->waittype;
	cmd->dlen =  lcd_kit_cmds->dlen;
	cmd->payload = lcd_kit_cmds->payload;
	lcd_kit_dump_cmd(cmd);
	return LCD_KIT_OK;
}

static int lcd_kit_cmd_is_write(struct dsi_cmd_desc* cmd)
{
	int ret = LCD_KIT_FAIL;

	switch (DSI_HDR_DTYPE(cmd->dtype)) {
		case DTYPE_GEN_WRITE:
		case DTYPE_GEN_WRITE1:
		case DTYPE_GEN_WRITE2:
		case DTYPE_GEN_LWRITE:
		case DTYPE_DCS_WRITE:
		case DTYPE_DCS_WRITE1:
		case DTYPE_DCS_LWRITE:
		case DTYPE_DSC_LWRITE:
			ret = 1;
			break;
		case DTYPE_GEN_READ:
		case DTYPE_GEN_READ1:
		case DTYPE_GEN_READ2:
		case DTYPE_DCS_READ:
			ret = 0;
			break;
		default:
			ret = -1;
			break;
	}
	return ret;
}

static int lcd_kit_get_data_by_property(const char *compatible,
	const char *propertyname, int **data, u32 *len)
{
	void* fdt = NULL;
	struct fdt_operators* fdt_ops = NULL;
	struct dtb_operators *dtimage_ops = NULL;
	struct fdt_property* property = NULL;
	int offset = 0;

	fdt_ops = get_operators(FDT_MODULE_NAME_STR);
	if (NULL == fdt_ops) {
		LCD_KIT_ERR("can't get fdt operators\n");
		return LCD_KIT_FAIL;
	}
	dtimage_ops = get_operators(DTIMAGE_MODULE_NAME_STR);
	if (NULL == dtimage_ops) {
		LCD_KIT_ERR("failed to get dtimage operators\n");
		return LCD_KIT_FAIL;
	}
	fdt_ops->fdt_operate_lock();
	fdt = dtimage_ops->get_dtb_addr();
	if(NULL == fdt){
		LCD_KIT_ERR("[%s]: fdt is null!!!\n",__func__);
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}
	offset = fdt_ops->fdt_node_offset_by_compatible(fdt, 0, compatible);
	if (offset < 0) {
		LCD_KIT_ERR("-----can not find %s node by compatible \n", compatible);
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}

	property = (struct fdt_property *)fdt_ops->fdt_get_property(
		fdt, offset, propertyname, (int *)len);
	if (!property) {
		LCD_KIT_INFO("-----can not find %s \n", propertyname);
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}

	if (!property->data) {
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}
	*data = (int *)property->data;
	fdt_ops->fdt_operate_unlock();
	return LCD_KIT_OK;
}

static int lcd_kit_get_string_by_property(const char* compatible, const char* propertyname, char *out_str, unsigned int length)
{
	int offset = 0;
	int len = 0;
	void *fdt = NULL;
	struct fdt_property *property = NULL;
	struct fdt_operators* fdt_ops = NULL;
	struct dtb_operators *dtimage_ops = NULL;

	if (!compatible || !propertyname || !out_str) {
		LCD_KIT_ERR("null pointer,compatible:%s, propertyname:%s, out_str:%s\n", compatible, propertyname, out_str);
		return LCD_KIT_FAIL;
	}
	fdt_ops = get_operators(FDT_MODULE_NAME_STR);
	if (NULL == fdt_ops) {
		LCD_KIT_ERR("can't get fdt operators\n");
		return LCD_KIT_FAIL;
	}
	dtimage_ops = get_operators(DTIMAGE_MODULE_NAME_STR);
	if (NULL == dtimage_ops) {
		LCD_KIT_ERR("failed to get dtimage operators\n");
		return LCD_KIT_FAIL;
	}
	fdt_ops->fdt_operate_lock();
	fdt = dtimage_ops->get_dtb_addr();
	if(NULL == fdt){
		LCD_KIT_ERR("[%s]: fdt is null!!!\n",__func__);
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}
	offset = fdt_ops->fdt_node_offset_by_compatible(fdt, 0, compatible);
	if (offset < 0) {
		LCD_KIT_ERR("-----can not find %s node by compatible \n", compatible);
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}
	property = (struct fdt_property *)fdt_ops->fdt_get_property(
		fdt, offset, propertyname, &len);
	if (!property) {
		LCD_KIT_INFO("-----can not find %s \n", propertyname);
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}
	if (!property->data) {
		fdt_ops->fdt_operate_unlock();
		return LCD_KIT_FAIL;
	}
	length = length > strlen((char *)property->data)?strlen((char *)property->data):length;
	memcpy_s(out_str, (unsigned int)length + 1, (char *)property->data, (unsigned int)length + 1);
	fdt_ops->fdt_operate_unlock();
	return LCD_KIT_OK;
}

int lcd_kit_dsi_diff_cmds_tx(void* hld, struct lcd_kit_dsi_panel_cmds* dsi0_cmds,
			struct lcd_kit_dsi_panel_cmds *dsi1_cmds)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dsi_cmd_desc *dsi0_cmd = NULL;
	struct dsi_cmd_desc *dsi1_cmd = NULL;

	if ((dsi0_cmds == NULL)||(dsi1_cmds == NULL)) {
		LCD_KIT_ERR("cmd cnt is 0!\n");
		return LCD_KIT_FAIL;
	}
	if ((dsi0_cmds->cmds == NULL || dsi0_cmds->cmd_cnt <= 0)
		|| (dsi1_cmds->cmds == NULL || dsi1_cmds->cmd_cnt <= 0)) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type*) hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	dsi0_cmd = alloc(sizeof(struct dsi_cmd_desc) * dsi0_cmds->cmd_cnt);
	if(!dsi0_cmd)
	{
		LCD_KIT_ERR("dsi0_cmd is null!\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < dsi0_cmds->cmd_cnt; i++) {
		lcd_cmds_to_dsi_cmds_ex(&dsi0_cmds->cmds[i],
			(dsi0_cmd + i), dsi0_cmds->link_state);
	}
	dsi1_cmd = alloc(sizeof(struct dsi_cmd_desc) * dsi1_cmds->cmd_cnt);
	if(!dsi1_cmd)
	{
		LCD_KIT_ERR("dsi1_cmd is null!\n");
		free(dsi0_cmd);
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < dsi1_cmds->cmd_cnt; i++) {
		lcd_cmds_to_dsi_cmds_ex(&dsi1_cmds->cmds[i],
			(dsi1_cmd + i), dsi1_cmds->link_state);
	}
	(void)mipi_dual_dsi_cmds_tx(dsi0_cmd, dsi0_cmds->cmd_cnt,
			hisifd->mipi_dsi0_base, dsi1_cmd, dsi1_cmds->cmd_cnt,
			hisifd->mipi_dsi1_base, EN_DSI_TX_NORMAL_MODE);
	free(dsi0_cmd);
	free(dsi1_cmd);
	return ret;
}
/*
 *  dsi send cmds
*/
int lcd_kit_dsi_cmds_tx(void* hld, struct lcd_kit_dsi_panel_cmds* cmds)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct hisi_fb_data_type* hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;

	if (cmds == NULL) {
		LCD_KIT_ERR("cmd cnt is 0!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(struct dsi_cmd_desc));

	if (cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = (struct hisi_fb_data_type*) hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	/*switch to LP mode*/
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE) {
		lcd_kit_set_mipi_tx_link(hisifd, LCD_KIT_DSI_LP_MODE);
	}
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, cmds->link_state);
		if (!lcd_kit_dsi_fifo_is_full(hisifd->mipi_dsi0_base)) {
			mipi_dsi_cmds_tx(&dsi_cmd, 1, hisifd->mipi_dsi0_base);
		}
		if (disp_info->dsi1_cmd_support) {
			if (!lcd_kit_dsi_fifo_is_full(hisifd->mipi_dsi1_base)) {
				mipi_dsi_cmds_tx(&dsi_cmd, 1, hisifd->mipi_dsi1_base);
			}
		}
		lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype);
	}
	/*switch to HS mode*/
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE) {
		lcd_kit_set_mipi_tx_link(hisifd, LCD_KIT_DSI_HS_MODE);
	}
	return ret;

}

/*
 *  dsi receive cmds
*/
int lcd_kit_dsi_cmds_rx(void* hld, uint8_t* out, struct lcd_kit_dsi_panel_cmds* cmds)
{
	#define READ_MAX 100
	uint32_t tmp_value[READ_MAX] = {0};
	int ret = LCD_KIT_OK;
	int i = 0;
	int dlen = 0;
	int cnt = 0;
	int start_index = 0;
	struct hisi_fb_data_type* hisifd = NULL;
	struct dsi_cmd_desc dsi_cmd;

	hisifd = (struct hisi_fb_data_type*) hld;
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null!\n");
		return LCD_KIT_FAIL;
	}
	if (cmds == NULL) {
		LCD_KIT_ERR("cmds is null!\n");
		return LCD_KIT_FAIL;
	}
	if (cmds->cmds == NULL || cmds->cmd_cnt <= 0) {
		LCD_KIT_ERR("cmds is null, or cmds->cmd_cnt <= 0!\n");
		return LCD_KIT_FAIL;
	}
	memset(&dsi_cmd, 0, sizeof(struct dsi_cmd_desc));
	/*switch to LP mode*/
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE) {
		lcd_kit_set_mipi_rx_link(hisifd, LCD_KIT_DSI_LP_MODE);
	}
	for (i = 0; i < cmds->cmd_cnt; i++) {
		lcd_kit_cmds_to_dsi_cmds(&cmds->cmds[i], &dsi_cmd, cmds->link_state);
		if (lcd_kit_cmd_is_write(&dsi_cmd)) {
			if (!lcd_kit_dsi_fifo_is_full(hisifd->mipi_dsi0_base)) {
				mipi_dsi_cmds_tx(&dsi_cmd, 1, hisifd->mipi_dsi0_base);
				lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype);
			} else {
				LCD_KIT_ERR("mipi write error\n");
				ret = LCD_KIT_FAIL;
				break;
			}
		} else {
			if (!lcd_kit_dsi_fifo_is_full(hisifd->mipi_dsi0_base)) {
				ret = mipi_dsi_lread(tmp_value, &dsi_cmd, dsi_cmd.dlen, (char *)(unsigned long)hisifd->mipi_dsi0_base);
				if (ret) {
					LCD_KIT_ERR("mipi read error\n");
					break;
				}
				start_index = 0;
				if (dsi_cmd.dlen > 1) {
					start_index = (int)dsi_cmd.payload[1];
				}
				for (dlen = 0; dlen < dsi_cmd.dlen; dlen++) {
					if (dlen < (start_index - 1)){
						continue;
					}
					switch (dlen % 4) {
					case 0:
						out[cnt] = (uint8_t)(tmp_value[dlen / 4] & 0xFF);
						break;
					case 1:
						out[cnt] = (uint8_t)((tmp_value[dlen / 4] >> 8) & 0xFF);
						break;
					case 2:
						out[cnt] = (uint8_t)((tmp_value[dlen / 4] >> 16) & 0xFF);
						break;
					case 3:
						out[cnt] = (uint8_t)((tmp_value[dlen / 4] >> 24) & 0xFF);
						break;
					default:
						break;
					}
					cnt++;
				}
				lcd_kit_delay(cmds->cmds[i].wait, cmds->cmds[i].waittype);
			} else {
				LCD_KIT_ERR("mipi write error\n");
				ret = LCD_KIT_FAIL;
				break;
			}
		}
	}
	/*switch to HS mode*/
	if (cmds->link_state == LCD_KIT_DSI_LP_MODE) {
		lcd_kit_set_mipi_rx_link(hisifd, LCD_KIT_DSI_HS_MODE);
	}
	return ret;
}

static int lcd_kit_buf_trans(const char* inbuf, int inlen, char** outbuf, int* outlen)
{
	char* buf;
	int i;
	int bufsize = inlen;

	if (!inbuf) {
		LCD_KIT_ERR("inbuf is null point!\n");
		return LCD_KIT_FAIL;
	}
	/*The property is 4bytes long per element in cells: <>*/
	bufsize = bufsize / 4;
	/*If use bype property: [], this division should be removed*/
	buf = alloc(sizeof(char) * bufsize);
	if (!buf) {
		LCD_KIT_INFO("buf is null point!\n");
		return LCD_KIT_FAIL;
	}
	//For use cells property: <>
	for (i = 0; i < bufsize; i++) {
		buf[i] = inbuf[i * 4 + 3];
	}
	*outbuf = buf;
	*outlen = bufsize;
	return LCD_KIT_OK;
}


static int lcd_kit_gpio_enable(u32 type)
{

	lcd_kit_gpio_tx(type, GPIO_REQ);
	lcd_kit_gpio_tx(type, GPIO_HIGH);
	return LCD_KIT_OK;
}

static int lcd_kit_gpio_disable(u32 type)
{
	lcd_kit_gpio_tx(type, GPIO_LOW);
	lcd_kit_gpio_tx(type, GPIO_RELEASE);
	return LCD_KIT_OK;
}

static int lcd_kit_regulator_enable(u32 type)
{
	int ret = LCD_KIT_OK;

	switch(type)
	{
		case LCD_KIT_VCI:
		case LCD_KIT_IOVCC:
		case LCD_KIT_VDD:
			ret = lcd_kit_pmu_ctrl(type, 1);
			break;
		case LCD_KIT_VSP:
		case LCD_KIT_VSN:
		case LCD_KIT_BL:
			ret = lcd_kit_charger_ctrl(type, 1);
			break;
		default:
			ret = LCD_KIT_FAIL;
			LCD_KIT_ERR("regulator type:%d not support\n", type);
			break;
	}
	return ret;
}

static int lcd_kit_regulator_disable(u32 type)
{
	int ret = LCD_KIT_OK;

	switch(type)
	{
		case LCD_KIT_VCI:
		case LCD_KIT_IOVCC:
			ret = lcd_kit_pmu_ctrl(type, 0);
			break;
		case LCD_KIT_VSP:
		case LCD_KIT_VSN:
		case LCD_KIT_BL:
			ret = lcd_kit_charger_ctrl(type, 0);
			break;
		default:
			LCD_KIT_ERR("regulator type:%d not support\n", type);
			break;
	}
	return ret;
}

int lcd_kit_change_dts_status_by_compatible(char *name)
{
	struct fdt_operators *fdt_ops = NULL;
	struct dtb_operators *dtimage_ops = NULL;
	void *fdt = NULL;
	int ret = 0;
	int offset = 0;
	if(NULL == name){
		return ret;
	}
	fdt_ops = get_operators(FDT_MODULE_NAME_STR);
	if(!fdt_ops){
		LCD_KIT_ERR("can not get fdt_ops!\n");
		return ret;
	}
	dtimage_ops = get_operators(DTIMAGE_MODULE_NAME_STR);
	if (NULL == dtimage_ops){
		LCD_KIT_ERR("failed to get dtimage operators!\n");
		return ret;
	}
	fdt_ops->fdt_operate_lock();
	fdt = dtimage_ops->get_dtb_addr();
	if (NULL == fdt){
		LCD_KIT_ERR("msl_debug failed to get fdt addr!\n");
		fdt_ops->fdt_operate_unlock();
		return ret;
	}
	ret = fdt_ops->fdt_open_into(fdt, fdt, DTS_SPACE_LEN);
	if (ret < 0){
		LCD_KIT_ERR("fdt_open_into failed!\n");
		fdt_ops->fdt_operate_unlock();
		return ret;
	}
	ret = fdt_ops->fdt_node_offset_by_compatible(fdt, 0, name);
	if (ret < 0){
		LCD_KIT_ERR("Could not find panel node, change fb dts failed\n");
		fdt_ops->fdt_operate_unlock();
		return ret;
	}

	offset = ret;

	ret = fdt_ops->fdt_setprop_string(fdt, offset, (const char*)"status", (const void*)"ok");
	if (ret) {
		LCD_KIT_ERR("Cannot update lcd panel type(errno=%d)!\n", ret);
	}
	fdt_ops->fdt_operate_unlock();
	return ret;
}

struct lcd_kit_adapt_ops adapt_ops = {
	.mipi_tx = lcd_kit_dsi_cmds_tx,
	.daul_mipi_diff_cmd_tx = lcd_kit_dsi_diff_cmds_tx,
	.mipi_rx = lcd_kit_dsi_cmds_rx,
	.gpio_enable = lcd_kit_gpio_enable,
	.gpio_disable = lcd_kit_gpio_disable,
	.regulator_enable = lcd_kit_regulator_enable,
	.regulator_disable = lcd_kit_regulator_disable,
	.buf_trans = lcd_kit_buf_trans,
	.get_data_by_property = lcd_kit_get_data_by_property,
	.get_string_by_property = lcd_kit_get_string_by_property,
	.change_dts_status_by_compatible = lcd_kit_change_dts_status_by_compatible,
};
int lcd_kit_adapt_init(void)
{
	int ret = LCD_KIT_OK;
	ret = lcd_kit_adapt_register(&adapt_ops);
	return ret;
}
