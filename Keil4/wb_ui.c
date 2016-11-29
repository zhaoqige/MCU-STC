/*
 * WB UI CONTROL
 * by Qige from 6Harmoncs @ 2016.11.06
 *
 *
 *
 * pay attention to these conditions
 * 1. when edit, should not echo "loading"
 * 2. when edit, should not change the value from uart
 * 3. 
 */

//#define UART_PRINT_PARSE_RESULT


#include <stdio.h>
#include <string.h>

#include <reg51.h>
#include <intrins.h>

//* components
#include "stc_define.h"
#include "stc_uart.h"
#include "std_drv_1602.h"

extern void delay(uint x);

#include "std_buffer.h"
extern uchar std_buffer[];


//* prototype
//#include "wb_proto.h"
#include "wb_ui.h"


#define LCD_EDIT_CHANNEL_MARK_POS	0x45
#define LCD_EDIT_CHANBW_MARK_POS	0x4B
#define LCD_EDIT_REGION_MARK_POS	0x4F
#define LCD_EDIT_TXPWR_MARK_POS		0x4B
#define LCD_EDIT_MODE_MARK_POS		0x4F

#define BTN_DETECT_MASK 			0x1E
#define BTN_1_MASK 					0x1C	// M
#define BTN_2_MASK 					0x1A	// +
#define BTN_3_MASK 					0x16	// -
#define BTN_4_MASK 					0x0E	// S
#define BTN_5_MASK 					0x0C	// S M, exit busy/loading
#define BTN_6_MASK					0x12	// + -, 
#define BTN_7_MASK					0x14	// M -
#define BTN_8_MASK					0x0A	// S +
#define BTN_9_MASK					0x18	// M +
#define BTN_10_MASK					0x06	// S -


#define BTN_DELAY 					20000

enum UI_BTN {
	UI_BTN_NONE = 0,
	UI_BTN_SW = 1,
	UI_BTN_PLUS,
	UI_BTN_MINUS,
	UI_BTN_SET,
	UI_BTN_DEBUG,
	UI_BTN_X1,
	UI_BTN_X2,
	UI_BTN_X3,
	UI_BTN_X4,
	UI_BTN_X5
};

enum UI_STAT {
	UI_LOADING,
	UI_PRINT_PAGE_STS,
	UI_PRINT_PAGE_GWS,
	UI_PRINT_EDIT_TXPWR,
	UI_PRINT_EDIT_CHANNEL,
	UI_PRINT_EDIT_CHANBW,
	UI_PRINT_EDIT_REGION,
	UI_PRINT_EDIT_MODE
};

enum GWS_MODE {
	GWS_MODE_MESH = 0,
	GWS_MODE_STA,
	GWS_MODE_AP,
	GWS_MODE_ADHOC
};


//* counter of un-received uart msg from host
#define UI_UART_TIMEOUT			100
#define WBP_MSG_LENGTH_MIN		10 // +wbsts:,,,
#define WBP_SIGNAL_MAX			5 
#define WBP_SIGNAL_VALUE_MAX	99

#define LCD_LINE_LENGTH			16

#define GWS_TXPWR_MIN			0
#define GWS_TXPWR_MAX			40

#define GWS_R1FREQ_START		474
#define GWS_R1CHAN_BW			8
#define GWS_R1CH_MIN			21
#define GWS_R1CH_MAX			60

#define GWS_R0FREQ_START		473
#define GWS_R0CHAN_BW			6
#define GWS_R0CH_MIN			14
#define GWS_R0CH_MAX			60

#define GWS_CHANBW_MIN			1
#define GWS_CHANBW_MAX			24

#define GWS_REGION_DFL			1
#define GWS_CHANNEL_DFL			GWS_R1CH_MIN
#define GWS_CHANBW_DFL			8
#define GWS_RXGAIN_DFL			10

#define GWS_MODE_MIN			0
#define GWS_MODE_MAX			3

