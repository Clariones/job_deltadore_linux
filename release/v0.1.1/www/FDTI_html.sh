	<div id="internal_divider1" class="internal_divider"></div>	
	<fieldset>
			<legend data-text="TitleControl">Device Controlling</legend>	
			<div>
				<label data-text="TitleSelectCommand">Select a command</label>
				<select onchange="onCommandChanged(this.value)">
					<option data-cmd-name="selectOneCommand" selected="selected"></option>
					<option data-cmd-name="controlLight"></option>
					<option data-cmd-name="controlLightLevel"></option>
					<option data-cmd-name="controlLightColor"></option>
					<option data-cmd-name="controlRollerShutter"></option>
					<option data-cmd-name="deleteNode"></option>
					<option data-cmd-name="forceRefreshTopology"></option>
					<option data-cmd-name="getTopology"></option>
					<option data-cmd-name="queryLightColor"></option>
					<option data-cmd-name="queryLightInfo"></option>
					<option data-cmd-name="queryLightStatus"></option>
					<option data-cmd-name="queryRollerShutterInfo"></option>
					<option data-cmd-name="queryRollerShutterStatus"></option>
					<option data-cmd-name="registerNode"></option>
					<option data-cmd-name="setNodeName"></option>
				</select>
			</div>
	
			<div id="cmdiv_controlLightColor" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="ControlLightColorDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
			<div>
				<label class="narrowleftcolumn" data-text="ParamRed"></label>
				<input data-param-verify="int,0,255" data-param-name="red"/><label>[0,255]</label>
			</div>
			<div>
				<label class="narrowleftcolumn" data-text="ParamGreen"></label>
				<input data-param-verify="int,0,255" data-param-name="green"/><label>[0,255]</label>
			</div>
			<div>
				<label class="narrowleftcolumn" data-text="ParamBlue"></label>
				<input data-param-verify="int,0,255" data-param-name="blue"/><label>[0,255]</label>
			</div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'controlLightColor', ['network','node','red','green','blue'], refreshAfterDone)" type="button"/>
			</div>
	
			<!-- level -->
			<div id="cmdiv_controlLightLevel" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="ControlLightLevelDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
			<div>
				<label class="narrowleftcolumn" data-text="ParamRed"></label>
				<input data-param-verify="int,0,100" data-param-name="level"/><label>[0,100]</label>
			</div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'controlLightLevel', ['network','node','level'], refreshAfterDone)" type="button"/>
			</div>
			<!-- -->
			<div id="cmdiv_forceRefreshTopology" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="ForceRefreshTopologyDscp"></label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'forceRefreshTopology', [], refreshDeviceTable)" type="button"/>
			</div>
	
			<div id="cmdiv_controlLight" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="ControlLightDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
		<div>
			<label class="narrowleftcolumn" data-text="ParamOption"></label>
			<select data-param-verify="none" data-param-name="option">
						<option value="alarm-pairing">alarm-pairing</option>
						<option value="off">off</option>
						<option value="on">on</option>
						<option value="preset-1">preset-1</option>
						<option value="preset-2">preset-2</option>
						<option value="ram-down">ram-down</option>
						<option value="ram-up">ram-up</option>
						<option value="stand-out">stand-out</option>
						<option value="stop">stop</option>
						<option value="toggle">toggle</option>
			</select>
		</div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'controlLight', ['network','node','option'], refreshAfterDone)" type="button"/>
			</div>
	
			<div id="cmdiv_queryRollerShutterInfo" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="QueryRollerShutterInfoDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'queryRollerShutterInfo', ['network','node'], popupResult)" type="button"/>
			</div>
	
			<div id="cmdiv_setNetworkName" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="SetNetworkNameDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
			<div>
				<label class="narrowleftcolumn" data-text="ParamName"></label>
				<input data-param-verify="string,MAX_NAME_LENGTH" data-param-name="name"/><label style="display:none" data-text="NotNull"></label>
			</div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'setNetworkName', ['network','name'], refreshAfterDone)" type="button"/>
			</div>
	
			<div id="cmdiv_queryRollerShutterStatus" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="QueryRollerShutterStatusDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'queryRollerShutterStatus', ['network','node'], popupResult)" type="button"/>
			</div>
	
			<div id="cmdiv_controlRollerShutter" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="ControlRollerShutterDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
		<div>
			<label class="narrowleftcolumn" data-text="ParamOption"></label>
			<select data-param-verify="none" data-param-name="option">
						<option value="down">down</option>
						<option value="down-slow">down-slow</option>
						<option value="stop">stop</option>
						<option value="up">up</option>
						<option value="up-slow">up-slow</option>
			</select>
		</div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'controlRollerShutter', ['network','node','option'], refreshAfterDone)" type="button"/>
			</div>
	
			<div id="cmdiv_queryLightStatus" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="QueryLightStatusDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'queryLightStatus', ['network','node'], popupResult)" type="button"/>
			</div>
	
			<div id="cmdiv_registerNode" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="RegisterNodeDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'registerNode', ['network'], refreshAfterDone)" type="button"/>
			</div>
	
			<div id="cmdiv_getTopology" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="GetTopologyDscp"></label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'getTopology', [], refreshDeviceTable)" type="button"/>
			</div>
	
			<div id="cmdiv_queryLightColor" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="QueryLightColorDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'queryLightColor', ['network','node'], popupResult)" type="button"/>
			</div>
	
			<div id="cmdiv_queryLightInfo" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="QueryLightInfoDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'queryLightInfo', ['network','node'], popupResult)" type="button"/>
			</div>
	
			<div id="cmdiv_setNodeName" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="SetNodeNameDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
			<div>
				<label class="narrowleftcolumn" data-text="Area"></label>
				<input data-param-verify="string,MAX_NAME_LENGTH" data-param-name="region"/><label style="display:none" data-text="NotNull"></label>
			</div>
			<div>
				<label class="narrowleftcolumn" data-text="ParamName"></label>
				<input data-param-verify="string,MAX_NAME_LENGTH" data-param-name="name"/><label style="display:none" data-text="NotNull"></label>
			</div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'setNodeName', ['network','node','region','name'], refreshAfterDone)" type="button"/>
			</div>
	
			<div id="cmdiv_deleteNode" style="display:none;">
				<div><label data-text="TitleCmdParameters">Parameters</label>:<label data-text="DeleteNodeDscp"></label></div>
				<div><label class="narrowleftcolumn" data-text="AreaID"></label><input data-param-verify="int,0,11" data-param-name="network"/><label>[0,11]</label></div>
				<div><label class="narrowleftcolumn" data-text="NodeID"></label><input data-param-verify="int,0,15" data-param-name="node"/><label>[0,15]</label></div>
				<input class="default_button" data-text="Execute" onclick="executeUdpCmd(this, 'deleteNode', ['network','node'], refreshAfterDone)" type="button"/>
			</div>
	</fieldset>
	<div id="internal_divider1" class="internal_divider"></div>
	<fieldset>
		<legend data-text="AllInTable">设备列表</legend>
		
		<table>
		<tr class="header_row">
			<th data-text="Area"></th>
			<th data-text="Name"></th>
			<th data-text="Node"></th>
			<th data-text="DeviceType"></th>
			<th data-text="State"></th></tr>
			<tbody id="delta_dore_device_table"></tbody>
		</table>
		<input class="default_button" data-text="Refresh" onclick="cmdRefreshDeviceTable()" type="button"/>
	</fieldset>
	<div id="modal-overlay" style="visibility:hidden; position: absolute; left: 0px; top: 0px; width:100%; height:100%; text-align:center; z-index: 1000; background-color: rgba(3,3,3,0.2); "> 
		<div style="width:300px; margin: 100px auto; background-color: rgba(255,255,255,1); border:1px solid #000; padding:15px; text-align:center; z-index: 1100;">
			<button onclick="document.getElementById('modal-overlay').style.visibility='hidden'" style="float:right;">X</button>
			<label data-text="ExecuteOk"></label>
			<hr/>
			<div id="modal_result_table" style="text-align: left; margin: 1px;"></div>
		</div>
	</div>
	<script type="text/javascript">
		initText();
	</script>
	
	
