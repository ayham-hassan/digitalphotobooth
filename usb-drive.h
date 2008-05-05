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
void getUSBDriveName( char *fileName );

/* unmountUSBDrive()
 * Unmounts the USB drive. This function uses the getUSBDriveName function to
 *  determine the mountpoint for the USB drive.
 *
 * Returns 0 when the drive has been unmounted. Returns nonzero if there was a
 *  problem.
 */
int unmountUSBDrive();

/* writeFileToUSBDrive()
 * Writes a file to the currently connected USB drive. This function uses the
 *  getUSBDriveName function to determine the mountpoint for the USB drive.
 *  If a folder named "DigitalPhotoBooth" does not yet exist on the root of the
 *  drive, it will be created and the specified file will be placed in it. If
 *  it does exist, a number will be added to the end of the name, starting at
 *  1 and increasing until an unused one is found.
 *  
 * Parameters:
 *  fileName: the name of the file (current directory or path) to write
 *
 * Returns 0 if successful, nonzero if an error occurred.
 */
int writeFileToUSBDrive(char *fileName);

#endif