#define GWS_NOISE_WP			-101
#define GWS_NOISE_MIN			-110


static uint ui_host_timeout 	= 0;
static uint ui_screen = UI_LOADING, ui_screen_page = 0;
static uint ui_screen_echo_change_counter = 0;
static uchar gws_cmd[STD_BUFFER_LENGTH] = {""};
static struct {
	int	is_data_changed;

	int chanbw;
	int rxgain;

	int	mode;
	int region;
	int	channel;

	int	txpwr;
	int	is_tx_on;

	int		signal[WBP_SIGNAL_MAX];
	int		noise;
} gws_ui;

struct {
	int is_data_changed;
	int chanbw;
	int rxgain;
	int channel;
	int _noise_;
	int txpwr;
	int mode;
	int region;
} gws_read_sts;

struct {
	uint is_data_changed;
	int	 _signal_[WBP_SIGNAL_MAX];
} gws_read_sig;


void _debug_buffer(uint x)
{
	uint buffer_length = readDataLength();

	memset(gws_cmd, 0, sizeof(gws_cmd));	
	sprintf(gws_cmd, "\n\n%d buffer content: (%d)\n", x, buffer_length);
	uart_send(gws_cmd);	
	uart_send(std_buffer);
	uart_send_byte('\n');
}


extern void delay(uint x);

//* main function
//* - decide ui display
void ui_calc_ui(void)
{
	uint i, btn_no = 0, buffer_length = 0, wbp_data_move = 0;
	uchar act[STD_BUFFER_LENGTH] = {""}, wbp_data[STD_BUFFER_LENGTH] = {""};

	btn_no = ui_detect_btn();
	ui_calc_pages(btn_no);
	ui_echo_page();

	delay(2);
	buffer_length = readDataLength();
	delay(2);
	if (buffer_length > 0) {
		//_debug_buffer(1);
		if (strstr(std_buffer, "+")) {
			//uart_send("msg head included\n");
			if (buffer_length > WBP_MSG_LENGTH_MIN) {
				//uart_send("msg valid\n");
				//_debug_buffer(3);

				wbp_parser(std_buffer, buffer_length, &wbp_data_move);
				delay(2);
				//wbp_data_move = 10;
				moveBuffer(wbp_data_move);
				delay(5);

				//_debug_buffer(4);
			} else {
				//uart_send("data is too short\n");
			}
		} else {
			//_debug_buffer(2);
			//uart_send("no msg head\n");
			initBuffer();
		}
	} else {
		//uart_send("no data\n");
		initBuffer();
	}


	if (ui_screen < UI_PRINT_EDIT_TXPWR) {
		if (gws_read_sts.is_data_changed) {
			gws_ui.mode = gws_read_sts.mode;
			gws_ui.chanbw = gws_read_sts.chanbw;
			gws_ui.region = (gws_read_sts.region != 0 ? 1 : 0);
			gws_ui.channel = gws_read_sts.channel;
			gws_ui.txpwr = gws_read_sts.txpwr;
			gws_ui.is_tx_on = gws_ui.txpwr > 0 ? 1 : 0;
			gws_ui.rxgain = gws_read_sts.rxgain;
			gws_ui.noise = gws_read_sts._noise_ + GWS_NOISE_WP;
	
			gws_read_sts.is_data_changed = 0;

			ui_host_timeout = 0;
			if (ui_screen == UI_LOADING) ui_screen = UI_PRINT_PAGE_STS;
		}

		if (gws_read_sig.is_data_changed) {
			if (gws_ui.noise < GWS_NOISE_MIN) gws_ui.noise = GWS_NOISE_MIN;
			for(i = 0; i < WBP_SIGNAL_MAX; i ++) {
				if (gws_read_sig._signal_[i] >= 99) {
					gws_ui.signal[i] = 99;
				} else if (gws_read_sig._signal_[i] == 0) {
					gws_ui.signal[i] = gws_ui.noise;
				} else {
					gws_ui.signal[i] = gws_read_sig._signal_[i] + gws_ui.noise;
				}
				if (gws_ui.signal[i] > WBP_SIGNAL_VALUE_MAX) gws_ui.signal[i] = WBP_SIGNAL_VALUE_MAX;
				if (gws_ui.signal[i] < GWS_NOISE_MIN) gws_ui.signal[i] = GWS_NOISE_MIN;
			}

			gws_read_sig.is_data_changed = 0;

			ui_host_timeout = 0;
			if (ui_screen == UI_LOADING) ui_screen = UI_PRINT_PAGE_GWS;
		}


		//* auto switch
		//uart_send("sw screen\n");
		if (ui_screen_echo_change_counter % 30 == 0) {
			//uart_send("sw screen judge\n");
			if (ui_screen == UI_PRINT_PAGE_GWS) {
				//uart_send("sw screen to sts\n");
				ui_screen = UI_PRINT_PAGE_STS;
			} else if (ui_screen == UI_PRINT_PAGE_STS) {
				//uart_send("sw screen to gws\n");
				ui_screen = UI_PRINT_PAGE_GWS;
			}
		}
	}

	ui_host_timeout ++;
	if (ui_host_timeout > UI_UART_TIMEOUT) {
		ui_screen = UI_LOADING;
	}


	ui_screen_echo_change_counter ++;
}

