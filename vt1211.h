#ifndef VT1211_H
#define VT1211_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
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
