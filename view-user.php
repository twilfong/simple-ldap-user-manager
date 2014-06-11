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
uid - user name
attributes - attribute=value,attribute=value...


*/





// If we are given POST data, intercept it and act
if(isset($_REQUEST['uid'])){

	// Collect post variables for group creation
	// TODO: Input validation
	$uid = $_REQUEST['uid'];

	// Start LDAP connection
	$connection = new ldapConnection();
	// TODO: LDAP search
	// TODO: LDAP user attributes fetch
	// TODO: Format and display attributes

} else { 
	include('view-user-form.inc');

} // end else post operation is not set


?>
