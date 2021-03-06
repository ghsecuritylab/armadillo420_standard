# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# $Id: configure.ac 1170 2006-03-02 01:32:37Z lennart $

# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

AC_PREREQ(2.57)
AC_INIT([avahi],[0.6.9],[avahi (at) lists (dot) freedesktop (dot) org])
AC_CONFIG_SRCDIR([avahi-core/server.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign 1.9 -Wall])

AC_SUBST(PACKAGE_URL, [http://avahi.org/])

AC_SUBST(LIBAVAHI_COMMON_VERSION_INFO, [6:0:3])
AC_SUBST(LIBAVAHI_CORE_VERSION_INFO, [4:1:0])
AC_SUBST(LIBAVAHI_CLIENT_VERSION_INFO, [4:1:1])
AC_SUBST(LIBAVAHI_GLIB_VERSION_INFO, [1:0:0])
AC_SUBST(LIBAVAHI_QT3_VERSION_INFO, [1:1:0])
AC_SUBST(LIBAVAHI_QT4_VERSION_INFO, [1:1:0])
AC_SUBST(LIBAVAHI_COMPAT_LIBDNS_SD_VERSION_INFO, [1:0:0])
AC_SUBST(LIBAVAHI_COMPAT_HOWL_VERSION_INFO, [0:0:0])
AC_SUBST(HOWL_COMPAT_VERSION, [0.9.8])

if type -p stow > /dev/null && test -d /usr/local/stow ; then
    AC_MSG_NOTICE([*** Found /usr/local/stow: default install prefix set to /usr/local/stow/${PACKAGE_NAME}-${PACKAGE_VERSION} ***])
    ac_default_prefix="/usr/local/stow/${PACKAGE_NAME}-${PACKAGE_VERSION}"
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_GNU_SOURCE
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_GCC_TRADITIONAL

# libtool stuff
AC_PROG_LIBTOOL

ACX_PTHREAD(,AC_MSG_ERROR([Missing POSIX Threads support]))

#
# Check for netlink.h
#
AC_CHECK_HEADER(linux/netlink.h, 
HAVE_NETLINK=yes
AC_DEFINE([HAVE_NETLINK],[],[Support for Linux netlink])
, [], [
#include <sys/socket.h>
#include <asm/types.h>
])

AM_CONDITIONAL(HAVE_NETLINK, [ test x"$HAVE_NETLINK" = xyes ])

#
# Check for net/route.h
#
AC_CHECK_HEADER(net/route.h, 
HAVE_PF_ROUTE=yes
AC_DEFINE([HAVE_PF_ROUTE],[],[Support for PF_ROUTE])
, [], [
#include <sys/types.h>
#include <sys/socket.h>
])

AM_CONDITIONAL(HAVE_PF_ROUTE, [ test x"$HAVE_PF_ROUTE" = xyes ])

#
# Check for struct ip_mreqn
#
AC_MSG_CHECKING(for struct ip_mreqn)
AC_TRY_COMPILE([#include <netinet/in.h>], [
	struct ip_mreqn mreq;
	mreq.imr_address.s_addr = 0;
], [
	# Yes, we have it...
	AC_MSG_RESULT(yes)
	AC_DEFINE([HAVE_STRUCT_IP_MREQN],[],[Support for struct ip_mreqn])
], [
	# We'll just have to try and use struct ip_mreq
	AC_MSG_RESULT(no)
	AC_MSG_CHECKING(for struct ip_mreq)
	AC_TRY_COMPILE([#include <netinet/in.h>], [
		struct ip_mreq mreq;
		mreq.imr_interface.s_addr = 0;
	], [
		# Yes, we have it...
		AC_MSG_RESULT(yes)
		AC_DEFINE([HAVE_STRUCT_IP_MREQ],[],[Support for struct ip_mreq])
	], [
		# No multicast support
			AC_MSG_RESULT(no)
	])
])

#
# Detecting the linux distribution for specific things like init scripts.
#
AC_ARG_WITH(distro, AS_HELP_STRING([--with-distro=DISTRO],[Specify the distribution to target: One of debian, gentoo, archlinux, fedora, mandriva, darwin, netbsd, freebsd, slackware or none]))
if test "z$with_distro" = "z"; then
    AC_CHECK_FILE(/etc/SuSE-release,with_distro="suse")
    AC_CHECK_FILE(/etc/gentoo-release,with_distro="gentoo")
    AC_CHECK_FILE(/etc/arch-release,with_distro="archlinux")
    AC_CHECK_FILE(/etc/debian_version,with_distro="debian")
    AC_CHECK_FILE(/etc/redhat-release,with_distro="fedora")
    AC_CHECK_FILE(/etc/mandriva-release,with_distro="mandriva")
    AC_CHECK_FILE(/etc/slackware-version,with_distro="slackware")
    if test "z$with_distro" = "z"; then
        with_distro=`uname -s`
    fi
fi
with_distro=`echo ${with_distro} | tr '[[:upper:]]' '[[:lower:]]' `

case $with_distro in
    debian|gentoo|archlinux|suse|fedora|mandriva|darwin|freebsd|slackware|none)
     ;;
    netbsd)
     AC_MSG_WARN([Your distribution (${with_distro}) is supported but no init script exist yet! (patches welcome)])
     ;;
    linux)
     AC_MSG_ERROR([Linux distribution autodetection failed, you must specify the distribution to target using --with-distro=DISTRO, set DISTRO to none if your distribution is not supported.])
     ;;
    *)
     AC_MSG_ERROR([Your distribution (${with_distro}) is not yet supported, init scripts and dbus configuration will not be installed! (patches welcome), you can specify --with-distro=none to skip this check])
     ;;
