# New ports collection makefile for:    ser
# Date created:        6 November 2002
# Whom:                andrei
#
# $OpenBSD$
#
# $Id: Makefile,v 1.3.2.1 2003/11/14 15:55:58 andrei Exp $
#

COMMENT= "SIP Express Router"

PORTNAME= ser
PORTVERSION= 0.8.12
CATEGORIES= net
MASTER_SITES= ftp://ftp.berlios.de/pub/ser/0.8.12/src/

MAINTAINER= pelinescu-onciul@fokus.fraunhofer.de

MAN8=ser.8
MAN5=ser.cfg.5
# MANCOMPRESSED must not be defined in openbsd (defined==yes)

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes


USE_GMAKE= yes
# we override the DISTFILE NAME
DISTNAME= ${PORTNAME}-${PORTVERSION}
DISTFILES= ${DISTNAME}_src.tar.gz
WRKSRC= ${WRKDIR}/${PORTNAME}-${PORTVERSION}

# build by default mysql and jabber
MAKE_ENV= include_modules="mysql jabber"

# depends on mysql and libexpat because of the above modules
BUILD_DEPENDS= :expat-1.*:textproc/expat \
               :mysql-client-3.23.*:databases/mysql
LIB_DEPENDS=   expat.2:expat-1.*:textproc/expat \
               lib/mysql/mysqlclient.10:mysql-client-3.23.*:databases/mysql



pre-fetch:


.include <bsd.port.mk>


