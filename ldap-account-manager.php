# Author: Tim Wilfong <tim@wilfong.me>

<?php

#### Configuration variables ####

$LDAPDN = "dc=example,dc=com";
$LDAPURL = "ldapi:///";
#$LDAPURL = "ldaps://some.remote.server/";
$UIDBASE = "ou=People,$LDAPDN";

$FULL_NAME_ATTR = 'cn';

# array of LDAP attributes to allow the user to modify
# elements should be 'attribute' => 'description' 
$LDAP_ATTRS = array(
	'mobile' => 'Mobile Numer',
	'homephone' => 'Home or Alternate Phone'
);

$LDAPPASSWD_CMD = "/usr/local/bin/ldappasswd-wrapper $LDAPURL";

$MSG_BG_COLORS = array(
	'#99d0ff',	# Bg color for information/success
	'#ffd0d0'	# Bg color for error/warning 
);

########

$messages = array();
$msg_details = array();
$error = false;

function sanitize($data) {
  $data = trim($data);
  $data = htmlspecialchars($data);
  return $data;
}

function require_ssl() {
  if(empty($_SERVER["HTTPS"])) { 
    $newurl = "https://" . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"]; 
    header("Location: $newurl"); 
    exit(); 
  }
}

function force_http_auth($msg = "You must login to use this page.") {
  global $LDAPDN;
  header("WWW-Authenticate: Basic realm=\"LDAP Login $LDAPDN " . date('Ymd') . "\"");
  header('HTTP/1.0 401 Unauthorized');
  print $msg;
  exit();
}

function changeSambaNTPass($ldapconn,$userdn,$newPass) {
  global $messages, $msg_details;

  // Get the LDAP entry for the authenticated user
  $result = ldap_get_entries($ldapconn, ldap_read($ldapconn, $userdn, '(objectclass=*)',
                                                  array('sambaNTPassword','sambaPwdLastSet')));
  $user_entry = $result[0];
  if (isset($user_entry['sambantpassword'])) {
    $pwdhash = bin2hex(mhash(MHASH_MD4, iconv("UTF-8","UTF-16LE",$newPass)));
    $entry = array(
      'sambaNTPassword' => array (strtoupper($pwdhash)),
      'sambaPwdLastSet' => array((string)time())
    );
  }
  if (ldap_modify($ldapconn,$userdn,$entry)) {
    return true;
  } else {
    $msg_details[] = "Note: Error changing samba password, but main password was changed.";
  }
}

function changePass($ldapconn,$userdn,$newPass,$oldPass) {
  global $LDAPPASSWD_CMD, $messages, $msg_details;

  // Test some Password conditions
  $pass = true;
  if (strlen($newPass) < 8) {
    $msg_details[] = "Your password must be at least 8 characters long.";
    $pass = false;
  }
  if (preg_match("/^[a-zA-Z]+$/",$newPass)) {
    $msg_details[] = "Your password must contain at least one non-alphabetic character.";
    $pass = false;
  }
  if (!$pass) {
    $messages[] = "Your new password does not meet the minimum requirements.";
    return false;
  }

  // Change the password
  // use ldappasswd wrapper hack to pass new password via stdin (to avoid showing it in proc list)
  $proc = proc_open ("$LDAPPASSWD_CMD $userdn",
           array( array("pipe","r"), array("pipe","w"), array("file","/dev/null","a")),
           $pipes); 

  if (is_resource($proc)) {
    // send two lines to ldappasswd-wrapper stdin. newpass, then oldpass
    fwrite($pipes[0], "$newPass\n$oldPass\n");
    fclose($pipes[0]);
    // the stdout will be empty if success, or have an error
    $errstr = trim(stream_get_contents($pipes[1]));
    fclose($pipes[1]);
    $rval = proc_close($process);
  } else die("Error running $LDAPPASSWD_CMD");
     
  if ($rval == 0) {
    $messages[] = "Password changed.";
    // This hack is only required if the smbk5passwd overlay is not installed on the ldap server
    changeSambaNTPass($ldapconn,$userdn,$newPass);
    return true;
  } else {
    $messages[] = "Error changing password.";
    $msg_details[] = $errstr;
    return false;
  }
}

function modifyUser($ldapconn,$userdn,$oldattrs,$newattrs,$newPass='',$passConf='',$oldPass='') {
  global $messages, $msg_details;

  // First change the password if newPass is set
  if ($newPass) {
    if ($newPass != $passConf ) {
      $messages[] = "Your new passwords do not match!";
      return false;
    }
    if (!changePass($ldapconn,$userdn,$newPass,$oldPass)) return false;
  }

  // Then modify other attributes, if any.
  // compile list of changed attributes
  $attrs = array();
  foreach ($newattrs as $key => $val) {
    if ($oldattrs[$key] != $val) $attrs[$key] = ($val) ? $val : array();
  }
  if ($attrs) {
    if (ldap_modify($ldapconn,$userdn,$attrs)) {
      $messages[] = "Attribute(s) modified.";
      return true;
    } else {
      $messages[] = "Attributes not modified. " . ldap_error($ldapconn);
      return false;
    }
  } else {
    if (!$messages) $messages[] = "No changes -- account not modified.";
  }
  return true;
}

