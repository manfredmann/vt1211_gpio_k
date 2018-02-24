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

#include "vt1211.h"

static bool port_1 = true;
static bool port_3_6 = false;

module_param(port_1, bool, 0644);
module_param(port_3_6, bool, 0644);
MODULE_PARM_DESC(port_1, "PORT1 Enable");
MODULE_PARM_DESC(port_3_6, "From PORT3 to PORT6 Enable");

struct vt1211_gpio {
  struct gpio_chip chip;
  spinlock_t lock;
  u8 dev_id;
  u8 dev_rev;
  u16 io_base;
  u16 io_size;
};

static struct vt1211_gpio vt1211_gpio_chip = {
  .lock = __SPIN_LOCK_UNLOCKED(vt1211_gpio_chip.lock),
};

static u8 vt1211_get_port_offset(unsigned gpio_offset) {
  if ((gpio_offset & VT_PORT_6) == VT_PORT_6) {
    return VT_PORT_6_OFFSET;
  } else if ((gpio_offset & VT_PORT_5) == VT_PORT_5) {
    return VT_PORT_5_OFFSET;
  } else if (gpio_offset & VT_PORT_4) {
    return VT_PORT_4_OFFSET;
  } else if (gpio_offset & VT_PORT_3) {
    return VT_PORT_3_OFFSET;
  } else {
    return VT_PORT_1_OFFSET;
  }
}

static u8 vt1211_get_pin_n(u8 gpio_offset) {
  return (gpio_offset & 0x7) ;
}

static void vt1211_config_mode(void) {
  outb(VT_ENTER_CONFIG, VT_CIR);
  outb(VT_ENTER_CONFIG, VT_CIR);
}

static void vt1211_config_mode_exit(void) {
  outb(VT_EXIT_CONFIG, VT_CIR);
}

static void vt1211_config_mode_gpio(void) {
  outb(VT_CONFIG_LDN, VT_CIR);
  outb(VT_LDN_GPIO, VT_CDR);
}

static u16 vt1211_get_id_and_rev(void) {
  u8 id, rev;

  vt1211_config_mode();

  outb(VT_CONFIG_DEV_ID, VT_CIR);
  id = inb(VT_CDR);

  outb(VT_CONFIG_DEV_REV, VT_CIR);
  rev = inb(VT_CDR);

  vt1211_config_mode_exit();

  return ((id << 8) | rev);
}

static void vt1211_port_as_gpio(u8 port) {
  u8 d_tmp;

  vt1211_config_mode();

  switch(port) {
    case VT_CONFIG_PORT_1: {
      outb(VT_CONFIG_GPIO_P1_PIN_SELECT, VT_CIR);
      outb(0xFF, VT_CDR);
    }
    break;
    case VT_CONFIG_PORT_3_6: {
      outb(VT_CONFIG_GPIO_P2_PIN_SELECT, VT_CIR);
      d_tmp = inb(VT_CDR);
      d_tmp |= 0b00100000;

      outb(VT_CONFIG_GPIO_P2_PIN_SELECT, VT_CIR);
      outb(d_tmp, VT_CDR);
    }
    break;
  }

  vt1211_config_mode_exit();
}

static u16 vt1211_get_baddr(void) {
  u8 b1, b2;

  vt1211_config_mode();
  vt1211_config_mode_gpio();

  outb(VT_LDN_GPIO_BADDR_B1, VT_CIR);
  b1 = inb(VT_CDR);

  outb(VT_LDN_GPIO_BADDR_B2, VT_CIR);
  b2 = inb(VT_CDR);

  vt1211_config_mode_exit();

  return ((b1 << 8) | b2);
}

static void vt1211_gpio_activate(void) {
  vt1211_config_mode();
  vt1211_config_mode_gpio();

  outb(VT_LDN_GPIO_ACTIVATE, VT_CIR);
  outb(0x01, VT_CDR);

  vt1211_config_mode_exit();
}

