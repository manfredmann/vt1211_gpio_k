/*
 * GPIO interface for VT1211 Super I/O chip
 *
 * Copyright 2018 by Roman Serov <roman@serov.co>
 * 
 * This file is part of VT1211 GPIO interface.
 *
 * VT1211 GPIO interface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * VT1211 GPIO interface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with VT1211 GPIO interface. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
*/

#ifndef VT1211_H
#define VT1211_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mogduleparam.h>
#include <linux/gpio.h>
#include <linux/gpio/driver.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/types.h>

#define INFO_PREFIX                   "VT1211: "

#define VT_DRVNAME                    "vt1211_gpio"

#define VT_MAX_GPIO_PORT1             8
#define VT_MAX_GPIO_PORT3_6           27

#define VT_INIT_OK                    0x00
#define VT_INIT_NOT_FOUND             0x01
#define VT_INIT_NO_PORT               0x02

#define VT_CIR                        0x2E
#define VT_CDR                        0x2F

#define VT_ENTER_CONFIG               0x87
#define VT_EXIT_CONFIG                0xAA

#define VT_PORT_1                     0x00
#define VT_PORT_3                     0x08
#define VT_PORT_4                     0x10
#define VT_PORT_5                     0x18
#define VT_PORT_6                     0x20

#define VT_PORT_1_OFFSET              0x00
#define VT_PORT_3_OFFSET              0x01
#define VT_PORT_4_OFFSET              0x02
#define VT_PORT_5_OFFSET              0x03
#define VT_PORT_6_OFFSET              0x04

#define VT_CONFIG_LDN                 0x07
#define VT_CONFIG_DEV_ID              0x20
#define VT_CONFIG_DEV_REV             0x21
#define VT_CONFIG_GPIO_P1_PIN_SELECT  0x24
#define VT_CONFIG_GPIO_P2_PIN_SELECT  0x25

#define VT_CONFIG_PORT_1              0x01
#define VT_CONFIG_PORT_3_6            0x04

#define VT_LDN_GPIO                   0x08

#define VT_LDN_GPIO_ACTIVATE          0x30
#define VT_LDN_GPIO_BADDR_B1          0x60
#define VT_LDN_GPIO_BADDR_B2          0x61
#define VT_LDN_GPIO_PORT_SELECT       0xF0
#define VT_LDN_GPIO_MODE_CONFIG       0xF1
#define VT_LDN_GPIO_POLAR_CONFIG      0xF2

#define VT_PIN_INPUT                  0x1
#define VT_PIN_OUTPUT                 0x0

#endif
