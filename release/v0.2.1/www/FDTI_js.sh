<script type="text/javascript">
function createTD(text){
	var tdObj = document.createElement("TD");
	tdObj.appendChild(document.createTextNode(text));
	return tdObj;
}

function setRowClass(trObj, context){
	context.row = context.row +1;
	if (context.row % 2 == 0){
		trObj.className = "even";
	}else{
		trObj.className = "odd";
	}
}
function appendTBody(tbody, networkId, networkData,context){
	if (typeof (networkData.nodes) == "undefined" || networkData.nodes.length==0){
		if (networkData.name == "NONAME"){
			return;
		}
		var trObj = document.createElement("TR");
		setRowClass(trObj, context);
		trObj.style.textAlign = "center";
		var tdNetwork = createTD(networkData.name);
		//tdNetwork.appendChild(document.createTextNode("(ID-"+networkId+")"));
		tdNetwork.rowSpan = 1;
		trObj.appendChild(tdNetwork);
		trObj.appendChild(createTD(""));
		trObj.appendChild(createTD(""));
		trObj.appendChild(createTD(""));
		trObj.appendChild(createTD(""));
		tbody.appendChild(trObj);
		return;
	}
	var rows = networkData.nodes.length;
	var firstRow = true;
	for(var i=0;i<networkData.nodes.length;i++){
		var node = networkData.nodes[i];
		var trObj = document.createElement("TR");
		setRowClass(trObj, context);
		trObj.style.textAlign = "center";
		if (firstRow){
			firstRow = false;
			var tdNetwork = createTD(networkData.name);
			//tdNetwork.appendChild(document.createElement("BR"));
			//tdNetwork.appendChild(document.createTextNode("(ID-"+networkId+")"));
			tdNetwork.rowSpan = rows;
			trObj.appendChild(tdNetwork);
		}
		var tdObj;
		// name
		tdObj = createTD(node.name);
		trObj.appendChild(tdObj);
		// id
		tdObj = createTD(node.network+","+node.id);
		trObj.appendChild(tdObj);
		// type
		var typeName = setS.DevicesTypes[node.type];
		if (typeName == null){
			setS.DevicesTypes['unknown'];
		}
		tdObj = createTD(typeName);
		trObj.appendChild(tdObj);
		// status
		if (node.online == 0){
			tdObj = createTD(setS.DevicesOffline);
		}else if (node.type=="unknown"){
			tdObj = createTD(setS.DevicesOffline);
		}else if(node.type=="light"){
			var valueString = node.level+"%";
			if (typeof node.colorRed == "number" && typeof node.colorGreen == "number" && typeof node.colorBlue == "number"){
				valueString ="RGB("+node.colorRed+","+node.colorGreen+","+node.colorBlue+") " + valueString;
			}
			tdObj = createTD(valueString);
		}else if(node.type=="rollerShutter"){
			tdObj = createTD(getPositionName(node.position));
		}else if(node.type=="temperatureSensor"){
			tdObj = createTD(node.temperature+"℃");
		}else{
			tdObj = createTD(setS.DevicesUnknown);
		}
		trObj.appendChild(tdObj);
		
		tbody.appendChild(trObj);
	}
}
function createDeltaDoreDeviceTable(topology){
	if (!topology.success){
		alert("操作失败：" + topology["message"]);
		return;
	}
	var context = {
		row: 1
	};
	var tbody = document.getElementById("delta_dore_device_table");
	tbody.innerHTML = "";
	var datas = topology.data;
	var dataGroupedByRegion = {};
	for(var networkId in datas){
		var networkData = datas[networkId].nodes;
		if (typeof networkData != "object"){
			alert(typeof networkData);
			continue;
		}
		for(var nodeId=0;nodeId<networkData.length;nodeId++){
			var nodeData = networkData[nodeId];
			var regionName = nodeData.region;
			if (typeof regionName=="undefined" || regionName == "" || regionName == "NO_NAME"){
				regionName = "NO_NAME";
			}
			if (typeof dataGroupedByRegion[regionName] == "undefined"){
				dataGroupedByRegion[regionName] = {name:regionName, nodes:[]};
			}
			var regionData = dataGroupedByRegion[regionName];
			nodeData.network = networkId;
			regionData.nodes.push(nodeData);
		}
		
	}
	//logJson(dataGroupedByRegion);
	for(var regionName in dataGroupedByRegion){
		var networkData = dataGroupedByRegion[regionName];
		appendTBody(tbody, 1, networkData, context)
	}
	
	//alert("TODO: 还没做完");
}

