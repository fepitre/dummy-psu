#ifndef _UAPI__DUMMY_PSU_H
#define _UAPI__DUMMY_PSU_H

#define MAX_KEYLENGTH 256

#define IOCTL_MAGIC_NUMBER 'p'
#define IOCTL_PSU_REGISTER _IO(IOCTL_MAGIC_NUMBER, 0x1)
#define IOCTL_PSU_CREATE _IOW(IOCTL_MAGIC_NUMBER, 0x2, struct ioctl_psu)
#define IOCTL_PSU_ADD_PSP _IOW(IOCTL_MAGIC_NUMBER, 0x3, int)
#define IOCTL_PSU_ADD_USB_TYPE _IOW(IOCTL_MAGIC_NUMBER, 0x4, int)
#define IOCTL_PSU_ADD_SUPPLIED_TO _IOW(IOCTL_MAGIC_NUMBER, 0x5, char[MAX_KEYLENGTH])
#define IOCTL_PSU_UPDATE_PROPVAL _IOW(IOCTL_MAGIC_NUMBER, 0x6, struct ioctl_propval)

struct ioctl_psu {
	int dev_type;
	char dev_name[MAX_KEYLENGTH];
};

struct ioctl_propval {
	int psp;
	int intval;
	char strval[MAX_KEYLENGTH];
};

#endif /* _UAPI__DUMMY_PSU_H */