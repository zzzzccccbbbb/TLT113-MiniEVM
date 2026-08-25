#!/bin/bash

rm -rf ./RI-2020.4-linux
if [ ! -d "./RI-2020.4-linux" ]; then
        tar zxvf downloads/aw_axi_cfg0_linux.tgz
        cd ./RI-2020.4-linux
        mkdir -p config
        cd ./aw_axi_cfg0
        ./install --xtensa-tools "../../install/tools/RI-2020.4-linux/XtensaTools" --no-default --no-replace --registry "../config"
fi