static void ui_calc_pages(uint btn)
{
	uint flag = 0;
	switch(ui_screen) {
		case UI_LOADING:
			ui_calc_screen_echo_loading(btn);
			gws_ui_init();
			break;
		case UI_PRINT_PAGE_STS:
			ui_calc_scrren_echo_sts(btn);
			break;
		case UI_PRINT_PAGE_GWS:
			ui_calc_scrren_echo_gws(btn);
			break;
		case UI_PRINT_EDIT_TXPWR:
			ui_calc_screen_edit_txpwr(btn);
			break;
		case UI_PRINT_EDIT_CHANNEL:
			ui_calc_screen_edit_channel(btn);
			break;
		case UI_PRINT_EDIT_CHANBW:
			ui_calc_screen_edit_chanbw(btn);
			break;
		case UI_PRINT_EDIT_REGION:
			ui_calc_screen_edit_region(btn);
			break;
		case UI_PRINT_EDIT_MODE:
			ui_calc_screen_edit_mode(btn);
			break;
		default:
			break;
	}

	switch(btn) {
		case UI_BTN_X1: // + -
			flag =  1;
			sprintf(gws_cmd, "\r\n+wbcmd:reboot\r\n");
			break;
		case UI_BTN_X2:	// M -
			flag =  1;
			sprintf(gws_cmd, "\r\n+wbcmd:wifi down\r\n");
			break;
		case UI_BTN_X3:	// S +
			flag =  1;
			sprintf(gws_cmd, "\r\n+wbcmd:gwsmax\r\n");
			break;
		case UI_BTN_X4:	// M +
			flag =  1;
			sprintf(gws_cmd, "\r\n+wbcmd:wifi\r\n");
			break;
		case UI_BTN_X5:	// S -
			flag =  1;
			sprintf(gws_cmd, "\r\n+wbcmd:gwsmin\r\n");
			break;
	}
	if (flag) {
		uart_send(gws_cmd);
		memset(gws_cmd, 0, sizeof(gws_cmd));
		flag = 0;
		delay(1000);
	}
}

