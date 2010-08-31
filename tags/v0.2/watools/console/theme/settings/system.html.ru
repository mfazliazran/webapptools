﻿<script>
var tabSystemIndex = 2;
$(function() {
    $('#savebtn').button({
            icons: {
                primary: 'ui-icon-check'
            }
        });
    $('#themestable button').each(function(){
        $(this).button({
            icons: {primary: 'ui-icon-image'}
            })
    });
    $('#instWarn').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                $(this).dialog('close');
            }
        }
    });
    $('#themeWarn').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                $(this).dialog('close');
            }
        }
    });
});
function SaveTheme() {
    th = $("#themeID").val();
    ln = $("#langID").val();
	alert(th + "; " + ln);
    if (th == -1) {
        $('#instWarn').dialog('open');
    }
    else {
        $('#themeWarnMsg').html('');
        $('#themeWarnMsg').load('themesModyfy.php', {'object': 'themeset', 'theme': th, 'lang': ln },
                function(data) {
                    resp = jQuery.trim(data);
                    if (resp == "") {
                        $('#themeWarnMsg').html("Unknown error");
                    }
                    if (resp != "OK") {
                        $('#themeWarn').dialog('open');
                    }
                    else {
                        //refresh page: $("#setPage").load("setsUsers.php");
                    }
                });
    }
}
function InstallTheme(name) {
    $('#themeWarnMsg').html('');
    $('#themeWarnMsg').load('themesModyfy.php', {'object': 'themeinst', 'theme': name },
            function(data) {
                resp = jQuery.trim(data);
                if (resp == "") {
                    $('#themeWarnMsg').html("Unknown error");
                }
                if (resp != "OK") {
                    $('#themeWarn').dialog('open');
                }
                else {
                    $("#tabs").tabs( 'load' , tabSystemIndex );
                }
            });
}
</script>
<div class="ui-widget">
<table>
<tr><td>Системная тема:</td><td><select id="themeID">[{html_options values=$themes output=$thNames selected=$defTheme}]</select></td></tr>
<tr><td>Системный язык:</td><td><select id="langID">[{html_options values=$langs output=$lnNames selected=$defLang}]</select></td></tr>
<tr><td>&nbsp;</td><td><button id="savebtn" onclick="SaveTheme();">Сохранить</button></td></tr>
</table>
<table width="100%" id='themestable'>
    <tr class="ui-widget-header">
        <td width="150">Название</td>
        <td width="350">Предпросмотр</td>
        <td>Описание</td>
    </tr>
    [{section name=thm loop=$themes}]
    <tr valign="top">
        <td width="150">[{$thNames[thm]}]
            [{if $themes[thm] == -1}]
            <br/><button onclick="InstallTheme('[{$thNames[thm]}]');">Установить</button>
            [{/if}]
        </td>
        <td width="350"><img src="/[{$thNames[thm]}]/images/screen_s.jpg" width="320" height="200"/></td>
        <td>[{$thDesc[thm]}]<br/><p><b>Поддерживаемые языки:</b></p>
        [{section name=thl loop=$thLangs[thm]}]
        <ul id="thl[{$thm}]">
            <li><img src='/[{$theme}]/images/flag_[{$thLangs[thm][thl][0]}].png'>[{$thLangs[thm][thl][1]}]</li>
        </ul>
        [{/section}]
        </td>
    </tr>
    [{/section}]
</table>
</div>
<div id="instWarn" title='Warning'>
    <table><tr>
    <td><img src="/[{$theme}]/images/warning.png"</td>
    <td><p>Вы должны сперва установить эту тему!</p></td>
    </tr></table>
</div>
<div id='themeWarn' title='Warning' class='ui-state-error'>
    <table class='ui-state-error' width='100%'><tr>
    <td><img src="/[{$theme}]/images/warning.png"</td>
    <td><p id='themeWarnMsg' width='100%'>Вы должны сперва установить эту тему!</p></td>
    </tr></table>
</div>