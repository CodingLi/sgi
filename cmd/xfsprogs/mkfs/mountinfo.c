/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 * 
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 * 
 * http://www.sgi.com 
 * 
 * For further information regarding this notice, see: 
 * 
 * http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
 */

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "mountinfo.h"

#define	inline	__inline

int
mnt_check_init(mnt_check_state_t **check_state) {
	return(0);
}

int
mnt_find_mount_conflicts(mnt_check_state_t *check_state, char *name) {
	FILE *f;
	struct mntent *mnt;

#if XFS_PORT
	if ((f = setmntent (MOUNTED, "r")) == NULL) {
		return 0;
	}

	while ((mnt = getmntent (f)) != NULL) {
		if (strcmp (name, mnt->mnt_fsname) == 0) {
			break;
		}
	}
	endmntent (f);
	if (!mnt) {
		return(0);
	} else {
		return(1);
	}
#endif
	return 0; /* keep compilier happy */
}

int
mnt_causes_test(mnt_check_state_t *check_state, int cause)
{
	switch(cause) {
	case MNT_CAUSE_MOUNTED:
		return(1);

	default:
		fprintf(stderr, "mnt_causes_test: unknown cause %d\n", cause);
		exit(99);
	}
}

void
mnt_causes_show(mnt_check_state_t *check_state, FILE *fp, char *prefix)
{
	fprintf(fp, "mnt_causes_show: not implemented. Called with %s\n",
		prefix);
	exit(98);
}

void
mnt_plist_show(mnt_check_state_t *check_state, FILE *fp, char *prefix)
{
	/*
	 * Need to do some work for volume mgmt.
	 */
}

int
mnt_check_end(mnt_check_state_t *check_state)
{

	return(0);
}

char *
mnt_known_fs_type(const char *device)
{
	static char *fstype (const char *);

#if XFS_PORT
	return fstype(device);
#endif
	return (NULL);
}


/*
 * From mount(8) source by Andries Brouwer.  Hacked for XFS by mkp.
 * Recent sync's to mount source:
 *	- util-linux-2.10o ... 06 Sep 00
 *	- util-linux-2.10r ... 06 Dec 00
 */

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

static inline unsigned short
swapped(unsigned short a) {
     return (a>>8) | (a<<8);
}

/* udf magic - I find that trying to mount garbage as an udf fs
   causes a very large kernel delay, almost killing the machine.
   So, we do not try udf unless there is positive evidence that it
   might work. Try iso9660 first, it is much more likely.
   Strings below taken from ECMA 167. */
static char
*udf_magic[] = { "BEA01", "BOOT2", "CD001", "CDW02", "NSR02",
		 "NSR03", "TEA01" };

static int
may_be_udf(const char *id) {
    char **m;

    for (m = udf_magic; m - udf_magic < SIZE(udf_magic); m++)
       if (!strncmp(*m, id, 5))
	  return 1;
    return 0;
}

static int
may_be_swap(const char *s) {
	return (strncmp(s-10, "SWAP-SPACE", 10) == 0 ||
		strncmp(s-10, "SWAPSPACE2", 10) == 0);
}

/* rather weak necessary condition */
static int
may_be_adfs(const u_char *s) {
	u_char *p;
	int sum;

	p = (u_char *) s + 511;
	sum = 0;
	while(--p != s)
		sum = (sum >> 8) + (sum & 0xff) + *p;

	return (sum == p[511]);
}