static void ui_echo_page(void)
{
	static uint i = 0;
	uint gws_freq = 0;
	uchar l1[LCD_LINE_LENGTH+1] = {""}, l2[LCD_LINE_LENGTH+1] = {""}, edit_mark_pos = 0;

	if (gws_ui.region) {
		if (gws_ui.channel < GWS_R1CH_MIN) gws_ui.channel = GWS_R1CH_MIN;
		if (gws_ui.channel > GWS_R1CH_MAX) gws_ui.channel = GWS_R1CH_MAX;
		gws_freq = GWS_R1FREQ_START + (gws_ui.channel - GWS_R1CH_MIN) * GWS_R1CHAN_BW;
	} else {
		if (gws_ui.channel < GWS_R0CH_MIN) gws_ui.channel = GWS_R0CH_MIN;
		if (gws_ui.channel > GWS_R0CH_MAX) gws_ui.channel = GWS_R0CH_MAX;
		gws_freq = GWS_R0FREQ_START + (gws_ui.channel - GWS_R0CH_MIN) * GWS_R0CHAN_BW;
	}

	switch(ui_screen) {
		case UI_PRINT_EDIT_MODE:
			sprintf(l1, " Change Mode:   ");
			sprintf(l2, " %15s", gws_mode_get());
			edit_mark_pos = LCD_EDIT_MODE_MARK_POS;
			break;
		case UI_PRINT_EDIT_TXPWR:
			sprintf(l1, " Change TxPwr:  ");
			sprintf(l2, " %11d dBm", gws_ui.txpwr);
			edit_mark_pos = LCD_EDIT_TXPWR_MARK_POS;
			break;
		case UI_PRINT_EDIT_CHANNEL:
			sprintf(l1, " Change Channel:");
			sprintf(l2, "    %2d @ %3d MHz", gws_ui.channel, gws_freq);
			edit_mark_pos = LCD_EDIT_CHANNEL_MARK_POS;
			break;
		case UI_PRINT_EDIT_CHANBW:
			sprintf(l1, " Change ChanBw: ");
			sprintf(l2, "          %2d MHz", gws_ui.chanbw);
			edit_mark_pos = LCD_EDIT_CHANBW_MARK_POS;
			break;
		case UI_PRINT_EDIT_REGION:
			sprintf(l1, " Change Region: ");
			if (gws_ui.region != 0) {
				gws_ui.region = 1;
				sprintf(l2, "        Region 1");
			} else {
				gws_ui.region = 0;
				sprintf(l2, "        Region 0");
			}
			edit_mark_pos = LCD_EDIT_REGION_MARK_POS;
			break;
		case UI_PRINT_PAGE_STS:
			//sprintf(l1, "Tx: %2dM, %-7s", gws_ui.chanbw, gws_mode_get());
			//sprintf(l2, "Rg %1d, C%2d/%3dMHz", gws_ui.region, gws_ui.channel, gws_freq);
			sprintf(l1, "Tx: %2d dBm, %4s", gws_ui.txpwr, gws_ui.is_tx_on > 0 ? "ON" : "OFF");
			sprintf(l2, "Rg: %1d - %-8s", gws_ui.region, gws_mode_get());
			edit_mark_pos = 0;
			break;
		case UI_PRINT_PAGE_GWS:
			//sprintf(l1, "Tx: %2d dBm, %4s", gws_ui.txpwr, gws_ui.is_tx_on > 0 ? "ON" : "OFF");
			sprintf(l1, "Tx: %2d/%3dM, %2dM", gws_ui.channel, gws_freq, gws_ui.chanbw);
			if (gws_ui.noise < GWS_NOISE_MIN) gws_ui.noise = GWS_NOISE_MIN;
			if (gws_ui.signal[ui_screen_page] >= 99) {
				sprintf(l2, "Rx: *> %4d gain", gws_ui.rxgain);
			} else {
				sprintf(l2, "Rx: %d> %d/%-d dBm", ui_screen_page+1, gws_ui.signal[ui_screen_page], gws_ui.noise);
			}
			edit_mark_pos = 0;
			break;
		case UI_LOADING:
		default:
			switch(i / 32 % 3) {
				case 2:
					sprintf(l1, "  Tech Support  ");
					sprintf(l2, "+86-135-11053556"); 
					break;
				case 1:
					sprintf(l1, "     6WiLink    ");
					sprintf(l2, " +86-10-82825799"); 
					break;
				case 0: 
				default:
					sprintf(l1, "      Model     ");
					sprintf(l2, "  ARN3450KP24C  "); 
					break;
			}
			edit_mark_pos = 0;
			break;
	}

	//* print to lcd
	lcd_print(0, 1, l1);
	lcd_print(0, 2, l2);
	if (edit_mark_pos) {
		lcd_edit_mark(edit_mark_pos);
	} else {
		lcd_edit_done();
	}
	i ++;
}

