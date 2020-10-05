#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/power_supply.h>

#include <uapi/linux/power_supply.h>
#include "dummy_psu.h"

static int strtoenum(const char *str_enum)
{
	if (!strcmp("POWER_SUPPLY_PROP_STATUS", str_enum)) {
		return POWER_SUPPLY_PROP_STATUS;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_TYPE", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_TYPE;
	};
	if (!strcmp("POWER_SUPPLY_PROP_HEALTH", str_enum)) {
		return POWER_SUPPLY_PROP_HEALTH;
	};
	if (!strcmp("POWER_SUPPLY_PROP_PRESENT", str_enum)) {
		return POWER_SUPPLY_PROP_PRESENT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_ONLINE", str_enum)) {
		return POWER_SUPPLY_PROP_ONLINE;
	};
	if (!strcmp("POWER_SUPPLY_PROP_AUTHENTIC", str_enum)) {
		return POWER_SUPPLY_PROP_AUTHENTIC;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TECHNOLOGY", str_enum)) {
		return POWER_SUPPLY_PROP_TECHNOLOGY;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CYCLE_COUNT", str_enum)) {
		return POWER_SUPPLY_PROP_CYCLE_COUNT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MAX", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MIN", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_MIN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_NOW", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_NOW;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_AVG", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_AVG;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_OCV", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_OCV;
	};
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_BOOT", str_enum)) {
		return POWER_SUPPLY_PROP_VOLTAGE_BOOT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_MAX", str_enum)) {
		return POWER_SUPPLY_PROP_CURRENT_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_NOW", str_enum)) {
		return POWER_SUPPLY_PROP_CURRENT_NOW;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_AVG", str_enum)) {
		return POWER_SUPPLY_PROP_CURRENT_AVG;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_BOOT", str_enum)) {
		return POWER_SUPPLY_PROP_CURRENT_BOOT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_POWER_NOW", str_enum)) {
		return POWER_SUPPLY_PROP_POWER_NOW;
	};
	if (!strcmp("POWER_SUPPLY_PROP_POWER_AVG", str_enum)) {
		return POWER_SUPPLY_PROP_POWER_AVG;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_EMPTY_DESIGN", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_EMPTY_DESIGN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_FULL", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_FULL;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_EMPTY", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_EMPTY;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_NOW", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_NOW;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_AVG", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_AVG;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_COUNTER", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_COUNTER;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT", str_enum)) {
		return POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX",
		    str_enum)) {
		return POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE", str_enum)) {
		return POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE_MAX",
		    str_enum)) {
		return POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_START_THRESHOLD",
		    str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_CONTROL_START_THRESHOLD;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD",
		    str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD;
	};
	if (!strcmp("POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT", str_enum)) {
		return POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT", str_enum)) {
		return POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_INPUT_POWER_LIMIT", str_enum)) {
		return POWER_SUPPLY_PROP_INPUT_POWER_LIMIT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN", str_enum)) {
		return POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_EMPTY_DESIGN", str_enum)) {
		return POWER_SUPPLY_PROP_ENERGY_EMPTY_DESIGN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_FULL", str_enum)) {
		return POWER_SUPPLY_PROP_ENERGY_FULL;
	};
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_EMPTY", str_enum)) {
		return POWER_SUPPLY_PROP_ENERGY_EMPTY;
	};
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_NOW", str_enum)) {
		return POWER_SUPPLY_PROP_ENERGY_NOW;
	};
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_AVG", str_enum)) {
		return POWER_SUPPLY_PROP_ENERGY_AVG;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY", str_enum)) {
		return POWER_SUPPLY_PROP_CAPACITY;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN", str_enum)) {
		return POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX", str_enum)) {
		return POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY_LEVEL", str_enum)) {
		return POWER_SUPPLY_PROP_CAPACITY_LEVEL;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_MAX", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_MIN", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP_MIN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_ALERT_MIN", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP_ALERT_MIN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_ALERT_MAX", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP_ALERT_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_AMBIENT", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP_AMBIENT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MAX", str_enum)) {
		return POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MAX;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW", str_enum)) {
		return POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG", str_enum)) {
		return POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_FULL_NOW", str_enum)) {
		return POWER_SUPPLY_PROP_TIME_TO_FULL_NOW;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_FULL_AVG", str_enum)) {
		return POWER_SUPPLY_PROP_TIME_TO_FULL_AVG;
	};
	if (!strcmp("POWER_SUPPLY_PROP_TYPE", str_enum)) {
		return POWER_SUPPLY_PROP_TYPE;
	};
	if (!strcmp("POWER_SUPPLY_PROP_USB_TYPE", str_enum)) {
		return POWER_SUPPLY_PROP_USB_TYPE;
	};
	if (!strcmp("POWER_SUPPLY_PROP_SCOPE", str_enum)) {
		return POWER_SUPPLY_PROP_SCOPE;
	};
	if (!strcmp("POWER_SUPPLY_PROP_PRECHARGE_CURRENT", str_enum)) {
		return POWER_SUPPLY_PROP_PRECHARGE_CURRENT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT", str_enum)) {
		return POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT;
	};
	if (!strcmp("POWER_SUPPLY_PROP_CALIBRATE", str_enum)) {
		return POWER_SUPPLY_PROP_CALIBRATE;
	};
	if (!strcmp("POWER_SUPPLY_PROP_MODEL_NAME", str_enum)) {
		return POWER_SUPPLY_PROP_MODEL_NAME;
	};
	if (!strcmp("POWER_SUPPLY_PROP_MANUFACTURER", str_enum)) {
		return POWER_SUPPLY_PROP_MANUFACTURER;
	};
	if (!strcmp("POWER_SUPPLY_PROP_SERIAL_NUMBER", str_enum)) {
		return POWER_SUPPLY_PROP_SERIAL_NUMBER;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_MAINS", str_enum)) {
		return POWER_SUPPLY_TYPE_MAINS;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_BATTERY", str_enum)) {
		return POWER_SUPPLY_TYPE_BATTERY;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB", str_enum)) {
		return POWER_SUPPLY_TYPE_USB;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_UNKNOWN", str_enum)) {
		return POWER_SUPPLY_TYPE_UNKNOWN;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_BATTERY", str_enum)) {
		return POWER_SUPPLY_TYPE_BATTERY;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_UPS", str_enum)) {
		return POWER_SUPPLY_TYPE_UPS;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_MAINS", str_enum)) {
		return POWER_SUPPLY_TYPE_MAINS;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB", str_enum)) {
		return POWER_SUPPLY_TYPE_USB;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB_DCP", str_enum)) {
		return POWER_SUPPLY_TYPE_USB_DCP;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB_CDP", str_enum)) {
		return POWER_SUPPLY_TYPE_USB_CDP;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB_ACA", str_enum)) {
		return POWER_SUPPLY_TYPE_USB_ACA;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB_TYPE_C", str_enum)) {
		return POWER_SUPPLY_TYPE_USB_TYPE_C;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB_PD", str_enum)) {
		return POWER_SUPPLY_TYPE_USB_PD;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_USB_PD_DRP", str_enum)) {
		return POWER_SUPPLY_TYPE_USB_PD_DRP;
	};
	if (!strcmp("POWER_SUPPLY_TYPE_APPLE_BRICK_ID", str_enum)) {
		return POWER_SUPPLY_TYPE_APPLE_BRICK_ID;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_UNKNOWN", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_UNKNOWN;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_SDP", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_SDP;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_DCP", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_DCP;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_CDP", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_CDP;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_ACA", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_ACA;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_C", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_C;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_PD", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_PD;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_PD_DRP", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_PD_DRP;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_PD_PPS", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_PD_PPS;
	};
	if (!strcmp("POWER_SUPPLY_USB_TYPE_APPLE_BRICK_ID", str_enum)) {
		return POWER_SUPPLY_USB_TYPE_APPLE_BRICK_ID;
	};

	return -1;
}