static char *
fstype(const char *device) {
    int fd;
    char *type = NULL;
    union {
	struct minix_super_block ms;
	struct ext_super_block es;
	struct ext2_super_block e2s;
    } sb;
    union {
	struct xiafs_super_block xiasb;
	char romfs_magic[8];
	char qnx4fs_magic[10];	/* ignore first 4 bytes */
	long bfs_magic;
	struct ntfs_super_block ntfssb;
	struct fat_super_block fatsb;
	struct xfs_super_block xfsb;
	struct cramfs_super_block cramfssb;
    } xsb;
    struct ufs_super_block ufssb;
    union {
	struct iso_volume_descriptor iso;
	struct hs_volume_descriptor hs;
    } isosb;
    struct hfs_super_block hfssb;
    struct hpfs_super_block hpfssb;
    struct adfs_super_block adfssb;
    struct stat statbuf;

    /* opening and reading an arbitrary unknown path can have
       undesired side effects - first check that `device' refers
       to a block device */
    if (stat (device, &statbuf) || !S_ISBLK(statbuf.st_mode))
      return 0;

    fd = open(device, O_RDONLY);
    if (fd < 0)
      return 0;

    if (lseek(fd, 1024, SEEK_SET) != 1024
	|| read(fd, (char *) &sb, sizeof(sb)) != sizeof(sb))
	 goto io_error;

    if (ext2magic(sb.e2s) == EXT2_SUPER_MAGIC
	|| ext2magic(sb.e2s) == EXT2_PRE_02B_MAGIC
	|| ext2magic(sb.e2s) == swapped(EXT2_SUPER_MAGIC))
	 type = "ext2";

    else if (minixmagic(sb.ms) == MINIX_SUPER_MAGIC
	     || minixmagic(sb.ms) == MINIX_SUPER_MAGIC2
	     || minixmagic(sb.ms) == swapped(MINIX_SUPER_MAGIC2))
	 type = "minix";

    else if (extmagic(sb.es) == EXT_SUPER_MAGIC)
	 type = "ext";

    if (!type) {
	 if (lseek(fd, 0, SEEK_SET) != 0
	     || read(fd, (char *) &xsb, sizeof(xsb)) != sizeof(xsb))
	      goto io_error;

	 if (xiafsmagic(xsb.xiasb) == _XIAFS_SUPER_MAGIC)
	      type = "xiafs";
	 else if(!strncmp(xsb.romfs_magic, "-rom1fs-", 8))
	      type = "romfs";
	 else if(!strncmp(xsb.xfsb.s_magic, XFS_SUPER_MAGIC, 4) ||
		 !strncmp(xsb.xfsb.s_magic, XFS_SUPER_MAGIC2, 4))
	      type = "xfs";
	 else if(!strncmp(xsb.qnx4fs_magic+4, "QNX4FS", 6))
	      type = "qnx4fs";
	 else if(xsb.bfs_magic == 0x1badface)
	      type = "bfs";
	 else if(!strncmp(xsb.ntfssb.s_magic, NTFS_SUPER_MAGIC,
			  sizeof(xsb.ntfssb.s_magic)))
	      type = "ntfs";
	 else if(cramfsmagic(xsb.cramfssb) == CRAMFS_SUPER_MAGIC)
	      type = "cramfs";
	 else if ((!strncmp(xsb.fatsb.s_os, "MSDOS", 5) ||
		   !strncmp(xsb.fatsb.s_os, "MSWIN", 5) ||
		   !strncmp(xsb.fatsb.s_os, "MTOOL", 5) ||
		   !strncmp(xsb.fatsb.s_os, "mkdosfs", 7) ||
		   !strncmp(xsb.fatsb.s_os, "kmkdosfs", 8))
		  && (!strncmp(xsb.fatsb.s_fs, "FAT12   ", 8) ||
		      !strncmp(xsb.fatsb.s_fs, "FAT16   ", 8) ||
		      !strncmp(xsb.fatsb.s_fs2, "FAT32   ", 8)))
	      type = "vfat";	/* only guessing - might as well be fat or umsdos */
    }

    if (!type) {
	 if (lseek(fd, 8192, SEEK_SET) != 8192
	     || read(fd, (char *) &ufssb, sizeof(ufssb)) != sizeof(ufssb))
	      goto io_error;

	 if (ufsmagic(ufssb) == UFS_SUPER_MAGIC) /* also test swapped version? */
	      type = "ufs";
    }

    if (!type) {
	 if (lseek(fd, 0x8000, SEEK_SET) != 0x8000
	     || read(fd, (char *) &isosb, sizeof(isosb)) != sizeof(isosb))
	      goto io_error;

	 if(strncmp(isosb.iso.id, ISO_STANDARD_ID, sizeof(isosb.iso.id)) == 0
	    || strncmp(isosb.hs.id, HS_STANDARD_ID, sizeof(isosb.hs.id)) == 0)
	      type = "iso9660";
	 else if (may_be_udf(isosb.iso.id))
	      type = "udf";
    }

    if (!type) {
        if (lseek(fd, 0x400, SEEK_SET) != 0x400
            || read(fd, (char *) &hfssb, sizeof(hfssb)) != sizeof(hfssb))
             goto io_error;

        /* also check if block size is equal to 512 bytes,
           since the hfs driver currently only has support
           for block sizes of 512 bytes long, and to be
           more accurate (sb magic is only a short int) */
        if ((hfsmagic(hfssb) == HFS_SUPER_MAGIC &&
	     hfsblksize(hfssb) == 0x20000) ||
            (swapped(hfsmagic(hfssb)) == HFS_SUPER_MAGIC &&
             hfsblksize(hfssb) == 0x200))
             type = "hfs";
    }

    if (!type) {
        if (lseek(fd, 0x2000, SEEK_SET) != 0x2000
            || read(fd, (char *) &hpfssb, sizeof(hpfssb)) != sizeof(hpfssb))
             goto io_error;

        if (hpfsmagic(hpfssb) == HPFS_SUPER_MAGIC)
             type = "hpfs";
    }

    if (!type) {
        if (lseek(fd, 0xc00, SEEK_SET) != 0xc00
            || read(fd, (char *) &adfssb, sizeof(adfssb)) != sizeof(adfssb))
             goto io_error;

	/* only a weak test */
        if (may_be_adfs((u_char *) &adfssb)
            && (adfsblksize(adfssb) >= 8 &&
                adfsblksize(adfssb) <= 10))
             type = "adfs";
    }

    if (!type) {
	    /* perhaps the user tries to mount the swap space
	       on a new disk; warn her before she does mkfs on it */
	    int pagesize = getpagesize();
	    int rd;
	    char buf[32768];

	    rd = pagesize;
	    if (rd < 8192)
		    rd = 8192;
	    if (rd > sizeof(buf))
		    rd = sizeof(buf);
	    if (lseek(fd, 0, SEEK_SET) != 0
		|| read(fd, buf, rd) != rd)
		    goto io_error;
	    if (may_be_swap(buf+pagesize) ||
		may_be_swap(buf+4096) || may_be_swap(buf+8192))
		    type = "swap";
    }

    close (fd);
    return(type);

io_error:
    perror(device);
    close(fd);
    return 0;
}
