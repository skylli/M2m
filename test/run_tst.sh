#!/bin/bash
#./test是c程序，该程序 返回0

echo "----->runing tst_device  tst_device test"
../bin/tst_server &
../bin/tst_device &
echo "----->runing tst_mutiple_session_client test"
../bin/tst_mutiple_session_client
ret_mult_session=$?
echo "----->runing tst_mutiple_network_client test"
#../bin/tst_mutiple_network_client
ret_mult_network=$?
echo "----->runing tst_logic_client test"
../bin/tst_logic_client
ret_logic=$?


echo "XXXX killing test server and server"

kill $(pidof tst_server)
kill $(pidof tst_device)
echo "test result:"
if [ ${ret_mult_session} -ne 0  ];then
    echo "------------>> tst_mutiple_session_client have failt!!"
else 
    echo "------------>> tst_mutiple_session_client test successfully"
fi

if [ ${ret_mult_network} -ne 0  ];then
    echo "------------>> ret_mult_network have failt!!"
else 
    echo "------------>> ret_mult_network test successfully"
fi


if [ ${ret_logic} -ne 0  ];then
    echo "------------>> tst_logic_client have failt!!"
else 
    echo "------------>> tst_logic_client test successfully"
fi