require_ssl();

//Load username and password vars
$uid = isset($_SERVER['PHP_AUTH_USER']) ? $_SERVER['PHP_AUTH_USER'] : '';
$pass = isset($_SERVER['PHP_AUTH_PW']) ? $_SERVER['PHP_AUTH_PW'] : '';

// Require Login
// Use HTTP auth so that username and password are kept by browser and no session is needed
if (strlen("$uid $pass") < 4) force_http_auth();

// Open LDAP connection, using login credentials to bind.
$ldapconn = ldap_connect($LDAPURL);
ldap_set_option($ldapconn,LDAP_OPT_PROTOCOL_VERSION, 3);	# Most LDAP servers only acceppt v3 connections
$userdn = "uid=$uid,$UIDBASE";
if (!@ldap_bind($ldapconn, $userdn, $pass)) {
  $ldaperror = ldap_error($ldapconn);
  // If the error string is about credentials then force reauthentication, otherwise display the error
  if (strstr($ldaperror,"credentials"))
    force_http_auth("Invalid username and passsword. Try again.");
  else die("LDAP bind error connecting to '$LDAPURL' as '$userdn': " . $ldaperror);
}

// Get the LDAP entry for the authenticated user
$result = ldap_get_entries($ldapconn, ldap_read($ldapconn, $userdn, '(objectclass=*)'));
$user_entry = $result[0];

//Load cmd value (which might be a GET or POST)
$cmd = isset($_REQUEST['cmd']) ? sanitize($_REQUEST['cmd']) : '';

//Populate LDAP attribute list
$attrs = array();
$fullname = isset($user_entry[$FULL_NAME_ATTR]) ? $user_entry[$FULL_NAME_ATTR][0] : '';
foreach ($LDAP_ATTRS as $key => $val) {
  $attrs[$key] = isset($user_entry[$key]) ? $user_entry[$key][0] : '';
}

// Check if this is a modify action
// If so, run the modify routine
// Otherwise, load user data from LDAP
if (isset($_POST['modify'])) {
  $error = ! modifyUser($ldapconn,$userdn,$attrs,$_POST['attrs'],$_POST['newPass'],$_POST['passConf'],$pass);
  $attrs = $_POST['attrs'];
}
?>

<html>
<head>
<title>Change LDAP Password and Details</title>
</head>
<body>
<center>

<h1>Change LDAP Password and User Details</h1>

<!--BEGIN MESSAGE-->
<?php
// Display a message if there is one
if ($messages) {
  echo "<p><table cellspacing=0 cellpadding=10 border=1 align=center>
        <tr><td bgcolor=${MSG_BG_COLORS[$error]}>";
  foreach ($messages as $msg) print "<font size=+2><center><b>$msg</b></center></font>";
  echo "<p></p>\n";
  foreach ($msg_details as $det) print "<font size=+1>$det</font><br>";
  echo "</td></tr></table><p><hr><p>\n";
}
?>
<!--END MESSAGE-->

<!--BEGIN FORM-->
<form action="<?php echo $_SERVER['PHP_SELF'] ?>" name="modUser" method="post">
<table>
<tr><th>Username:</th><td><center><b><?php echo $uid ?></b></center></td></tr>
<tr><th>Full Name:</th><td><center><b><?php echo $fullname ?></b></center></td></tr>
<?php foreach ($attrs as $key => $val) echo "<tr><th>${LDAP_ATTRS[$key]}:</th>
  <td><input name=\"attrs[${key}]\" type=text size=25 value=\"$val\" autocomplete=off/></td></tr>\n";
?>
<tr><td colspan=2 align=center valign=bottom>
  <font size=+1><i>Leave blank to keep existing password.</i></font></td></tr>
<tr><th>New password:</th><td><input name="newPass" size=25 type="password" autocomplete=off/></td></tr>
<tr><th>New password (again):</th><td><input name="passConf" size=25 type="password" autocomplete=off/></td></tr>
<tr><td colspan=2><br></td></tr>
<tr>
  <td align="center"><input name="modify" type="submit" value="Modify Account"/></td>
  <td align="center">
    <input name="Cancel" type="submit" value="Cancel"/> &nbsp;
    <button onclick="javascript:parent.location.href='<?php echo
      'https://x:x@' . $_SERVER["SERVER_NAME"]. $_SERVER["REQUEST_URI"] ?>'; return false;">Logout</button>
  </td>
</tr>
</table>
</form>
<!--END FORM-->

</center>
</body>
</html>