static int vt1211_init(u8 ports) {
  struct vt1211_gpio *gpio_data = &vt1211_gpio_chip;

  u16 id_rev;
  bool port_selected;

  id_rev = vt1211_get_id_and_rev();

  gpio_data->dev_id  = ((id_rev & 0xFF00) >> 8);
  gpio_data->dev_rev = (id_rev & 0x00FF);

  if (gpio_data->dev_id != 0x3C) {
    return VT_INIT_NOT_FOUND;
  }

  if (ports & VT_CONFIG_PORT_1) {
    vt1211_port_as_gpio(VT_CONFIG_PORT_1);
    gpio_data->io_size = VT_PORT_1_OFFSET + 1;
    port_selected = true;
  }

  if (ports & VT_CONFIG_PORT_3_6) {
    vt1211_port_as_gpio(VT_CONFIG_PORT_3_6);
    gpio_data->io_size = VT_PORT_6_OFFSET + 1;
    port_selected = true;
  }

  if (!port_selected) {
    return VT_INIT_NO_PORT;
  }

  gpio_data->io_base = vt1211_get_baddr();
  vt1211_gpio_activate();

  return VT_INIT_OK;
}

static void vt1211_set_pin_direction(u8 port, u8 pin, u8 direction) {
  vt1211_config_mode();
  vt1211_config_mode_gpio();

  outb(VT_LDN_GPIO_PORT_SELECT, VT_CIR);
  outb(port, VT_CDR);

  outb(VT_LDN_GPIO_MODE_CONFIG, VT_CIR);
  u8 m_data = inb(VT_CDR);

  if (direction == VT_PIN_OUTPUT) {
    m_data |= pin;
  } else {
    m_data &= ~(pin);
  }
  
  outb(VT_LDN_GPIO_MODE_CONFIG, VT_CIR);
  outb(m_data, VT_CDR);

  vt1211_config_mode_exit();
}

static int vt1211_gpio_direction_input(struct gpio_chip *chip, unsigned offset) {
  struct vt1211_gpio *gpio_data = &vt1211_gpio_chip;

  u8 port = vt1211_get_port_offset(offset);
  u8 pin = vt1211_get_pin_n(offset);

  spin_lock(&gpio_data->lock);

  vt1211_set_pin_direction(port, (0x1 << pin), VT_PIN_INPUT);

  spin_unlock(&gpio_data->lock);

  return 0;
}

static int vt1211_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value) {
  struct vt1211_gpio *gpio_data = &vt1211_gpio_chip;

  u8 port = vt1211_get_port_offset(offset);
  u8 pin = vt1211_get_pin_n(offset);

  spin_lock(&gpio_data->lock);

  vt1211_set_pin_direction(port, (0x1 << pin), VT_PIN_OUTPUT);

  spin_unlock(&gpio_data->lock);

  return 0;
}

static int vt1211_gpio_get(struct gpio_chip *chip, unsigned offset) {
  struct vt1211_gpio *gpio_data = &vt1211_gpio_chip;

  u8 port = vt1211_get_port_offset(offset);
  u8 pin = vt1211_get_pin_n(offset);
  u8 data = inb(gpio_data->io_base + port);

  return (data & (0x1 << pin));
}

static void vt1211_gpio_set(struct gpio_chip *chip, unsigned offset, int value) {
  struct vt1211_gpio *gpio_data = &vt1211_gpio_chip;

  u8 port = vt1211_get_port_offset(offset);
  u8 pin = vt1211_get_pin_n(offset);

  u8 data = inb(gpio_data->io_base + port);

  if (value) {
    data |= 0x1 << pin;
  } else {
    data &= ~(0x1 << pin);
  }

  outb(data, gpio_data->io_base + port);
}

static int vt1211_gpio_get_direction(struct gpio_chip *gc, unsigned offset) {
  struct vt1211_gpio *gpio_data = &vt1211_gpio_chip;

  spin_lock(&gpio_data->lock);

  vt1211_config_mode();
  vt1211_config_mode_gpio();

  u8 port = vt1211_get_port_offset(offset);
  u8 pin = vt1211_get_pin_n(offset);

  outb(VT_LDN_GPIO_PORT_SELECT, VT_CIR);
  outb(port, VT_CDR);

  outb(VT_LDN_GPIO_MODE_CONFIG, VT_CIR);
  u8 data = inb(VT_CDR);

  vt1211_config_mode_exit();

  spin_unlock(&gpio_data->lock);

  return ~(data & (0x1 << pin));
}