esac

AM_CONDITIONAL(TARGET_SUSE, test x"$with_distro" = xsuse)
AM_CONDITIONAL(TARGET_GENTOO, test x"$with_distro" = xgentoo)
AM_CONDITIONAL(TARGET_DEBIAN, test x"$with_distro" = xdebian)
AM_CONDITIONAL(TARGET_ARCHLINUX, test x"$with_distro" = xarchlinux)
AM_CONDITIONAL(TARGET_FEDORA, test x"$with_distro" = xfedora)
AM_CONDITIONAL(TARGET_MANDRIVA, test x"$with_distro" = xmandriva)
AM_CONDITIONAL(TARGET_DARWIN, test x"$with_distro" = xdarwin)
AM_CONDITIONAL(TARGET_NETBSD, test x"$with_distro" = xnetbsd)
AM_CONDITIONAL(TARGET_FREEBSD, test x"$with_distro" = xfreebsd)
AM_CONDITIONAL(TARGET_SLACKWARE, test x"$with_distro" = xslackware)

test_gcc_flag() {
    AC_LANG_CONFTEST([int main() {}])
    $CC -c conftest.c $CFLAGS $@ > /dev/null 2> /dev/null
    ret=$?
    rm -f conftest.o
    return $ret
}

# If using GCC specify some additional parameters
if test "x$GCC" = "xyes" ; then

    DESIRED_FLAGS="-Wall -W -Wextra -pedantic -pipe -Wformat -Wold-style-definition -Wdeclaration-after-statement -Wfloat-equal -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wmissing-noreturn -Wshadow -Wendif-labels -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Winline"

    if test "x$HAVE_NETLINK" = "xyes" ; then
        # Test whether rtnetlink.h can be included when compiled with -std=c99
        # some distributions (e.g. archlinux) have broken headers that dont
        # define __u64 with -std=c99
        AC_MSG_CHECKING([checking whether rtnetlink.h can be included with -std=c99])
        OLDCFLAGS="$CFLAGS"
        CFLAGS="-std=c99"
        AC_TRY_COMPILE([#include <linux/rtnetlink.h>], [],
            use_stdc99=yes, use_stdc99=no)

        if test x"$use_stdc99" = xyes; then
            DESIRED_FLAGS="-std=c99 $DESIRED_FLAGS"
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
        fi

        CFLAGS="$OLDCFLAGS"
    else
        DESIRED_FLAGS="-std=c99 $DESIRED_FLAGS"
    fi

    for flag in $DESIRED_FLAGS ; do
        AC_MSG_CHECKING([whether $CC accepts $flag])
        if test_gcc_flag $flag ; then 
           CFLAGS="$CFLAGS $flag"
           AC_MSG_RESULT([yes])
        else
           AC_MSG_RESULT([no])
        fi
    done 
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h netdb.h syslog.h])
AC_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_HEADER_SYS_WAIT

# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostname memchr memmove memset mkdir select socket strchr strcspn strdup strerror strrchr strspn strstr uname setresuid setreuid setresgid setregid strcasecmp gettimeofday putenv strncasecmp strlcpy gethostbyname seteuid setegid setproctitle getprogname])

AC_FUNC_CHOWN
AC_FUNC_STAT
AC_TYPE_MODE_T
AC_TYPE_PID_T

AC_CHECK_DECLS(environ)

enable_chroot=yes
AC_CHECK_HEADERS([sys/capability.h],,enable_chroot=no)
AC_CHECK_HEADERS([sys/prctl.h],,enable_chroot=no)
AC_CHECK_FUNCS([chroot],,enable_chroot=no)

