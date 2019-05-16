#rm ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz
#tar czvf ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz app/* lib/*
#./makeappbin RES ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz NC01 11B1 GYT
#cp ZJ11BCBIP_V1.0_UPDATA_V2.0.tar.gz.bin ../../ZJ11BCBIP_V1.0_UPDATA_V2.0.bin
filename=ZJ5ACHIP_RES_V2.0
rm $filename.tar.gz
tar czvf $filename.tar.gz app/* lib/*
./makeappbin RES $filename.tar.gz P006 15AH GYT
mv $filename.tar.gz.bin ./$filename.bin
baseDir=$(cd "$(dirname "$0")"; pwd)
echo 资源包为: $baseDir/$filename.bin
