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
cn - group name
attributes = key=var,key=var,key=var



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

	// Start LDAP connection
	$connection = new ldapConnection();

	// Create new ldap user
	$uidNumber = $connection->createUser($uid, $template, $password, $attributes);

	// echo UID number on success
	echo $uidNumber;


} else { 
	// if no template option is specified, show input form
	include('header.inc');
	include('input-form.inc');
	include('footer.inc');

} // end else post operation is not set


?>