AM_CONDITIONAL(ENABLE_CHROOT, test "x$enable_chroot" = "xyes")

if test "x$enable_chroot" = "xyes" ; then
   AC_DEFINE([ENABLE_CHROOT], 1, [Enable chroot() usage])
fi

AC_CHECK_LIB(dl, dlopen, [ AC_CHECK_HEADERS(dlfcn.h, HAVE_DLOPEN=yes, HAVE_DLOPEN=no) ], HAVE_DLOPEN=no)
if test "x$HAVE_DLOPEN" = "xyes" ; then
    AC_DEFINE([HAVE_DLOPEN],1,[Have dlopen()])
fi
AM_CONDITIONAL(HAVE_DLOPEN, test "x$HAVE_DLOPEN" = "xyes")

# Check for pkg-config manually first, as if its not installed the
# PKG_PROG_PKG_CONFIG macro won't be defined.
AC_CHECK_PROG(have_pkg_config, pkg-config, yes, no)

if test x"$have_pkg_config" = "xno"; then
    AC_MSG_ERROR(pkg-config is required to install this program)
fi

PKG_PROG_PKG_CONFIG

#
# Check for GLIB 2.0
#
AC_ARG_ENABLE(glib,
        AS_HELP_STRING([--disable-glib],[Disable use of GLib]),
        [case "${enableval}" in
                yes) HAVE_GLIB=yes ;;
                no)  HAVE_GLIB=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-glib) ;;
        esac],
        [HAVE_GLIB=yes])

if test "x$HAVE_GLIB" = "xyes" ; then
        PKG_CHECK_MODULES(GLIB20, [ glib-2.0 >= 2.4.0 ])
        AC_SUBST(GLIB20_CFLAGS)
        AC_SUBST(GLIB20_LIBS)
fi
AM_CONDITIONAL(HAVE_GLIB, test "x$HAVE_GLIB" = "xyes")

#
# Check for Qt 3
#
AC_ARG_ENABLE(qt3,
        AS_HELP_STRING([--disable-qt3],[Disable building of Qt3 mainloop integration]),
        [case "${enableval}" in
                yes) HAVE_QT3=yes ;;
                no)  HAVE_QT3=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-qt3) ;;
        esac],
        [HAVE_QT3=yes])