// battery_property_map related picked and adapted from test_power.c
// power_supply_sysfs.c holds all the values
struct battery_property_map {
	int value;
	char const *key;
};

static struct battery_property_map map_status[] = {
	{ POWER_SUPPLY_STATUS_UNKNOWN, "Unknown" },
	{ POWER_SUPPLY_STATUS_CHARGING, "charging" },
	{ POWER_SUPPLY_STATUS_DISCHARGING, "discharging" },
	{ POWER_SUPPLY_STATUS_NOT_CHARGING, "not-charging" },
	{ POWER_SUPPLY_STATUS_FULL, "full" },
	{ -1, NULL },
};

static struct battery_property_map map_health[] = {
	{ POWER_SUPPLY_HEALTH_UNKNOWN, "Unknown" },
	{ POWER_SUPPLY_HEALTH_GOOD, "good" },
	{ POWER_SUPPLY_HEALTH_OVERHEAT, "overheat" },
	{ POWER_SUPPLY_HEALTH_DEAD, "dead" },
	{ POWER_SUPPLY_HEALTH_OVERVOLTAGE, "overvoltage" },
	{ POWER_SUPPLY_HEALTH_UNSPEC_FAILURE, "failure" },
	{ -1, NULL },
};

static struct battery_property_map map_technology[] = {
	{ POWER_SUPPLY_TECHNOLOGY_UNKNOWN, "Unknown" },
	{ POWER_SUPPLY_TECHNOLOGY_NiMH, "NiMH" },
	{ POWER_SUPPLY_TECHNOLOGY_LION, "Li-ion" },
	{ POWER_SUPPLY_TECHNOLOGY_LIPO, "Li-poly" },
	{ POWER_SUPPLY_TECHNOLOGY_LiFe, "LiFe" },
	{ POWER_SUPPLY_TECHNOLOGY_NiCd, "NiCd" },
	{ POWER_SUPPLY_TECHNOLOGY_LiMn, "LiMn" },
	{ -1, NULL },
};