/**
 * functions for I18N when page loaded
 * 	  initTagText：used to set i18n text which has ‘data-text’ by tag name
 *    initCommandText：used to init command-selection 
 *    initText：  the whole entry point
 */
function initTagText(tagName, callback){
	var objList = document.getElementsByTagName(tagName);
	for(var i=0;i<objList.length;i++){
		var obj = objList[i];
		var textKey = obj.getAttribute("data-text");
		if (!textKey){
			continue;
		}
		var textVal = setS["Devices"+textKey];
		if (!textVal){
			textVal = textKey+"?";
		}
		callback(obj,textVal);
	}
}
function initCommandText(){
	var objList = document.getElementsByTagName("option");
	for(var i=0;i<objList.length;i++){
		var obj = objList[i];
		var textKey = obj.getAttribute("data-cmd-name");
		if (!textKey){
			continue;
		}
		var textVal = setS.DevicesCmdName[textKey];
		if (!textVal){
			textVal = textKey+"?";
		}
		obj.value=textKey;
		obj.innerHTML=textVal;
	}
}
function initText(){
	initTagText("input", function (elem, text) {
		elem.value = text;
	});
	initTagText("th", function (elem, text) {
		elem.innerHTML = text;
	});
	initTagText("label", function (elem, text) {
		elem.innerHTML = text;
	});
	initTagText("legend", function (elem, text) {
		elem.innerHTML = text;
	});
	initCommandText();
}
function getPositionName(pos){
	var name = setS.DevicesPositionName[pos];
	if (typeof name == "string"){
		return name;
	}
	return setS.DevicesUnknown;
}
/**
 * functions for construct command-string
 *     onCommandChanged： used to show/hide correct command-div block
 *     findChildByParamName: used to find element which annoted by 'data-param-name'
 *     getCmdParameterValue: used to get value of input/selection which annoted by 'data-param-name'
 *     executeUdpCmd: used to invoke function '???' to send & get response of selected command
 */
var commandDiv;
var paramVerify;
var max_param_length = 30;
function onCommandChanged(cmdName){
	if (commandDiv){
		commandDiv.style.display = 'none';
	}
	if (cmdName=='selectOneCommand'){
		return;
	}
	commandDiv = document.getElementById("cmdiv_"+cmdName);
	commandDiv.style.display = "block";
}
function findChildByParamName(obj, paramName){
	if (obj.getAttribute("data-param-name") == paramName){
		return obj;
	}
	if (!obj.children){
		return undefined;
	}
	for(var i=0;i<obj.children.length;i++){
		var subObj = findChildByParamName(obj.children[i], paramName);
		if (subObj){
			return subObj;
		}
	}
	return undefined;
}
function verifyParamValue(obj){
	var verifyRule = obj.getAttribute("data-param-verify");
	if (typeof verifyRule != "string"){
		return;
	}
	var ruleInputs = verifyRule.split(",");
	var isValidParam = true;
	var value=obj.value;
	switch (ruleInputs[0]){
	case "int":
		isValidParam=(value != '') && (!isNaN(value)) && (Number(value)>=Number(ruleInputs[1]) && Number(value)<=Number(ruleInputs[2]));
		paramVerify=paramVerify&&isValidParam;
		if (!isValidParam){
			obj.nextSibling.style.color="RED";
		}else{
			obj.nextSibling.style.color="";
		}
		break;
	case "string":
		
		isValidParam=(value != '');
		if (isValidParam && ruleInputs[1]=="MAX_NAME_LENGTH"){
			isValidParam = value.length <= max_param_length;
		}
		if (value.indexOf(" ")>=0){
			obj.value=value.replace(/ /ig,"_"); 
		}
		paramVerify=paramVerify&&isValidParam;
		if (!isValidParam){
			obj.nextSibling.style.color="RED";
			obj.nextSibling.style.display="";
		}else{
			obj.nextSibling.style.display="none";
		}
		break;
	case "none":
		break;
	default:
		alert(ruleInputs[0]);
	}
}