if test "x$HAVE_QT3" = "xyes" ; then
        PKG_CHECK_MODULES( QT3, [ qt-mt >= 3.0.0 ])
        AC_SUBST(QT3_CFLAGS)
        AC_SUBST(QT3_LIBS)
	    QT3_PREFIX="`$PKG_CONFIG --variable=prefix qt-mt`/bin"
        MOC_QT3="no"
	    AC_CHECK_FILE( "$QT3_PREFIX/moc-qt3", [ MOC_QT3=$QT3_PREFIX/moc-qt3 ], [
        	    AC_CHECK_FILE("$QT3_PREFIX/moc", [ MOC_QT3=$QT3_PREFIX/moc ], [ 
                AC_MSG_ERROR([Couldn't find QT3 moc])])])
        AC_SUBST(MOC_QT3)
fi
AM_CONDITIONAL(HAVE_QT3, test "x$HAVE_QT3" = "xyes")

#
# Check for Qt 4
#
AC_ARG_ENABLE(qt4,
        AS_HELP_STRING([--disable-qt4],[Disable building of Qt4Core mainloop integration]),
        [case "${enableval}" in
                yes) HAVE_QT4=yes ;;
                no)  HAVE_QT4=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-qt4) ;;
        esac],
        [HAVE_QT4=yes])

if test "x$HAVE_QT4" = "xyes" ; then
        PKG_CHECK_MODULES( QT4, [ QtCore >= 4.0.0 ])
        AC_SUBST(QT4_CFLAGS)
        AC_SUBST(QT4_LIBS)
	    QT4_PREFIX="`$PKG_CONFIG --variable=prefix QtCore`/bin"
        MOC_QT4="no"
	    AC_CHECK_FILE( "$QT4_PREFIX/moc-qt4", [ MOC_QT4=$QT4_PREFIX/moc-qt4 ], [
               AC_CHECK_FILE("$QT4_PREFIX/moc", [ MOC_QT4=$QT4_PREFIX/moc ], [ 
               AC_MSG_ERROR([Couldn't find QT4 moc])])])
        AC_SUBST(MOC_QT4)
fi
AM_CONDITIONAL(HAVE_QT4, test "x$HAVE_QT4" = "xyes")

#
# Check for GTK+
#
AC_ARG_ENABLE(gtk,
        AS_HELP_STRING([--disable-gtk],[Disable use of GTK+]),
        [case "${enableval}" in
                yes) HAVE_GTK=yes ;;
                no)  HAVE_GTK=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-gtk) ;;
        esac],
        [HAVE_GTK=yes])

if test "x$HAVE_GTK" = "xyes" ; then
        # Check for GTK 2.0
        PKG_CHECK_MODULES(GTK20, [ gtk+-2.0 >= 2.4.0 ])
        AC_SUBST(GTK20_CFLAGS)
        AC_SUBST(GTK20_LIBS)
        
        # Check for GLADE 2.0
        PKG_CHECK_MODULES(GLADE20, [ libglade-2.0 >= 2.4.0 ])
        AC_SUBST(GLADE20_CFLAGS)
        AC_SUBST(GLADE20_LIBS)
fi
AM_CONDITIONAL(HAVE_GTK, test "x$HAVE_GTK" = "xyes")

#
# D-BUS
#
AC_ARG_ENABLE(dbus,
        AS_HELP_STRING([--disable-dbus],[Disable use of D-BUS]),
        [case "${enableval}" in
                yes) HAVE_DBUS=yes ;;
                no)  HAVE_DBUS=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-dbus) ;;
        esac],
        [HAVE_DBUS=yes]) 

AC_ARG_WITH(dbus-sys, AS_HELP_STRING([--with-dbus-sys=<dir>], [where D-BUS system.d directory is]))
AC_ARG_WITH(dbus-system-socket, AS_HELP_STRING([--with-dbus-system-address=<address>], [where the dbus system socket is, you probably want to put unix:path= at the start]))

DBUS_VERSION="Disabled"
DBUS_SYS_DIR="Disabled"
DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="Disabled"
if test "x$HAVE_DBUS" = "xyes" ; then
    PKG_CHECK_MODULES(DBUS, [ dbus-1 >= 0.34 ])

    AC_DEFINE(HAVE_DBUS, 1, [Whether we have D-BUS or not])

    DBUS_VERSION=`$PKG_CONFIG dbus-1 --modversion`
    DBUS_VERSION_MAJOR=`echo $DBUS_VERSION | awk -F. '{print $1}'`
    DBUS_VERSION_MINOR=`echo $DBUS_VERSION | awk -F. '{print $2}'`
    DBUS_VERSION_MICRO=`echo $DBUS_VERSION | awk -F. '{print $3}'`
    if test "z$DBUS_VERSION_MAJOR" = "z"; then
        DBUS_VERSION_MAJOR="0"
    fi
    if test "z$DBUS_VERSION_MINOR" = "z"; then
        DBUS_VERSION_MINOR="0"
    fi
    if test "z$DBUS_VERSION_MICRO" = "z"; then
        DBUS_VERSION_MICRO="0"
    fi

    if test "z$DBUS_VERSION_MAJOR" = "z0" -a "z$DBUS_VERSION_MINOR" = "z0" -a "z$DBUS_VERSION_MICRO" = "z0"; then
        echo "Error: Couldn't determine the version of your DBUS package."
        echo "  This is probably an error in this script, please report it"
        echo "  along with the following information:"
        echo "      Base DBUS version ='$DBUS_VERSION'"
        echo "      DBUS_VERSION_MAJOR='$DBUS_VERSION_MAJOR'"
        echo "      DBUS_VERSION_MINOR='$DBUS_VERSION_MINOR'"
        echo "      DBUS_VERSION_MICRO='$DBUS_VERSION_MICRO'"
        exit 1
    else
        echo "Your dbus version is $DBUS_VERSION_MAJOR,$DBUS_VERSION_MINOR,$DBUS_VERSION_MICRO."
        DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_VERSION_MAJOR=$DBUS_VERSION_MAJOR"
        DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_VERSION_MINOR=$DBUS_VERSION_MINOR"
        DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_VERSION_MICRO=$DBUS_VERSION_MICRO"
    fi

    DBUS_CFLAGS="$DBUS_CFLAGS -DDBUS_API_SUBJECT_TO_CHANGE"
    AC_SUBST(DBUS_CFLAGS)
    AC_SUBST(DBUS_LIBS)

    if ! test -z "$with_dbus_sys" ; then
        DBUS_SYS_DIR="$with_dbus_sys"
    else
        DBUS_SYS_DIR="${sysconfdir}/dbus-1/system.d"
    fi
    AC_SUBST(DBUS_SYS_DIR)

    if ! test -z "$with_dbus_system_address" ; then
        DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="$with_dbus_system_address"
    else
        # This is ugly, but D-BUS doesn't export this address for us
        # so we have to guess, pretty much all setups i've seen have 
        # it in /var/lib/dbus or /var/run/dbus, and its defaulted to
        # /var/run upstream so we will try guess first then default
        # to /var/run/dbus

        DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="unix:path=/var/run/dbus/system_bus_socket"
        TRY_SOCKETS="/var/lib/dbus/system_bus_socket /var/run/dbus/system_bus_socket ${localstatedir}/run/dbus/system_bus_socket ${prefix}/var/run/dbus/system_bus_socket"
        for sock in $TRY_SOCKETS; do
            if test -S $sock; then
                DBUS_SYSTEM_BUS_DEFAULT_ADDRESS="unix:path=$sock"
            fi
        done
    fi
    AC_SUBST(DBUS_SYSTEM_BUS_DEFAULT_ADDRESS)
    
fi
AM_CONDITIONAL(HAVE_DBUS, test "x$HAVE_DBUS" = "xyes")

#
# Expat
#
AC_ARG_ENABLE(expat,
        AS_HELP_STRING([--disable-expat],[Disable use of Expat]),
        [case "${enableval}" in
                yes) HAVE_EXPAT=yes ;;
                no)  HAVE_EXPAT=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-expat) ;;
        esac],
        [HAVE_EXPAT=yes]) 

