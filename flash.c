#include "flash.h"
#include "nrf_fstorage_sd.h"

#include "log.h"

static uint32_t work_address = 0;
static uint8_t  buff[SIZE_STEP_FLASH];

static void fstorage_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("fstorage: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("fstorage: wrote %d bytes at address 0x%x.", p_evt->len, p_evt->addr);
        } 
        break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("fstorage: erased %d page from address 0x%x.", p_evt->len, p_evt->addr);
        } 
        break;

        default:
            break;
    }
}

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    .evt_handler = fstorage_handler,
    .start_addr = FIRST_PAGE_FLASH,
    .end_addr   = END_PAGE_FLASH
};

uint32_t get_start_addr(uint32_t address)
{
    uint8_t *temp = (uint8_t*) address;
    if(temp[3] != 0 && temp[3] != 0xFF)
    {
        nrf_fstorage_erase(&fstorage, address, 1, NULL);
        return fstorage.start_addr + SIZE_STEP_FLASH;
    }
    while (temp[3] == 0)
    {
        temp = (uint8_t*) address;
        if(temp[3] != 0xFF)
        {
            if(temp[3] == 0)
            {
                address += SIZE_STEP_FLASH;
            }
            else
            {
                nrf_fstorage_erase(&fstorage, fstorage.start_addr, 1, NULL);
                address = fstorage.start_addr + SIZE_STEP_FLASH;
                break;
            }
        }
        if(address >= END_PAGE_FLASH)
        {
            nrf_fstorage_erase(&fstorage, fstorage.start_addr, 1, NULL);
            address = fstorage.start_addr + SIZE_STEP_FLASH;
            break;
        }
    }
    return address;
}

void init_flash(rgb_t *rgb)
{
    nrf_fstorage_init(&fstorage, &nrf_fstorage_sd, NULL);
    work_address = get_start_addr(fstorage.start_addr);
    nrf_fstorage_read(&fstorage, (work_address - SIZE_STEP_FLASH), &buff, sizeof(buff));
    rgb->r = buff[0];
    rgb->g = buff[1];
    rgb->b = buff[2];
}

bool read_flash(rgb_t* out)
{
    if(work_address <= FIRST_PAGE_FLASH)
    {
        return false;
    }
    nrf_fstorage_read(&fstorage, (work_address - SIZE_STEP_FLASH), &buff, sizeof(buff));
    out->r = buff[0];
    out->g = buff[1];
    out->b = buff[2];

    return true;
}

void write_to_flash(const rgb_t* in)
{   
    buff[0] = in->r;
    buff[1] = in->g;
    buff[2] = in->b;
    buff[3] = 0;

    if(fstorage.end_addr < work_address + SIZE_STEP_FLASH || ((uint8_t*)work_address)[3] != 0xFF)
    {
        nrf_fstorage_erase(&fstorage, fstorage.start_addr, 1, NULL);
        work_address = fstorage.start_addr;
    }
    nrf_fstorage_write(&fstorage,
                     work_address,
                     &buff,
                     sizeof(buff), NULL);
    work_address += SIZE_STEP_FLASH;
}