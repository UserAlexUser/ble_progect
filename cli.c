#include <stdio.h>

#include "cli.h"
#include "log.h"
#include "pwm.h"
#include "flash.h"
#include "color_converter.h"

#define READ_SIZE 1
#define CMD_SIZE 15
#define CMD_SIZE_RED 7
#define CMD_SIZE_BLUE 8
#define CMD_SIZE_GREEN 9

uint8_t count_cmd = 0;
uint8_t old_count_cmd = 0;

static char m_rx_buffer[READ_SIZE];
char cmd[CMD_SIZE];
char old_cmd[CMD_SIZE];

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event);

/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

#define KEY_DELETE      127
#define KEY_PREW_CMD    27

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

void cmd_rgb(void);
void cmd_help(void);

static cli_cmd_t cli_cmd[] =
{
    {"rgb", cmd_rgb},
    {"help", cmd_help},
};

static char *cmd_request_error[] =
{
    [0] = "\ncommand not executed",
    [1] = "write help to see how to write a command",
};

void send_help_cmd(char const req[], size_t len)
{   
    ret_code_t ret = 0;

    do
    {
        ret = app_usbd_cdc_acm_write(&usb_cdc_acm, req, len);
    } while (ret == NRF_ERROR_BUSY);

    do
    {
        ret = app_usbd_cdc_acm_write(&usb_cdc_acm, "\r\n", 3);
    } while (ret == NRF_ERROR_BUSY);
}

void init_usb_cli(void)
{
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);
}

static void prepare_cmd(void)
{
    cmd[count_cmd]=m_rx_buffer[0];
    count_cmd++;
}

void check_cmd()
{   
    for(size_t i = 0; i < ARRAY_SIZE(cli_cmd); i++)
    {
        const cli_cmd_t *p_cli_cmd = &cli_cmd[i];

        if(strncmp(cmd, p_cli_cmd->cmd_name, strlen(p_cli_cmd->cmd_name)) == 0)
        {
            p_cli_cmd->cmd_handler();
        }
    }

    old_count_cmd = count_cmd;
    count_cmd = 0;

    for(int i=0;i<CMD_SIZE;i++)
    {
        old_cmd[i] = cmd[i];
        cmd[i]=0;//обнуляем команду
    }
}

void cmd_rgb()
{
    NRF_LOG_INFO("cmd_rgb");
    char str[80];
    int r;
    int g;
    int b;

    sscanf(cmd, "%s%d%d%d", str, &r, &g, &b);

    if((r>255) || (g>255) || (b>255))
    {
        NRF_LOG_INFO("cmd_error");

        for (size_t i = 0; i < 2; i++)
            send_help_cmd(cmd_request_error[i], strlen(cmd_request_error[i]) + 1);
    }
    else
    {
        rgb_color.r = r;
        rgb_color.g = g;
        rgb_color.b = b;
        write_to_flash(&rgb_color);
        rgb_on(rgb_color.r, rgb_color.g, rgb_color.b);
        rgb2hsv(&rgb_color, &hsv_color);
    }
}

void cmd_help()
{   
    static char *cmd_request[] =
    {
        [0] = "\nSupport commands:",
        [1] = "1. 'help' to see this list",
        [2] = "2. 'rgb <r> <g> <b>' (0...255) example: 'rgb 255 255 255'",
    };

    for (size_t i = 0; i < 3; i++)
        send_help_cmd(cmd_request[i], strlen(cmd_request[i]) + 1);
}

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
    {
        ret_code_t ret;
        ret = app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        NRF_LOG_INFO("cli open %d", m_rx_buffer[0]);
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
    {
        NRF_LOG_INFO("cli close");
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
    {
        //NRF_LOG_INFO("tx done");
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
    {
        ret_code_t ret;
        do
        {
            //size_t size = app_usbd_cdc_acm_rx_size(&usb_cdc_acm);
            //NRF_LOG_INFO("rx size: %d", size);

            //NRF_LOG_INFO("cli test %d", m_rx_buffer[0]);

            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n')
            {
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, "\r\n", 2);
                if (m_rx_buffer[0] == '\r')
                 check_cmd();
            }
            
            else if (m_rx_buffer[0] == KEY_DELETE || m_rx_buffer[0] == '\b')
            {
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, "\b \b", 3);
                if (count_cmd > 0)
                    count_cmd--;
            }

            else if (m_rx_buffer[0] == KEY_PREW_CMD)
            {
                size_t rx_bufer_size = app_usbd_cdc_acm_bytes_stored(&usb_cdc_acm);

                if (rx_bufer_size > 0)
                {
                    uint8_t rx_big_bufer[rx_bufer_size - 1];
                    ret = app_usbd_cdc_acm_read(&usb_cdc_acm, rx_big_bufer, rx_bufer_size);
                }

                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, old_cmd, old_count_cmd);
                
                count_cmd = old_count_cmd;
                for(int i=0;i<CMD_SIZE;i++)
                {
                    cmd[i] = old_cmd[i];
                }
            }

            else
            {
                prepare_cmd();
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
            }

            /* Fetch data until internal buffer is empty */
            ret = app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        } while (ret == NRF_SUCCESS);

        break;
    }
    default:
        break;
    }
}