if test "x$HAVE_EXPAT" = "xyes" ; then
    AC_CHECK_LIB(expat, XML_ParserCreate, [ AC_CHECK_HEADERS(expat.h, have_expat=true, have_expat=false) ], have_expat=false)

    if ! $have_expat ; then
        AC_MSG_ERROR([*** libexpat not found ***])
    fi
fi
AM_CONDITIONAL(HAVE_EXPAT, test "x$HAVE_EXPAT" = "xyes")

#
# GDBM
#
AC_ARG_ENABLE(gdbm,
        AS_HELP_STRING([--disable-gdbm],[Disable use of GDBM]),
        [case "${enableval}" in
                yes) HAVE_GDBM=yes ;;
                no)  HAVE_GDBM=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-gdbm) ;;
        esac],
        [HAVE_GDBM=yes]) 

if test "x$HAVE_GDBM" = "xyes" ; then
    AC_CHECK_LIB(gdbm, gdbm_open, [ AC_CHECK_HEADERS(gdbm.h, have_gdbm=true, have_gdbm=false) ], have_gdbm=false)

    if ! $have_gdbm ; then
        AC_MSG_ERROR([*** libgdbm not found ***])
    fi
    AC_DEFINE([HAVE_GDBM],[],[Support for GDBM])
fi
AM_CONDITIONAL(HAVE_GDBM, test "x$HAVE_GDBM" = "xyes")

#
# libdaemon
#
AC_ARG_ENABLE(libdaemon,
        AS_HELP_STRING([--disable-libdaemon],[Disable use of libdaemon]),
        [case "${enableval}" in
                yes) HAVE_LIBDAEMON=yes ;;
                no)  HAVE_LIBDAEMON=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-libdaemon) ;;
        esac],
        [HAVE_LIBDAEMON=yes]) 

if test "x$HAVE_LIBDAEMON" = "xyes" ; then
    PKG_CHECK_MODULES(LIBDAEMON, [ libdaemon >= 0.5 ])
    AC_SUBST(LIBDAEMON_CFLAGS)
    AC_SUBST(LIBDAEMON_LIBS)
fi
AM_CONDITIONAL(HAVE_LIBDAEMON, test "x$HAVE_LIBDAEMON" = "xyes")

#
# Python stuff
#
AC_ARG_ENABLE(python,
    AS_HELP_STRING([--disable-python], [Disable scripts that depends on python]),
    [case "${enableval}" in
          yes) HAVE_PYTHON=yes ;;
          no)  HAVE_PYTHON=no ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-python]) ;;
    esac],[HAVE_PYTHON=yes])

HAVE_PYTHON_DBUS=no
HAVE_PYGTK=no

