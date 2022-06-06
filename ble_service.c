/**
 * Copyright 2022 Evgeniy Morozov
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
*/

#include "ble_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"

//static ret_code_t estc_ble_add_characteristics(ble_estc_service_t *service);
static ret_code_t add_characteristics(ble_estc_service_t *service, uint16_t UUID, ble_gatts_char_handles_t	* handles, uint8_t n_bytes, bool write, bool read, bool notify, bool identify);

ret_code_t estc_ble_service_init(ble_estc_service_t *service)
{
    ret_code_t      err_code;
    ble_uuid_t    service_uuid_notify;
    ble_uuid128_t base_uuid = ESTC_BASE_UUID;

    // TODO: 3. Add service UUIDs to the BLE stack table using `sd_ble_uuid_vs_add`
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid_notify.type);
    VERIFY_SUCCESS(err_code);

    service_uuid_notify.uuid = ESTC_SERVICE_UUID;

    // TODO: 4. Add service to the BLE stack using `sd_ble_gatts_service_add`
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                    &service_uuid_notify,
                                    &service->service_handle);
    VERIFY_SUCCESS(err_code);

    // NRF_LOG_DEBUG("%s:%d | Service UUID: 0x%04x", __FUNCTION__, __LINE__, service_uuid_notify.uuid);
    // NRF_LOG_DEBUG("%s:%d | Service UUID type: 0x%02x", __FUNCTION__, __LINE__, service_uuid_notify.type);
    // NRF_LOG_DEBUG("%s:%d | Service handle: 0x%04x", __FUNCTION__, __LINE__, service->service_handle);

    //err_code = estc_ble_add_characteristics(service);
    err_code = add_characteristics(service, ESTC_GATT_CHAR_NOTIFY_UUID, &service->notify_handles, 3,1,0,1,0);
    APP_ERROR_CHECK(err_code);

    err_code = add_characteristics(service, ESTC_GATT_CHAR_LED_CONTROL_UUID, &service->identify_handles, 3,0,0,0,1);
    APP_ERROR_CHECK(err_code);

    return 0;
}

static ret_code_t add_characteristics(ble_estc_service_t *service, uint16_t UUID, ble_gatts_char_handles_t	* handles, uint8_t n_bytes, bool write, bool read, bool notify, bool identify)
{
    ret_code_t    err_code  = NRF_SUCCESS;
    ble_uuid_t    service_uuid;
    ble_uuid128_t base_uuid = ESTC_BASE_UUID;
    service_uuid.uuid = UUID;

    //TODO: 6.1. Add custom characteristic UUID using `sd_ble_uuid_vs_add`, same as in step 4
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    VERIFY_SUCCESS(err_code);
    // TODO: 6.5. Configure Characteristic metadata (enable read and write)
    ble_gatts_char_md_t char_md = { 0 };
    char_md.char_props.write = write;     // Разрешение на запись;
	char_md.char_props.read   = read;     // Разрешение на чтение;
    char_md.char_props.notify = notify;     // Разрешение на уведомление;
    char_md.char_props.indicate = identify; // Разрешение на индикацию;

    // Configures attribute metadata. For now we only specify that the attribute will be stored in the softdevice
    ble_gatts_attr_md_t attr_md = { 0 };
    attr_md.vloc = BLE_GATTS_VLOC_STACK;

    // TODO: 6.6. Set read/write security levels to our attribute metadata using `BLE_GAP_CONN_SEC_MODE_SET_OPEN`
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // TODO: 6.2. Configure the characteristic value attribute (set the UUID and metadata)
    ble_gatts_attr_t attr_char_value = { 0 };
    service_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;
    attr_char_value.p_uuid    = &service_uuid;
	attr_char_value.p_attr_md = &attr_md;

    // // TODO: 6.7. Set characteristic length in number of bytes in attr_char_value_notify structure
	attr_char_value.init_len  = n_bytes;           
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = n_bytes;

    // TODO: 6.4. Add new characteristic to the service using `sd_ble_gatts_characteristic_add`
    sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_char_value, handles);

    return err_code;
}