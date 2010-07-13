﻿<html>
<head>
<title>Web "A" Tools</title>
<link rel="stylesheet" type="text/css" href="/[{$theme}]/wat.css" />
<script language="JavaScript" src="scripts/jquery.js"></script>
<script type="text/javascript" src="/scripts/jquery-ui.js"></script>
<script language="JavaScript">
    var prevButton = "";
    var prevIndex = -1;
    var methods = Array('tasks.php','profiles.php','reports.php','dicts.php','settings.php');

    function showDate() {
        today = new Date();
        var hours=today.getHours();
        var minutes=today.getMinutes();
        var seconds=today.getSeconds();
        if (hours<=9)
            hours="0"+hours;
        if (minutes<=9)
            minutes="0"+minutes;
        if (seconds<=9)
            seconds="0"+seconds;
        
        myclock=hours+":"+minutes+":"+seconds;
        
        $("#liveclock").html(today.toLocaleDateString() + " " + myclock);
        setTimeout("showDate()",1000);
    }
    
    $(function() {
        $("#main-menu").selectable({
                stop: function(){
                    var sz = $(".ui-selected", this).size();
                    var newPrev = -1;
                    $(".ui-selected", this).each(function(){
                        var index = $("#main-menu li").index(this);
                        if (index == prevIndex) {
                            if (sz > 1) {
                                $(this).toggleClass("ui-selected");
                            }
                        }
                        else {
                            newPrev = index;
                            if (newPrev == 5) {
                                document.location = "/logout.php";
                                return;
                            }
                            $("#mainView").load(methods[newPrev]);
                        }
                    });
                    prevIndex = newPrev;
                }
            });
    });

</script>
</head>
<body onload="showDate();">
<div id="bg"><img src="/[{$theme}]/images/bg01.jpg" width="100%" height="100%" alt=""></div>
<div id="toolbar" width="100%" class="toolbar">
    <table width="100%" border="0"><tr valign="top">
    <td width="1"><a href="/main.php"><img id="appIcon" src="/[{$theme}]/images/appIcon.png" alt="" border="0"></a></td>
    <td><p class="toolbar">Пользователь: [{$UserName}]</p></td><td align="right"><p class="toolbar" id="liveclock" style="font-size: 0.8em;"></p></td>
    </tr></table>
</div>
<div id="content">
<table width="100%" border="0"><tr>
    <td class="button-panel" valign="top" width="150px">
        <ol id="main-menu" class="leftMenu">
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/task.png" border="0" align="middle"/>Задачи</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/scanner.png" border="0" align="middle" />Профили</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/report.png" border="0" align="middle" />Отчеты</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/dictionary.png" border="0" align="middle"/>Словари</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/settings.png" border="0" align="middle" />Настройки</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/exit.png" border="0" align="middle" />Выход</li>
        </ol>
    </td>
<td valign="top">
<div width="100%" height="100%" class="ui-widget-content ui-corner-all" id="mainView"> </div>
</td>
</tr></table>
</div>
<script language="JavaScript">
$(document).ready(function() {
    // load the dashboard
    $("#mainView").load("dashboard.php");
});
</script>
</body>
</html>