static struct battery_property_map map_capacity[] = {
	{ POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN, "Unknown" },
	{ POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL, "Critical" },
	{ POWER_SUPPLY_CAPACITY_LEVEL_LOW, "Low" },
	{ POWER_SUPPLY_CAPACITY_LEVEL_NORMAL, "Normal" },
	{ POWER_SUPPLY_CAPACITY_LEVEL_HIGH, "High" },
	{ POWER_SUPPLY_CAPACITY_LEVEL_FULL, "Full" },
	{ -1, NULL },
};

static int map_get_value(struct battery_property_map *map, const char *key)
{
	char buf[MAX_KEYLENGTH];
	int cr;

	strncpy(buf, key, MAX_KEYLENGTH);
	buf[MAX_KEYLENGTH - 1] = '\0';

	cr = strnlen(buf, MAX_KEYLENGTH) - 1;
	if (cr < 0)
		return -1;
	if (buf[cr] == '\n')
		buf[cr] = '\0';

	while (map->key) {
		if (strncasecmp(map->key, buf, MAX_KEYLENGTH) == 0)
			return map->value;
		map++;
	}

	return -1;
}

union psp_propval {
	int intval;
	char strval[MAX_KEYLENGTH];
};

struct psu {
	struct power_supply *psy;
	struct power_supply_desc desc;
	struct power_supply_config config;

	char dev_name[MAX_KEYLENGTH];

	int num_properties;
	enum power_supply_property props[MAX_PSU_PROPS];

	int num_usb_types;
	enum power_supply_usb_type usb_types[MAX_PSU_UBS_TYPES];

	int num_supplicants;
	char *supplied_to[MAX_PSU];

	union psp_propval psp_val[MAX_PSU_PROPS];
};

static int dummy_get_psp(struct power_supply *psy,
			 enum power_supply_property psp,
			 union power_supply_propval *val)
{
	int ret = 0;
	struct file *filep;
	struct psu *pobj;

	filep = psy->drv_data;
	if (filep->private_data) {
		pobj = filep->private_data;
		if (pobj->psp_val) {
			if (power_supply_is_str_property(psp)) {
				val->strval = pobj->psp_val[psp].strval;
			} else {
				val->intval = pobj->psp_val[psp].intval;
			}
		} else {
			ret = -EINVAL;
		}
	} else {
		ret = -EINVAL;
	}
	return ret;
}

static ssize_t dummy_psu_read(struct file *file, char __user *buf, size_t count,
			      loff_t *ppos)
{
	pr_debug("dummy_psu_read size(%ld)\n", count);
	return 0;
}

static ssize_t dummy_psu_write(struct file *filep, const char *buf, size_t size,
			       loff_t *offp)
{
	pr_debug("dummy_psu_write size(%ld)\n", size);
	return size;
}

int dummy_psu_open(struct inode *inode, struct file *filep)
{
	filep->private_data = NULL;
	return 0;
}

int dummy_psu_release(struct inode *inode, struct file *filep)
{
	struct psu *pobj;
	if (filep->private_data) {
		pobj = filep->private_data;
		printk("%s", pobj->dev_name);
		if (pobj->psy) {
			power_supply_unregister(pobj->psy);
		}
		kfree(pobj);
	}
	return 0;
}