function getCmdParameterValue(divObj, paramName){
	var obj = findChildByParamName(divObj, paramName);
	if (obj){
		verifyParamValue(obj);
		return obj.value;
	}
	return paramName+"=?";
}
function executeUdpCmd(inputObj, cmdName, cmdParams, resultCallback){
	var cmdStr = "" + cmdName;
	var divObj = inputObj.parentNode;
	paramVerify = true;
	for(var i=0;i<cmdParams.length;i++){
		cmdStr = cmdStr +" "+getCmdParameterValue(divObj, cmdParams[i]);
	}
	if (!paramVerify){
		alert(setS.DevicesVerifyFail);
		return;
	}
	// TODO: logical later
	//runAjax("GET","tryFdti.sh",cmdStr.replace(" ","+"),resultCallback);
	runAjax("GET","fdti_driver.sh",cmdStr, resultCallback);
	//resultCallback(cmdStr);
}

/**
 *	functions for handle shell command result
 */
function popupResult(response){
	if(response.readyState == 4)
	{
		var reqRespStr = response.responseText;
		var result = JSON.parse(reqRespStr);
		if (result.success){
			showResult(result.data);
		}else{
			var errMsg = setS.DevicesExecuteFail[result.message];
			if (typeof errMsg=="string"){
				alert(errMsg);
			}else if (typeof result.message=="string"){
				alert(result.message);
			}else{
				alert(setS.DevicesExecuteFail.other);
			}
		}
	}else{
		//alert(response.readyState);
	}
}
function showResult(data){
	var divObj = document.getElementById("modal_result_table");
	divObj.innerHTML="";
	divObj.appendChild(createResultTable(data));
	var showObj = document.getElementById("modal-overlay");
	showObj.style.visibility="";
}
function createResultTable(data){
	var tblObj = document.createElement("TABLE");
	//tblObj.setAttribute("border", "1px");
	//tblObj.style.backgroundColor="#eee";
	for( name in data){
		var trObj=document.createElement("TR");
		var tdName=document.createElement("TD");
		tdName.style.margin="1px";
		tdName.style.backgroundColor="#eee";
		tdName.appendChild(document.createTextNode(getI18nText(name)));
		var tdValue = document.createElement("TD");
		if (typeof data[name] == "object"){
			tdValue.appendChild(createResultTable(data[name]));
		}else{
			tdValue.appendChild(document.createTextNode(data[name]));
			tdValue.style.margin="1px";
			tdValue.style.backgroundColor="#eee";
		}
		trObj.appendChild(tdName);
		trObj.appendChild(tdValue);
		tblObj.appendChild(trObj);
	}
	return tblObj;
}
function getI18nText(msg){
	var key = "Devices"+msg;
	if (typeof setS[key] == "string"){
		return setS[key];
	}
	return msg;
}
function refreshAfterDone(response){
	if(response.readyState == 4){
		var reqRespStr = response.responseText;
		var result = JSON.parse(reqRespStr);
		if (result.success){
			cmdRefreshDeviceTable();
			alert(setS.DevicesExecuteOk);
		}else{
			var errMsg = setS.DevicesExecuteFail[result.message];
			if (typeof errMsg=="string"){
				alert(errMsg);
			}else if (typeof result.message=="string"){
				alert(result.message);
			}else{
				alert(setS.DevicesExecuteFail.other);
			}
		}
	}
}
function refreshDeviceTable(response){
	if(response.readyState == 4){
                var reqRespStr = response.responseText;
                var result = JSON.parse(reqRespStr);

		createDeltaDoreDeviceTable(result);
	}
}
function cmdRefreshDeviceTable(){
	runAjax("GET","fdti_driver.sh","getTopology", refreshDeviceTable);
}

</script>
