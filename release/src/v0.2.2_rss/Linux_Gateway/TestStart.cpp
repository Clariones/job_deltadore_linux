#include "stdio.h"
#include "mngModule/DeltaDoreServer.h"
#include <string>


#include "test/TestTool.h"

using namespace std;


int main(void){
	printf("Start the FDTI gateway in test mode\n");

	DeltaDoreDriver_load();
	DeltaDoreDriver_init(1);
	DeltaDoreDriver_start();

//	string key="Object.Update";
//	string message = string(loadJson("deviceStateUpdate1.json"));
//	test_onMessage(key, message);
//	test_onRequest(string(loadJson("deviceStateUpdate1.json")));

	Helper::sleep_ms(300);
	//testSetValueAction();
	printf("Test done\n");
    Helper::sleep_ms(500000);
	DeltaDoreDriver_stop();
	DeltaDoreDriver_deinit();
	return 0;
}