long dummy_psu_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int i;
	bool psp_found;
	struct psu *pobj;
	struct ioctl_psu psu_spec;
	struct ioctl_pspval propval;
	char psp[MAX_KEYLENGTH];
	char usbtype[MAX_KEYLENGTH];
	char supplied_to[MAX_KEYLENGTH];

	enum power_supply_property parsed_psp;
	enum power_supply_usb_type parsed_usbtype;

	switch (cmd) {
	case IOCTL_PSU_CREATE:
		if (filep->private_data)
			return -EBUSY;

		if (copy_from_user(&psu_spec, (void __user *)arg,
				   sizeof(struct ioctl_psu)) != 0)
			return -EFAULT;

		pobj = kzalloc(sizeof(struct psu), GFP_KERNEL);
		if (IS_ERR(pobj)) {
			pr_err("IOCTL_PSU_CREATE: Failed to create PSU object\n");
			return PTR_ERR(pobj);
		}
		filep->private_data = pobj;

		strncpy(pobj->dev_name, psu_spec.dev_name, MAX_KEYLENGTH);
		pobj->dev_name[MAX_KEYLENGTH - 1] = '\0';
		pobj->desc.type = strtoenum(psu_spec.dev_type);

		pobj->num_properties = 0;
		pobj->num_usb_types = 0;
		pobj->num_supplicants = 0;
		break;
	case IOCTL_PSU_ADD_PSP:
		if (filep->private_data) {
			pobj = filep->private_data;
			if (pobj->num_properties >= MAX_PSU_PROPS) {
				pr_err("IOCTL_PSU_ADD_PSP: %s: Cannot add property. Number of properties is reached\n",
				       pobj->dev_name);
				return -EFAULT;
			}
			if (copy_from_user(&psp, (void __user *)arg,
					   sizeof(char[MAX_KEYLENGTH])) != 0)
				return -EFAULT;
			parsed_psp = strtoenum(psp);
			if ((parsed_psp >= MAX_PSU_PROPS) || (parsed_psp < 0)) {
				pr_err("IOCTL_PSU_ADD_PSP: %d: Invalid property.\n",
				       parsed_psp);
				return -EFAULT;
			}
			pr_debug(
				"IOCTL_PSU_ADD_PSP: %s: Adding power_supply_property %s (%d)",
				pobj->dev_name, psp, parsed_psp);
			pobj->props[pobj->num_properties] = parsed_psp;
			pobj->num_properties += 1;
		} else {
			pr_err("IOCTL_PSU_ADD_PSP: Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
			return -EINVAL;
		}
		break;
	case IOCTL_PSU_ADD_USB_TYPE:
		if (filep->private_data) {
			pobj = filep->private_data;
			if (pobj->num_usb_types >= MAX_PSU_UBS_TYPES) {
				pr_err("IOCTL_PSU_ADD_USB_TYPE: %s: Cannot add USB type. Number of USB types is reached\n",
				       pobj->dev_name);
				return -EFAULT;
			}
			if (copy_from_user(&usbtype, (void __user *)arg,
					   sizeof(char[MAX_KEYLENGTH])) != 0)
				return -EFAULT;
			parsed_usbtype = strtoenum(usbtype);
			if ((parsed_usbtype >= MAX_PSU_UBS_TYPES) ||
			    (parsed_usbtype < 0)) {
				pr_err("IOCTL_PSU_ADD_USB_TYPE: %d: Invalid USB type.\n",
				       parsed_usbtype);
				return -EFAULT;
			}
			pr_debug(
				"IOCTL_PSU_ADD_USB_TYPE: %s: Adding USB type %d",
				pobj->dev_name, parsed_usbtype);
			pobj->usb_types[pobj->num_usb_types] = parsed_usbtype;
			pobj->num_usb_types += 1;
		} else {
			pr_err("IOCTL_PSU_ADD_USB_TYPE: Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
			return -EINVAL;
		}
		break;
	case IOCTL_PSU_ADD_SUPPLIED_TO:
		if (filep->private_data) {
			pobj = filep->private_data;
			if (pobj->num_supplicants >= MAX_SUPPLICANTS) {
				pr_err("IOCTL_PSU_ADD_SUPPLIED_TO: %s: Cannot add supplicant. Number of supplicants is reached\n",
				       pobj->dev_name);
				return -EFAULT;
			}
			if (copy_from_user(&supplied_to, (void __user *)arg,
					   sizeof(char[MAX_KEYLENGTH])) != 0)
				return -EFAULT;
			pobj->supplied_to[pobj->num_supplicants] = supplied_to;
			pobj->num_supplicants += 1;
		} else {
			pr_err("IOCTL_PSU_ADD_SUPPLIED_TO: Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
			return -EINVAL;
		}
		break;
	case IOCTL_PSU_UPDATE_PROPVAL:
		psp_found = false;
		if (filep->private_data) {
			pobj = filep->private_data;
			if (pobj->psy) {
				if (copy_from_user(
					    &propval, (void __user *)arg,
					    sizeof(struct ioctl_pspval)) != 0)
					return -EFAULT;
				parsed_psp = strtoenum(propval.psp);
				for (i = 0; i <= pobj->num_properties; i++) {
					if (parsed_psp == pobj->props[i]) {
						if (power_supply_is_str_property(
							    parsed_psp)) {
							strncpy(pobj->psp_val[parsed_psp]
									.strval,
								propval.val,
								MAX_KEYLENGTH);
							pobj->psp_val[parsed_psp]
								.strval[MAX_KEYLENGTH -
									1] =
								'\0';
						} else if (parsed_psp ==
							   POWER_SUPPLY_PROP_STATUS) {
							pobj->psp_val[parsed_psp]
								.intval = map_get_value(
								map_status,
								propval.val);
						} else if (parsed_psp ==
							   POWER_SUPPLY_PROP_HEALTH) {
							pobj->psp_val[parsed_psp]
								.intval = map_get_value(
								map_health,
								propval.val);
						} else if (parsed_psp ==
							   POWER_SUPPLY_PROP_TECHNOLOGY) {
							pobj->psp_val[parsed_psp]
								.intval = map_get_value(
								map_technology,
								propval.val);
						} else if (parsed_psp ==
							   POWER_SUPPLY_PROP_CAPACITY_LEVEL) {
							pobj->psp_val[parsed_psp]
								.intval = map_get_value(
								map_capacity,
								propval.val);
						} else {
							if (kstrtoint(
								    propval.val,
								    10,
								    &pobj->psp_val[parsed_psp]
									     .intval) !=
							    0) {
								pr_debug(
									"IOCTL_PSU_UPDATE_PROPVAL: %s: %s: Failed to get property value\n",
									pobj->dev_name,
									propval.psp);
								return -EINVAL;
							}
						}
						power_supply_changed(pobj->psy);
						psp_found = true;
					}
				}
				if (!psp_found)
					pr_err("IOCTL_PSU_UPDATE_PROPVAL: %s: %s: Unknown property\n",
					       pobj->dev_name, propval.psp);
			} else {
				pr_err("IOCTL_PSU_UPDATE_PROPVAL: Cannot find PSU device. Missing IOCTL_PSU_REGISTER call?\n");
				return -EINVAL;
			}
		} else {
			pr_err("IOCTL_PSU_UPDATE_PROPVAL: Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
			return -EINVAL;
		}
		break;
	case IOCTL_PSU_REGISTER:
		if (filep->private_data) {
			pobj = filep->private_data;
			if (!(pobj->psy)) {
				pobj->desc.name = pobj->dev_name;
				pobj->desc.properties = pobj->props;
				pobj->desc.num_properties =
					pobj->num_properties;
				pobj->desc.get_property = dummy_get_psp,
				pobj->psy = power_supply_register(
					NULL, &pobj->desc, &pobj->config);
				if (IS_ERR(pobj->psy)) {
					ret = PTR_ERR(pobj->psy);
					pr_err("IOCTL_PSU_REGISTER: Failed to register\n");
					pobj->psy = NULL;
					return ret;
				} else {
					pobj->psy->drv_data = filep;
				}
			} else {
				return -EBUSY;
			}
		}
		break;
	default:
		pr_err("dummy_psu: Invalid IOCTL command\n");
		return -ENOTTY;
	}
	return ret;
}

static struct file_operations dummy_psu_fops = {
	read: dummy_psu_read,
	write: dummy_psu_write,
	open: dummy_psu_open,
	release: dummy_psu_release,
	unlocked_ioctl: dummy_psu_ioctl,
};

static struct miscdevice dummy_psu_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "dummy_psu",
	.fops = &dummy_psu_fops,
	.mode = S_IRWXUGO,
};

static int __init dummy_power_init(void)
{
	int ret = 0;
	pr_info("Loading dummy power supply driver\n");
	ret = misc_register(&dummy_psu_dev);
	if (ret)
		pr_err("Unable to register dummy_psu device\n");
	return ret;
}

static void __exit dummy_power_exit(void)
{
	pr_info("Unloading dummy power supply driver\n");
	misc_deregister(&dummy_psu_dev);
}

module_init(dummy_power_init);
module_exit(dummy_power_exit);

MODULE_AUTHOR("Frédéric Pierret (fepitre) <frederic.pierret@qubes-os.org>");
MODULE_DESCRIPTION("Dummy power supply driver");
MODULE_LICENSE("GPL");
