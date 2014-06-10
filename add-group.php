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
if(isset($_REQUEST['cn'])){

	// Collect post variables for group creation
	// TODO: Input validation
	$cn = $_REQUEST['cn'];
	if(!empty($_REQUEST['attributes'])) {
		$attributes = $_REQUEST['attributes'];
	} else {
		$attributes = "";
	}
	// Start LDAP connection
	$connection = new ldapConnection();

	// Create new ldap group
	$gidNumber = $connection->createGroup($cn,$attributes);

	echo $gidNumber;


} else { 
	include('group-create-form.inc');

} // end else post operation is not set


?>