static uint ui_detect_btn(void)
{
	uint btn_pressed = 0;
	uchar btn;
	btn = P2 & BTN_DETECT_MASK;

	if(btn == BTN_10_MASK)	btn_pressed = 10;
	if(btn == BTN_9_MASK)	btn_pressed = 9;
	if(btn == BTN_8_MASK)	btn_pressed = 8;
	if(btn == BTN_7_MASK)	btn_pressed = 7;
	if(btn == BTN_6_MASK)	btn_pressed = 6;
	if(btn == BTN_5_MASK)	btn_pressed = 5;
	if(btn == BTN_4_MASK)	btn_pressed = 4;
	if(btn == BTN_3_MASK)	btn_pressed = 3;
	if(btn == BTN_2_MASK)	btn_pressed = 2;
	if(btn == BTN_1_MASK)	btn_pressed = 1;

	if (btn_pressed > 0)	delay(100);
	return btn_pressed;
}



static void ui_calc_screen_echo_loading(uint btn)
{
	if (btn == UI_BTN_DEBUG) {
		ui_screen = UI_PRINT_PAGE_STS;
		ui_host_timeout = 0;
	}
}
static void ui_calc_scrren_echo_sts(uint btn)
{
	if (btn == UI_BTN_PLUS || btn == UI_BTN_MINUS || btn == UI_BTN_SW) {
		ui_screen = UI_PRINT_PAGE_GWS;
		ui_screen_echo_change_counter = 1;
	}

	if (btn == UI_BTN_SET) {
		ui_screen = UI_PRINT_EDIT_TXPWR;
	}
}
static void ui_calc_scrren_echo_gws(uint btn)
{
	if (btn == UI_BTN_PLUS) {
		ui_screen_echo_change_counter = 1;
		ui_screen_page ++;
		if (ui_screen_page >= WBP_SIGNAL_MAX) ui_screen_page = WBP_SIGNAL_MAX - 1;
	}
	if (btn == UI_BTN_MINUS) {
		ui_screen_echo_change_counter = 1;
		if (ui_screen_page) ui_screen_page --;
	}
	if (btn == UI_BTN_SW) {
		ui_screen = UI_PRINT_PAGE_STS;
		ui_screen_echo_change_counter = 1;
	}

	if (btn == UI_BTN_SET)
		ui_screen = UI_PRINT_EDIT_TXPWR;
}
static void ui_calc_screen_edit_txpwr(uint btn)
{
	if (btn == UI_BTN_SW) {
		ui_screen = UI_PRINT_EDIT_CHANNEL;
		gws_ui.is_data_changed = 0;
	}
	if (btn == UI_BTN_PLUS) {
		gws_ui.txpwr += (gws_ui.txpwr >= 30 ? 1 : 3); // 27 + 3 = 30; 30 + 1 = 31
		gws_ui.is_data_changed = 1;
	}
	if (btn == UI_BTN_MINUS) {
		gws_ui.txpwr -= (gws_ui.txpwr > 30 ? 1 : 3); // 31 - 1 = 30; 30 - 1 = 29
		gws_ui.is_data_changed = 1;
	}
	if (gws_ui.txpwr >= GWS_TXPWR_MAX)			gws_ui.txpwr = GWS_TXPWR_MAX;
	if (gws_ui.txpwr <= GWS_TXPWR_MIN)			gws_ui.txpwr = GWS_TXPWR_MIN;
	if (btn == UI_BTN_SET) {
		ui_screen = UI_PRINT_PAGE_STS;
		if (gws_ui.is_data_changed) {
			sprintf(gws_cmd, "\r\n+wbsetpwr:%d\r\n", gws_ui.txpwr);
			uart_send(gws_cmd);
			memset(gws_cmd, 0, sizeof(gws_cmd));
		}
	}
	ui_host_timeout = 0;
}
static void ui_calc_screen_edit_channel(uint btn)
{
	if (btn == UI_BTN_SW) {
		ui_screen = UI_PRINT_EDIT_CHANBW;
		gws_ui.is_data_changed = 0;
	}
	if (btn == UI_BTN_PLUS) {
		gws_ui.channel ++;
		gws_ui.is_data_changed = 1;
	}
	if (btn == UI_BTN_MINUS) {
		gws_ui.channel --;
		gws_ui.is_data_changed = 1;
	}
	if (gws_ui.region) {
		if (gws_ui.channel < GWS_R1CH_MIN) 	gws_ui.channel = GWS_R1CH_MIN;
		if (gws_ui.channel > GWS_R1CH_MAX) 	gws_ui.channel = GWS_R1CH_MAX;
	} else {
		if (gws_ui.channel < GWS_R0CH_MIN) 	gws_ui.channel = GWS_R0CH_MIN;
		if (gws_ui.channel > GWS_R0CH_MAX) 	gws_ui.channel = GWS_R0CH_MAX;
	}
	if (btn == UI_BTN_SET) {
		ui_screen = UI_PRINT_PAGE_STS;
		if (gws_ui.is_data_changed) {
			//sprintf(gws_cmd, "\r\n+wbsetchn:%d,%d\r\n", gws_ui.channel, gws_ui.region);
			sprintf(gws_cmd, "\r\n+wbsetchn:%d\r\n", gws_ui.channel);
			uart_send(gws_cmd);
			memset(gws_cmd, 0, sizeof(gws_cmd));
		}
	}
	ui_host_timeout = 0;
}
static void ui_calc_screen_edit_chanbw(uint btn)
{
	if (btn == UI_BTN_SW) {
		ui_screen = UI_PRINT_EDIT_REGION;
		gws_ui.is_data_changed = 0;
	}
	if (btn == UI_BTN_PLUS) {
		gws_ui.chanbw ++;
		gws_ui.is_data_changed = 1;
	}
	if (btn == UI_BTN_MINUS) {
		gws_ui.chanbw --;
		gws_ui.is_data_changed = 1;
	}
	if (gws_ui.chanbw < GWS_CHANBW_MIN) gws_ui.chanbw = GWS_CHANBW_MIN;
	if (gws_ui.chanbw > GWS_CHANBW_MAX) gws_ui.chanbw = GWS_CHANBW_MAX;
	if (btn == UI_BTN_SET) {
		ui_screen = UI_PRINT_PAGE_STS;
		if (gws_ui.is_data_changed) {
			sprintf(gws_cmd, "\r\n+wbsetchnbw:%d\r\n", gws_ui.chanbw);
			uart_send(gws_cmd);
			memset(gws_cmd, 0, sizeof(gws_cmd));
		}
	}
	ui_host_timeout = 0;
}
static void ui_calc_screen_edit_region(uint btn)
{
	if (btn == UI_BTN_SW) {
		ui_screen = UI_PRINT_EDIT_MODE;
		gws_ui.is_data_changed = 0;
	}
	if (btn == UI_BTN_PLUS) {
		gws_ui.region = 1;
		gws_ui.is_data_changed = 1;
	}
	if (btn == UI_BTN_MINUS) {
		gws_ui.region = 0;
		gws_ui.is_data_changed = 1;
	}
	if (btn == UI_BTN_SET) {
		ui_screen = UI_PRINT_PAGE_STS;
		if (gws_ui.is_data_changed) {
			sprintf(gws_cmd, "\r\n+wbsetrgn:%d\r\n", gws_ui.region);
			uart_send(gws_cmd);
			memset(gws_cmd, 0, sizeof(gws_cmd));
		}
	}
	ui_host_timeout = 0;
}

