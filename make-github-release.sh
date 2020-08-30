#!/bin/bash
VERSION="v${1:-0.1.0}";
TEMP_DIR=`mktemp -d`;
ZIP_DIR="${TEMP_DIR}/UsbCPic32Breakout";
BASE_DIR="${ZIP_DIR}/${VERSION}";
FULLFAT_BASE_DIR="${ZIP_DIR}/${VERSION}/FullFat";
LITE_BASE_DIR="${ZIP_DIR}/${VERSION}/Lite";
SRC_DIR=`pwd`;
echo "*** TEMPORARY FILES WILL BE WRITTEN TO ${TEMP_DIR}";
mkdir -p ${FULLFAT_BASE_DIR};
mkdir -p ${LITE_BASE_DIR};

cp "src/schematics/fullfat/Plots/UsbCPic32Breakout - Project.pdf" ${FULLFAT_BASE_DIR}/UsbCPic32Breakout-FullFat-Schematics.pdf;
cp src/firmware/src/UsbCPic32Breakout-FullFat.{hex,elf,map} ${FULLFAT_BASE_DIR};

cp "src/schematics/lite/Plots/UsbCPic32BreakoutLite - Project.pdf" ${LITE_BASE_DIR}/UsbCPic32Breakout-Lite-Schematics.pdf;
cp src/firmware/src/UsbCPic32Breakout-Lite.{hex,elf,map} ${LITE_BASE_DIR};

cd ${TEMP_DIR};
zip -9r UsbCPic32Breakout-${VERSION}.zip UsbCPic32Breakout;
tar -czv UsbCPic32Breakout > UsbCPic32Breakout-${VERSION}.tar.gz;

echo "*** OUTPUTS:";
ls -l ${TEMP_DIR}/*.{zip,tar.gz};
