本次修改内容：
1. 修正了没有USB控制棒，Stop时的Segement Fault

部署说明：

1. 概要说明
	目录 so 下是达泰多驱动的SO文件
	目录 udpclient 是 webUI 使用的UDP工具
	目录 www 下是路由器的WEB UI控制界面需要的文件
	目录 config 是达泰多驱动的相关配置文件
	
2. 部署达泰多SO文件
	1. 将 so 目录下的 libdeltadore.so 放在 /usr/bin 下
	2. 将 config/modules_config.xml 中达泰多相关的配置加入 /etc/luoping/modules_config.xml 中。 只有达泰多设备的话，就直接拷贝到/etc/luoping下
	3. 将 config/save/dtdconfig.json 和 config/save/dtddata.json 拷贝到 /etc/luoping/save 中。
	4. 给以上文件正确的权限。 
	
3. 部署WEB UI相关文件
	1. 将 udpclient 目录下的 UdpClient.out 拷贝到 /root 目录下
	2. 给其可执行权限，例如使用命令 "chmod +x *.*"
	3. 将 www 目录下的所有文件，拷贝到 /www 下
	4. 给所有文件可执行权限，例如使用命令 "chmod +x *.*"
	
	
4. WEB UI相关文件详细说明
	有几个文件，是修改原系统所带文件的，如果原系统有变化，就不能简单的覆盖原文件了。
	1. wwww/settings.sh
		这个文件中加入了FDTI的相关控制代码， 修改是以下两行：
			line 52:  <%in FDTI_js.sh%> 
			line 53:  <%in FDTI_html.sh%>
		如果以后系统的settings.sh文件有变化，将以上两行放入即可。
		
	2. www/FDTI_html.sh
		这个文件是FDTI控制命令的界面代码。 是新增文件，拷贝到 /www即可
	
	3. www/FDTI_js.sh
		这个文件是FDTI控制命令的JavaScript代码。 是新增文件，拷贝到 /www即可
		
	4. www/fdti_driver.sh
		这个文件是FDTI控制命令服务端脚本, 会调用/root/UdpClient.out 来发送内部命令。 是新增文件，拷贝到 /www即可
		
	5. www/i18n/English-EN/settings.json
		这个文件是FDTI控制命令的界面I18N资源，是在原系统文件基础上，增加了FDTI驱动相关的字符串资源。
		修改的部分是
			line 56:  setS.DevicesArea = "Area";
			line 57:  setS.DevicesNode = "Node";
			line 58:  setS.DevicesName = "Name";
			line ......
		即所有以 setS.Devices 开头的变量，都是设备控制使用的资源
		如果以后系统的settings.sh文件有变化，将这些修改合并放入即可。
		
	6. www/i18n/SimplifiedChinese-ZH-CN/settings.json
		这个文件是FDTI控制命令的界面I18N资源，是在原系统文件基础上，增加了FDTI驱动相关的字符串资源。
		修改的部分是
			line 58:  setS.DevicesArea = "区域";
			line 59:  setS.DevicesNode = "节点";
			line 60:  setS.DevicesName = "名称";
			line ......
		即所有以 setS.Devices 开头的变量，都是设备控制使用的资源
		如果以后系统的settings.sh文件有变化，将这些修改合并放入即可。
		
	7. udpclient/UdpClient.out
		这个文件是用来发送内部消息的。
		它在 www/fdti_driver.sh 被调用：
			/root/UdpClient.out 127.0.0.1 1800 "$d"
		三个参数依次是：
			IP 地址： 默认是本机地址 127.0.0.1. 也支持其他地址。
			端口号：默认是 1800， 这个参数需要和 /etc/luoping/save/dtdconfig.json 中参数 webCommandPort 一致
			命令字符串：这个是从web UI 发送过来的命令。 手工使用时，可以使用 help 获得相关的命令说明。 例如：/root/UdpClient.out 127.0.0.1 1800 "help"
	
5. 设备驱动相关文件详细说明
	1. config/modules_config.xml
		管理模块配置入口文件。不再赘述。
		
	2. config/save/dtdconfig.json
		达泰多设备驱动的配置文件。 内容是：
			"moduleId": 模块ID， 默认是 "DeltaDoreDriver",
			"manufacturer": 制造商名称，默认是"Delta Dore",
			"webCommandPort": 接收WebUI 命令的UDP端口值，默认是1800, 需要和www/fdti_driver.sh中使用的端口号保持一致。
			"deviceName": 设备名称。 默认是"/dev/ttyUSB0"
	3. config/save/dtddata.json
		达泰多设备管理模块的数据文件。
		文件使用JSON格式，内容由程序读写，不必关心格式。
		

附：手工启动命令：
首先停止原来的主程序，依次执行以下命令：
/etc/init.d/smarthome stop
killall -9 HomeControlCenter

然后启动主程序：
	直接在前台启动： /usr/bin/HomeControlCenter
		启动为前台进程，直接Ctrl+C 退出
	或者启动为后台进程，并记录log到文件log.txt: /usr/bin/HomeControlCenter > log.txt &
		启动为后台进程也可以用killall -9 HomeControlCenter 杀掉
