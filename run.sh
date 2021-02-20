date=$1

echo $date


check_cmd="aws s3 ls s3://sprs.push.us-east-1.prod/data/warehouse/model/train_data/$date/_SUCCESS | wc -l"
ret=`eval $check_cmd`
echo $ret

while [ $ret -ne 1 ]
do
	echo 'wait $check_cmd'
	sleep 10
	ret=`eval $cmd`
	echo $ret
done

cd /root/train_data

rm -rf $date

aws s3 sync s3://sprs.push.us-east-1.prod/data/warehouse/model/train_data/$date $date

cd /root/project/feature_test

rm -rf ../../ads_train_data/${date}
mkdir  ../../ads_train_data/${date}

ls ../../train_data/$date/p* | xargs python extract_feature.py

cd /root/ads_train_data/$date

mkdir feature_index
mkdir pre_data
mkdir train_data
mv part-0019* pre_data
mv part* train_data
cp /root/test/20210118/feature_index/* feature_index
