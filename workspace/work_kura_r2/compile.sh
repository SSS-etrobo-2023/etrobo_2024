#!/bin/bash

DEV_DIR="${HOME}/etrobo_2024/workspace/"
EXE_DIR="${HOME}/work/RasPike/sdk/workspace/"

if [ $# != 1 ]; then
    echo "make 対象のディレクトリを指定してください"
    exit 1
fi

MAKE_DIR=$1

cd ${DEV_DIR}
if [ ! -d ${MAKE_DIR} ]; then
    echo "make 対象のディレクトリが存在しません"
    exit 1
fi

# 開発環境から実行環境にファイルをコピーする
mkdir -p ${EXE_DIR}/${MAKE_DIR}
cp -r ${DEV_DIR}/${MAKE_DIR}/* ${EXE_DIR}/${MAKE_DIR}/

# プログラムをコンパイル
cd ${EXE_DIR}
make img=${MAKE_DIR}

if [ $? -ne 0 ]; then
    exit 1
fi

read -p "Hit enter to start: "

# プログラム実行
make start

exit 0
