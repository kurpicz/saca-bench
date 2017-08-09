
dnl
dnl @synopsis KBS_CHECK_VERSION
dnl
dnl Displays the version of KBS being built and sets
dnl the variables "kbs_devel_version" and "kbs_prerelease".
dnl
AC_DEFUN(KBS_CHECK_VERSION,[
changequote(, ) dnl
echo "configuring KBS version $VERSION"
if echo "$VERSION" | grep 'cvs' >/dev/null ; then
  kbs_devel_version=yes
  echo "WARNING: This is a development version. Expect bugs."
else
  kbs_devel_version=no
fi
if echo "$VERSION" | grep 'pre' > /dev/null ; then
    kbs_prerelease=yes
    echo "WARNING: This is a prerelease. Be careful."
else
    kbs_prerelease=no
fi
changequote([, ]) dnl
AC_SUBST(kbs_devel_version)
if test $kbs_devel_version = yes ; then
  AC_DEFINE(DEVEL_VERSION, 1, Define if building a development version of KBS)
fi])

dnl
dnl @synopsis KBS_SEARCH_PROG
dnl
dnl KBS_SEARCH_PROG(VARIABLE-NAME,PROGRAMS-LIST,ACTION-IF-FOUND)
dnl
define(KBS_SEARCH_PROG,[dnl
for ac_prog in $2 ; do
### Extract the first word of "$ac_prog", so it can be a program name with args.
  set dummy $ac_prog ; ac_word=$[2]
  if test -z "[$]$1"; then
    IFS="${IFS= }"; ac_save_ifs="$IFS"; IFS=":"
    for ac_dir in $PATH; do
      test -z "$ac_dir" && ac_dir=.
      if test -f [$ac_dir/$ac_word]; then
        $1="$ac_prog"
        break
      fi
    done
    IFS="$ac_save_ifs"
  fi

  if test -n "[$]$1"; then
    ac_result=yes
  else
    ac_result=no
  fi
  ifelse($3,,,[$3])
  test -n "[$]$1" && break
done
])


dnl
dnl NOTE TO SELF : REPLACE CXX CHECKS WITH C CHECKS
dnl

dnl
dnl @synopsis KBS_PROG_CXX
dnl
dnl Checks for availability of proper C++ compiler.
dnl
AC_DEFUN(KBS_PROG_CXX,
[AC_MSG_CHECKING([for a good enough C++ compiler])
KBS_SEARCH_PROG(CXX, $CXX $CCC g++ gcc c++ CC cxx xlC cc++, [KBS_PROG_CXX_WORKS])

if test -z "$CXX" ; then
    AC_ERROR([Unable to find a good enough C++ compiler])
fi
AC_MSG_RESULT($CXX)

AC_PROG_CXX

### Enable or disable warnings.
AC_ARG_ENABLE(warnings,
  [  --enable-warnings       tell the compiler to display more warnings],,
  [ if test $kbs_devel_version = yes -o $kbs_prerelease = yes && test $ac_cv_prog_gxx = yes ; then
        enable_warnings=yes;
    else
        enable_warnings=no;
    fi;])
if test x$enable_warnings = xyes ; then
  kbs_flags="$kbs_flags warnings"
  AC_DEFINE(WITH_WARNINGS, 1,
  [Define this if you want to see the warning directives put here and
   there to get attention])
fi

### Enable or disable profiling.
AC_ARG_ENABLE(profiling,
  [  --enable-profiling      tell the compiler to enable profiling],,
        enable_profiling=no;)
if test x$enable_profiling = xyes ; then
  kbs_flags="$kbs_flags profiling"
  AC_DEFINE(WITH_PROFILING, 1,
  [Define this if you want to have profiling information])
fi

### Enable or disable debug.
AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debug information],,
  [ if test $kbs_devel_version = yes -o $kbs_prerelease = yes && test $ac_cv_prog_gxx = yes ; then
        enable_debug=yes;
    else
        enable_debug=no;
    fi;])
if test x$enable_debug = xyes ; then
  kbs_flags="$kbs_flags debug"
  AC_DEFINE(WITH_DEBUG, 1,
  [Define this if you want to see the debug information])
fi

### Setup optimization
AC_ARG_ENABLE(optimization,
  [  --enable-optimization[=value]   enable compiler optimization],,
        enable_optimization=yes;)
case $enable_optimization in
  yes) kbs_opt=-O2;;
   no) kbs_opt=;;
    *) kbs_opt=${enable_optimization};;
esac

### Set correct debug flags.
if test x$GXX = xyes ; then
  gxx_version=`${CXX} -dumpversion`
  CXX_VERSION="($gxx_version)"

  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  else
    case $gxx_version in
      2.95.1)  CXXFLAGS="$kbs_opt -fpermissive -fno-rtti -fno-exceptions";;
      2.95.2)  CXXFLAGS="$kbs_opt -fno-exceptions";;
      2.95.*)  CXXFLAGS="$kbs_opt";;
      2.96*)   CXXFLAGS="$kbs_opt -fno-exceptions";;
      3.0*)    CXXFLAGS="$kbs_opt -fno-exceptions";;
      *2.91.*) CXXFLAGS="$kbs_opt -fno-exceptions";;
      *)       CXXFLAGS="$kbs_opt -fno-rtti -fno-exceptions";;
    esac
    if test x$enable_debug = xyes ; then
        CXXFLAGS="-g $CXXFLAGS"
    fi
    if test x$enable_profiling = xyes ; then
        CXXFLAGS="-pg $CXXFLAGS"
    fi
  fi
  if test x$enable_warnings = xyes ; then
    case $gxx_version in
        2.95.*) CXXFLAGS="$CXXFLAGS -W -Wall";;
        2.96*)  CXXFLAGS="$CXXFLAGS -W -Wall";;
        *)      CXXFLAGS="$CXXFLAGS -W -Wall";;
    esac
    if test $kbs_devel_version = yes ; then
      case $gxx_version in
          2.95.*) ;;
          2.96*) ;;
          2.97*) ;;
          *2.91*) ;;
          *) ;;
      esac
    fi
  fi
fi])
