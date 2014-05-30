<?php
  echo "<p><table cellspacing=0 cellpadding=10 border=1 align=center>
        <tr><td bgcolor=${MSG_BG_COLORS[$error]}>";
  foreach ($messages as $msg) print "<font size=+2><center><b>$msg</b></center></font>";
  echo "<p></p>\n";
  foreach ($msg_details as $det) print "<font size=+1>$det</font><br>";
  echo "</td></tr></table><p><hr><p>\n";
?>
