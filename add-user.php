<?php

// Include configuration and functions
require_once('config.inc');
require_once('functions.inc');
require_once('ldapconnect.inc');

// Require SSL if configured to
if($requireSSL === TRUE){
	require_ssl();
}



/*
GET/POST INPUT VARIABLES:
operation - createUser or createGroup
uid - username
template - name of default attributes file
password - plaintext password
gid


*/





// If we are given POST data, intercept it and act
if(isset($_REQUEST['template'])){

	// Collect post variables for user creation
	// TODO: Input validation
	$template = $_REQUEST['template'];
	$password = $_REQUEST['password'];
	$uid = $_REQUEST['uid'];


	// Start LDAP connection
	$connection = new ldapConnection();

	// Create new ldap user
	$connection->createUser($uid, $template, $password);

	// TODO: Move to account created html include
	echo "Account $username created with template $template.";

} else { // end if post operation variable is set

	include('input-form.inc');

} // end else post operation is not set


?>
