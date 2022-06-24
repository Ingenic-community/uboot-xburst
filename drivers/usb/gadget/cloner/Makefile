#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libcloner_module_mg.o

COBJS-$(CONFIG_JZ_VERDOR_BURN_EXTPOL) += extend_policy_manager.o
SUBOBJS-$(CONFIG_JZ_VERDOR_BURN_EXTPOL) += expy/libexpy.o

COBJS-$(if $(CONFIG_MTD_SFCNOR)$(CONFIG_MTD_SFCNAND)$(CONFIG_MTD_SPINAND),y,n) += cloner_module_spisfc.o
COBJS-$(if $(CONFIG_JZ_SPINAND_SN)$(CONFIG_JZ_SPINAND_MAC),y,n) += cloner_spinand_firmware.o
COBJS-$(if $(CONFIG_JZ_NAND_MGR)$(CONFIG_MTD_NAND_JZ),y,n) += cloner_module_nand.o
COBJS-$(CONFIG_JZ_MMC) += cloner_module_mmc.o


COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SUBOBJS := $(addprefix $(obj),$(SUBOBJS-y))
OBJS	+= $(SUBOBJS)

all:	$(LIB)

.PHONY : $(SUBOBJS)
$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(SUBOBJS):
	$(MAKE) -C $(dir $(SUBOBJS))
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
