<?php
/**
* Simple LDAP User Manager
*
* Simple Web GUI for user management. Currently only implements end-user
* self-service of LDAP accounts, allowing users to change their password and
* edit attributes such as phone number, etc.
*
* @author Tim Wilfong <tim@wilfong.me>
*/
 
// Include configuration and functions
require_once('config.inc');
require_once('functions.inc');

// We are asking for passwords, so ensure the user is connecting with SSL
require_ssl();

// Require authenticated login
// With HTTP auth LDAP credentials are kept by browser and no session is needed
if(!isset($_SERVER['PHP_AUTH_USER'])) {
    force_http_auth();
} else {
    $uid = $_SERVER['PHP_AUTH_USER'];
    $pass = isset($_SERVER['PHP_AUTH_PW']) ? $_SERVER['PHP_AUTH_PW'] : '';
}

// Open LDAP connection, using login credentials to bind.
// Force v3 connection to be compatible with most LDAP servers
$ldapconn = ldap_connect($LDAPURL);
ldap_set_option($ldapconn, LDAP_OPT_PROTOCOL_VERSION, 3);
$userdn = "uid=$uid, $UIDBASE";

// Perform authenticated bind to LDAP server, checking for auth errors
if (!@ldap_bind($ldapconn, $userdn, $pass)) {
    $ldaperror = ldap_error($ldapconn);
    // If the error is about creds, force reauth, otherwise display the error
    if (strstr($ldaperror, "credentials")) {
        force_http_auth("Invalid username and passsword. Try again.");
    } else {
        die("Error binding to '$LDAPURL' as '$userdn': " . $ldaperror);
    }
}

// Get the LDAP entry for the authenticated user
$result = ldap_get_entries($ldapconn,
                           ldap_read($ldapconn, $userdn, '(objectclass=*)'));
$user_entry = $result[0];

//Load cmd value (which might be a GET or POST)
$cmd = isset($_REQUEST['cmd']) ? sanitize($_REQUEST['cmd']) : '';

//Populate LDAP attribute list
$attrs = array();
$fullname = isset($user_entry[$FULL_NAME_ATTR]) ? $user_entry[$FULL_NAME_ATTR][0] : '';
foreach ($LDAP_ATTRS as $key => $val) {
  $attrs[$key] = isset($user_entry[$key]) ? $user_entry[$key][0] : '';
}

// If this is a modify action, modify LDAP user and set attrs to new attributes
$error = false;
if (isset($_POST['modify'])) {
    $newAttrs = isset($_POST['attrs']) ? $_POST['attrs'] : '';
    $newPass = isset($_POST['newPass']) ? sanitize($_POST['newPass']) : '';
    $passConf = isset($_POST['passConf']) ? sanitize($_POST['passConf']) : '';
    $error = ! modifyUser($ldapconn, $userdn, $attrs,
                          $newAttrs, $newPass, $passConf, $pass);
    $attrs = $newAttrs;
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