static void ui_calc_screen_edit_mode(uint btn)
{
	if (btn == UI_BTN_SW) {
		ui_screen = UI_PRINT_EDIT_TXPWR;
		gws_ui.is_data_changed = 0;
	}
	if (btn == UI_BTN_PLUS) {
		gws_ui.mode ++;
		gws_ui.is_data_changed = 1;
	}
	if (btn == UI_BTN_MINUS) {
		if (gws_ui.mode) gws_ui.mode --;
		gws_ui.is_data_changed = 1;
	}
	if (gws_ui.mode >= GWS_MODE_MAX)			gws_ui.mode = GWS_MODE_MAX;
	if (gws_ui.mode <= GWS_MODE_MIN)			gws_ui.mode = GWS_MODE_MIN;
	if (btn == UI_BTN_SET) {
		ui_screen = UI_PRINT_PAGE_STS;
		if (gws_ui.is_data_changed) {
			sprintf(gws_cmd, "\r\n+wbsetmod:%d\r\n", gws_ui.mode);
			uart_send(gws_cmd);
			memset(gws_cmd, 0, sizeof(gws_cmd));
		}
	}
	ui_host_timeout = 0;
}




static void gws_ui_init(void)
{
	uint i;

	gws_read_sts.is_data_changed = 0;
	gws_read_sts.chanbw = 8;

	gws_read_sig.is_data_changed = 0;

	gws_ui.is_data_changed = 0;
	gws_ui.mode		= GWS_MODE_MESH;
	gws_ui.is_tx_on	= 0;
	gws_ui.txpwr	= GWS_TXPWR_MIN;
	gws_ui.region	= GWS_REGION_DFL;
	gws_ui.channel	= GWS_CHANNEL_DFL;
	gws_ui.chanbw	= GWS_CHANBW_DFL;

	for(i = 0; i < WBP_SIGNAL_MAX; i ++) {
		gws_ui.signal[i] = GWS_NOISE_MIN;
		gws_read_sig._signal_[i] = 0;
	}
	gws_ui.noise	= GWS_NOISE_MIN;

	gws_read_sts.channel = 0;
	gws_read_sts._noise_ = 0;
	gws_read_sts.mode = 0;
	gws_read_sts.txpwr = 0;
}
static uchar * gws_mode_get(void)
{
	switch(gws_ui.mode) {
		case GWS_MODE_ADHOC:
			return "Ad-Hoc";
		case GWS_MODE_AP:
			return "CAR BS*";
		case GWS_MODE_STA:
			return "EAR CPE";
		case GWS_MODE_MESH:
		default:
			return "Mesh *";
	}
}




