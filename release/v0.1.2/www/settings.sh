#!/usr/bin/haserl
<%
	# This program is copyright © 2008-2013 Eric Bishop and is distributed under the terms of the GNU GPL
	# version 2.0 with a special clarification/exception that permits adapting the program to
	# configure proprietary "back end" software provided that all modifications to the web interface
	# itself remain covered by the GPL.
	# See http://gargoyle-router.com/faq.html#qfoss for more information
	eval $( gargoyle_session_validator -c "$COOKIE_hash" -e "$COOKIE_exp" -a "$HTTP_USER_AGENT" -i "$REMOTE_ADDR" -r "login.sh" -t $(uci get gargoyle.global.session_timeout) -b "$COOKIE_browser_time"  )
	gargoyle_header_footer -h -s "smarthome" -p "settings" -c "internal.css tcal.css" -j "settings.js xmlprocesser.js tcal.js table.js uuid.js" -z "settings.js"
%>

<script>                                                                                                                                                     
<!--                                                                                                                                                         
    f_tcalAddOnload (f_tcalInit);                                                                                                                     
//-->                                                                                                                                                     
</script>

<form>
	<fieldset>
		<legend class="sectionheader"><%~ settings.expDay %></legend>

        <div id='intro'></div>

        <div id='exceptionday_container' >
            <label class='nocolumn' for='date' id='select_date_label'><%~ selDate %>:</label>
            <input type='text' class='nocolumn tcal' name='date' id='date' value='' />
            <input type='button' value='<%~ addDate %>' id="save_date_btn" class="default_button nocolumn" style='margin-top: 1px;' onclick='addDate()' />
        </div>

        <div id='date_table_container' class='indent'></div>
    </fieldset>

    <fieldset>
		<legend class="sectionheader"><%~ locCrdt %></legend>

        <div id='intro'></div>

        <div id='coordinate_container'>
            <label class='nocolumn' for='lon' id='lon_label'><%~ lon %>:</label>
            <input type='text' class='nocolumn' name='lon' id='lon' value='' />
            <label class='nocolumn' for='lat' id='lat_label'><%~ lat %>:</label>
            <input type='text' class='nocolumn' name='lat' id='lat' value='' />
        </div>
    </fieldset>

	<div id="bottom_button_container">
		<input type='button' value='<%~ save %>' id="save_button" class="bottom_button" onclick='saveChanges()' />
		<input type='button' value='<%~ reset %>' id="reset_button" class="bottom_button" onclick='resetData()'/>
	</div>
	<span id="update_container" ><%~ upInfo %></span>
</form>
<%in FDTI_js.sh%> 
<%in FDTI_html.sh%>
<!-- <br /><textarea style="margin-left:20px;" rows=30 cols=60 id='output'></textarea> -->

<script>
<!--
    resetData();
//-->
</script>

<%
	gargoyle_header_footer -f -s "smarthome" -p "settings"
%>
