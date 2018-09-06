#!/bin/bash
script_name=$(readlink -e $0)
script_dir=$(dirname $script_name)
prj_dir=$(dirname $script_dir)

date
echo "$prj_dir"
rm --force ./cppcheck.cppcheck.log
#-I $prj_dir/wfc -I $prj_dir/examples -I $prj_dir/tests
$script_dir/cppcheck-exec.sh $@ $prj_dir/iow $prj_dir/tests  |& tee ./cppcheck.cppcheck.log
sh -c "! grep '\[' ./cppcheck.cppcheck.log"
res=$?
rm --force ./cppcheck.cppcheck.log
exit $res