if test "x$HAVE_PYTHON" = "xyes" ; then
    AM_PATH_PYTHON([2.4])

    AC_ARG_ENABLE(pygtk,
        AS_HELP_STRING([--disable-pygtk],[Disable use of GTK in Python]),
        [case "${enableval}" in
                yes) HAVE_PYGTK=yes ;;
                no)  HAVE_PYGTK=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-pygtk) ;;
        esac],
        [HAVE_PYGTK=yes])
    
    if test "x$HAVE_PYGTK" = "xyes" ; then
        AM_CHECK_PYMOD(gtk,,,[AC_MSG_ERROR(Could not find Python module gtk)])
    fi


    if test "x$HAVE_DBUS" = "xyes" ; then
        AC_ARG_ENABLE(python-dbus,
	    AS_HELP_STRING([--disable-python-dbus],[Disable use of D-BUS in Python]),
            [case "${enableval}" in
                   yes) HAVE_PYTHON_DBUS=yes ;;
                   no)  HAVE_PYTHON_DBUS=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-python-dbus) ;;
            esac],
            [HAVE_PYTHON_DBUS=yes])
    
        if test "x$HAVE_PYTHON_DBUS" = "xyes"; then
            AM_CHECK_PYMOD(dbus,,,[AC_MSG_ERROR(Could not find Python module dbus)])
        fi

        AM_CHECK_PYMOD(socket,,,[AC_MSG_ERROR(Could not find Python module socket)])
        AM_CHECK_PYMOD(gdbm,,,[AC_MSG_ERROR(Could not find Python module gdbm)])
    fi
fi
AM_CONDITIONAL(HAVE_PYTHON, [test "x$HAVE_PYTHON" = "xyes" ])
AM_CONDITIONAL(HAVE_PYGTK, test "x$HAVE_PYGTK" = "xyes")
AM_CONDITIONAL(HAVE_PYTHON_DBUS, test "x$HAVE_PYTHON_DBUS" = "xyes")

#
# Check for mono stuff
#
HAVE_MONO=no
if test "x$HAVE_DBUS" = "xyes" ; then
    AC_ARG_ENABLE(mono,
            AS_HELP_STRING([--disable-mono],[Disable mono bindings]),
            [case "${enableval}" in
                    yes) HAVE_MONO=yes ;;
                    no)  HAVE_MONO=no ;;
                    *) AC_MSG_ERROR(bad value ${enableval} for --enable-mono) ;;
            esac],
            [HAVE_MONO=yes])

    if test "x$HAVE_MONO" = "xyes" ; then
        AC_PATH_PROG(MCS, mcs)
        if test "x$MCS" = "x" ; then
            AC_MSG_ERROR([Can not find "mcs" in your PATH])
        fi

        AC_PATH_PROG(GACUTIL, gacutil)
        if test "x$GACUTIL" = "x" ; then
            AC_MSG_ERROR([Can not find "gacutil" in your PATH])
        fi

        AC_SUBST(MCS)
        AC_SUBST(GACUTIL)
    fi
fi
AM_CONDITIONAL(HAVE_MONO, test "x$HAVE_MONO" = "xyes")

#
# Check for monodoc stuff
#
HAVE_MONODOC=no
AC_ARG_ENABLE(monodoc,
        AS_HELP_STRING([--disable-monodoc],[Disable documentation for mono bindings]),
        [case "${enableval}" in
                yes) HAVE_MONODOC=yes ;;
                no)  HAVE_MONODOC=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-monodoc) ;;
        esac],
        [HAVE_MONODOC=yes])

if test "x$HAVE_MONO" = "xyes" && test "x$HAVE_MONODOC" = "xyes" ; then
        PKG_CHECK_MODULES(MONODOC, [monodoc >= 1.1.8])
        MONODOC_DIR=`$PKG_CONFIG --variable=sourcesdir monodoc`        

        AC_PATH_PROG(MONODOCER, monodocer)
        AC_PATH_PROG(MDASSEMBLER, mdassembler)

        AC_SUBST(MONODOC_DIR)
        AC_SUBST(MONODOCER)
        AC_SUBST(MDASSEMBLER)
fi
AM_CONDITIONAL(HAVE_MONODOC, test "x$HAVE_MONODOC" = "xyes")

#
# Defining Avahi User and Group.
#
AC_ARG_WITH(avahi_user, AS_HELP_STRING([--with-avahi-user=<user>],[User for running the Avahi daemon (avahi)]))
if test -z "$with_avahi_user" ; then
    AVAHI_USER=avahi
else
    AVAHI_USER=$with_avahi_user
fi
AC_SUBST(AVAHI_USER)
AC_DEFINE_UNQUOTED(AVAHI_USER,"$AVAHI_USER", [User for running the Avahi daemon])

AC_ARG_WITH(avahi_group,AS_HELP_STRING([--with-avahi-group=<group>],[Group for Avahi (avahi)]))
if test -z "$with_avahi_group" ; then
    AVAHI_GROUP=avahi
else
    AVAHI_GROUP=$with_avahi_group
