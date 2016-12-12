mkdir tmppatch

filename="version.txt"
version=$(cat $filename)
vNums=( $(echo $version | awk -F "." '{print $1, $2, $3}') )
versionNew="$((vNums[0])).$((vNums[1])).$((vNums[2]+1))"
echo $versionNew  > $filename
version=$(cat $filename)
echo "Generated new version: $version"
git add version.txt
git commit -m "commit version file: $version"
git push -u origin stable

wget http://serv2.radio.pub.ro/gitlab/pilots/ewallhome/raw/master/ewallhome.tar.gz?private_token=6uxej1RJNS4DjNQiXCRD -O ewallhome.tar.gz
tar -zxf ewallhome.tar.gz -C tmppatch/
rm ewallhome.tar.gz

git archive stable | gzip > temp.tar.gz
tar -zxf temp.tar.gz -C tmppatch/
rm temp.tar.gz

cd tmppatch
rm -r Accelerometer
rm -r libs
rm -r lightControl
rm -r mobile
rm -r SocketSense
rm -r SocketSense_win
rm -r UserSim
rm fitbit_sync/usbreset
rm noninMAC.txt
rm createPatch.sh
rm add_as_startup.sh
find . -name "*.git*" -exec rm -R {} \;
tar -zcvf ../ewallhome.tar.gz *
cd ..
rm -r tmppatch


