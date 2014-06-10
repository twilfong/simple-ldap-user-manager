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
	$uid = $_REQUEST['uid'];
	$template = $_REQUEST['template'];
	$password = $_REQUEST['password'];
	if(!empty($_REQUEST['attributes'])) {
		$attributes = $_REQUEST['attributes'];
	} else {
		$attributes = "";
	}
echo "attributes: $attributes";

	// Start LDAP connection
	$connection = new ldapConnection();

	// Create new ldap user
	$connection->createUser($uid, $template, $password, $attributes);

	// TODO: Move to account created html include
	echo PHP_EOL."Account $uid created with template $template.";


} else { 
	// if no template option is specified, show input form
	include('input-form.inc');

} // end else post operation is not set


?>
