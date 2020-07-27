# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			test_yaml.c \

###### 目标文件、安装目录配置区
include ../makeinstall
BIN		=	test_yaml \

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(_HDERINST) \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(_LIBINST) \
			-lmyyaml \

###### 额外宏定义区
CLEAN_ADDITION	=

###### 加载mktpl模板库
#@ make_all
#@ make_clean

include $(MKTPL2_HOME)/makeobj_$(MKTPL2_OS).inc

###### 目标文件依赖关系
test_yaml			:	test_yaml.o
	$(CC) -o $@ test_yaml.o $(LFLAGS)

