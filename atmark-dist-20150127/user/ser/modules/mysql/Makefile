# $Id: Makefile,v 1.22 2003/09/16 18:55:07 janakj Exp $
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=mysql.so


# mysql.h locations (freebsd,openbsd  solaris)
DEFS +=-I$(LOCALBASE)/include -I$(LOCALBASE)/include/mysql \
		-I$(LOCALBASE)/mysql/include -I/usr/pkg/include
# libmysqlclient locations on RH/Suse, Solaris /OpenBSD, FreeBSD
# (Debian does the right thing and puts it in /usr/lib)
LIBS=-L/usr/lib/mysql -L$(LOCALBASE)/lib -L$(LOCALBASE)/lib/mysql \
		-L$(LOCALBASE)/mysql/lib/mysql/ -L/usr/pkg/lib/mysql \
		-L$(LOCALBASE)/mysql/lib \
		-lz $(ROOTDIR)/user/mysql/libmysql/.libs/libmysqlclient.a

include ../../Makefile.modules
