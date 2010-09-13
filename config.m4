dnl $Id$
dnl config.m4 for extension event

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(event, for libevent support,
    Make sure that the comment is aligned:
    [  --with-event             Include libevent support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(event, whether to enable event support,
dnl Make sure that the comment is aligned:
dnl [  --enable-event           Enable event support])

if test "$PHP_EVENT" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-event -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/evhttp.h"  # you most likely want to change this
  if test -r $PHP_EVENT/$SEARCH_FOR; then # path given as parameter
    EVENT_DIR=$PHP_EVENT
  else # search default path list
    AC_MSG_CHECKING([for libevent files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        EVENT_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$EVENT_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the event distribution])
  fi

  dnl # --with-event -> add include path
  PHP_ADD_INCLUDE($EVENT_DIR/include)

  dnl # --with-event -> check for lib and symbol presence
  LIBNAME=event # you may want to change this
  LIBSYMBOL=event_init # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $EVENT_DIR/lib, EVENT_SHARED_LIBADD)
    AC_DEFINE(HAVE_EVENTLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong event lib version or lib not found])
  ],[
    -L$EVENT_DIR/lib -lm -lc
  ])
  PHP_SUBST(EVENT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(event, event.c, $ext_shared)
fi
