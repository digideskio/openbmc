/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
 * This file provides platform specific implementation of FRUID information
 *
 * FRUID specification can be found at
 * www.intel.com/content/dam/www/public/us/en/documents/product-briefs/platform-management-fru-document-rev-1-2-feb-2013.pdf
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <stdint.h>
#include "fruid.h"
#include <facebook/fby2_common.h>

#define EEPROM_DC       "/sys/class/i2c-adapter/i2c-%d/%d-0051/eeprom"
#define EEPROM_SPB      "/sys/class/i2c-adapter/i2c-8/8-0051/eeprom"
#define EEPROM_NIC      "/sys/class/i2c-adapter/i2c-12/12-0051/eeprom"

#define BIN_SPB         "/tmp/fruid_spb.bin"
#define BIN_NIC         "/tmp/fruid_nic.bin"
#define BIN_SLOT        "/tmp/fruid_slot%d.bin"

#define FRUID_SIZE        256

#define SLOT_FILE       "/tmp/slot.bin"

enum {
  IPMB_BUS_SLOT1 = 1,
  IPMB_BUS_SLOT2 = 3,
  IPMB_BUS_SLOT3 = 5,
  IPMB_BUS_SLOT4 = 7,
};

// Helper Functions
static int
read_device(const char *device, int *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
#ifdef DEBUG
    syslog(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%d", value);
  fclose(fp);
  if (rc != 1) {
#ifdef DEBUG
    syslog(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  } else {
    return 0;
  }
}

/*
 * Get SLOT type
 * PAL_TYPE[7:6] = 0(TwinLake), 1(Crace Flat), 2(Glacier Point)
 * PAL_TYPE[5:4] = 0(TwinLake), 1(Crace Flat), 2(Glacier Point)
 * PAL_TYPE[3:2] = 0(TwinLake), 1(Crace Flat), 2(Glacier Point)
 * PAL_TYPE[1:0] = 0(TwinLake), 1(Crace Flat), 2(Glacier Point)
 */
int
plat_get_slot_type(uint8_t fru) {
  int type;

  if (read_device(SLOT_FILE, &type)) {
    printf("Get slot type failed\n");
    return -1;
  }

  switch(fru)
  {
    case 1:
      type = (type & (0x3 << 0)) >> 0;
    break;
    case 2:
      type = (type & (0x3 << 2)) >> 2;
    break;
    case 3:
      type = (type & (0x3 << 4)) >> 4;
    break;
    case 4:
      type = (type & (0x3 << 6)) >> 6;
    break;
  }

  return type;
}

static int
plat_get_ipmb_bus_id(uint8_t slot_id) {
  int bus_id;

  switch(slot_id) {
  case FRU_SLOT1:
    bus_id = IPMB_BUS_SLOT1;
    break;
  case FRU_SLOT2:
    bus_id = IPMB_BUS_SLOT2;
    break;
  case FRU_SLOT3:
    bus_id = IPMB_BUS_SLOT3;
    break;
  case FRU_SLOT4:
    bus_id = IPMB_BUS_SLOT4;
    break;
  default:
    bus_id = -1;
    break;
  }

  return bus_id;
}

/*
 * copy_eeprom_to_bin - copy the eeprom to binary file im /tmp directory
 *
 * @eeprom_file   : path for the eeprom of the device
 * @bin_file      : path for the binary file
 *
 * returns 0 on successful copy
 * returns non-zero on file operation errors
 */
int copy_eeprom_to_bin(const char * eeprom_file, const char * bin_file) {

  int eeprom;
  int bin;
  uint64_t tmp[FRUID_SIZE];
  ssize_t bytes_rd, bytes_wr;

  errno = 0;

  if (access(eeprom_file, F_OK) != -1) {

    eeprom = open(eeprom_file, O_RDONLY);
    if (eeprom == -1) {
      syslog(LOG_ERR, "copy_eeprom_to_bin: unable to open the %s file: %s",
          eeprom_file, strerror(errno));
      return errno;
    }

    bin = open(bin_file, O_WRONLY | O_CREAT, 0644);
    if (bin == -1) {
      syslog(LOG_ERR, "copy_eeprom_to_bin: unable to create %s file: %s",
          bin_file, strerror(errno));
      return errno;
    }

    bytes_rd = read(eeprom, tmp, FRUID_SIZE);
    if (bytes_rd != FRUID_SIZE) {
      syslog(LOG_ERR, "copy_eeprom_to_bin: write to %s file failed: %s",
          eeprom_file, strerror(errno));
      return errno;
    }

    bytes_wr = write(bin, tmp, bytes_rd);
    if (bytes_wr != bytes_rd) {
      syslog(LOG_ERR, "copy_eeprom_to_bin: write to %s file failed: %s",
          bin_file, strerror(errno));
      return errno;
    }

    close(bin);
    close(eeprom);
  }

  return 0;
}

int plat_fruid_init(void) {

  int ret;
  int fru=0;
  char path[128] = {0};
  char fpath[64] = {0};

  for (fru = 1; fru <= MAX_NUM_FRUS; fru++) {

    switch(fru) {
      case FRU_SLOT1:
      case FRU_SLOT2:
      case FRU_SLOT3:
      case FRU_SLOT4:
        switch(plat_get_slot_type(fru))
        {
           case SLOT_TYPE_SERVER:
             // Do not access EEPROM
             break;
           case SLOT_TYPE_CF:
           case SLOT_TYPE_GP:
             sprintf(path, EEPROM_DC, plat_get_ipmb_bus_id(fru), plat_get_ipmb_bus_id(fru));
             sprintf(fpath, BIN_SLOT, fru);
             ret = copy_eeprom_to_bin(path, fpath);
             break;
        }
        break;
      case FRU_SPB:
        ret = copy_eeprom_to_bin(EEPROM_SPB, BIN_SPB);
        break;
      case FRU_NIC:
        ret = copy_eeprom_to_bin(EEPROM_NIC, BIN_NIC);
        break;
      default:
        break;
    }  
  }

  return ret;
}

int plat_fruid_size(unsigned char payload_id) {
  char fpath[64] = {0};
  struct stat buf;
  int ret;

  // Fill the file path for a given slot
  sprintf(fpath, BIN_SLOT, payload_id);

  // check the size of the file and return size
  ret = stat(fpath, &buf);
  if (ret) {
    return 0;
  }

  return buf.st_size;
}

int plat_fruid_data(unsigned char payload_id, int offset, int count, unsigned char *data) {
  char fpath[64] = {0};
  int fd;
  int ret;

  // Fill the file path for a given slot
  sprintf(fpath, BIN_SLOT, payload_id);

  // open file for read purpose
  fd = open(fpath, O_RDONLY);
  if (fd < 0) {
    return fd;
  }

  // seek position based on given offset
  ret = lseek(fd, offset, SEEK_SET);
  if (ret < 0) {
    close(fd);
    return ret;
  }

  // read the file content
  ret = read(fd, data, count);
  if (ret != count) {
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}
