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

// Header waiting for upstream
#include "power_supply.h"
#include "dummy_psu.h"

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
	struct ioctl_propval propval;
	char supplied_to[MAX_KEYLENGTH];
	enum power_supply_property psp;
	enum power_supply_usb_type usbtype;

	switch (cmd) {
	case IOCTL_PSU_CREATE:
		if (filep->private_data)
			return -EBUSY;

		if (copy_from_user(&psu_spec, (void __user *)arg,
				   sizeof(struct ioctl_psu)) != 0)
			return -EFAULT;

		pobj = kzalloc(sizeof(struct psu), GFP_KERNEL);
		if (IS_ERR(pobj)) {
			pr_err("Failed to create PSU object\n");
			return PTR_ERR(pobj);
		}
		filep->private_data = pobj;

		strncpy(pobj->dev_name, psu_spec.dev_name, MAX_KEYLENGTH);
		pobj->dev_name[MAX_KEYLENGTH - 1] = '\0';
		pobj->desc.type = psu_spec.dev_type;

		pobj->num_properties = 0;
		pobj->num_usb_types = 0;
		pobj->num_supplicants = 0;
		break;
	case IOCTL_PSU_ADD_PSP:
		if (filep->private_data) {
			pobj = filep->private_data;
			if (copy_from_user(
				    &psp, (void __user *)arg,
				    sizeof(enum power_supply_property)) != 0)
				return -EFAULT;
			if (pobj->num_properties >= MAX_PSU_PROPS) {
				pr_err("%s: Cannot add property. Number of properties is reached\n",
				       pobj->dev_name);
				return -EFAULT;
			}
			pr_debug("%s: Adding power_supply_property %d",
				 pobj->dev_name, psp);
			pobj->props[pobj->num_properties] = psp;
			pobj->num_properties += 1;
		} else {
			pr_err("Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
			return -EINVAL;
		}
		break;
	case IOCTL_PSU_ADD_USB_TYPE:
		if (filep->private_data) {
			pobj = filep->private_data;
			if (copy_from_user(
				    &usbtype, (void __user *)arg,
				    sizeof(enum power_supply_usb_type)) != 0)
				return -EFAULT;
			if (pobj->num_usb_types >= MAX_PSU_UBS_TYPES) {
				pr_err("%s: Cannot add USB type. Number of USB types is reached\n",
				       pobj->dev_name);
				return -EFAULT;
			}
			pobj->usb_types[pobj->num_usb_types] = usbtype;
			pobj->num_usb_types += 1;
		} else {
			pr_err("Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
			return -EINVAL;
		}
		break;
	case IOCTL_PSU_ADD_SUPPLIED_TO:
		if (filep->private_data) {
			pobj = filep->private_data;
			if (copy_from_user(&supplied_to, (void __user *)arg,
					   sizeof(char[MAX_KEYLENGTH])) != 0)
				return -EFAULT;
			if (pobj->num_supplicants >= MAX_SUPPLICANTS) {
				pr_err("%s: Cannot add supplicant. Number of supplicants is reached\n",
				       pobj->dev_name);
				return -EFAULT;
			}
			pobj->supplied_to[pobj->num_supplicants] = supplied_to;
			pobj->num_supplicants += 1;
		} else {
			pr_err("Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
			return -EINVAL;
		}
		break;
	case IOCTL_PSU_UPDATE_PROPVAL:
		psp_found = false;
		if (filep->private_data) {
			pobj = filep->private_data;
			if (pobj->psy) {
				if (copy_from_user(&propval, (void __user *)arg, sizeof(struct ioctl_propval)) != 0)
					return -EFAULT;
				for (i = 0; i <= pobj->num_properties; i++) {
					if (propval.psp == pobj->props[i]) {
						if (power_supply_is_str_property(propval.psp)) {
							strncpy(pobj->psp_val[propval.psp].strval, propval.strval, MAX_KEYLENGTH);
							pobj->psp_val[propval.psp].strval[MAX_KEYLENGTH - 1] = '\0';
						} else {
							pobj->psp_val[propval.psp].intval = propval.intval;
						}
						power_supply_changed(pobj->psy);
						psp_found = true;
					}
				}
				if (!psp_found)
					pr_err("%s: Unknown property\n",
					       pobj->dev_name);
			} else {
				pr_err("Cannot find PSU device. Missing IOCTL_PSU_REGISTER call?\n");
				return -EINVAL;
			}
		} else {
			pr_err("Cannot find PSU specs. Missing IOCTL_PSU_CREATE call?\n");
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
				pobj->psy = power_supply_register(NULL, &pobj->desc, &pobj->config);
				if (IS_ERR(pobj->psy)) {
					pr_err("Failed to register\n");
					pobj->psy = NULL;
					return PTR_ERR(pobj->psy);
				} else {
					pobj->psy->drv_data = filep;
				}
			} else {
				return -EBUSY;
			}
		}
		break;
	default:
		pr_err("Invalid IOCTL command\n");
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