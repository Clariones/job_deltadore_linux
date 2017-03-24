#!/usr/bin/haserl
<? 
	# This program is copyright ? 2008 Eric Bishop and is distributed under the terms of the GNU GPL
	# version 2.0 with a special clarification/exception that permits adapting the program to
	# configure proprietary "back end" software provided that all modifications to the web interface
	# itself remain covered by the GPL.
	# See http://gargoyle-router.com/faq.html#qfoss for more information
	echo "Content-type: application/json"
	echo ""
	a=$QUERY_STRING
	d=$(echo $a | sed -e's/%\([0-9A-F][0-9A-F]\)/\\\\\x\1/g' | xargs echo -e)
	#b=`echo $b | sed 's/\\/\\\\/g;s/\(%\)\([0-9a-fA-F][0-9a-fA-F]\)/\\x\2/g' | xargs echo -e`
	/root/UdpClient.out 127.0.0.1 1800 "$d"
	#echo ${b}>>/www/rcd.txt
	#echo '{"message":"success","success":true,"input":"'${d}'"}'
?>
