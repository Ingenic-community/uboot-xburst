include $(TOPDIR)/config.mk

LIB	:= $(obj)libscboot.o

COBJS-$(CONFIG_JZ_SCBOOT) += aes.o test_common.o spi_checksum.o
COBJS-$(CONFIG_JZ_SECURE_SUPPORT) += scboot.o md5.o flash.o
COBJS-$(CONFIG_JZ_CKEYAES) += ckey_aes.o
ifdef CONFIG_BURNER
COBJS-$(CONFIG_JZ_SCBOOT) += secure.o rsa.o otp.o get_rn.o cmd_sctest.o\
				test_scboot.o test_nku.o test_otp.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
