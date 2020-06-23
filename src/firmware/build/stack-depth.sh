#!/bin/bash
THIS_DIR="$(dirname "$(readlink -f "$0")")";
perl "${THIS_DIR}/avstack.pl" `find "${THIS_DIR}/../src/" -iname "*.su" | sed s/\.su/.o/g`

echo "********************************************************************";
echo "Look for all '*Task()' entrypoints.  And don't forget to add to each";
echo "of these on any interrupt handlers that execute above FreeRTOS's";
echo "maximum priority, ie. for high-priority DMA transfers.";
echo "********************************************************************";
