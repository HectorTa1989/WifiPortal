AUTOMAKE_OPTIONS = foreign nostdinc
bin_PROGRAMS = wiPort
wihand_LDADD = -lpthread

if WITH_SSL
wihand_CPPFLAGS = -DMG_ENABLE_SSL -DCONFDIR='"$(confdir)"'
wihand_LDADD += -lssl -lcrypto
endif

wihand_SOURCES = \
	src/host.c \
	src/iptables.c \
	src/radius.c \
	src/utils.c \
	src/tc.c \
	src/base64.c \
	src/wihand.c \
	src/lma_cache.c \
	mongoose/mongoose.c \
	src/wai.c

confdir = $(sysconfdir)/@PACKAGE@

conf_DATA = example/conf

dist_conf_SCRIPTS = utils/setrules.sh
