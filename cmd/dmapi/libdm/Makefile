#
# Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
# 
# This program is distributed in the hope that it would be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# 
# Further, this software is distributed without any warranty that it is
# free of the rightful claim of any third person regarding infringement
# or the like.  Any license provided herein, whether implied or
# otherwise, applies only to this software file.  Patent licenses, if
# any, provided herein do not apply to combinations of this program with
# other software, or any other product whatsoever.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write the Free Software Foundation, Inc., 59
# Temple Place - Suite 330, Boston MA 02111-1307, USA.
# 
# Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
# Mountain View, CA  94043, or:
# 
# http://www.sgi.com 
# 
# For further information regarding this notice, see: 
# 
# http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
#

TOPDIR = ..
include $(TOPDIR)/include/builddefs

LIBTARGET = libdm.so.1
STATICLIBTARGET = libdm.a

HFILES = dmapi_lib.h
CFILES = dmapi_lib.c dm_attr.c dm_bulkattr.c dm_config.c dm_dmattr.c \
	 dm_event.c dm_handle.c dm_handle2path.c dm_hole.c dm_mountinfo.c \
	 dm_region.c dm_right.c dm_rdwr.c dm_session.c
LCFLAGS = -D_REENTRANT

default: $(LIBTARGET) $(STATICLIBTARGET)

include $(BUILDRULES)

install: default
	$(INSTALL) -m 755 -d $(PKG_LIB_DIR)
	$(INSTALL) -m 755 $(LIBTARGET) $(PKG_LIB_DIR)

install-dev: default
	$(INSTALL) -m 755 -d $(PKG_LIB_DIR)
	$(INSTALL) -m 644 $(STATICLIBTARGET) $(PKG_LIB_DIR)
	$(INSTALL) -S $(PKG_LIB_DIR)/$(LIBTARGET) $(PKG_LIB_DIR)/libdm.so
