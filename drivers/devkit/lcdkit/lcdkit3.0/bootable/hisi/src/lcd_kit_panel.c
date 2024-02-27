#include "lcd_kit_disp.h"
#include "lcd_kit_panel.h"
#include "panel/alps_jdi_nt36860c.c"

static struct lcd_kit_panel_map panel_map[] = {
	//{PANEL_JDI_NT36860C, jdi_nt36860c_proble},
};

struct lcd_kit_panel_ops *g_lcd_kit_panel_ops = NULL;
int lcd_kit_panel_ops_register(struct lcd_kit_panel_ops * ops)
{
	if (!g_lcd_kit_panel_ops) {
		g_lcd_kit_panel_ops = ops;
		LCD_KIT_INFO("ops register success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops have been registered!\n");
	return LCD_KIT_FAIL;
}
int lcd_kit_panel_ops_unregister(struct lcd_kit_panel_ops * ops)
{
	if (g_lcd_kit_panel_ops == ops) {
		g_lcd_kit_panel_ops = NULL;
		LCD_KIT_INFO("ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_panel_ops *lcd_kit_panel_get_ops(void)
{
	return g_lcd_kit_panel_ops;
}

int lcd_kit_panel_init(void)
{
	int i = 0;
	int ret = LCD_KIT_OK;

	if (!disp_info->compatible) {
		LCD_KIT_ERR("compatible is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < ARRAY_SIZE(panel_map); i++) {
		if (!strncmp(disp_info->compatible, panel_map[i].compatible, strlen(disp_info->compatible))) {
			ret = panel_map[i].callback();
			if (ret) {
				LCD_KIT_ERR("ops init fail\n");
				return LCD_KIT_FAIL;
			}
			break;
		}
	}
	/*init ok*/
	if (i >= ARRAY_SIZE(panel_map)) {
		LCD_KIT_INFO("not find ops\n");
	}
	return LCD_KIT_OK;
}
