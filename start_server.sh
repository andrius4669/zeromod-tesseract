#!/bin/sh
# TESS_DATA should refer to the directory in which Tesseract data files are placed.
#TESS_DATA=~/tesseract
#TESS_DATA=/usr/local/tesseract
TESS_DATA=.

# TESS_BIN should refer to the directory in which Tesseract executable files are placed.
TESS_BIN=${TESS_DATA}/bin_unix

# TESS_OPTIONS contains any command line options you would like to start Tesseract with.
TESS_OPTIONS=""

# SYSTEM_NAME should be set to the name of your operating system.
#SYSTEM_NAME=Linux
SYSTEM_NAME=`uname -s`

# MACHINE_NAME should be set to the name of your processor.
#MACHINE_NAME=i686
MACHINE_NAME=`uname -m`

case ${SYSTEM_NAME} in
Linux)
  SYSTEM_NAME=linux_
  ;;
*)
  SYSTEM_NAME=unknown_
  ;;
esac

case ${MACHINE_NAME} in
i486|i586|i686)
  MACHINE_NAME=
  ;;
x86_64|amd64)
  MACHINE_NAME=64_
  ;;
*)
  if [ ${SYSTEM_NAME} != native_ ]
  then
    SYSTEM_NAME=native_
  fi
  MACHINE_NAME=
  ;;
esac

if [ -x ${TESS_BIN}/native_server ]
then
  SYSTEM_NAME=native_
  MACHINE_NAME=
fi

if [ -x ${TESS_BIN}/${SYSTEM_NAME}${MACHINE_NAME}server ]
then
  cd ${TESS_DATA}
  ulimit -c unlimited
  exec ${TESS_BIN}/${SYSTEM_NAME}${MACHINE_NAME}server ${TESS_OPTIONS} "$@"
else
  echo "Your platform does not have a pre-compiled Tesseract server."
  echo "Please follow the following steps to build a native server:"
  echo "1) Ensure you have the zlib headers installed."
  echo "2) Type \"make -C src install\"."
  echo "3) If the build succeeds, run this script again."
  exit 1
fi 
