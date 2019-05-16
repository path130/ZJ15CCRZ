#rm ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz
#tar czvf ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz app/* lib/*
#./makeappbin RES ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz NC01 11B1 GYT
#cp ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz.bin ../../ZJ11BCBIP_V1.0_UPDATA_V2.0.bin

cp ./../$1 app/exec_update.elf
#cp ./../$1 /ruan/ipnc_targetfs/

if test -e $1.tar.gz
then
rm $1.tar.gz
fi

tar czvf $1.tar.gz app/* 
./makeappbin RES $1.tar.gz P006 15CH WRM
#cp $1.tar.gz.bin ../../$1.bin
mv $1.tar.gz.bin ./$1.bin
cp ./$1.bin /mnt/hgfs/tftpboot/targetfs/Z15CCRZ/pack
baseDir=$(cd "$(dirname "$0")"; pwd)
echo 升级包为: $baseDir/$1.bin
