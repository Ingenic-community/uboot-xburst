#
# Ingenic SOC Makefile
#
# Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
# Author: Zoro <ykli@ingenic.cn>
# Based on: arch/mips/cpu/xburst/jz4780/Makefile
#           Written by Paul Burton <paul.burton@imgtec.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-y	= soc.o

COBJS-y += clk.o
COBJS-y += pllsetting.o

COBJS-y += ddr_set_dll.o
COBJS-y += ../ddr_innophy.o

COBJS-$(CONFIG_DDR_TEST) += ../ddr_test.o
COBJS-$(CONFIG_DDR_AUTO_REFRESH_TEST) += ../ddr_test_auto_refresh.o
COBJS-$(CONFIG_BURNER) += ../param_manage.o


SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

ifndef CONFIG_FPGA
.depend.pllsetting:$(TOPDIR)/include/generated/pll_reg_values.h
$(obj)pllsetting.o:$(TOPDIR)/include/generated/pll_reg_values.h

$(TOPDIR)/include/generated/pll_reg_values.h:$(TOPDIR)/$(dir)/pll_params_creator.exe
	$< > $@

$(TOPDIR)/$(dir)/pll_params_creator.exe:$(OBJTREE)/$(dir)/pll_params_creator.c | $(SRCTREE)/include/configs/$(BOARD).h
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -I$(SRCTREE)/include/ -o $@ $^

.depend.clk:$(TOPDIR)/include/generated/clk_reg_values.h
$(obj)clk.o:$(TOPDIR)/include/generated/clk_reg_values.h

$(TOPDIR)/include/generated/clk_reg_values.h:$(TOPDIR)/$(dir)/clk_params_creator.exe
	$< > $@

$(TOPDIR)/$(dir)/clk_params_creator.exe:$(OBJTREE)/$(dir)/clk_params_creator.c | $(SRCTREE)/include/configs/$(BOARD).h
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -I$(SRCTREE)/include/ -o $@ $^
endif
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
