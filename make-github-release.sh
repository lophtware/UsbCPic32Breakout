#!/bin/bash
VERSION="v${1:-0.1.0}";
TEMP_DIR=`mktemp -d`;
ZIP_DIR="${TEMP_DIR}/UsbCPic32Breakout";
BASE_DIR="${ZIP_DIR}/${VERSION}";
SRC_DIR=`pwd`;
echo "*** TEMPORARY FILES WILL BE WRITTEN TO ${TEMP_DIR}";
mkdir -p ${BASE_DIR};

cp "src/schematics/Plots/UsbCPic32Breakout - Project.pdf" ${BASE_DIR}/UsbCPic32Breakout-Schematics.pdf;
cp src/firmware/src/UsbCPic32Breakout.{hex,elf,map} ${BASE_DIR};

cd ${TEMP_DIR};
zip -9r UsbCPic32Breakout-${VERSION}.zip UsbCPic32Breakout;
tar -czv UsbCPic32Breakout > UsbCPic32Breakout-${VERSION}.tar.gz;

echo "*** OUTPUTS:";
ls -l ${TEMP_DIR}/*.{zip,tar.gz};
