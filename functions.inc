<?php

// Sanitize a string
function sanitize($data) {
    $data = trim($data);
    $data = htmlspecialchars($data);
    return $data;
}


// Redirect to HTTPS
function require_ssl() {
    if(empty($_SERVER["HTTPS"])) {
        $newurl = "https://" . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"];
        header("Location: $newurl");
        exit();
    }
}


// Deny access to unauthenticated user
function force_http_auth($msg = "You must login to use this page.") {
    global $LDAPDN;
    header("WWW-Authenticate: Basic realm=\"LDAP Login $LDAPDN "
           . date('Ymd') . "\"");
    header('HTTP/1.0 401 Unauthorized');
    print $msg;
    exit();
}


// Change NT Password
function changeSambaNTPass($ldapconn,$userdn,$newPass) {
    global $messages, $msg_details;

    // Get the LDAP entry for the authenticated user
    $result = ldap_get_entries(
            $ldapconn,
            ldap_read($ldapconn, $userdn, '(objectclass=*)',
                      array('sambaNTPassword','sambaPwdLastSet')));
    $user_entry = $result[0];
    if (isset($user_entry['sambantpassword'])) {
        $pwdhash = bin2hex(mhash(MHASH_MD4, iconv("UTF-8","UTF-16LE",$newPass)));
        $entry = array('sambaNTPassword' => array (strtoupper($pwdhash)),
                       'sambaPwdLastSet' => array((string)time()));
    }
    if (ldap_modify($ldapconn,$userdn,$entry)) {
        return true;
    } else {
        $msg_details[] = "Note: Error changing samba password.";
    }
}


// Change LDAP Password
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
    // use ldappasswd wrapper hack to pass new password via stdin
    // (to avoid showing it in proc list)
    $proc = proc_open ("$LDAPPASSWD_CMD $userdn",
                       array(array("pipe","r"), array("pipe","w"),
                             array("file","/dev/null","a")),
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
        // hack only required if the smbk5passwd overlay is not installed
        changeSambaNTPass($ldapconn,$userdn,$newPass);
        return true;
    } else {
        $messages[] = "Error changing password.";
        $msg_details[] = $errstr;
        return false;
    }
}


// Modify user attributes
function modifyUser($ldapconn, $userdn, $oldattrs,
                    $newattrs, $newPass='', $passConf='', $oldPass='') {
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