fi
AC_SUBST(AVAHI_GROUP)
AC_DEFINE_UNQUOTED(AVAHI_GROUP,"$AVAHI_GROUP", [Group for Avahi])

#
# Avahi runtime dir
#
avahi_runtime_dir="${localstatedir}/run"
avahi_socket="${avahi_runtime_dir}/avahi-daemon/socket"
AC_SUBST(avahi_runtime_dir)
AC_SUBST(avahi_socket)

#
# Avahi interfaces dir
#
if test "x$HAVE_PYTHON_DBUS" = "xyes" -o "x$HAVE_GTK" = "xyes"; then
	interfacesdir="${datadir}/${PACKAGE}/interfaces/"
	AC_SUBST(interfacesdir)
fi

#
# Doxygen
#
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(avahi, doxygen.cfg, doxygen)

#
# Build and Install man pages
#
AC_ARG_ENABLE(manpages,
        AS_HELP_STRING([--disable-manpages],[Disable building and installation of man pages]),
[case "${enableval}" in
  yes) manpages=yes ;;
  no)  manpages=no ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --disable-manpages]) ;;
esac],[manpages=yes])

if test x$manpages = xyes ; then
    #
    # XMLTOMAN manpage generation
    #
    AC_ARG_ENABLE(xmltoman,
    AS_HELP_STRING([--disable-xmltoman],[Disable rebuilding of man pages with xmltoman]),
    [case "${enableval}" in
      yes) xmltoman=yes ;;
      no)  xmltoman=no ;;
      *) AC_MSG_ERROR([bad value ${enableval} for --disable-xmltoman]) ;;
    esac],[xmltoman=yes])
    
    if test x$xmltoman = xyes ; then
        AC_CHECK_PROG(have_xmltoman, xmltoman, yes, no)
    fi

    if test x$have_xmltoman = xno -o x$xmltoman = xno; then
        if ! test -e man/avahi-daemon.8 ; then
            AC_MSG_ERROR([*** xmltoman was not found or was disabled, it is required to build the manpages as they have not been pre-built, install xmltoman, pass --disable-manpages or dont pass --disable-xmltoman])
            exit 1
        fi
        AC_MSG_WARN([*** Not rebuilding man pages as xmltoman is not found ***])
        xmltoman=no
    fi
fi
AM_CONDITIONAL([USE_XMLTOMAN], [test "x$xmltoman" = xyes])
AM_CONDITIONAL([BUILD_MANPAGES], [test "x$manpages" = xyes])

#
# Conditionally compile test and example programs
#
AC_ARG_ENABLE(tests,
        AS_HELP_STRING([--enable-tests],[Enable building of tests and examples]),
        [case "${enableval}" in
                yes) ENABLE_TESTS=yes ;;
                no)  ENABLE_TESTS=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-tests) ;;
        esac],
        [ENABLE_TESTS=no])

AM_CONDITIONAL([ENABLE_TESTS], [test "x$ENABLE_TESTS" = "xyes"])

#
# Optionally enable libdns_sd compatibility support
#
AC_ARG_ENABLE(compat-libdns_sd,
        AS_HELP_STRING([--enable-compat-libdns_sd],[Enable compatibility layer for libdns_sd]),
        [case "${enableval}" in
                yes) ENABLE_COMPAT_LIBDNS_SD=yes ;;
                no)  ENABLE_COMPAT_LIBDNS_SD=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-compat-libdns_sd) ;;
        esac],
        [ENABLE_COMPAT_LIBDNS_SD=no])

AM_CONDITIONAL([ENABLE_COMPAT_LIBDNS_SD], [test "x$ENABLE_COMPAT_LIBDNS_SD" = "xyes"])

#
# Optionally enable HOWL compatibility support
#
AC_ARG_ENABLE(compat-howl,
        AS_HELP_STRING([--enable-compat-howl],[Enable compatibility layer for HOWL]),
        [case "${enableval}" in
                yes) ENABLE_COMPAT_HOWL=yes ;;
                no)  ENABLE_COMPAT_HOWL=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-compat-howl) ;;
        esac],
        [ENABLE_COMPAT_HOWL=no])

AM_CONDITIONAL([ENABLE_COMPAT_HOWL], [test "x$ENABLE_COMPAT_HOWL" = "xyes"])