//* WB Protocol Parser
//* Parser Cmds
//* "+wbsts:<ch>,<_noise+101>,<txpwr>,<mode>,<rxgain>,<chanbw>\r\n"
//* "+wbsigs:<p1sig>,<p2sig>,<p3sig>,<p4sig>,<p5sig>\r\n"
void wbp_parser(uchar * p_data, uint u_data_length, uint * p_move)
{
	uint _ch = 0, _mode = 0, _region = 1, _chanbw = 8;
	int _noise = 0, _txpwr = 0, _rxgain = 0;
	int _signals[WBP_SIGNAL_MAX];

	uint i, j = 0, wbp_data_length = 0, wbp_current_pos = 0, wbp_start_pos = 0, wbp_stop_pos = 0;
	uchar wbp_data[STD_BUFFER_LENGTH];

	(*p_move) = 0;
	if (u_data_length < WBP_MSG_LENGTH_MIN) return;

	do {
		wbp_start_pos = 0, wbp_stop_pos = 0;

		//* find last '+' pos
		for(i = wbp_current_pos; i < u_data_length; i ++) {
			if (p_data[i] == '+') {
				wbp_start_pos = i;
				//uart_send("- msg head found\n");
				break;
			}
			wbp_current_pos = i;
		}

		//* find last '\r' or '\n' pos
		for(j = wbp_current_pos + 1; j < u_data_length; j ++) {
			if (p_data[j] == '\r' || p_data[j] == '\n') {
				wbp_stop_pos = j;
				//uart_send("- msg tail found\n");
				break;
			}
			wbp_current_pos = j;
		}

		//* no tail found
		if (! wbp_stop_pos) break;


		//* save move counter
		(*p_move) = wbp_current_pos;
		wbp_current_pos ++;

		//* parse data	
		wbp_data_length = wbp_stop_pos - wbp_start_pos + 1;
		if (wbp_data_length >= WBP_MSG_LENGTH_MIN) {

			//uart_send("- msg length valid, cp msg\n");
			memset(wbp_data, 0, sizeof(wbp_data));
			//strcpy(wbp_data, &p_data[wbp_start_pos]);
			//memcpy(wbp_data, &p_data[wbp_start_pos], wbp_data_length);
			for(i = 0, j = wbp_start_pos; j < wbp_stop_pos; i ++, j ++) {
				if (p_data[j] != '\r' && p_data[j] != '\n') {
					wbp_data[i] = p_data[j];
				} else {
					break;
				}
			}

			//uart_send(wbp_data);


			if (strstr(wbp_data, "wbsts:") > 0) {
				//uart_send("parse msg +wbsts\n");
				if (sscanf(wbp_data, "+wbsts:%d,%d,%d,%d,%d,%d,%d", &_ch, &_noise, &_txpwr, &_mode, &_region, &_chanbw, &_rxgain) != -1) {
					gws_read_sts.is_data_changed = 1;
					gws_read_sts.chanbw = (int) (_chanbw > 0 ? _chanbw : 8);
					gws_read_sts.channel = _ch;
					gws_read_sts.rxgain = _rxgain;
					gws_read_sts._noise_ = _noise;
					gws_read_sts.txpwr = _txpwr;
					gws_read_sts.mode = _mode;
					gws_read_sts.region = (_region != 0 ? 1 : 0);

#ifdef UART_PRINT_PARSE_RESULT
					memset(gws_cmd, 0, sizeof(gws_cmd));
					sprintf(gws_cmd, "+sts:%d,%d,%d,%d\r\n", _ch, _noise, _txpwr, _mode);
					uart_send(gws_cmd);
#endif
				}
			}

			if (strstr(wbp_data, "wbsigs:") > 0) {
				//uart_send("parse msg +wbsigs\n");
				if (sscanf(wbp_data, "+wbsigs:%d,%d,%d,%d,%d", &_signals[0], &_signals[1], &_signals[2], &_signals[3], &_signals[4]) != -1) {
					gws_read_sig.is_data_changed = 1;
					for(i = 0; i < WBP_SIGNAL_MAX; i ++) {
						gws_read_sig._signal_[i] = _signals[i];
					}

#ifdef UART_PRINT_PARSE_RESULT
					memset(gws_cmd, 0, sizeof(gws_cmd));
					sprintf(gws_cmd, "+sigs:%d,%d,%d,%d,%d\r\n", _signals[0], _signals[1], _signals[2], _signals[3], _signals[4]);
					uart_send(gws_cmd);
#endif
				}
			}
		//} else {
			//uart_send("- msg length invalid\n");
		}
	} while (u_data_length - wbp_current_pos >= WBP_MSG_LENGTH_MIN);

	(*p_move) = wbp_current_pos;
}