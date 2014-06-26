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
uid - user name


*/





// If we are given POST data, intercept it and act
if(isset($_REQUEST['cn'])){

	// Collect post variables for group creation
	// TODO: Input validation
	$cn = $_REQUEST['cn'];
	$uid = $_REQUEST['uid'];

	// Start LDAP connection
	$connection = new ldapConnection();

	// Create new ldap group
	$connection->removeFromGroup($uid,$cn);

	echo "[SUCCESS]";

} else { 
	include('header.inc');
	include('group-remove-form.inc');
	include('footer.inc');

} // end else post operation is not set


?>