# ==========================================================================
AC_CONFIG_FILES([
Makefile 
avahi-common/Makefile 
avahi-core/Makefile 
avahi-glib/Makefile 
avahi-qt/Makefile
avahi-daemon/Makefile 
avahi-daemon/avahi-dbus.conf
avahi-discover-standalone/Makefile 
avahi-client/Makefile 
initscript/Makefile 
initscript/debian/Makefile
initscript/gentoo/Makefile
initscript/archlinux/Makefile
initscript/suse/Makefile
initscript/fedora/Makefile
initscript/mandriva/Makefile
initscript/darwin/Makefile
initscript/freebsd/Makefile
initscript/slackware/Makefile
avahi-dnsconfd/Makefile
avahi-utils/Makefile
avahi-python/Makefile
avahi-python/avahi/Makefile
examples/Makefile
common/Makefile
man/Makefile
tests/Makefile
service-type-database/Makefile
avahi-sharp/Makefile
avahi-compat-libdns_sd/Makefile
avahi-compat-howl/Makefile
avahi-compat-howl/samples/Makefile
])
AC_OUTPUT

# ==========================================================================
echo "
 ---{ $PACKAGE_NAME $VERSION }---

    prefix:                 ${prefix}
    sysconfdir:             ${sysconfdir}
    localstatedir:          ${localstatedir}
    avahi socket:           ${avahi_socket}
    dbus-1 system.d dir:    ${DBUS_SYS_DIR}
    dbus-1 version:         ${DBUS_VERSION}
    dbus-1 system socket    ${DBUS_SYSTEM_BUS_DEFAULT_ADDRESS}
    compiler:               ${CC}
    cflags:                 ${CFLAGS}
    Enable GLIB:            ${HAVE_GLIB}
    Enable GTK:             ${HAVE_GTK}
    Enable D-BUS:           ${HAVE_DBUS}
    Enable Expat:           ${HAVE_EXPAT}
    Enable GDBM:            ${HAVE_GDBM}
    Enable libdaemon:       ${HAVE_LIBDAEMON}
    Enable Python:          ${HAVE_PYTHON}
    Enable pygtk:           ${HAVE_PYGTK}
    Enable python-dbus:     ${HAVE_PYTHON_DBUS}
    Enable QT3:             ${HAVE_QT3}
    Enable QT4:             ${HAVE_QT4}
    Enable Mono:            ${HAVE_MONO}
    Enable Monodoc:         ${HAVE_MONODOC}
    Distribution/OS:        ${with_distro}
    User for Avahi:         ${AVAHI_USER}
    Group for Avahi:        ${AVAHI_GROUP}
    Enable chroot():        ${enable_chroot}
"

BUILD_DAEMON="no   (You need libdaemon and expat!)"

if test "x$HAVE_EXPAT" = "xyes" -a "x$HAVE_LIBDAEMON" = "xyes" ; then
    BUILD_DAEMON=yes
fi

BUILD_PYTHON="no   (You need python, pygtk and python-dbus!)"

if test "x$BUILD_DAEMON" = "xyes" -a "x$HAVE_DBUS" = "xyes" -a "x$HAVE_PYTHON" = "xyes" -a "x$HAVE_PYTHON_DBUS" = "xyes" -a "x$HAVE_PYGTK" = "xyes" ; then
    BUILD_PYTHON=yes
fi

BUILD_CLIENT="no   (You need avahi-daemon and dbus!)"

if test "x$BUILD_DAEMON" = "xyes" -a "x$HAVE_DBUS" = "xyes" ; then
    BUILD_CLIENT=yes
fi

if test "x$ENABLE_COMPAT_LIBDNS_SD" = "xyes" -a "x$BUILD_CLIENT" != "xyes" ; then
   ENABLE_COMPAT_LIBDNS_SD="no   (You need libavahi-client!)"
fi
if test "x$ENABLE_COMPAT_HOWL" = "xyes" -a "x$BUILD_CLIENT" != "xyes" ; then
   ENABLE_COMPAT_HOWL="no   (You need libavahi-client!)"
fi


echo "\
    Building libavahi-core              yes
    Building avahi-daemon:              ${BUILD_DAEMON}
    Building avahi-dnsconfd:            ${BUILD_DAEMON}
    Building libavahi-client:           ${BUILD_CLIENT}
    Building avahi-utils:               ${BUILD_CLIENT}
    Building avahi-python:              ${BUILD_PYTHON}
    Building libavahi-glib:             ${HAVE_GLIB}
    Building avahi-discover-standalone: ${HAVE_GTK}
    Building libavahi-qt3:              ${HAVE_QT3}
    Building libavahi-qt4:              ${HAVE_QT4}
    Building avahi-sharp:               ${HAVE_MONO}
    Building avahi-compat-libdns_sd:    ${ENABLE_COMPAT_LIBDNS_SD}
    Building avahi-compat-howl:         ${ENABLE_COMPAT_HOWL}
    Building tests:                     ${ENABLE_TESTS}
"
