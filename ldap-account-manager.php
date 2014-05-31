<?php
# Author: Tim Wilfong <tim@wilfong.me>

// Include configuration and functions
require_once('config.inc');
require_once('functions.inc');

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
$error = false;
if (isset($_POST['modify'])) {
  $error = ! modifyUser($ldapconn,$userdn,$attrs,$_POST['attrs'],$_POST['newPass'],$_POST['passConf'],$pass);
  $attrs = $_POST['attrs'];
}

// Start HTML Output

// Include HTML header file
include_once('header.inc');

// Display user messages if any
displayMessages($error)
?>

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

<?php
// Include HTML footer file
include_once('footer.inc');
?>
