<?php

// Include configuration and functions
require_once('config.inc');
require_once('functions.inc');
require_once('ldapconnect.inc');

// Require SSL if configured to
if($requireSSL === TRUE){
	require_ssl();
}




// If we are given POST data, intercept it and act
if(isset($_POST['operation'])){

	// Collect post variables for user creation
	$operation = $_POST['operation'];

	// Start LDAP connection
	$connection = new ldapConnection();

	// Create new ldap user
	$connection->createUser($uid, $template, $password);

} else { // end if post operation variable is set

	// TODO: Display input form here

} // end else post operation is not set


?>
