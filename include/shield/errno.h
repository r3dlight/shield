// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef ERRNO_H_
#define ERRNO_H_

#ifndef TEST_MODE
/*
 * subtsituing errno is complex. In UT mode, glibc errno symbols must not be
 * overloaded to avoid any corruption. Here we use __shield prefix, and macros
 * and aliasing to handle effective replacement in embedded mode.
 */
#define shield_errno (__shield_errno_location())

/*
 * system level error codes
 *
 * Based on POSIX generic naming, without out of scope (filesystem....)
 * using hardened 32 bits random values with enough hamming distances,
 * no zero comparison
 */
#define	EPERM		 0x2af5e248u	/* Operation not permitted */
#define	ENOENT		 0x39993cc3u	/* No such file or directory */
#define	ESRCH		 0x3f34f248u	/* No such process */
#define	EINTR		 0x41de4352u	/* Interrupted system call */
#define	EIO		     0x455a5555u	/* I/O error */
#define	ENXIO		 0x55a555aau	/* No such device or address */
#define	E2BIG		 0x6a555a5au	/* Argument list too long */
#define	ENOEXEC		 0x73a5753cu	/* Exec format error */
#define	EBADF		 0x753c95a5u	/* Bad file number */
#define	ECHILD		 0x7a59a833u	/* No child processes */
#define	EAGAIN		 0x7aaa5aa5u	/* Try again */
#define	ENOMEM		 0x7f38a4dfu	/* Out of memory */
#define	EACCES		 0xc9a9de4du	/* Permission denied */
#define	EFAULT		 0xc9b3682bu	/* Bad address */
#define	ENOTBLK		 0xca9d8516u	/* Block device required */
#define	EBUSY		 0xcb0b87b8u	/* Device or resource busy */
#define	EEXIST		 0xcc1a0dcfu	/* File exists */
#define	EXDEV		 0xcc1cc8fcu	/* Cross-device link */
#define	ENODEV		 0xcdb7e2d7u	/* No such device */
#define	ENOTDIR		 0xce87fe5bu	/* Not a directory */
#define	EISDIR		 0xcf3029eeu	/* Is a directory */
#define	EINVAL		 0xcfdc42ffu	/* Invalid argument */
#define	ENFILE		 0xd2d4772au	/* File table overflow */
#define	EMFILE		 0xd34ceab1u	/* Too many open files */
#define	ENOTTY		 0xd3d8d228u	/* Not a typewriter */
#define	ETXTBSY		 0xd557703eu	/* Text file busy */
#define	EFBIG		 0xd7ae5135u	/* File too large */
#define	ENOSPC		 0xea81e11eu	/* No space left on device */
#define	ESPIPE		 0xe1458a11u	/* Illegal seek */
#define	EROFS		 0xe855a984u	/* Read-only file system */
#define	EMLINK		 0xf1e5a143u	/* Too many links */
#define	EPIPE		 0xf3751957u	/* Broken pipe */
#define	EDOM		 0xf76aa1d2u	/* Math argument out of domain of func */
#define	ERANGE		 0xf8110a2du	/* Math result not representable */
#define ENOTSUP      0xfbacfec0u    /* operation not supported */

int __shield_errno_location(void);

/* substituing errno only when not in UT*/
#define errno shield_errno

#endif


#endif/*ERR_H_ */
