/*
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __OBMC_PAL_H__
#define __OBMC_PAL_H__

#include <stdint.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  PAL_EOK = 0,
  PAL_ENOTSUP = -ENOTSUP,
  /* non system errors start from -256 downwards */
};

int pal_is_fw_update_ongoing(uint8_t fru);
void set_fw_update_ongoing(uint8_t fru, uint16_t tmout);

int pal_init_sensor_check(uint8_t fru, uint8_t snr_num, void *snr);

int pal_set_last_boot_time(uint8_t slot, uint8_t *last_boot_time);
int pal_get_last_boot_time(uint8_t slot, uint8_t *last_boot_time);

void pal_get_chassis_status(uint8_t slot, uint8_t *req_data, uint8_t *res_data, uint8_t *res_len);
int pal_get_80port_record(uint8_t slot, uint8_t *req_data, uint8_t req_len, uint8_t *res_data, uint8_t *res_len);
int pal_set_boot_order(uint8_t slot, uint8_t *boot, uint8_t *res_data, uint8_t *res_len);
int pal_get_boot_order(uint8_t slot, uint8_t *req_data, uint8_t *boot, uint8_t *res_len);
void pal_set_post_start(uint8_t slot, uint8_t *req_data, uint8_t *res_data, uint8_t *res_len);
void pal_set_post_end(uint8_t slot, uint8_t *req_data, uint8_t *res_data, uint8_t *res_len);
int pal_get_board_id(uint8_t slot, uint8_t *req_data, uint8_t req_len, uint8_t *res_data, uint8_t *res_len);
int pal_parse_oem_sel(uint8_t fru, uint8_t *sel, char *error_log);
int pal_set_ppin_info(uint8_t slot, uint8_t *req_data, uint8_t req_len, uint8_t *res_data, uint8_t *res_len);
int pal_bmc_err_enable(void);
int pal_bmc_err_disable(void);
int pal_set_bios_current_boot_list(uint8_t slot, uint8_t *boot_list, uint8_t list_length, uint8_t *cc);
int pal_get_bios_current_boot_list(uint8_t slot, uint8_t *boot_list, uint8_t *list_length);
int pal_set_bios_fixed_boot_device(uint8_t slot, uint8_t *fixed_boot_device);
int pal_get_bios_fixed_boot_device(uint8_t slot, uint8_t *fixed_boot_device);
int pal_set_bios_restores_default_setting(uint8_t slot, uint8_t *default_setting);
int pal_get_bios_restores_default_setting(uint8_t slot, uint8_t *default_setting);
uint8_t pal_set_power_restore_policy(uint8_t slot, uint8_t *pwr_policy, uint8_t *res_data);
void pal_set_boot_option(unsigned char para,unsigned char* pbuff);
int pal_get_boot_option(unsigned char para,unsigned char* pbuff);
int pal_get_poss_pcie_config(uint8_t slot, uint8_t *req_data, uint8_t req_len, uint8_t *res_data, uint8_t *res_len);

#ifdef __cplusplus
}
#endif

#endif
