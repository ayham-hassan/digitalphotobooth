/*
 * usb-drive.h
 *
 * Author: Nathan Prosser  ntp0166@rit.edu
 *
 */

#ifndef _USBDRIVE_H_
#define _USBDRIVE_H_


/* usbDriveName()
 * Finds the absolute path of a USB flash drive. This function makes several
 *  assumptions. First is that it looks in /media, which is where Ubuntu
 *  automounts flash drives. Second is that it assumes only one external drive
 *  will be inserted at a time. Third, it uses a list of hard-coded devices
 *  which are expected to be in /media, and are not flash drives. It does not
 *  matter if any of these are not present, but if additional ones are present
 *  (eg a CD-ROM), that may be detected instead of the flash drive.
 */
char *getUSBDriveName();

#endif
