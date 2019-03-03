ps -efw|grep httpclient|awk '{print $2}'|xargs kill -9