static const struct gpio_chip vt1211_gpio_chip_tmpl = {
  .label             = "vt1211",
  .owner            = THIS_MODULE,
  .direction_input  = vt1211_gpio_direction_input,
  .get              = vt1211_gpio_get,
  .direction_output = vt1211_gpio_direction_output,
  .set              = vt1211_gpio_set,
  .get_direction    = vt1211_gpio_get_direction,
  .base             = -1,
  .can_sleep        = 0,
};

static int __init vt1211_gpio_init( void ) {
  printk(KERN_INFO INFO_PREFIX "GPIO init\n");

  struct vt1211_gpio *vt1211_gpio = &vt1211_gpio_chip;
  char *labels;
  char **labels_table;

  vt1211_gpio->chip = vt1211_gpio_chip_tmpl;
  vt1211_gpio->chip.ngpio = 0;

  u16 ports = 0;

  if (port_1) {
    ports |= VT_CONFIG_PORT_1;
    vt1211_gpio->chip.ngpio += VT_MAX_GPIO_PORT1;
  }

  if (port_3_6) {
    ports |= VT_CONFIG_PORT_3_6;
    vt1211_gpio->chip.ngpio += VT_MAX_GPIO_PORT3_6;
  }

  int init_res = vt1211_init(ports);
  int err;

  switch (init_res) {
    case VT_INIT_NOT_FOUND: {
      printk(KERN_ERR INFO_PREFIX "Device not found\n");
      return -ENODEV;
    }
    case VT_INIT_NO_PORT: {
      printk(KERN_ERR INFO_PREFIX "No port selected\n");
      return -ENODEV;
    }
    case VT_INIT_OK:
    default: {
      printk(KERN_NOTICE INFO_PREFIX "ID: %02X, Revision: %02X, Base addr.: %04X\n", vt1211_gpio->dev_id, vt1211_gpio->dev_rev, vt1211_gpio->io_base);
    }
  }

  if (!request_region(vt1211_gpio->io_base, vt1211_gpio->io_size, VT_DRVNAME))
    return -EBUSY;

  labels = kcalloc(vt1211_gpio->chip.ngpio, sizeof("vt1211_gpXY"), GFP_KERNEL);
  labels_table = kcalloc(vt1211_gpio->chip.ngpio, sizeof(const char *), GFP_KERNEL);

  if (!labels || !labels_table) {
    err = -ENOMEM;
    goto EXIT_ON_FAIL;
  }

  for (u8 i = 0; i < vt1211_gpio->chip.ngpio; ++i) {
    char *label = &labels[i * sizeof("vt1211_gpXY")];

    u8 port = 1 + (i / 8);

    if (port != 1 && port_1) {
      ++port;
    }

    if (!port_1 && port_3_6) {
      port += 2;
    }

    u8 pin = vt1211_get_pin_n(i);

    sprintf(label, "vt1211_gp%u%u", port, pin);
    labels_table[i] = label;
  }

  vt1211_gpio->chip.names = (const char *const*)labels_table;

  err = gpiochip_add_data(&vt1211_gpio->chip, vt1211_gpio);
  if (err)
    goto EXIT_ON_FAIL;

  return 0;

EXIT_ON_FAIL:
  kfree(labels_table);
  kfree(labels);
  release_region(vt1211_gpio->io_base, vt1211_gpio->io_size);
  return err;
}

static void __exit vt1211_gpio_exit(void) {
  struct vt1211_gpio *vt1211_gpio = &vt1211_gpio_chip;

  gpiochip_remove(&vt1211_gpio->chip);
  release_region(vt1211_gpio->io_base, vt1211_gpio->io_size);
  kfree(vt1211_gpio->chip.names[0]);
  kfree(vt1211_gpio->chip.names);

  printk(KERN_INFO INFO_PREFIX "Goodbye cruel world!\n" );
}

module_init(vt1211_gpio_init);
module_exit(vt1211_gpio_exit);

MODULE_AUTHOR( "Roman Serov <roman@serov.co>" );
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0") ;
MODULE_DESCRIPTION("GPIO interface for VT1211 Super I/O chip");
